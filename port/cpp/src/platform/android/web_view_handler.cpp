// web_view_handler — Android (JNI) platform partial: a REAL android.webkit.WebView held as a JNI
// global reference in web_view_platform::native. The android twin of
// src/platform/apple_shared/web_view_handler.mm (the WKWebView recipe) and the real-native sibling of the
// in-memory headless mirror (src/platform/headless/web_view_handler.cpp). Ported DIRECTLY from
// WebViewHandler.Android.cs + Platform/Android/MauiWebView.cs (the IWebViewDelegate.LoadHtml/LoadUrl
// recipe) + ViewExtensions.cs (the generic-IView pushes) + ContextExtensions.cs (ToPixels).
//
// SCOPE (wave 22): the STATIC HtmlWebViewSource is the target — map_source routes the source through the
// platform-as-i_web_view_delegate, and load_html drives android.webkit.WebView.loadDataWithBaseURL(
// baseUrl ?? "file:///android_asset/", html, "text/html", "UTF-8", null) — exactly MauiWebView's
// IWebViewDelegate.LoadHtml. load_url drives WebView.loadUrl. The headless mirror's synchronous
// back-forward simulation (send_navigating/send_navigated over the in-memory history) stays live for the
// VM-less cross-platform suite, so its navigation tests still observe the recipe.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each an infrastructure gap, NOT a behavior guess):
//   - No MauiWebViewClient / MauiWebChromeClient: C# installs a WebViewClient that forwards
//     onPageStarted/onPageFinished/shouldOverrideUrlLoading into Navigating/Navigated and a WebChromeClient
//     for the JS dialogs (the android analog of the WKNavigationDelegate / WKUIDelegate the apple .mm
//     installs). Those clients are dev.mauicpp Java classes the AAR-less app_process backend does not carry
//     yet, so the REAL native navigation callbacks (and the JS alert/confirm/prompt panels) are NOT wired —
//     remote-URL navigation events + the JS bridge are DEFERRED. The static HtmlWebViewSource render needs
//     none of them. The headless mirror still drives the simulated navigation channel for the unit suite.
//   - UserAgent: WebView's user agent is on android.webkit.WebSettings (getUserAgentString /
//     setUserAgentString) rather than WKWebView's CustomUserAgent KVC. map_user_agent pushes a set value to
//     WebSettings.setUserAgentString and reads the default back into the virtual view (the bidirectional
//     WebViewExtensions.UpdateUserAgent shape), keeping the platform->user_agent mirror live.
//   - Shadow / Clip / InputTransparent have no plain-android.view.View analog (WrapperView-only in C#),
//     so those generic-IView properties keep ONLY the headless mirror (the progress_bar/image precedent).
//     is_enabled keeps the base mirror: ViewExtensions.UpdateIsEnabled's non-control branch is the
//     interaction toggle; a WebView's interactivity is governed by its own content, and C# has no
//     MapIsEnabled for it (parallel to the apple twins, where WKWebView is not an NSControl/UIControl).
//   - The flicker-avoidance empty-ClipBounds tweak (MauiWebView ctor + UpdateClipBounds, dotnet/maui#31475)
//     is NOT replicated: a static page never shows the transient full-screen flash, so it is not
//     load-bearing for the parity render — platform_arrange frames the view exactly, which is enough.
//
// VM-less degradation (the per-control fan-out invariant): every JNI path checks scoped_env/app_context()
// and quietly skips, while the headless mirrors (source kind / last_html / last_url / eval_scripts / the
// base IView mirrors) are ALWAYS maintained — so the pure-native cross-platform suite (run on the emulator
// WITHOUT a Java VM) observes exactly the headless partial's behavior, and the gallery app host (a real
// Activity WITH a JavaVM + Activity context) additionally drives the real android.webkit.WebView.

#include "maui/core/web_view_handler.hpp"

#include <jni.h>

#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;
    using maui::platform::android::to_utf8;

    // All instance methods resolve through the widget's own class (GetMethodID walks the superclasses, so
    // the android.view.View surface resolves through android/webkit/WebView too).
    constexpr const char* k_web_view_class = "android/webkit/WebView";
    constexpr const char* k_web_settings_class = "android/webkit/WebSettings";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // MauiWebView.AssetBaseUrl — the base url an html source with no BaseUrl falls back to.
    constexpr const char* k_asset_base_url = "file:///android_asset/";
    // LoadDataWithBaseURL's fixed mime + encoding (MauiWebView.IWebViewDelegate.LoadHtml).
    constexpr const char* k_html_mime = "text/html";
    constexpr const char* k_html_encoding = "UTF-8";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling.
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO.
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::web_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject widget, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_web_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_web_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation (the ceil
    // already produced an integral value, so truncation is exact).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). 1.0 on any failure.
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_web_view_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
            density_field == nullptr)
        {
            return 1.0F;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(widget, get_context)};
        if (clear_pending(env) || !context)
        {
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
        if (clear_pending(env) || !resources)
        {
            return 1.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
        if (clear_pending(env) || !metrics)
        {
            return 1.0F;
        }
        const jfloat density = env->GetFloatField(metrics.get(), density_field);
        if (clear_pending(env) || density == 0.0F)
        {
            return 1.0F;
        }
        return density;
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.webkit.WebView (the JNI shape of the
    // pimpl-owned-native-view doctrine: the apple twin CFReleases its WKWebView here).
    web_view_platform::~web_view_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env; // any-thread teardown, exactly like global_ref::reset
            if (env)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    // ---- i_web_view_delegate (the source's load sink; MauiWebView.IWebViewDelegate.LoadHtml/LoadUrl) ----

    void web_view_platform::load_html(std::string_view html, std::string_view base_url)
    {
        // Headless-mirror first (VM-less cross-platform suite): record the loaded content. C# LoadHtml
        // passes html ?? string.Empty to LoadDataWithBaseURL — an empty html still loads (unlike the WK
        // twin, where LoadHtml early-returns on a null html).
        last_source_kind = web_view_source_kind::html;
        last_html = std::string(html);
        last_base_url = std::string(base_url);
        if (native == nullptr)
        {
            return; // VM-less: the headless mirror is the whole story
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // MauiWebView: LoadDataWithBaseURL(baseUrl ?? AssetBaseUrl, html ?? "", "text/html", "UTF-8", null).
        // Signature: (String baseUrl, String data, String mimeType, String encoding, String historyUrl).
        jmethodID load_data = default_jni_cache().method(
            env.get(), k_web_view_class, "loadDataWithBaseURL",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        if (load_data == nullptr)
        {
            return;
        }
        const std::string resolved_base = base_url.empty() ? std::string(k_asset_base_url) : std::string(base_url);
        const local_ref<jstring> j_base = to_jstring(env.get(), resolved_base);
        const local_ref<jstring> j_html = to_jstring(env.get(), html);
        const local_ref<jstring> j_mime = to_jstring(env.get(), k_html_mime);
        const local_ref<jstring> j_encoding = to_jstring(env.get(), k_html_encoding);
        env->CallVoidMethod(widget_of(*this), load_data, j_base.get(), j_html.get(), j_mime.get(), j_encoding.get(),
                            static_cast<jstring>(nullptr));
        clear_pending(env.get());
    }

    void web_view_platform::load_url(std::string_view url)
    {
        last_source_kind = web_view_source_kind::url;
        last_url = std::string(url);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // MauiWebView.IWebViewDelegate.LoadUrl: a bare relative path (no leading '/' and not an absolute
        // Uri) is rebased under the android asset folder; otherwise WebView.loadUrl(url) as-is. The remote
        // navigation EVENTS are deferred (no MauiWebViewClient), but the load itself works.
        std::string resolved(url);
        const bool has_scheme = resolved.find("://") != std::string::npos;
        const bool leading_slash = !resolved.empty() && resolved.front() == '/';
        if (!resolved.empty() && !leading_slash && !has_scheme)
        {
            resolved = std::string(k_asset_base_url) + resolved;
        }
        jmethodID load_url =
            default_jni_cache().method(env.get(), k_web_view_class, "loadUrl", "(Ljava/lang/String;)V");
        if (load_url == nullptr)
        {
            return;
        }
        const local_ref<jstring> j_url = to_jstring(env.get(), resolved);
        env->CallVoidMethod(widget_of(*this), load_url, j_url.get());
        clear_pending(env.get());
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each calls
    // the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform suite — then
    // pushes to the real widget when one exists.

    void web_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), widget_of(*this), "setVisibility", state);
    }

    void web_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_float(env.get(), widget_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void web_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        if (native == nullptr || value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*this);
        auto& cache = default_jni_cache();
        jmethodID get_important = cache.method(env.get(), k_web_view_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_web_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(widget, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(widget, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), widget, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    void web_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void web_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void web_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void web_view_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<web_view_platform> web_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<web_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass web_view_class = cache.find_class(env.get(), k_web_view_class);
        if (web_view_class == nullptr)
        {
            // android.webkit.WebView not resolvable (no WebView provider) — stay on the headless mirror.
            clear_pending(env.get());
            return platform;
        }
        // WebViewHandler.CreatePlatformView: `new MauiWebView(handler, Context)`. The android.webkit.WebView
        // (Context) ctor requires a WebView provider + a per-app data directory; under a real Activity host
        // (the gallery APK) both exist, so it constructs. clear_pending guards the case where it cannot
        // (e.g. a data-dir-less app_process host): the widget stays null and the handler falls back to the
        // headless mirror, never crashing the page (the network-image-stack deferral shape).
        jmethodID ctor = cache.method(env.get(), k_web_view_class, "<init>", "(Landroid/content/Context;)V");
        if (ctor == nullptr)
        {
            return platform;
        }
        jobject created = env->NewObject(web_view_class, ctor, context);
        if (clear_pending(env.get()) || created == nullptr)
        {
            return platform; // WebView.<init> threw (no provider / data dir) — headless mirror it is
        }
        const local_ref<jobject> widget{env.get(), created};
        // WebViewHandler.Android: platformView.Settings.JavaScriptEnabled = true. getSettings() returns the
        // WebSettings; setJavaScriptEnabled(true). Best-effort — a missing settings surface does not block
        // the static HTML render.
        jmethodID get_settings =
            cache.method(env.get(), k_web_view_class, "getSettings", "()Landroid/webkit/WebSettings;");
        if (get_settings != nullptr)
        {
            const local_ref<jobject> settings{env.get(), env->CallObjectMethod(widget.get(), get_settings)};
            if (!clear_pending(env.get()) && settings)
            {
                jmethodID set_js = cache.method(env.get(), k_web_settings_class, "setJavaScriptEnabled", "(Z)V");
                if (set_js != nullptr)
                {
                    env->CallVoidMethod(settings.get(), set_js, JNI_TRUE);
                    clear_pending(env.get());
                }
                // Overview/wide-viewport rendering so the static document fits the view width at a compact
                // text size — Android WebView's default is device-width (large text) where iOS WKWebView
                // fits-to-overview by default. The parity ground truth (the iOS reference) renders this HTML
                // small, and a smaller, content-fitting render keeps the whole document (the <h1> AND the
                // <p>) inside the natural-content cell the no-HeightRequest WebView measures to — the port's
                // layout pass is measure-once (view::invalidate_measure is the M3 no-op seam), so there is no
                // post-load re-measure to grow the cell after Chromium's async content layout arrives. These
                // are pure render settings (no content edit): setUseWideViewPort(true) + setLoadWithOverview
                // Mode(true), the WebSettings analog of WKWebView's default overview fit.
                jmethodID set_wide_viewport =
                    cache.method(env.get(), k_web_settings_class, "setUseWideViewPort", "(Z)V");
                if (set_wide_viewport != nullptr)
                {
                    env->CallVoidMethod(settings.get(), set_wide_viewport, JNI_TRUE);
                    clear_pending(env.get());
                }
                jmethodID set_overview_mode =
                    cache.method(env.get(), k_web_settings_class, "setLoadWithOverviewMode", "(Z)V");
                if (set_overview_mode != nullptr)
                {
                    env->CallVoidMethod(settings.get(), set_overview_mode, JNI_TRUE);
                    clear_pending(env.get());
                }
            }
        }
        // MATCH_PARENT LayoutParams up front (a WebView fills its host cell). The layout container re-parents
        // and frames it; this is the leaf-handler convention shared with button/progress_bar.
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_web_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_match_parent, k_match_parent)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(widget.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        platform->native = env->NewGlobalRef(widget.get()); // released in ~web_view_platform
        return platform;
    }

    void web_view_handler::on_connect_handler(web_view_platform& platform)
    {
        // The static-HTML target needs no native WebViewClient: map_source drives the load directly, and the
        // simulated navigation channel (headless mirror) keeps CanGoBack/CanGoForward consistent for the
        // unit suite. The real onPageFinished → Navigated forwarding is the deferred MauiWebViewClient item.
        platform.connected_view = virtual_view();
    }

    void web_view_handler::on_disconnect_handler(web_view_platform& platform)
    {
        platform.connected_view = nullptr;
    }

    // WebViewHandler.MapSource + WebViewExtensions.UpdateSource: the platform view is the i_web_view_delegate
    // the source loads into, then UpdateCanGoBackForward.
    void web_view_handler::map_source(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto* source = view.source())
        {
            source->load(*platform); // → load_html / load_url on the platform (real WebView + mirror)
        }
        // Without a native navigation client the real WebView's canGoBack/canGoForward are not yet driven;
        // a fresh static load has neither, so reflect that into the virtual view (matches the mirror).
        view.set_can_go_back(false);
        view.set_can_go_forward(false);
    }

    // WebViewHandler.Android.MapUserAgent: WebView's user agent lives on WebSettings (not WKWebView's
    // CustomUserAgent). Push a set value; otherwise read the platform default back into the virtual view
    // (the bidirectional WebViewExtensions.UpdateUserAgent shape). The user_agent mirror is always kept.
    void web_view_handler::map_user_agent(web_view_handler& handler, i_web_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->user_agent = std::string(view.user_agent()); // headless mirror first
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID get_settings =
            cache.method(env.get(), k_web_view_class, "getSettings", "()Landroid/webkit/WebSettings;");
        if (get_settings == nullptr)
        {
            return;
        }
        const local_ref<jobject> settings{env.get(), env->CallObjectMethod(widget_of(*platform), get_settings)};
        if (clear_pending(env.get()) || !settings)
        {
            return;
        }
        if (!view.user_agent().empty())
        {
            jmethodID set_ua =
                cache.method(env.get(), k_web_settings_class, "setUserAgentString", "(Ljava/lang/String;)V");
            if (set_ua != nullptr)
            {
                const local_ref<jstring> j_ua = to_jstring(env.get(), view.user_agent());
                env->CallVoidMethod(settings.get(), set_ua, j_ua.get());
                clear_pending(env.get());
            }
            return;
        }
        // Unset: read the platform default (WebSettings.getUserAgentString) back into the virtual view.
        jmethodID get_ua = cache.method(env.get(), k_web_settings_class, "getUserAgentString", "()Ljava/lang/String;");
        if (get_ua == nullptr)
        {
            return;
        }
        const local_ref<jstring> ua{env.get(), static_cast<jstring>(env->CallObjectMethod(settings.get(), get_ua))};
        if (clear_pending(env.get()))
        {
            return;
        }
        const std::string resolved = to_utf8(env.get(), ua.get());
        platform->user_agent = resolved;
        view.set_user_agent(resolved); // re-enters map_user_agent on the set branch (terminates)
    }

    // ---- navigation commands (WebViewHandler.CommandMapper) ----
    // Without a native WebViewClient the back-forward EVENTS are not observed, but loadUrl/goBack/goForward/
    // reload still work natively. (The simulated history for the unit suite is the headless mirror's job.)

    void web_view_handler::map_go_back(web_view_handler& handler, i_web_view& /*view*/, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        jmethodID can_go_back = cache.method(env.get(), k_web_view_class, "canGoBack", "()Z");
        if (can_go_back != nullptr && env->CallBooleanMethod(widget, can_go_back) == JNI_TRUE &&
            !clear_pending(env.get()))
        {
            platform->current_navigation_event = web_navigation_event::back;
            if (jmethodID go_back = cache.method(env.get(), k_web_view_class, "goBack", "()V"))
            {
                env->CallVoidMethod(widget, go_back);
                clear_pending(env.get());
            }
        }
    }

    void web_view_handler::map_go_forward(web_view_handler& handler, i_web_view& /*view*/, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        jmethodID can_go_forward = cache.method(env.get(), k_web_view_class, "canGoForward", "()Z");
        if (can_go_forward != nullptr && env->CallBooleanMethod(widget, can_go_forward) == JNI_TRUE &&
            !clear_pending(env.get()))
        {
            platform->current_navigation_event = web_navigation_event::forward;
            if (jmethodID go_forward = cache.method(env.get(), k_web_view_class, "goForward", "()V"))
            {
                env->CallVoidMethod(widget, go_forward);
                clear_pending(env.get());
            }
        }
    }

    void web_view_handler::map_reload(web_view_handler& handler, i_web_view& /*view*/, const std::any& /*args*/)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->current_navigation_event = web_navigation_event::refresh;
        ++platform->reload_count;
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (jmethodID reload = default_jni_cache().method(env.get(), k_web_view_class, "reload", "()V"))
        {
            env->CallVoidMethod(widget_of(*platform), reload);
            clear_pending(env.get());
        }
    }

    // WebViewHandler.MapEval: fire-and-forget evaluation via WebView.evaluateJavascript(script, null).
    void web_view_handler::map_eval(web_view_handler& handler, i_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* script = std::any_cast<std::string>(&args);
        if (platform == nullptr || script == nullptr)
        {
            return;
        }
        platform->eval_scripts.push_back(*script); // headless mirror first
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jmethodID evaluate = default_jni_cache().method(env.get(), k_web_view_class, "evaluateJavascript",
                                                        "(Ljava/lang/String;Landroid/webkit/ValueCallback;)V");
        if (evaluate == nullptr)
        {
            return;
        }
        const local_ref<jstring> j_script = to_jstring(env.get(), *script);
        env->CallVoidMethod(widget_of(*platform), evaluate, j_script.get(), static_cast<jobject>(nullptr));
        clear_pending(env.get());
    }

    // WebViewHandler.MapEvaluateJavaScriptAsync: the async-result ValueCallback bridge is the deferred JS
    // item (it needs a RegisterNatives ValueCallback trampoline, like the MauiWebViewClient). Record the
    // script and complete through the headless canned-result seam so the request never wedges.
    void web_view_handler::map_evaluate_java_script(web_view_handler& handler, i_web_view& /*view*/,
                                                    const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<evaluate_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<evaluate_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            request->complete("null");
            return;
        }
        platform->eval_scripts.push_back(request->script());
        std::string result =
            platform->eval_result_provider ? platform->eval_result_provider(request->script()) : "null";
        request->complete(std::move(result));
    }

    maui::graphics::size web_view_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less / WebView-less degradation: the headless MinimumSize (44) fallback per dimension when
            // the constraint is unbounded or non-positive (WebViewHandler.GetDesiredSize).
            double width = 0;
            double height = 0;
            if (width_constraint <= 0 || !std::isfinite(width_constraint))
            {
                width = minimum_size;
            }
            if (height_constraint <= 0 || !std::isfinite(height_constraint))
            {
                height = minimum_size;
            }
            return {width, height};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_web_view_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_web_view_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_web_view_class, "getMeasuredHeight", "()I");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || get_measured_width == nullptr ||
            get_measured_height == nullptr || measure_spec_class == nullptr)
        {
            return {0, 0};
        }
        const float density = display_density(env.get(), widget);
        const auto spec_for = [&](double constraint) -> jint {
            const jint size = std::isfinite(constraint) ? to_pixels(constraint, density) : 0;
            const jint mode = std::isfinite(constraint) ? k_measure_spec_at_most : k_measure_spec_unspecified;
            const jint spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, size, mode);
            return clear_pending(env.get()) ? 0 : spec;
        };
        const jint width_spec = spec_for(width_constraint);
        const jint height_spec = spec_for(height_constraint);
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        const jint measured_width = env->CallIntMethod(widget, get_measured_width);
        const jint measured_height = env->CallIntMethod(widget, get_measured_height);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        double width = static_cast<double>(measured_width) / density;
        double height = static_cast<double>(measured_height) / density;
        // A WebView measures 0 under an unbounded constraint before content lays out; the MinimumSize (44)
        // fallback keeps the cell from collapsing (WebViewHandler.GetDesiredSize's per-dimension floor).
        if (width == 0 && (width_constraint <= 0 || !std::isfinite(width_constraint)))
        {
            width = minimum_size;
        }
        if (height == 0 && (height_constraint <= 0 || !std::isfinite(height_constraint)))
        {
            height = minimum_size;
        }
        return {width, height};
    }

    void web_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless: no native layout to apply
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_web_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_web_view_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), widget);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(widget, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

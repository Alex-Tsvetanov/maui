// web_view_handler — Android (JNI) platform partial: a REAL android.webkit.WebView held as a JNI

// NATURAL HEIGHT: THE PORT MEASURES AN UNLOADED WebView, MAUI MEASURES A LOADED ONE (measured
// 2026-08-21, android emulator API 34, context_flyout's `<WebView Source="https://example.com/"
// MinimumHeightRequest="400" />` inside a ScrollView, so the height constraint is infinite).
//
//     MAUI renders the WebView band 607px tall   -- example.com's actual content height
//     the port renders it            1100px      -- 400dp x 2.75 density, i.e. the MINIMUM, not a measure
//     the port with that floor removed  121px    -- what its WebView actually measures: an EMPTY page
//
// So the port's 1100px was never a measurement; it was MinimumHeightRequest masking a 121px natural
// height. MAUI's 607 comes from the native measure alone -- GetDesiredSizeFromHandler
// (ViewHandlerExtensions.Android.cs:79) creates the spec and returns the platform measure with nothing
// applied afterwards (the `Math.Max(platformView.MinimumHeight, ...)` at :72 is a DIFFERENT function and
// does not run for a leaf). The real gap is that MAUI re-measures once the page finishes loading and this
// partial does not: it measures once, before any content exists, and never invalidates.
//
// AND DO NOT "FIX" THIS BY MATCHING MAUI'S MINIMUM RULE FIRST. MAUI's android leaf measure genuinely
// ignores Minimum*Request when no explicit Width/Height is set -- ContextExtensions.cs:418 reads
// minimumSize ONLY inside `if (IsExplicitSet(explicitSize))`, and nothing downstream re-applies it
// (VerticalStackLayoutManager:40 clamps the STACK's own minimum, not the child's). That is true, it was
// implemented, and it made the board WORSE: context_flyout barely moved (4.04%/26.19% -> 3.39%/25.90%,
// still red) while border_stroke went yellow -> RED (2.76% -> 9.82%), because its Labels carry
// MinimumHeightRequest="20" with no explicit height and the port's floor is currently COMPENSATING for
// their natural measure too. Reverted.
//
// THE RE-MEASURE WAS BUILT AND IS NOT ENOUGH (2026-08-21, verified on device). A dev.mauicpp
// MauiWebViewClient subclass forwarding onPageFinished into a native invalidate_measure() was
// implemented, ran correctly, and changed NOTHING. The logs are unambiguous:
//     measure -> 1080x0 px = 392.7x44.0 dp      first measure: height 0
//     onPageFinished -> invalidate_measure       the callback fires
//     measure -> 1080x0 px = 392.7x44.0 dp      the re-measure runs and STILL gets 0
//     contentHeight=0 css scale=2.750           getContentHeight() is 0 at that callback too
// So android.webkit.WebView reports 0 under an UNSPECIFIED spec before AND after load, and its content
// height is not available at onPageFinished either. (Correction to an earlier note: the port's "121px"
// was never a measurement — it is the 44dp MinimumSize fallback below, at 2.75 density.) Both the client
// and a getContentHeight() fallback were REVERTED as infrastructure with no working consumer.
//
// AND MAUI'S 607px IS STILL UNEXPLAINED. Every obvious source has been ruled out in the C# oracle:
//     ScrollViewHandler.Android.cs:276-277  passes double.PositiveInfinity down the scroll axis, exactly
//                                           as the port does -- so MAUI's WebView is measured UNSPECIFIED too
//     GetDesiredSizeFromHandler (:79)       returns the platform measure with nothing applied after it
//     MeasureVirtualView's Math.Max(platformView.MinimumHeight, ...) (:72) is the CONTAINER path and does
//                                           not run for a leaf
// Whatever produces 607 (= 220.7dp, ~example.com's content height) is somewhere none of those cover.
//
// TIMING IS NOT THE ANSWER EITHER (phased probe, on device). The callback was re-run at 0ms, 100ms and
// 800ms after onPageFinished, reading the WebView each time:
//     phase=0 measuredH=1100 contentH=0 scale=2.750
//     phase=1 measuredH=1100 contentH=0 scale=2.750
//     phase=2 measuredH=1100 contentH=0 scale=2.750
// getContentHeight() is 0 PERMANENTLY on the port's WebView -- not merely early -- even though the page
// visibly renders in the capture. So neither a later re-measure nor getContentHeight() can source a
// height here. Experiment reverted.
//
// WHAT THE DEVICE SAYS ABOUT MAUI, read with `uiautomator dump` (no rebuild needed -- this is the cheap
// instrument for any live layout question on this lane):
//     MAUI  android.webkit.WebView  y 908-1515  h=607
//     port  android.webkit.WebView  y 884-1984  h=1100   (the 400dp floor)
// and MAUI's height is CONTENT-DEPENDENT, not a constant: context_flyout 607, web_view 660,
// hybrid_web_view 1462. MauiWebView.Android.cs has NO OnMeasure override (all 111 lines read), so MAUI
// measures a stock WebView under the same infinite constraint the port uses.
//
// THE STRUCTURAL HYPOTHESIS IS DISPROVEN. It read: MAUI's android layout runs through ANDROID's measure
// pass so the WebView is sized by its parent ViewGroup, while the port arranges children to exact
// C++-computed frames. `uiautomator dump` of MauiReference kills it:
//     parent ViewGroup  y 136-2274  h=2138
//     children (Button 100, TextView 52, Switch 132, TextView 52, EditText 109, ImageView 162,
//               WebView 607, TextView 52, TextView 97) sum to 1363, and content ends at y=1719
// There is ~555px of SLACK. The WebView is not absorbing remaining space and is not filling its parent --
// it is ASSIGNED 607 as its own measured height. Do not rewrite the layout seam for this.
//
// AND THE PORT'S WebView NEVER REPORTS A HEIGHT, under any spec (measured, one pass, same widget):
//     spec probe: UNSPEC=0  AT_MOST(4000)=0  EXACTLY(4000)=4000
// It only ever fills what it is told. Its getContentHeight() is 0 permanently -- on a REMOTE url and on
// STATIC html alike, at 0/100/800ms after onPageFinished -- even though the page visibly renders. So the
// gap is not the measure spec, not the timing, and not the content source.
//
// ALSO RULED OUT: the missing WebChromeClient. C# installs a MauiWebChromeClient and this partial installs
// none (see the header), and a chrome-client-less WebView is a documented reason for content metrics never
// populating. Installing a base android.webkit.WebChromeClient changed NOTHING (same 0/0/4000).
//
// WHAT IS LEFT is narrow and specific: MAUI's WebView has a populated contentHeight and the port's does
// not, for a reason in how the widget is CREATED or HOSTED -- not in how it is measured, loaded, or timed.
// That is where the next attempt should start, with the C# CreatePlatformView compared line by line
// against this one, and `uiautomator dump` (no rebuild) as the instrument.
//
// ORDER OF WORK: settle that structural question FIRST. Then the natural measure, and only then the
// minimum rule. The floor is load-bearing scaffolding until those are right.
//
// 2026-08-22 — TWO MECHANISMS FOUND IN THE C# ORACLE, AND THE REMAINING BLOCKER NAMED.
//
// (1) ARRANGE MUST NOT MEASURE. PlatformArrangeHandler (ViewHandlerExtensions.Android.cs:120-133) calls
//     platformView.Layout(l,t,r,b) and NOTHING else; the port measured EXACTLY first. Fixed below. This
//     is a fidelity fix on its own; whether it is what unsticks Chromium's contentHeight is still
//     UNVERIFIED (see platform_arrange for why a post-load sample is not reachable from here yet).
//
// (2) THE MINIMUM RULE — WHY THE EARLIER EXPERIMENT REGRESSED border_stroke, and what the real shape is.
//     The note above is right that Android's measure spec ignores Minimum*Request; what it MISSES is
//     where the minimum then comes from. MAUI applies NO size request cross-platform at all —
//     LayoutExtensions.ComputeDesiredSize (src/Core/src/Layouts/LayoutExtensions.cs:11-31) adds the margin
//     to the handler's GetDesiredSize and that is the whole method. The minimum is PER-PLATFORM: on
//     Android it is pushed onto the widget, ViewHandler.cs:52 MapMinimumHeight ->
//     ViewExtensions.cs:433-438 UpdateMinimumHeight -> View.SetMinimumHeight(px). So every widget that
//     honours View.getSuggestedMinimumHeight() — TextView, Button, EditText, border_stroke's Labels with
//     MinimumHeightRequest="20" — keeps its floor, which is exactly what the earlier experiment destroyed
//     when it removed the port's cross-platform clamp wholesale. android.webkit.WebView is the widget
//     that does NOT honour it: AwContents.onMeasure reports the content size and never consults the view
//     minimum, which is why MAUI renders this page's `<WebView MinimumHeightRequest="400" />` at its
//     220 dp content height and the port renders it at 400 dp.
//     The port applies the minimum in maui::controls::view::measure (include/maui/controls/view.hpp,
//     resolve_size_request), i.e. cross-platform — the iOS shape (ViewHandlerExtensions.ResolveConstraints
//     applies it unconditionally) generalised to every backend. Undoing that for this ONE handler needs a
//     per-handler opt-out in a CORE header, which is not this file's to change; it is written up in the
//     android lane's report. Until it lands the 400 dp floor still wins and this page's WebView band stays
//     1100 px no matter what the native measure says.
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
//   - No MauiWebViewClient SUBCLASS / no MauiWebChromeClient: C# installs a WebViewClient that forwards
//     onPageStarted/onPageFinished/shouldOverrideUrlLoading into Navigating/Navigated and a WebChromeClient
//     for the JS dialogs (the android analog of the WKNavigationDelegate / WKUIDelegate the apple .mm
//     installs). Those *subclasses* are dev.mauicpp Java classes the AAR-less app_process backend does not
//     carry yet, so the REAL native navigation CALLBACKS (and the JS alert/confirm/prompt panels) are NOT
//     wired — remote-URL navigation EVENTS + the JS bridge are DEFERRED. The headless mirror still drives
//     the simulated navigation channel for the unit suite.
//     The BASE android.webkit.WebViewClient IS installed, though (create_platform_view), because that part
//     is load-bearing for the RENDER and needs no Java class: see the k_web_view_client_class block for why
//     a client-less WebView escapes a remote navigation to the system browser.
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
    constexpr const char* k_web_view_client_class = "android/webkit/WebViewClient";
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
    // Kept for reference: MAUI's arrange never uses an EXACTLY spec on a leaf (see platform_arrange).
    [[maybe_unused]] constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

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
        // WebViewHandler.Android.MapWebViewClient: `SetWebViewClient(new MauiWebViewClient(handler))`. The
        // port installs the BASE android.webkit.WebViewClient, which needs no dev.mauicpp Java class:
        // MauiWebViewClient.ShouldOverrideUrlLoading returns NavigatingCanceled(url) — i.e. FALSE for every
        // navigation the Navigating event does not cancel — and false is exactly what the base client
        // returns, so the RESTING render is oracle-identical.
        // Installing SOME client is load-bearing, not cosmetic. With NO client set, android.webkit.WebView
        // hands any navigation it did not itself originate — notably a server redirect, e.g.
        // https://bing.com -> https://www.bing.com/ — to the ActivityManager, which fires ACTION_VIEW and
        // launches the SYSTEM BROWSER over the app. That is what put Chrome's first-run screen on top of the
        // gallery for context_flyout (the page's <WebView Source="https://bing.com">), while a static
        // HtmlWebViewSource (the web_view page) never redirects and so never escaped.
        // Installed HERE rather than from a mapper so it is in place before map_source drives the first load.
        jclass client_class = cache.find_class(env.get(), k_web_view_client_class);
        jmethodID client_ctor = cache.method(env.get(), k_web_view_client_class, "<init>", "()V");
        jmethodID set_web_view_client =
            cache.method(env.get(), k_web_view_class, "setWebViewClient", "(Landroid/webkit/WebViewClient;)V");
        if (client_class != nullptr && client_ctor != nullptr && set_web_view_client != nullptr)
        {
            const local_ref<jobject> client{env.get(), env->NewObject(client_class, client_ctor)};
            if (!clear_pending(env.get()) && client)
            {
                // setWebViewClient retains the client java-side, so a local ref is right here.
                env->CallVoidMethod(widget.get(), set_web_view_client, client.get());
                clear_pending(env.get());
            }
        }
        // WebViewHandler.Android.CreatePlatformView applies EXACTLY these WebSettings and no more:
        //   Settings.JavaScriptEnabled = true; Settings.DomStorageEnabled = true;
        //   Settings.SetSupportMultipleWindows(true);
        // getSettings() returns the WebSettings; each set is best-effort — a missing settings surface does
        // not block the static HTML render. Crucially, C# leaves the layout-viewport settings at the Android
        // WebView DEFAULTS (UseWideViewPort=false, LoadWithOverviewMode=false), which renders a source with
        // no <meta viewport> at device-width using the browser's default UA stylesheet — so an <h1> is LARGE
        // and BOLD, matching the MAUI parity ground truth. (A prior port revision force-enabled
        // setUseWideViewPort(true)+setLoadWithOverviewMode(true) to shrink the page to a WKWebView-like
        // overview fit; that was an INVENTED deviation from the C# oracle and it shrank the <h1> to body-text
        // size — the exact web_view parity RED. Removed: mirror C# and let the defaults stand.)
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
                // Settings.DomStorageEnabled = true (C# CreatePlatformView).
                jmethodID set_dom_storage =
                    cache.method(env.get(), k_web_settings_class, "setDomStorageEnabled", "(Z)V");
                if (set_dom_storage != nullptr)
                {
                    env->CallVoidMethod(settings.get(), set_dom_storage, JNI_TRUE);
                    clear_pending(env.get());
                }
                // Settings.SetSupportMultipleWindows(true) (C# CreatePlatformView).
                jmethodID set_multiple_windows =
                    cache.method(env.get(), k_web_settings_class, "setSupportMultipleWindows", "(Z)V");
                if (set_multiple_windows != nullptr)
                {
                    env->CallVoidMethod(settings.get(), set_multiple_windows, JNI_TRUE);
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
        // Force a SOFTWARE render layer so the WebView draws IN-VIEW rather than through a hardware
        // SurfaceView that punches a WINDOW-LEVEL hole. That hole is what let the (white) window background
        // show through on WebView-hosting pages in dark — the apphost paints the content-root VIEW #121212,
        // but a SurfaceView composites against the window, not the view. An in-view (software) WebView lets
        // the dark #121212 root/ContentPage surface show below/around the cell, matching MAUI. (The window
        // background is also painted #121212 in the apphost as a belt-and-suspenders for the same reason.)
        if (jmethodID set_layer_type =
                cache.method(env.get(), k_web_view_class, "setLayerType", "(ILandroid/graphics/Paint;)V"))
        {
            constexpr jint k_layer_type_software = 1; // android.view.View.LAYER_TYPE_SOFTWARE
            env->CallVoidMethod(widget.get(), set_layer_type, k_layer_type_software, static_cast<jobject>(nullptr));
            clear_pending(env.get());
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
        jmethodID layout = cache.method(env.get(), k_web_view_class, "layout", "(IIII)V");
        if (layout == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), widget);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        // LAYOUT ONLY — NO MEASURE, matching PlatformArrangeHandler exactly. ViewHandlerExtensions.Android
        // .cs:120-133 converts the frame to pixels, flips it for RTL and then calls
        // `platformView.Layout(left, top, right, bottom)` — nothing else. MAUI has exactly one arrange-time
        // re-measure, PrepareForTextViewArrange (:143-168), and it is opt-in, TEXT-only and gated on
        // virtualView.NeedsExactMeasure(); a WebView never reaches it. This partial used to
        // `measure(EXACTLY w, EXACTLY h)` before the layout, which is a divergence with no oracle behind it.
        // (Motivation, stated as the hypothesis it still is: android.webkit.WebView is the one widget on the
        // board whose measure is Chromium's, not android.view.View's, and AwLayoutSizer is documented to stop
        // reporting a content height once the height spec is EXACTLY and the layout params are not
        // WRAP_CONTENT — the shape of the port's permanent getContentHeight()==0. NOT VERIFIED: reading
        // contentHeight after the page loads needs a load callback this partial does not have, so the
        // measured trail in the header still ends where it did. Removing the divergence is justified on its
        // own; do not record the Chromium theory as proven until a post-load sample exists.)
        env->CallVoidMethod(widget, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

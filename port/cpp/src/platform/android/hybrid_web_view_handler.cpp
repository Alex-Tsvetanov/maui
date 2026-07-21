// hybrid_web_view_handler — Android (JNI) platform partial: a REAL android.webkit.WebView held as a JNI
// global reference in hybrid_web_view_platform::native. The android twin of
// src/platform/apple_shared/hybrid_web_view_handler.mm (the WKWebView bridge) and the real-native sibling
// of the in-memory headless mirror (src/platform/headless/hybrid_web_view_handler.cpp). A MECHANICAL
// MIRROR of the sibling src/platform/android/web_view_handler.cpp (the same android.view.View surface):
// the JNI idioms, guards, global-ref lifecycle, and measure/arrange bodies are copied verbatim from it.
// Ported from HybridWebViewHandler.Android.cs (CreatePlatformView WebSettings + ConnectHandler LoadUrl)
// + HybridWebViewHandler.cs (AppOrigin) + the shared HybridWebViewHelper.cs message protocol.
//
// SCOPE (parity fix): the hybrid_web_view page was RED on android — the headless blank mirror rendered no
// pixels, while MAUI shows a real android.webkit.WebView attempting AppOrigin (https://0.0.0.1/) and
// painting the browser's white "Webpage not available" error page. create_platform_view builds a real
// WebView with the C# WebSettings (JavaScriptEnabled / DomStorageEnabled / SupportMultipleWindows) and
// on_connect_handler drives WebView.loadUrl(AppOrigin) so the port paints the SAME error page.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each an infrastructure gap, NOT a behavior guess):
//   - No MauiHybridWebViewClient / AddJavascriptInterface bridge: C# installs a WebViewClient and a
//     HybridWebViewJavaScriptInterface (a RegisterNatives @JavascriptInterface trampoline) so JS can call
//     SendMessage back into native (HybridWebViewHandler.MessageReceived). Those are dev.mauicpp Java
//     classes the AAR-less app_process backend does not carry, so the LIVE JS<->native bridge is DEFERRED —
//     exactly as the sibling web_view partial defers its MauiWebViewClient navigation callbacks. It is
//     invisible in a static parity shot: the three command maps keep their headless-mirror bodies (they
//     record the SAME native->JS scripts + the invoke-task table the .mm evaluates), so message_received /
//     create_invoke_task and the cross-platform round-trip suite still observe the recipe.
//   - The generic-IView pushes (update_visibility / update_opacity / update_transform /
//     update_flow_direction / update_background / update_semantics) are NOT overridden here: unlike the
//     sibling web_view_platform, hybrid_web_view_platform's header declares those overrides only under
//     MAUI_PLATFORM_APPLE / MAUI_PLATFORM_IOS (no MAUI_PLATFORM_ANDROID block), so the base
//     view_platform_base mirror handles them. None are load-bearing for the static parity render — the
//     WebView cell is Visible, opaque, untransformed, and paints its own error page over a default
//     background — so the base mirror is exact for this page. (If a later page needs the native pushes,
//     add the MAUI_PLATFORM_ANDROID override block to the handler header, mirroring web_view_handler.hpp.)
//
// VM-less degradation (the per-control fan-out invariant): every JNI path checks scoped_env/app_context()
// and quietly skips, while the headless mirrors (evaluated_scripts / sent_raw_messages / the invoke-task
// table / the init hooks) are ALWAYS maintained — so the pure-native cross-platform suite (run WITHOUT a
// Java VM) observes exactly the headless partial's behavior, and the gallery app host (a real Activity
// WITH a JavaVM + Activity context) additionally drives the real android.webkit.WebView.

#include "maui/controls/hybrid_web_view_handler.hpp"

#include <jni.h>

#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/i_initialization_aware_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // All instance methods resolve through the widget's own class (GetMethodID walks the superclasses, so
    // the android.view.View surface resolves through android/webkit/WebView too).
    constexpr const char* k_web_view_class = "android/webkit/WebView";
    constexpr const char* k_web_settings_class = "android/webkit/WebSettings";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // HybridWebViewHandler.AppOrigin on android: $"{AppHostScheme}://{AppHostAddress}/" = "https://0.0.0.1/"
    // (HybridWebViewHandler.cs — https is reserved for iOS/Catalyst, which use app://). The unreachable host
    // is intentional: ConnectHandler LoadUrls it, WebView paints the "Webpage not available" error page.
    constexpr const char* k_app_origin = "https://0.0.0.1/";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling.
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::controls::hybrid_web_view_platform& platform) noexcept
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

namespace maui::controls
{
    // Releases the global reference pinning the android.webkit.WebView (the JNI shape of the
    // pimpl-owned-native-view doctrine: the apple twin CFReleases its WKWebView here).
    hybrid_web_view_platform::~hybrid_web_view_platform()
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

    std::unique_ptr<hybrid_web_view_platform> hybrid_web_view_handler::create_platform_view()
    {
        // HybridWebViewHandler.CreatePlatformView fires the IInitializationAwareWebView hooks around
        // platform-view creation; fire started first (and completed last) so the VM-less path observes the
        // SAME lifecycle as the headless mirror / apple .mm.
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_started();
        }
        auto platform = std::make_unique<hybrid_web_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (env && context != nullptr)
        {
            auto& cache = default_jni_cache();
            jclass web_view_class = cache.find_class(env.get(), k_web_view_class);
            // android.webkit.WebView(Context) requires a WebView provider + a per-app data directory; under a
            // real Activity host (the gallery APK) both exist, so it constructs. clear_pending guards the case
            // where it cannot (e.g. a data-dir-less app_process host): the widget stays null and the handler
            // falls back to the headless mirror, never crashing the page (the web_view partial's shape).
            jmethodID ctor = cache.method(env.get(), k_web_view_class, "<init>", "(Landroid/content/Context;)V");
            if (web_view_class != nullptr && ctor != nullptr)
            {
                jobject created = env->NewObject(web_view_class, ctor, context);
                if (!clear_pending(env.get()) && created != nullptr)
                {
                    const local_ref<jobject> widget{env.get(), created};
                    // HybridWebViewHandler.Android.CreatePlatformView applies EXACTLY these WebSettings:
                    //   Settings.DomStorageEnabled = true; Settings.SetSupportMultipleWindows(true);
                    //   Settings.JavaScriptEnabled = true;
                    // getSettings() returns the WebSettings; each set is best-effort — a missing settings
                    // surface does not block the error-page render.
                    jmethodID get_settings =
                        cache.method(env.get(), k_web_view_class, "getSettings", "()Landroid/webkit/WebSettings;");
                    if (get_settings != nullptr)
                    {
                        const local_ref<jobject> settings{env.get(), env->CallObjectMethod(widget.get(), get_settings)};
                        if (!clear_pending(env.get()) && settings)
                        {
                            jmethodID set_dom_storage =
                                cache.method(env.get(), k_web_settings_class, "setDomStorageEnabled", "(Z)V");
                            if (set_dom_storage != nullptr)
                            {
                                env->CallVoidMethod(settings.get(), set_dom_storage, JNI_TRUE);
                                clear_pending(env.get());
                            }
                            jmethodID set_multiple_windows =
                                cache.method(env.get(), k_web_settings_class, "setSupportMultipleWindows", "(Z)V");
                            if (set_multiple_windows != nullptr)
                            {
                                env->CallVoidMethod(settings.get(), set_multiple_windows, JNI_TRUE);
                                clear_pending(env.get());
                            }
                            jmethodID set_js =
                                cache.method(env.get(), k_web_settings_class, "setJavaScriptEnabled", "(Z)V");
                            if (set_js != nullptr)
                            {
                                env->CallVoidMethod(settings.get(), set_js, JNI_TRUE);
                                clear_pending(env.get());
                            }
                        }
                    }
                    // MATCH_PARENT LayoutParams up front (C# sets LayoutParameters = MatchParent/MatchParent).
                    // The layout container re-parents and frames it; the leaf-handler convention (web_view).
                    jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
                    jmethodID layout_params_ctor =
                        cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
                    jmethodID set_layout_params = cache.method(env.get(), k_web_view_class, "setLayoutParams",
                                                               "(Landroid/view/ViewGroup$LayoutParams;)V");
                    if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
                    {
                        constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT
                        const local_ref<jobject> params{
                            env.get(),
                            env->NewObject(layout_params_class, layout_params_ctor, k_match_parent, k_match_parent)};
                        if (!clear_pending(env.get()) && params)
                        {
                            env->CallVoidMethod(widget.get(), set_layout_params, params.get());
                            clear_pending(env.get());
                        }
                    }
                    // SOFTWARE render layer so the WebView draws IN-VIEW, not through a hardware SurfaceView
                    // that punches a window-level hole (the dark page-surface bug — twin of web_view_handler).
                    if (jmethodID set_layer_type =
                            cache.method(env.get(), k_web_view_class, "setLayerType", "(ILandroid/graphics/Paint;)V"))
                    {
                        constexpr jint k_layer_type_software = 1; // android.view.View.LAYER_TYPE_SOFTWARE
                        env->CallVoidMethod(widget.get(), set_layer_type, k_layer_type_software,
                                            static_cast<jobject>(nullptr));
                        clear_pending(env.get());
                    }
                    platform->native = env->NewGlobalRef(widget.get()); // released in ~hybrid_web_view_platform
                }
                else
                {
                    clear_pending(env.get()); // WebView.<init> threw (no provider / data dir) — headless mirror
                }
            }
            else
            {
                clear_pending(env.get());
            }
        }
        if (auto* view = virtual_view())
        {
            view->web_view_initialization_completed();
        }
        return platform;
    }

    void hybrid_web_view_handler::on_connect_handler(hybrid_web_view_platform& platform)
    {
        // HybridWebViewHandler.Android.ConnectHandler: set the WebViewClient (deferred — no MauiHybridWeb-
        // ViewClient on this backend), then LoadUrl(AppOrigin). The load itself works even without the
        // client; it paints the "Webpage not available" error page (https://0.0.0.1/ is unreachable),
        // matching the MAUI parity ground truth.
        platform.connected_view = virtual_view();
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jmethodID load_url =
            default_jni_cache().method(env.get(), k_web_view_class, "loadUrl", "(Ljava/lang/String;)V");
        if (load_url == nullptr)
        {
            return;
        }
        const local_ref<jstring> j_url = to_jstring(env.get(), k_app_origin);
        env->CallVoidMethod(widget_of(platform), load_url, j_url.get());
        clear_pending(env.get());
    }

    void hybrid_web_view_handler::on_disconnect_handler(hybrid_web_view_platform& platform)
    {
        platform.connected_view = nullptr;
    }

    // ---- command maps (HybridWebViewHandler.CommandMapper) — headless-mirror bodies preserved verbatim ----
    // The LIVE JS bridge (evaluateJavascript + the @JavascriptInterface trampoline) is deferred (header
    // note): invisible in a static shot. These record the SAME native->JS scripts / invoke-task table the
    // apple .mm evaluates, so message_received / create_invoke_task and the round-trip suite still work.

    // HybridWebViewHandler.MapSendRawMessage → MauiHybridWebView.SendRawMessage: evaluate
    // window.external.receiveMessage(<json message>).
    void hybrid_web_view_handler::map_send_raw_message(hybrid_web_view_handler& handler,
                                                       maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        auto* platform = handler.typed_platform_view();
        const auto* message = std::any_cast<std::string>(&args);
        if (platform == nullptr || message == nullptr)
        {
            return;
        }
        platform->sent_raw_messages.push_back(*message);
        platform->evaluated_scripts.push_back(maui::core::build_send_raw_message_script(*message));
    }

    // HybridWebViewHandler.MapInvokeJavaScriptAsync → HybridWebViewHelper.ProcessInvokeJavaScriptAsync:
    // mint a task id, evaluate window.HybridWebView.__InvokeJavaScript(...). The completion arrives later
    // through message_received (the "__InvokeJavaScriptCompleted" raw message).
    void hybrid_web_view_handler::map_invoke_java_script(hybrid_web_view_handler& handler,
                                                         maui::core::i_hybrid_web_view& /*view*/, const std::any& args)
    {
        const auto* request_ptr = std::any_cast<std::shared_ptr<maui::core::invoke_java_script_request>>(&args);
        if (request_ptr == nullptr || *request_ptr == nullptr)
        {
            return;
        }
        const std::shared_ptr<maui::core::invoke_java_script_request> request = *request_ptr;
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            // C# SetCanceled when the platform view is gone — the port surfaces it as a null result.
            request->complete(std::nullopt);
            return;
        }
        const std::string task_id = handler.create_invoke_task(request);
        platform->evaluated_scripts.push_back(
            maui::core::build_invoke_java_script_script(task_id, request->method_name(), request->param_values()));
    }

    // HybridWebViewHandler.MapEvaluateJavaScriptAsync: record the (already escaped+wrapped) script and
    // complete the request through the answer seam (unset => "null", the WKWebView value for an
    // errored/void script). The same pipeline as web_view's headless map_evaluate_java_script.
    void hybrid_web_view_handler::map_evaluate_java_script(hybrid_web_view_handler& handler,
                                                           maui::core::i_hybrid_web_view& /*view*/,
                                                           const std::any& args)
    {
        const auto* request = std::any_cast<std::shared_ptr<maui::core::evaluate_java_script_request>>(&args);
        if (request == nullptr || *request == nullptr)
        {
            return;
        }
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            (*request)->complete("null");
            return;
        }
        platform->evaluated_scripts.push_back((*request)->script());
        std::string result = platform->script_responder ? platform->script_responder((*request)->script()) : "null";
        (*request)->complete(std::move(result));
    }

    maui::graphics::size hybrid_web_view_handler::get_desired_size(double width_constraint,
                                                                   double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less / WebView-less degradation: the headless MinimumSize (44) fallback per dimension when
            // the constraint is unbounded or non-positive (HybridWebViewHandler.GetDesiredSize).
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
        // fallback keeps the cell from collapsing (HybridWebViewHandler.GetDesiredSize's per-dimension floor).
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

    void hybrid_web_view_handler::platform_arrange(const maui::graphics::rect& frame)
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
} // namespace maui::controls

// refresh_view_handler — Android (JNI) platform partial: a real dev.mauicpp.MauiLayout ViewGroup that
// HOSTS the single scrollable Content child of a RefreshView. The android twin of
// src/platform/apple/refresh_view_handler.mm (a plain NSView host) / src/platform/ios/refresh_view_handler.mm
// (a UIView host + a real UIRefreshControl) and the real-native sibling of the headless mirror
// (src/platform/headless/refresh_view_handler.cpp).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 18: RefreshView on Android (the 2 gallery pages this unblocks)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// Every other backend renders these pages; Android rendered NEITHER the RefreshView's content because
// there was no Android RefreshView partial:
//   - swipe_refresh (was ⬛ blank): a refresh_view wrapping a vertical_stack(swipe_view, readout). With
//     no host, the column never mounted as a native subview, so the WHOLE page (the swipe row + the
//     "Ready" readout) was missing — only the app bar + background rendered.
//   - refresh_view (was 🟡): a refresh_view wrapping a CollectionView. The header/buttons/labels above
//     the refresh_view rendered, but its hosted content (the bottom status + the list) was missing.
// This file is that partial. It reuses the content_page/swipe_view single-content hosting infra (the
// MauiLayout no-op-onLayout ViewGroup): set_content re-parents the single Content's native view as a
// MATCH_PARENT child, and platform_arrange frames the host EXACTLY (measure Exactly + layout) — the
// content's own host-relative platform_arrange then frames it within (refresh_view::arrange was made
// host-relative in this wave, the swipe_view/border/templated_view family fix — the host is framed
// FIRST, then the content is arranged at {inset.left, inset.top} so it lands inside the already-framed
// host rather than double-offsetting by the host's own non-zero page Y).
//
// PULL GESTURE + SPINNER are the DOCUMENTED DEVIATION (the same scope the apple AppKit twin documents).
// MAUI's Android RefreshView is MauiSwipeRefreshLayout : AndroidX.SwipeRefreshLayout — but this APK-less
// backend carries NO AndroidX/AAR (the constraint that made button/editor use plain android.widget.*
// stand-ins), so SwipeRefreshLayout is unavailable. The host is a plain MauiLayout with NO pull
// recognizer and NO native spinner widget. The captures are STATIC and show the RefreshView resting (its
// content visible, no spinner), so the closed/resting Content is the faithful static render. The
// cross-platform refresh surface is still mirrored beside the (absent) native pushes: IsRefreshing /
// IsRefreshEnabled / the spinner color are stored on the platform mirror, and request_refresh() still
// writes IsRefreshing=true back through the virtual view (the MauiRefreshViewProxy.OnRefresh twin) so the
// programmatic/test path matches every backend. A future MauiLayout pull listener (or a vendored
// SwipeRefreshLayout) can drive the same request_refresh() entry point to add the live drag visual.
//
// VM-less degradation (like content_page/swipe_view/button): every JNI path checks scoped_env /
// app_context() and quietly skips when no Java VM exists (the android preset's pure-native cross-platform
// suite runs on the emulator without one), while the headless `hosted_content` mirror + the refresh
// mirrors are ALWAYS maintained so that suite observes exactly the headless partial's content tracking.

#include "maui/core/refresh_view_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string_view>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_important_for_accessibility_auto = 0;
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
    constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT — the content fills the host
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    [[nodiscard]] jobject host_of(const maui::core::refresh_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the partial must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, the same channel the test host uses
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject host, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "(I)V"))
        {
            env->CallVoidMethod(host, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject host, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "(F)V"))
        {
            env->CallVoidMethod(host, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The host's display density (Context.getResources().getDisplayMetrics().density), memoized
    // process-wide after the first read, exactly like content_page_handler.cpp's display_density. 1.0 when
    // any step fails (failures are NOT memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject host)
    {
        static std::atomic<float> memoized{0.0F};
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_maui_layout_class, "getContext", "()Landroid/content/Context;");
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
        const local_ref<jobject> context{env, env->CallObjectMethod(host, get_context)};
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
        memoized.store(density, std::memory_order_relaxed);
        return density;
    }

    // The content's native android.view.View, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Null when the content is unattached. Mirrors content_page_handler.cpp.
    [[nodiscard]] jobject native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Detach `child` from any ViewGroup parent (removeView), so addView never throws "already has a parent"
    // (the re-parent guard content_page/swipe_view/window handlers share).
    void detach_from_parent(JNIEnv* env, jobject child)
    {
        auto& cache = default_jni_cache();
        jmethodID get_parent = cache.method(env, "android/view/View", "getParent", "()Landroid/view/ViewParent;");
        if (get_parent == nullptr)
        {
            return;
        }
        const local_ref<jobject> parent{env, env->CallObjectMethod(child, get_parent)};
        if (clear_pending(env) || !parent)
        {
            return;
        }
        jclass view_group_class = cache.find_class(env, k_view_group_class);
        if (view_group_class == nullptr || env->IsInstanceOf(parent.get(), view_group_class) == JNI_FALSE)
        {
            return;
        }
        jmethodID remove_view = cache.method(env, k_view_group_class, "removeView", "(Landroid/view/View;)V");
        if (remove_view != nullptr)
        {
            env->CallVoidMethod(parent.get(), remove_view, child);
            clear_pending(env);
        }
    }

    // Add `child` to `host` with MATCH_PARENT/MATCH_PARENT layout params so it fills the host (the refresh
    // Content sizes to the host bounds; the content's own host-relative platform_arrange then frames it
    // within). Same call content_page_handler.cpp's add_filling_child uses.
    void add_filling_child(JNIEnv* env, jobject host, jobject child)
    {
        detach_from_parent(env, child);
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr)
        {
            return;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_match_parent, k_match_parent)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(host, add_view, child, params.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the MauiLayout refresh host (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView here).
    refresh_view_platform::~refresh_view_platform()
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

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Each calls the base body FIRST — the headless mirrors must stay live for the
    // VM-less cross-platform suite — then pushes to the real ViewGroup when one exists (the content_page
    // dual-path pattern). ----

    void refresh_view_platform::update_visibility(maui::core::visibility value)
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
        call_void_int(env.get(), host_of(*this), "setVisibility", state);
    }

    void refresh_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_float(env.get(), host_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void refresh_view_platform::update_automation_id(std::string_view value)
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
        jobject host = host_of(*this);
        auto& cache = default_jni_cache();
        // setContentDescription flips ImportantForAccessibility to YES; restore AUTO when that is what the
        // host had (the content_page/swipe_view partials document the same dance).
        jmethodID get_important = cache.method(env.get(), k_maui_layout_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_maui_layout_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(host, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(host, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), host, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    void refresh_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void refresh_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void refresh_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void refresh_view_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<refresh_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        // new MauiLayout(Context) — the refresh host (the no-op-onLayout ViewGroup, host-provided via
        // java/MauiLayout.java). The plain View ctor is theme-independent, so it constructs in the bare
        // app_process testhost. (No SwipeRefreshLayout — AndroidX is unavailable on this AAR-less backend;
        // the pull gesture + spinner are the documented deviation in the file header.)
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID ctor = cache.method(env.get(), k_maui_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform; // without MauiLayout the host stays the headless mirror (VM-less degradation)
        }
        const local_ref<jobject> host{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !host)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(host.get()); // released in ~refresh_view_platform
        return platform;
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# RefreshViewHandler.UpdateContent reads
        // VirtualView.Content).
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*platform);
        // C# UpdateContent: clear the old content (removeAllViews), then add the new content's native view
        // filling the host. The same swap content_page_handler.cpp does for its single content.
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env.get());
        }
        if (platform->hosted_content == nullptr)
        {
            return; // an empty refresh host (the previous child was just removed)
        }
        if (jobject child = native_child(*platform->hosted_content))
        {
            add_filling_child(env.get(), host, child);
        }
    }

    // The native pull-to-refresh control is absent (no AndroidX SwipeRefreshLayout on this backend), so
    // IsRefreshing is mirrored only — the documented deviation (the apple AppKit twin's body). A future
    // SwipeRefreshLayout host would drive setRefreshing here.
    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refreshing = virtual_view()->is_refreshing();
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# UpdateRefreshColor: the spinner tint follows RefreshColor (null = platform default). No native
        // spinner widget here, so the tint is mirrored only (the headless/AppKit spinner mirror).
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        if (paint != nullptr)
        {
            platform->has_refresh_color = true;
            platform->refresh_color_argb = paint->background_color().to_uint();
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
    }

    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refresh_enabled = virtual_view()->is_refresh_enabled();
    }

    // The native pull stand-in (MauiRefreshViewProxy.OnRefresh): write IsRefreshing=true back through the
    // virtual view (which re-enters the control's coercion → Refreshing + command). Identical to every
    // other backend so the programmatic/test path matches; a future MauiLayout pull listener calls this.
    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native host to position
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the host measures Exactly at the final
        // size and lays out (the two-step every android handler uses). MauiLayout.onLayout is a no-op so the
        // single Content child keeps the host-relative frame its own platform_arrange set.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_maui_layout_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_maui_layout_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), host);
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
        env->CallVoidMethod(host, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(host, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

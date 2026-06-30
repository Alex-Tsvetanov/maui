// flyout_page_handler — Android (JNI) platform partial: a real dev.mauicpp.MauiLayout ViewGroup that
// HOSTS the flyout page's DETAIL pane (the visible content on a phone). The android twin of
// src/platform/apple/flyout_page_handler.mm (an NSSplitViewController sidebar+content) /
// src/platform/ios/flyout_page_handler.mm (a UISplitViewController) and the real-native sibling of the
// headless two-pane mirror (src/platform/headless/flyout_page_handler.cpp).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 26: FlyoutPage on Android (the 2 gallery pages this unblocks)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// Both blank gallery pages ROOT on a flyout_page (their visible content lives in the DETAIL pane):
//   - ios_scroll_view (was ⬛ blank): a flyout_page whose detail is a content_page hosting a scroll_view
//     (slider + two link-style "Toggle"/"Return" buttons). With no Android FlyoutPage partial the page's
//     native_view() was null, so the window hosted NOTHING — only the app bar + background rendered.
//   - tabbed_flyout (was ⬛ blank): a flyout_page whose detail is a tabbed_page (Home/Settings). Same
//     cause — the flyout root produced no native view, so nothing below the app bar rendered. (Its detail
//     also needs the wave-26 Android tabbed_page partial to render the tab content + bar.)
// This file is that partial. It reuses the content_page/refresh_view single-content hosting infra (the
// MauiLayout no-op-onLayout ViewGroup): set_panes re-parents the DETAIL pane's native view as a
// MATCH_PARENT child, and platform_arrange frames the host EXACTLY (measure Exactly + layout). The
// flyout_page control's own host-relative arrange then frames the detail's content within (flyout_page
// ::arrange was already host-relative — it arranges both panes at {0,0,w,h}).
//
// WHY THE DETAIL, NOT BOTH PANES: on a phone (DeviceIdiom.Phone) FlyoutPage.ShouldShowSplitMode() is
// false and IsPresented defaults to false, so MAUI shows ONLY the detail; the flyout is an off-screen
// drawer the user swipes/taps in (the iOS reference capture shows exactly the detail content, no menu).
// MAUI's Android FlyoutPage is a DrawerLayout (the flyout = the drawer, the detail = the content). This
// AAR-less backend carries NO AndroidX DrawerLayout (the same constraint button/refresh document), so the
// host is a plain MauiLayout showing the detail; the FLYOUT drawer + the swipe-to-open gesture are the
// DOCUMENTED DEVIATION. The flyout pane is still MOUNTED (its handler attaches, its native view builds) so
// a future DrawerLayout host can overlay it; it is simply not added as a child while IsPresented is false.
// IsPresented / FlyoutBehavior / IsGestureEnabled are mirrored beside the (absent) drawer for the VM-less
// suite — when IsPresented flips true a future host would slide the drawer in via the same seam.
//
// VM-less degradation (like content_page/refresh_view/button): every JNI path checks scoped_env /
// app_context() and quietly skips when no Java VM exists (the android preset's pure-native cross-platform
// suite runs on the emulator without one), while the headless two-pane mirror (hosted_flyout /
// hosted_detail / presented / behavior / gesture_enabled) is ALWAYS maintained so that suite observes
// exactly the headless partial's pane + presentation tracking.

#include "maui/core/flyout_page_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
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
#include "maui/core/i_flyout_view.hpp"
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
    constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT — the detail fills the host
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    [[nodiscard]] jobject host_of(const maui::core::flyout_page_platform& platform) noexcept
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

    // The host's display density, memoized process-wide after the first read (content_page_handler.cpp's
    // display_density). 1.0 when any step fails (failures are NOT memoized).
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

    // The pane's native android.view.View, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Null when the pane is unattached. Mirrors content_page_handler.cpp.
    [[nodiscard]] jobject native_child(maui::core::i_view& pane)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(pane.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Detach `child` from any ViewGroup parent (removeView), so addView never throws "already has a parent"
    // (the re-parent guard content_page/refresh_view/window handlers share).
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

    // Add `child` to `host` with MATCH_PARENT/MATCH_PARENT layout params so it fills the host (the detail
    // pane sizes to the host bounds; the flyout_page control's own host-relative arrange then frames the
    // detail's content within). Same call content_page_handler.cpp's add_filling_child uses.
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
    // Releases the global reference pinning the MauiLayout flyout host (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView here). The hosted detail /
    // flyout panes are owned by their own page handlers (non-owning children) — nothing to release.
    flyout_page_platform::~flyout_page_platform()
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

    void flyout_page_platform::update_visibility(maui::core::visibility value)
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

    void flyout_page_platform::update_opacity(double value)
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

    void flyout_page_platform::update_automation_id(std::string_view value)
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

    void flyout_page_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void flyout_page_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void flyout_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void flyout_page_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<flyout_page_platform> flyout_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<flyout_page_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        // new MauiLayout(Context) — the flyout host (the no-op-onLayout ViewGroup, host-provided via
        // java/MauiLayout.java). The plain View ctor is theme-independent, so it constructs in the bare
        // app_process testhost. (No DrawerLayout — AndroidX is unavailable on this AAR-less backend; the
        // flyout drawer + swipe gesture are the documented deviation in the file header.)
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
        platform->native = env->NewGlobalRef(host.get()); // released in ~flyout_page_platform
        return platform;
    }

    void flyout_page_handler::set_panes(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless two-pane mirror is ALWAYS maintained (C# FlyoutViewHandler reads Flyout/Detail).
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->hosted_flyout = flyout->flyout_view();
            platform->hosted_detail = flyout->flyout_detail();
        }

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
        // Clear the old content, then add the DETAIL pane's native view filling the host. On a phone (no
        // split mode, IsPresented=false) only the detail shows; the flyout drawer is the documented
        // deviation (header). The flyout pane stays MOUNTED (its native view exists) for a future drawer.
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env.get());
        }
        if (platform->hosted_detail == nullptr)
        {
            return; // no detail pane set yet — an empty flyout host
        }
        if (jobject child = native_child(*platform->hosted_detail))
        {
            add_filling_child(env.get(), host, child);
        }
    }

    // IsPresented / FlyoutBehavior realize the drawer state. No native DrawerLayout on this backend, so the
    // presentation is mirrored only — the documented deviation (header). A future DrawerLayout host would
    // openDrawer/closeDrawer the flyout pane here from `presented`.
    void flyout_page_handler::update_presentation(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->presented = flyout->flyout_is_presented();
            platform->behavior = flyout->flyout_behavior_value();
        }
    }

    maui::graphics::size flyout_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The flyout page sizes from its panes, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void flyout_page_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // detail child keeps the host-relative frame the flyout_page control's arrange set.
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

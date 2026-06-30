// layout_handler — Android (JNI) platform partial: a real dev.mauicpp.MauiLayout ViewGroup that HOSTS
// the arranged children. The android twin of src/platform/apple/layout_handler.mm (a plain NSView
// container) and the real-native sibling of the headless child-count mirror
// (src/platform/headless/layout_handler.cpp). Translated from LayoutHandler.cs + LayoutHandler.Android.cs
// (Microsoft.Maui's LayoutViewGroup → the port's MauiLayout):
//   - The ViewGroup hosts the children and, on every system layout traversal, re-runs the cross-platform
//     arrange so the children land where maui's layout_manager places them. MauiLayout.onLayout crosses
//     back into native_arrange (RegisterNatives) -> the control's i_cross_platform_layout::
//     cross_platform_arrange over the panel's LOCAL, 0-origin bounds (children-only — the host's own frame
//     is owned by its handler's platform_arrange). This mirrors MAUI's LayoutViewGroup.OnLayout, which
//     likewise calls CrossPlatformArrange on each pass with a 0-origin destination. A one-shot child frame
//     does NOT survive on Android: any requestLayout() in the subtree (addView, setText, a property push)
//     fires a system traversal that re-lays-out children via onLayout, so a no-op onLayout strands any
//     nested layout (its no-op MauiLayout parent never re-lays it out) — re-arranging here fixes nesting.
//     on_connect_handler seeds the panel's cross-platform peer (this handler) + binds nativeArrange;
//     on_disconnect_handler clears the peer (0 = no-op). See java/MauiLayout.java for the full rationale.
//   - add / insert position the child at GetLayoutHandlerIndex (the z-ordered slot), exactly like
//     LayoutHandler.Add inserting at that index; the C++ children mirror tracks the same order so the
//     headless tests and the on-device subview order agree.
//   - The handler frames the ViewGroup EXACTLY in platform_arrange (measure Exactly + layout), the same
//     two-step every leaf android handler uses (Android requires a measure pass before layout).
//
// DOCUMENTED DEVIATIONS from the C# oracle (each an infrastructure gap, not a behavior guess):
//   - The container is a plain dev.mauicpp.MauiLayout (a ViewGroup subclass), NOT MAUI's
//     LayoutViewGroup (which carries an ICrossPlatformLayout back-ref for its own OnMeasure/OnLayout).
//     The port's container measure/arrange lives on the cross-platform CONTROL (its layout_manager), so
//     MauiLayout needs no back-ref: onMeasure just resolves the spec and onLayout is empty. This is the
//     direct analog of the AppKit panel hosting-only role.
//   - get_desired_size returns {0,0}: a layout sizes itself through its layout_manager (the control
//     overrides measure to delegate to the manager, not the handler), exactly like the apple/headless
//     twins. The ViewGroup is never asked to measure the children.
//
// VM-less degradation (like button_handler.cpp / window_handler.cpp): every JNI path checks
// scoped_env / app_context() and quietly skips when no Java VM exists (the pure-native cross-platform
// suite runs on the emulator without one), while the headless `children` mirror is ALWAYS maintained so
// that suite observes exactly the headless partial's child-tracking. The gallery app host additionally
// drives the real MauiLayout.

#include "maui/core/layout_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "android_clip_ops.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/i_cross_platform_layout.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_z_order.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // dev.mauicpp.MauiLayout is the host ViewGroup (the no-op-onLayout panel); the View/ViewGroup
    // surface (addView/removeView/measure/layout/setVisibility/…) resolves through it because
    // GetMethodID walks the superclasses. ViewGroup is named separately only where the signature must
    // disambiguate the overload (addView(View,int) vs the View base).
    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (PlatformInterop restores it after
    // setContentDescription auto-flips the view to YES).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec modes (ViewHandler.PlatformArrange measures Exactly before layout).
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    [[nodiscard]] jobject panel_of(const maui::core::layout_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state into
    // the cross-platform layer); true when one was pending — call sites skip the read-back.
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

    void call_void_int(JNIEnv* env, jobject panel, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "(I)V"))
        {
            env->CallVoidMethod(panel, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject panel, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "(F)V"))
        {
            env->CallVoidMethod(panel, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject panel, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, name, "(Z)V"))
        {
            env->CallVoidMethod(panel, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The panel's display density (Context.getResources().getDisplayMetrics().density), memoized
    // process-wide after the first read, exactly like button_handler.cpp's display_density (the JNI
    // walk is the same four calls). 1.0 when any step fails (failures are NOT memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject panel)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
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
        const local_ref<jobject> context{env, env->CallObjectMethod(panel, get_context)};
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

    // The child's native android.view.View to host, via its view-handler's native_view() (null if the
    // child is unattached or its handler has no native view). native_view() is C#'s ToPlatform() =
    // ContainerView ?? PlatformView, so a NeedsContainer child hands back its CONTAINER View here — not
    // the bare native (mirrors layout_handler.mm's native_child / window_handler.cpp's page_native_view).
    [[nodiscard]] jobject native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // The current parent of `child` (View.getParent() cast through the ViewGroup methods), or null. Used
    // to detach a re-parented child before addView (ViewGroup.addView throws IllegalStateException when
    // the child already has a parent — the same re-parent guard window_handler.cpp relies on).
    [[nodiscard]] local_ref<jobject> parent_of(JNIEnv* env, jobject child)
    {
        jmethodID get_parent =
            default_jni_cache().method(env, "android/view/View", "getParent", "()Landroid/view/ViewParent;");
        if (get_parent == nullptr)
        {
            return {};
        }
        local_ref<jobject> parent{env, env->CallObjectMethod(child, get_parent)};
        if (clear_pending(env))
        {
            return {};
        }
        return parent;
    }

    // Detach `child` from any current parent that is a ViewGroup (removeView), so addView never throws
    // "already has a parent". A non-ViewGroup ViewParent (e.g. a ViewRootImpl) is left alone.
    void detach_from_parent(JNIEnv* env, jobject child)
    {
        local_ref<jobject> parent = parent_of(env, child);
        if (!parent)
        {
            return;
        }
        auto& cache = default_jni_cache();
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

    // Add `child` to `panel` at subview index `target` (ViewGroup.addView(View, int)). A negative or
    // overflow index drops to the natural end — the same clamp the headless insert_at / Apple
    // place_subview_at use. Detaches the child from any prior parent first.
    void add_view_at(JNIEnv* env, jobject panel, jobject child, jint target)
    {
        detach_from_parent(env, child);
        auto& cache = default_jni_cache();
        jmethodID get_child_count = cache.method(env, k_maui_layout_class, "getChildCount", "()I");
        jmethodID add_view_indexed = cache.method(env, k_view_group_class, "addView", "(Landroid/view/View;I)V");
        if (get_child_count == nullptr || add_view_indexed == nullptr)
        {
            return;
        }
        const jint count = env->CallIntMethod(panel, get_child_count);
        if (clear_pending(env))
        {
            return;
        }
        const jint index = (target < 0 || target > count) ? count : target;
        env->CallVoidMethod(panel, add_view_indexed, child, index);
        clear_pending(env);
    }

    // dev.mauicpp.MauiLayout.nativeArrange(long peer, int widthPx, int heightPx) — the JNI callback the
    // ViewGroup's onLayout crosses into on every system layout traversal (the MAUI LayoutViewGroup.OnLayout
    // analog). The peer is the child's layout_handler; resolve its cross-platform layout and re-arrange the
    // children into this panel's LOCAL, 0-origin bounds (children-only — the host's own frame is not touched
    // here). `thiz` is the MauiLayout instance, used to read the display density: onLayout passes the size
    // in PIXELS, but the cross-platform layer speaks dp, so divide by density. A null/0 peer or an unbacked
    // control is a no-op (the VM-less / disconnected guard, like MauiShapeView.nativeDraw's null peer).
    void JNICALL native_arrange(JNIEnv* env, jobject thiz, jlong peer, jint width_px, jint height_px)
    {
        if (env == nullptr || peer == 0)
        {
            return;
        }
        auto* handler = reinterpret_cast<maui::core::layout_handler*>(peer);
        auto* cross_platform = dynamic_cast<maui::core::i_cross_platform_layout*>(handler->virtual_view());
        if (cross_platform == nullptr)
        {
            return;
        }
        const float density = display_density(env, thiz);
        const float scale = density > 0 ? density : 1.0F;
        const double width_dp = static_cast<double>(width_px) / scale;
        const double height_dp = static_cast<double>(height_px) / scale;
        // 0-origin: a subview's frame is in its superview's coordinate space (origin 0,0); carrying the
        // absolute window origin would double-offset every child. The manager re-positions the children only.
        cross_platform->cross_platform_arrange(maui::graphics::rect{0, 0, width_dp, height_dp});
    }

    // Binds nativeArrange to MauiLayout (RegisterNatives — no Java_* export needed). Idempotent
    // (RegisterNatives replaces an existing binding), so connecting handlers need no once-flag coordination.
    [[nodiscard]] bool register_arrange_natives(JNIEnv* env, jclass layout_class)
    {
        static const std::array<JNINativeMethod, 1> k_methods{
            JNINativeMethod{.name = const_cast<char*>("nativeArrange"),
                            .signature = const_cast<char*>("(JII)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_arrange)},
        };
        const jint status = env->RegisterNatives(layout_class, k_methods.data(), static_cast<jint>(k_methods.size()));
        if (status != JNI_OK)
        {
            clear_pending(env);
            return false;
        }
        return true;
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the MauiLayout ViewGroup (the JNI shape of the
    // pimpl-owned-native-view doctrine: the apple twin CFReleases its NSView here).
    layout_platform::~layout_platform()
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

    // ILayout.ClipsToBounds → the ViewGroup's child clipping. C#'s iOS LayoutViewExtensions
    // .UpdateClipsToBounds sets PlatformView.ClipsToBounds; the android analog is
    // ViewGroup.setClipChildren + setClipToPadding (a ViewGroup clips its children to its bounds when
    // both are true). Keep the headless mirror in sync first.
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value;
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        call_void_bool(env.get(), panel_of(*this), "setClipChildren", static_cast<jboolean>(value));
        call_void_bool(env.get(), panel_of(*this), "setClipToPadding", static_cast<jboolean>(value));
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform
    // suite — then pushes to the real ViewGroup when one exists (the button_handler dual-path pattern).

    void layout_platform::update_visibility(maui::core::visibility value)
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
        // ViewExtensions.UpdateVisibility → ToPlatformVisibility: Visible/Hidden/Collapsed map to
        // View.VISIBLE/INVISIBLE/GONE.
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), panel_of(*this), "setVisibility", state);
    }

    void layout_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateOpacity: platformView.Alpha = (float)opacity.
            call_void_float(env.get(), panel_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void layout_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId's IsNullOrWhiteSpace gate (a blank id is never pushed).
        if (native == nullptr || value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject panel = panel_of(*this);
        auto& cache = default_jni_cache();
        // PlatformInterop.setContentDescriptionForAutomationId: setting a ContentDescription flips
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had (the button
        // partial documents the same dance).
        jmethodID get_important = cache.method(env.get(), k_maui_layout_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_maui_layout_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(panel, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(panel, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), panel, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    // Render transform + flow direction + background + semantics pushed to the real ViewGroup via the
    // shared android ops (the same set the button partial widens). Each calls the base body FIRST (the
    // VM-less suite observes the headless mirror) — the shared ops are themselves VM-less safe.
    void layout_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void layout_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void layout_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void layout_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // VisualElement.Clip on the layout (wave 24): a ViewOutlineProvider + setClipToOutline(true) clips the
    // ViewGroup AND its children to the convex shape (the clip_views Grid clips its red panel + its "Grid"
    // label child to the shared EllipseGeometry). The base mirror runs FIRST (the VM-less suite observes
    // it). The borrow is stashed so the host's platform_arrange re-resolves the bounds-dependent geometry
    // after layout — the panel is 0×0 at map time (apply_outline_clip clears the clip then; the arrange
    // pass rebuilds it at the live size).
    void layout_platform::update_clip(const maui::graphics::i_shape* value)
    {
        view_platform_base::update_clip(value);
        clip_shape = value;
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // At map time the panel has no size yet — pass {0,0} so apply_outline_clip clears any prior clip;
        // platform_arrange re-resolves it against the live frame. (display_density needs the panel handle.)
        const float density = display_density(env.get(), panel_of(*this));
        maui::platform::android::apply_outline_clip(native, value, density, 0.0, 0.0);
    }

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        auto platform = std::make_unique<layout_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID ctor = cache.method(env.get(), k_maui_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform; // the MauiLayout class is host-provided (java/MauiLayout.java); without it
                             // the panel stays the headless mirror, like the VM-less degradation
        }
        // new MauiLayout(Context) — the LayoutViewGroup analog (the no-op-onLayout host panel).
        const local_ref<jobject> panel{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !panel)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(panel.get()); // released in ~layout_platform
        return platform;
    }

    // Wire the MauiLayout's onLayout callback to this handler: bind nativeArrange (RegisterNatives) and seed
    // the panel's cross-platform peer with this handler's address, so a system layout traversal re-runs the
    // cross-platform arrange (children re-positioned host-relative every pass — the nested-layout fix; see
    // java/MauiLayout.java). Detected by the view_handler base via `requires`; runs once per connect, after
    // create_platform_view (the panel + virtual_view are live here). VM-less / panel-less is a quiet skip.
    void layout_handler::on_connect_handler(layout_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID set_peer = cache.method(env.get(), k_maui_layout_class, "setCrossPlatformPeer", "(J)V");
        if (layout_class == nullptr || set_peer == nullptr || !register_arrange_natives(env.get(), layout_class))
        {
            return; // the MauiLayout class / method is host-provided; without it onLayout stays a no-op
        }
        env->CallVoidMethod(panel_of(platform), set_peer, reinterpret_cast<jlong>(this));
        clear_pending(env.get());
    }

    // Clear the panel's peer before the handler dies, so a later onLayout (the View may outlive the handler
    // briefly during teardown) is a no-op rather than a dangling dereference — the MauiShapeView setNativePtr(0)
    // recipe. The natives binding can stay (it is class-wide and idempotent).
    void layout_handler::on_disconnect_handler(layout_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jmethodID set_peer = default_jni_cache().method(env.get(), k_maui_layout_class, "setCrossPlatformPeer", "(J)V");
        if (set_peer != nullptr)
        {
            env->CallVoidMethod(panel_of(platform), set_peer, static_cast<jlong>(0));
            clear_pending(env.get());
        }
    }

    // C# LayoutHandler.Add inserts the subview at GetLayoutHandlerIndex (the child's z-ordered slot), so
    // the panel stays front-to-back by z-index. The child is already in the layout's logical list. The
    // headless `children` mirror is updated in lock-step (always, even VM-less) so the cross-platform
    // suite observes the same child tracking.
    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child)
                                                     : static_cast<int>(platform->children.size());
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (jobject subview = native_child(child))
        {
            add_view_at(env.get(), panel_of(*platform), subview, static_cast<jint>(target));
        }
    }

    void layout_handler::remove(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        std::erase(platform->children, &child);

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject subview = native_child(child);
        if (subview == nullptr)
        {
            return;
        }
        jmethodID remove_view =
            default_jni_cache().method(env.get(), k_view_group_class, "removeView", "(Landroid/view/View;)V");
        if (remove_view != nullptr)
        {
            env->CallVoidMethod(panel_of(*platform), remove_view, subview);
            clear_pending(env.get());
        }
    }

    void layout_handler::clear()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->children.clear();

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // ViewGroup.removeAllViews — the whole-panel teardown (the apple twin's makeObjectsPerformSelector
        // removeFromSuperview; window_handler.cpp uses the same call to clear its content host).
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(panel_of(*platform), remove_all);
            clear_pending(env.get());
        }
    }

    // C# LayoutHandler.Insert also positions the subview at GetLayoutHandlerIndex (the z-ordered slot),
    // not the logical `index` — the panel's subview order is z-index-driven. The child is in the logical
    // list. The headless mirror tracks the same z-ordered position.
    void layout_handler::insert(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : index;
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (jobject subview = native_child(child))
        {
            add_view_at(env.get(), panel_of(*platform), subview, static_cast<jint>(target));
        }
    }

    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        if (index < 0 || static_cast<std::size_t>(index) >= children.size())
        {
            return;
        }
        children[static_cast<std::size_t>(index)] = &child; // replace-in-place: count is unchanged

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // Swap the existing subview at `index` for the new child's, in place: removeViewAt(index) +
        // addView(newChild, index). The apple twin does the same removeFromSuperview + reinsert.
        auto& cache = default_jni_cache();
        jmethodID remove_view_at = cache.method(env.get(), k_view_group_class, "removeViewAt", "(I)V");
        if (remove_view_at != nullptr)
        {
            env->CallVoidMethod(panel_of(*platform), remove_view_at, static_cast<jint>(index));
            clear_pending(env.get());
        }
        if (jobject subview = native_child(child))
        {
            add_view_at(env.get(), panel_of(*platform), subview, static_cast<jint>(index));
        }
    }

    // C# LayoutHandler.EnsureZIndexOrder: move `child`'s subview to its z-ordered slot (remove + reinsert
    // at the target). The children mirror is re-synced to match.
    void layout_handler::update_z_index(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        const auto current = std::ranges::find(children, &child);
        if (current == children.end())
        {
            return; // not hosted (currentIndex == -1)
        }
        const int target = get_layout_handler_index(*virtual_view(), child);
        if (target < 0)
        {
            return;
        }
        children.erase(current);
        const auto position = std::min(static_cast<std::size_t>(target), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);

        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject subview = native_child(child);
        if (subview == nullptr)
        {
            return;
        }
        // removeView + addView(child, target) reorders the subview list to the z-ordered slot.
        jmethodID remove_view =
            default_jni_cache().method(env.get(), k_view_group_class, "removeView", "(Landroid/view/View;)V");
        if (remove_view != nullptr)
        {
            env->CallVoidMethod(panel_of(*platform), remove_view, subview);
            clear_pending(env.get());
        }
        add_view_at(env.get(), panel_of(*platform), subview, static_cast<jint>(target));
    }

    maui::graphics::size layout_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // A layout computes its own size through its layout_manager (the control overrides measure to
        // delegate to the manager, not the handler), so the handler reports nothing here — exactly like
        // the apple/headless twins. The ViewGroup is never asked to measure the children.
        return {0, 0};
    }

    void layout_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native panel to position
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject panel = panel_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the panel measures Exactly at the
        // final size (Android requires a measure pass before layout) and lays out. The same two-step
        // every leaf android handler uses (button_handler.cpp::platform_arrange). MauiLayout.onMeasure
        // resolves the Exactly spec to the panel size; its onLayout is a no-op (the children keep the
        // absolute frames their own platform_arrange already set).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_maui_layout_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_maui_layout_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), panel);
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
        env->CallVoidMethod(panel, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(panel, layout, left, top, left + width, top + height);
        clear_pending(env.get());

        // Re-resolve the clip against the just-laid-out bounds (the iOS reapply_clip analog): the outline
        // geometry resolves against the live frame size in points, and update_clip cleared it at map time
        // when the panel was 0×0. frame is in points.
        if (platform->clip_shape != nullptr)
        {
            maui::platform::android::apply_outline_clip(platform->native, platform->clip_shape, density, frame.width,
                                                        frame.height);
        }
    }
} // namespace maui::core

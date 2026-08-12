// scroll_view_handler — Android (JNI) platform partial: a real android.widget.ScrollView whose single
// child IS the content's native view. The android twin of src/platform/apple/scroll_view_handler.mm (an
// NSScrollView whose documentView is the content) and the real-native sibling of the headless mirror
// (src/platform/headless/scroll_view_handler.cpp). Translated from ScrollViewHandler.cs +
// ScrollViewHandler.Android.cs (MAUI's MauiScrollView):
//   - set_content re-parents the content's native view as the ScrollView's single child (C#
//     UpdateContentView: remove the old, AddView the new with MATCH_PARENT/WRAP-on-scroll-axis params —
//     the same re-host window/content_page do).
//   - Orientation: android.widget.ScrollView scrolls VERTICALLY only (the MauiScrollView default
//     orientation). A horizontal/both ScrollView would need HorizontalScrollView / a nested pair; that
//     is the documented deviation below. The control still measures/arranges the content at its full
//     (possibly overflowing) size, so the vertical scroller has the right content extent.
//   - scroll_to ports MapRequestScrollTo collapsed to its observable effects (clamp + offsets write-back
//     + ScrollFinished), driving the native ScrollView.scrollTo when one exists. The native USER-scroll
//     write-back proxy is deferred with the gesture/listener fan-out (see the deviation) — the headless
//     synchronous write-back keeps the Scrolled contract observable.
//
// DOCUMENTED DEVIATIONS from the C# oracle (infrastructure gaps, not behavior guesses):
//   - The scroller is android.widget.ScrollView (vertical). HORIZONTAL / BOTH orientation needs
//     HorizontalScrollView (or the nested MauiHorizontalScrollView/ScrollView pair MAUI builds); that
//     widget swap is deferred — vertical covers the gallery's content pages. Orientation still updates
//     the headless mirror so the cross-platform suite sees the value.
//   - ScrollView IS-A FrameLayout, so it re-measures + re-lays-out its single child on each pass. The
//     child here is the content's native view (a MauiLayout for a container content), which reports its
//     resolved size via onMeasure and whose own children keep their absolute frames (MauiLayout.onLayout
//     is a no-op). The child is laid out at the scroller's origin filling its measured size — exactly
//     the document-at-origin shape the apple documentView gives.
//   - The native scrolled write-back (ScrollEventProxy / OnScrollChangeListener) is deferred with the
//     listener fan-out; scroll_to writes the offsets back synchronously (the headless path), so the
//     Scrolled event still fires. The scroll-bar visibility maps to setVerticalScrollBarEnabled.
//
// VM-less degradation (like button/window/layout/content_page): every JNI path checks scoped_env /
// app_context() and quietly skips when no Java VM exists, while the headless mirrors (orientation / bar
// visibilities / hosted content / offsets / scroll_requests) are ALWAYS maintained.

#include "maui/core/scroll_view_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
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

    constexpr const char* k_scroll_view_class = "android/widget/ScrollView";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_important_for_accessibility_auto = 0;
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);
    constexpr jint k_match_parent = -1; // ViewGroup.LayoutParams.MATCH_PARENT
    constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    [[nodiscard]] jobject scroller_of(const maui::core::scroll_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

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

    void call_void_int(JNIEnv* env, jobject scroller, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_scroll_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(scroller, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject scroller, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_scroll_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(scroller, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject scroller, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_scroll_view_class, name, "(Z)V"))
        {
            env->CallVoidMethod(scroller, method, value);
            clear_pending(env);
        }
    }

    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    [[nodiscard]] float display_density(JNIEnv* env, jobject scroller)
    {
        static std::atomic<float> memoized{0.0F};
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_scroll_view_class, "getContext", "()Landroid/content/Context;");
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
        const local_ref<jobject> context{env, env->CallObjectMethod(scroller, get_context)};
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

    [[nodiscard]] jobject native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

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

    // Add `child` as the ScrollView's single document child: MATCH_PARENT width, WRAP_CONTENT height so
    // the content can overflow vertically and scroll (the android scroll-content convention — the apple
    // documentView analog). Detaches the child from any prior parent first.
    void add_scroll_child(JNIEnv* env, jobject scroller, jobject child)
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
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_match_parent, k_wrap_content)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(scroller, add_view, child, params.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    scroll_view_platform::~scroll_view_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env;
            if (env)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    // The generic-IView pushes (dual-path: base mirror FIRST, then the scroller when one exists).
    void scroll_view_platform::update_visibility(maui::core::visibility value)
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
        call_void_int(env.get(), scroller_of(*this), "setVisibility", state);
    }

    void scroll_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_float(env.get(), scroller_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void scroll_view_platform::update_automation_id(std::string_view value)
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
        jobject scroller = scroller_of(*this);
        auto& cache = default_jni_cache();
        jmethodID get_important = cache.method(env.get(), k_scroll_view_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_scroll_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(scroller, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(scroller, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), scroller, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    void scroll_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void scroll_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void scroll_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    void scroll_view_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<scroll_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation
        }
        auto& cache = default_jni_cache();
        jclass scroll_class = cache.find_class(env.get(), k_scroll_view_class);
        jmethodID ctor = cache.method(env.get(), k_scroll_view_class, "<init>", "(Landroid/content/Context;)V");
        if (scroll_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        const local_ref<jobject> scroller{env.get(), env->NewObject(scroll_class, ctor, context)};
        if (clear_pending(env.get()) || !scroller)
        {
            return platform;
        }
        // setFillViewport(true): the content fills the viewport when shorter than it, and overflows
        // (scrolls) when taller — the android equivalent of the scroller sizing its document.
        call_void_bool(env.get(), scroller.get(), "setFillViewport", JNI_TRUE);
        platform->native = env->NewGlobalRef(scroller.get()); // released in ~scroll_view_platform
        return platform;
    }

    // The native scrolled write-back proxy is deferred with the listener fan-out (see the deviation); the
    // headless scroll_to writes the offsets back directly, so connect/disconnect have nothing to wire.
    void scroll_view_handler::on_connect_handler(scroll_view_platform& /*platform*/)
    {
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& /*platform*/)
    {
    }

    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
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
        jobject scroller = scroller_of(*platform);
        // C# UpdateContentView: clear the old document child, then add the new content's native view.
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(scroller, remove_all);
            clear_pending(env.get());
        }
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (jobject child = native_child(*platform->hosted_content))
        {
            add_scroll_child(env.get(), scroller, child);
        }
    }

    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // android.widget.ScrollView is vertical-only; horizontal/both is the deferred widget swap (see the
        // deviation). Keep the headless mirror current so the cross-platform suite observes the value.
        platform->orientation = virtual_view()->orientation();
    }

    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->horizontal_bar_visibility = virtual_view()->horizontal_scroll_bar_visibility();
        // The vertical ScrollView has no horizontal bar; nothing native to push (deferred with the
        // horizontal-scroller swap). The mirror stays observable.
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const scroll_bar_visibility visibility = virtual_view()->vertical_scroll_bar_visibility();
        platform->vertical_bar_visibility = visibility;
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // C# Map*ScrollBarVisibility → never hides the bar; default/always show it. ScrollView has no
        // pin-on (always) mode, so default/always both enable the bar, never disables it.
        const jboolean enabled = visibility != scroll_bar_visibility::never ? JNI_TRUE : JNI_FALSE;
        call_void_bool(env.get(), scroller_of(*platform), "setVerticalScrollBarEnabled", enabled);
    }

    // C# MapRequestScrollTo, collapsed to its observable effects (the headless path): record the request,
    // clamp the target to the available scroll range, write the offsets back, acknowledge ScrollFinished —
    // and additionally drive the native ScrollView.scrollTo when one exists.
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request);

        const maui::graphics::size content = view->content_size();
        const maui::graphics::rect frame = view->frame();
        const double available_x = std::max(content.width - frame.width, 0.0);
        const double available_y = std::max(content.height - frame.height, 0.0);
        const double target_x = std::clamp(request.horizontal_offset, 0.0, available_x);
        const double target_y = std::clamp(request.vertical_offset, 0.0, available_y);

        platform->offset_x = target_x;
        platform->offset_y = target_y;
        view->set_horizontal_offset(target_x);
        view->set_vertical_offset(target_y);

        // Drive the native scroller (ScrollView.scrollTo takes pixels). The vertical ScrollView only
        // moves vertically; the horizontal target is still recorded in the mirror.
        if (platform->native != nullptr)
        {
            const scoped_env env;
            if (env)
            {
                jobject scroller = scroller_of(*platform);
                const float density = display_density(env.get(), scroller);
                jmethodID scroll_to = default_jni_cache().method(env.get(), k_scroll_view_class, "scrollTo", "(II)V");
                if (scroll_to != nullptr)
                {
                    env->CallVoidMethod(scroller, scroll_to, to_pixels(target_x, density),
                                        to_pixels(target_y, density));
                    clear_pending(env.get());
                }
            }
        }

        view->scroll_finished();
    }

    void scroll_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native scroller to frame
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject scroller = scroller_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the scroller measures Exactly at the
        // viewport size and lays out. ScrollView re-measures/lays-out its single child for the scrollable
        // extent (the content's onMeasure reports its full size; its own children keep absolute frames).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_scroll_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_scroll_view_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), scroller);

        // The SAFE AREA is applied as NATIVE PADDING, not as the content's arrange origin. Android's
        // ScrollView is a FrameLayout: the measure/layout pair below makes it re-lay-out its single child
        // at (paddingLeft, paddingTop), DISCARDING the origin scroll_view::arrange just gave that child.
        // So the cross-platform offset alone renders nothing — measured: border_stroke lost its whole
        // 136 px top inset and hid its first label behind the status bar.
        //
        // WHY THIS WAS INVISIBLE UNTIL NOW, and why the padding must stay even if the host reverts to a
        // single pre-inset rect: the offset that gets discarded is `safe_y`, and `safe_y` was ALWAYS ZERO
        // on this backend, because the apphost passed full == safe bounds to drive_layout and every
        // downstream safe-area path collapsed to a no-op. Discarding zero is not observable. scroll_view
        // is ALSO the only host in the port that expresses the inset as an origin on its CHILD'S frame —
        // layout::arrange insets its own children inside itself (host_relative), and MauiLayout.onLayout
        // replays those cached child frames, so a native ViewGroup cannot overwrite them. That asymmetry
        // is why ScrollView roots were the only ones that broke. Anyone who makes the page-level inset
        // nonzero for ANY reason walks straight back into this, so the mechanism is recorded here rather
        // than left to be rediscovered from a 32-page red column.
        //
        // This is MAUI's mechanism, not a workaround: SafeAreaExtensions.ApplyAdjustedSafeAreaInsetsPx
        // (src/Core/src/Platform/Android/SafeAreaExtensions.cs) ends in `view.SetPadding(...)` on the
        // MauiScrollView, whose OnLayout then just calls base.OnLayout. (MAUI's own ScrollView.Padding is
        // a SEPARATE layer there — a ContentViewGroup "paddingShim" between scroller and content,
        // ScrollViewHandler.Android.cs:240-250 — so it is deliberately NOT folded in here.)
        // The cross-platform origin stays as-is: the native layout simply wins, and both agree on the
        // same number because applied_safe_area_insets() IS effective_safe_area().
        if (const auto* safe_area_view = dynamic_cast<const maui::core::i_safe_area_view2*>(virtual_view()))
        {
            const maui::core::thickness safe_area = safe_area_view->applied_safe_area_insets();
            if (jmethodID set_padding = cache.method(env.get(), k_scroll_view_class, "setPadding", "(IIII)V");
                set_padding != nullptr)
            {
                env->CallVoidMethod(scroller, set_padding, to_pixels(safe_area.left, density),
                                    to_pixels(safe_area.top, density), to_pixels(safe_area.right, density),
                                    to_pixels(safe_area.bottom, density));
                clear_pending(env.get());
            }
        }

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
        env->CallVoidMethod(scroller, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(scroller, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

// indicator_view_handler — Android (JNI) platform partial: the position-indicator dot row drawn by hand.
//
// This AAR-less backend carries NO AndroidX, so there is no androidx.viewpager2 page indicator and no
// material TabLayout dot strip — the same dependency cut every android partial documents (button =
// AppCompatButton, radio = AppCompatRadioButton, …). So the dots are assembled by hand, EXACTLY mirroring
// the apple recipe (src/platform/apple/indicator_view_handler.mm, the NSStackView-of-dots): the managed
// platform view is a dev.mauicpp.MauiLayout host ViewGroup, and the handler realizes GetMaximumVisible
// small Views into it — each backed by a circular (or square) android.graphics.drawable.GradientDrawable
// — the current page tinted with SelectedIndicatorColor, the rest with IndicatorColor. The whole row is
// rebuilt on any count / size / shape / color / position change (the C# UpdateIndicatorCount +
// ResetIndicators collapsed — a handful of tiny Views, so a full rebuild is cheap, exactly as the apple
// twin does). The cross-platform mirror (dot_count / current_page / size / shape / colors) is kept live so
// the android pure-native cross-platform suite (no JavaVM) observes the headless partial's behavior
// unchanged.
//
// Ported from IndicatorViewHandler.cs (the cross-platform mapper + GetMaximumVisible/IsCircleShape, in
// src/core/indicator_view_handler.cpp) + IndicatorViewHandler.Android.cs / IndicatorViewExtensions.cs (the
// ResetIndicators dot-assembly recipe) + MauiPageControl.cs (the GetCurrentPage clamp). The dot Views are
// MauiLayout children laid out ABSOLUTELY (the no-op-onLayout container convention every android container
// handler shares — the frames survive because the handler installs NO crossPlatformPeer on this host, so
// MauiLayout.onLayout is a no-op and the absolute dot frames stick; see java/MauiLayout.java).
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (library / infrastructure gaps, not behavior guesses):
//   - The host is a plain dev.mauicpp.MauiLayout, not MAUI's MauiPageControl (a LinearLayout subclass with
//     the tap channel). A bare MauiLayout has no native tap channel, so the indicator Position is
//     virtual→native only here (set_position still works programmatically through the mapper); the inbound
//     dot-tap write-back (the apple twin omits it too — inert NSViews) is DEFERRED with the gesture/event
//     fan-out. The cross-platform mirror's current_page still tracks Position so the VM-less suite asserts
//     it.
//   - Each dot is a plain android.view.View with a GradientDrawable background (setShape OVAL for a circle,
//     RECTANGLE for a square) — the same GradientDrawable stand-in the radio/button border partials install.
//     The C# IndicatorTemplate (a custom per-dot View) is OMITTED (the indicator_view header's documented
//     template collapse — the port renders default dots only), so a transparent IndicatorColor /
//     SelectedIndicatorColor leaves the dots effectively invisible, exactly the C# template-glyph-instead-
//     of-dots intent the indicator_page's "Template" row relies on.
//   - The square-shape image swap (UIPageControl's "squareshape.fill") collapses to a square GradientDrawable
//     (setShape RECTANGLE), the closest plain-drawable analog (the same enum the i_indicator_view shape
//     carries).
//
// VM-less degradation (like every android handler): create_platform_view / the dot rebuild check scoped_env
// / app_context() and quietly skip when no Java VM exists — the headless mirror is always live. The gallery
// app host drives the real MauiLayout dot row.

#include "maui/core/indicator_view_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>

#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/controls/indicator_shape.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // The host ViewGroup (the no-op-onLayout panel; the dots are its absolutely-framed children). The
    // View/ViewGroup surface (addView/removeAllViews/measure/layout/setVisibility/…) resolves through it.
    constexpr const char* k_host_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";

    constexpr jint k_view_visible = 0; // android.view.View visibility states
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT

    // android.graphics.drawable.GradientDrawable shape constants.
    constexpr jint k_shape_rectangle = 0; // GradientDrawable.RECTANGLE (the square dot)
    constexpr jint k_shape_oval = 1;      // GradientDrawable.OVAL (the round dot)

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling.
    constexpr double k_to_pixels_epsilon = 0.0000000001;
    // IndicatorViewRenderer.DefaultIndicatorSize == 6 on Android (the C# default dot diameter in dp). The
    // iOS twin uses 7 (UIPageControl's dot); the android ResetIndicators sizes each dot at IndicatorSize
    // with a 6dp default, so an unset size renders 6dp dots.
    constexpr double k_default_indicator_size = 6.0;
    // The inter-dot gap (dp) — the apple NSStackView spacing == 4 (the C# DefaultPadding); mirror it so the
    // android row reads identically.
    constexpr double k_dot_spacing = 4.0;

    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, the channel the test host uses
        env->ExceptionClear();
        return true;
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    [[nodiscard]] jobject host_of(const maui::core::indicator_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // The display density (Context.getResources().getDisplayMetrics().density). 1.0 when any step fails —
    // the same walk every android handler shares (kept local so the partial stays independently buildable).
    [[nodiscard]] float display_density(JNIEnv* env, jobject view)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_view_class, "getContext", "()Landroid/content/Context;");
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
        const local_ref<jobject> context{env, env->CallObjectMethod(view, get_context)};
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

    void call_void_int(JNIEnv* env, jobject view, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(view, method, value);
            clear_pending(env);
        }
    }

    // Build ONE dot: a plain android.view.View whose background is a circular (or square) GradientDrawable
    // filled with `argb`, sized `size_px` square. Theme-independent ctor (View(Context)) so it builds in the
    // bare app_process testhost (the LESSON-2 constraint). Returns a local ref (caller adds + frames it).
    [[nodiscard]] local_ref<jobject> make_dot(JNIEnv* env, jobject context, jint argb, bool square, jint size_px)
    {
        auto& cache = default_jni_cache();
        jclass view_class = cache.find_class(env, k_view_class);
        jmethodID view_ctor = cache.method(env, k_view_class, "<init>", "(Landroid/content/Context;)V");
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID gradient_ctor = cache.method(env, k_gradient_drawable_class, "<init>", "()V");
        jmethodID set_shape = cache.method(env, k_gradient_drawable_class, "setShape", "(I)V");
        jmethodID set_color = cache.method(env, k_gradient_drawable_class, "setColor", "(I)V");
        jmethodID set_size = cache.method(env, k_gradient_drawable_class, "setSize", "(II)V");
        jmethodID set_background =
            cache.method(env, k_view_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (view_class == nullptr || view_ctor == nullptr || gradient_class == nullptr || gradient_ctor == nullptr ||
            set_shape == nullptr || set_color == nullptr || set_background == nullptr)
        {
            return {};
        }
        const local_ref<jobject> drawable{env, env->NewObject(gradient_class, gradient_ctor)};
        if (clear_pending(env) || !drawable)
        {
            return {};
        }
        env->CallVoidMethod(drawable.get(), set_shape, square ? k_shape_rectangle : k_shape_oval);
        clear_pending(env);
        env->CallVoidMethod(drawable.get(), set_color, argb);
        clear_pending(env);
        if (set_size != nullptr)
        {
            env->CallVoidMethod(drawable.get(), set_size, size_px, size_px); // the intrinsic dot extent
            clear_pending(env);
        }
        local_ref<jobject> dot{env, env->NewObject(view_class, view_ctor, context)};
        if (clear_pending(env) || !dot)
        {
            return {};
        }
        env->CallVoidMethod(dot.get(), set_background, drawable.get());
        clear_pending(env);
        return dot;
    }

    // Add `child` to `host` with WRAP/WRAP params, then measure Exactly + layout(l,t,r,b) it ABSOLUTELY in
    // PIXELS (the android container convention — MauiLayout.onLayout is a no-op with no peer, so this frame
    // survives). The same two-step every leaf android handler's platform_arrange does.
    void add_and_frame(JNIEnv* env, jobject host, jobject child, jint left, jint top, jint right, jint bottom)
    {
        auto& cache = default_jni_cache();
        jclass params_class = cache.find_class(env, k_layout_params_class);
        jmethodID params_ctor = cache.method(env, k_layout_params_class, "<init>", "(II)V");
        jmethodID add_view = cache.method(env, k_view_group_class, "addView",
                                          "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        jmethodID measure = cache.method(env, k_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env, k_view_class, "layout", "(IIII)V");
        jmethodID make_measure_spec =
            cache.static_method(env, "android/view/View$MeasureSpec", "makeMeasureSpec", "(II)I");
        jclass measure_spec_class = cache.find_class(env, "android/view/View$MeasureSpec");
        if (params_class == nullptr || params_ctor == nullptr || add_view == nullptr || measure == nullptr ||
            layout == nullptr || make_measure_spec == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const local_ref<jobject> params{env, env->NewObject(params_class, params_ctor, k_wrap_content, k_wrap_content)};
        if (clear_pending(env) || !params)
        {
            return;
        }
        env->CallVoidMethod(host, add_view, child, params.get());
        if (clear_pending(env))
        {
            return;
        }
        constexpr auto k_exactly = static_cast<jint>(0x40000000U); // View.MeasureSpec.EXACTLY
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, right - left, k_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, bottom - top, k_exactly);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(child, measure, width_spec, height_spec);
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(child, layout, left, top, right, bottom);
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Release the global reference pinning the host MauiLayout (the JNI shape of the pimpl-owned-native-view
    // doctrine; the apple twin CFReleases its NSStackView here). The dot children die with the host.
    indicator_view_platform::indicator_view_platform() = default;

    indicator_view_platform::~indicator_view_platform()
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

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each calls
    // the base body FIRST — the headless mirror must stay live for the VM-less cross-platform suite — then
    // pushes to the real host ViewGroup when one exists.

    void indicator_view_platform::update_visibility(maui::core::visibility value)
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

    void indicator_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            if (jmethodID set_alpha = default_jni_cache().method(env.get(), k_view_class, "setAlpha", "(F)V"))
            {
                env->CallVoidMethod(host_of(*this), set_alpha, static_cast<jfloat>(value));
                clear_pending(env.get());
            }
        }
    }

    void indicator_view_platform::update_automation_id(std::string_view value)
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
        jmethodID set_description =
            default_jni_cache().method(env.get(), k_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (set_description != nullptr)
        {
            const local_ref<jstring> description = maui::platform::android::to_jstring(env.get(), value);
            env->CallVoidMethod(host_of(*this), set_description, description.get());
            clear_pending(env.get());
        }
    }

    void indicator_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // The band behind the dots (the Colors row's Yellow): paint the host ViewGroup's background via the
        // shared op. The dots are CHILDREN of the host (not its background), so the fill sits behind them —
        // exactly the iOS UIPageControl band. A null paint clears it (the shared op's branch).
        maui::platform::android::apply_background(native, value);
    }

    std::unique_ptr<indicator_view_platform> indicator_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<indicator_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (file header)
        }
        auto& cache = default_jni_cache();
        jclass host_class = cache.find_class(env.get(), k_host_class);
        jmethodID host_ctor = cache.method(env.get(), k_host_class, "<init>", "(Landroid/content/Context;)V");
        if (host_class == nullptr || host_ctor == nullptr)
        {
            return platform; // MauiLayout is host-provided (java/MauiLayout.java); without it the mirror stands
        }
        const local_ref<jobject> host{env.get(), env->NewObject(host_class, host_ctor, context)};
        if (clear_pending(env.get()) || !host)
        {
            return platform;
        }
        platform->native = env->NewGlobalRef(host.get()); // released in ~indicator_view_platform
        return platform;
    }

    void indicator_view_handler::on_connect_handler(indicator_view_platform& /*platform*/)
    {
        // C# ConnectHandler: SetIndicatorView + UpdateIndicator. The dot row is built by the mapper pass
        // (which runs right after connect), so no extra work here (the apple twin is identical).
    }

    void indicator_view_handler::on_disconnect_handler(indicator_view_platform& /*platform*/)
    {
    }

    namespace
    {
        // Rebuild the dot row: GetMaximumVisible dots sized IndicatorSize, the current page tinted with the
        // selected color, the rest the indicator color, laid out absolutely left-to-right with a 4dp gap.
        // The C# UpdateIndicatorCount + ResetIndicators collapsed (a full rebuild on any change — a handful
        // of tiny Views, exactly as the apple NSStackView twin rebuilds). The cross-platform mirror is
        // written FIRST so the VM-less suite observes it even when no native host exists.
        void rebuild_dots(indicator_view_handler& handler, i_indicator_view& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            const int dots = max_visible_indicators(view);
            const int position = view.position();
            const int current = dots > 0 ? std::min(position, dots - 1) : -1;
            double size_dp = view.indicator_size();
            if (!(size_dp > 0))
            {
                size_dp = k_default_indicator_size; // FontManager-style "unset → default" (C# DefaultIndicatorSize)
            }

            // Mirror first (the oracle record) — the same fields the headless partial records.
            platform->dot_count = dots;
            platform->current_page = current;
            platform->indicator_size = view.indicator_size();
            platform->shape = view.indicators_shape();
            platform->indicator_color = view.indicator_color();
            platform->selected_indicator_color = view.selected_indicator_color();

            if (platform->native == nullptr)
            {
                return; // VM-less / context-less: the headless mirror is the asserted surface
            }
            const scoped_env env;
            jobject context = app_context();
            if (!env || context == nullptr)
            {
                return;
            }
            jobject host = host_of(*platform);
            auto& cache = default_jni_cache();
            const float density = display_density(env.get(), host);
            const bool square = view.indicators_shape() == maui::controls::indicator_shape::square;

            // Drop the previous row (ResetIndicators rebuilds from scratch).
            if (jmethodID remove_all = cache.method(env.get(), k_view_group_class, "removeAllViews", "()V"))
            {
                env->CallVoidMethod(host, remove_all);
                clear_pending(env.get());
            }

            const jint dot_px = std::max<jint>(1, to_pixels(size_dp, density));
            const jint gap_px = to_pixels(k_dot_spacing, density);
            jint cursor_px = 0;
            for (int index = 0; index < dots; ++index)
            {
                const maui::graphics::color tint =
                    index == current ? view.selected_indicator_color() : view.indicator_color();
                const local_ref<jobject> dot =
                    make_dot(env.get(), context, static_cast<jint>(tint.to_int()), square, dot_px);
                if (!dot)
                {
                    continue;
                }
                add_and_frame(env.get(), host, dot.get(), cursor_px, 0, cursor_px + dot_px, dot_px);
                cursor_px += dot_px + gap_px;
            }
        }
    } // namespace

    void indicator_view_handler::map_count(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_hide_single(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_position(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view); // re-tint the selected dot
    }

    void indicator_view_handler::map_indicator_size(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    void indicator_view_handler::map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view)
    {
        rebuild_dots(handler, view);
    }

    maui::graphics::size indicator_view_handler::get_desired_size(double /*width_constraint*/,
                                                                  double /*height_constraint*/) const
    {
        // A row of `dot_count` dots of indicator_size each, with a 4dp gap between them (the natural dot-row
        // extent — the dp the cross-platform measure speaks; add_and_frame already laid the dots out in
        // pixels). Mirrors the headless metric so the layout pass reserves the same row, defaulting an unset
        // size to the C# DefaultIndicatorSize. Height is one dot tall.
        const auto* platform = typed_platform_view();
        double size_dp = platform != nullptr ? platform->indicator_size : k_default_indicator_size;
        if (!(size_dp > 0))
        {
            size_dp = k_default_indicator_size;
        }
        const int dots = platform != nullptr ? platform->dot_count : 0;
        if (dots <= 0)
        {
            return {0, 0}; // a hidden / empty indicator (HideSingle collapse) reserves nothing
        }
        const double width = (size_dp * dots) + (k_dot_spacing * (dots - 1));
        return {width, size_dp};
    }

    void indicator_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native layout to apply
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*platform);
        auto& cache = default_jni_cache();
        jmethodID make_measure_spec =
            cache.static_method(env.get(), "android/view/View$MeasureSpec", "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_host_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_host_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), "android/view/View$MeasureSpec");
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), host);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        constexpr auto k_exactly = static_cast<jint>(0x40000000U); // View.MeasureSpec.EXACTLY
        const jint width_spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_exactly);
        const jint height_spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_exactly);
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

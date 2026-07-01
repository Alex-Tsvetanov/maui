// border_handler — Android (JNI) platform partial: a real dev.mauicpp.MauiLayout ViewGroup that
// HOSTS the single content child AND carries the border as a maui-managed
// android.graphics.drawable.GradientDrawable installed as the host's background. The android twin of
// src/platform/apple/border_handler.mm (a plain NSView host + a CAShapeLayer stroke) and the
// real-native sibling of the headless single-content + stroke-spec mirror
// (src/platform/headless/border_handler.cpp). Translated from BorderHandler.cs +
// BorderHandler.Android.cs (the ContentViewGroup host; UpdateContent: RemoveAllViews + AddView) + the
// StrokeExtensions / MauiCALayer funnel (every stroke map → one UpdateMauiCALayer refresh →
// update_border here).
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// THE C# ORACLE AND WHY THIS PARTIAL DEVIATES (documented, not stubbed)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// In .NET MAUI, Android Border is hosted by a Microsoft.Maui.Platform.ContentViewGroup whose Draw is
// intercepted by a Microsoft.Maui.Graphics.ShapeDrawable that paints the stroke/fill along the shape
// path through the Microsoft.Maui.Graphics CANVAS stack (BorderHandler.Android.cs +
// ContentViewGroup.cs). As with the BoxView partial (src/platform/android/box_view_handler.cpp), the
// fidelity-perfect port would be a custom-View onDraw(android.graphics.Canvas) backed by an
// i_canvas → android.graphics.Canvas bridge — a bridge that DOES NOT EXIST in the Android backend yet
// (no Skia/i_canvas Android implementation; the only shipped custom Java View is MauiLayout, a no-op
// ViewGroup). Building the canvas twin would require a new Java class + a native canvas bridge + a
// CMake/Java fan-out — out of scope for this two-file slice.
//
// So this partial reuses the SAME deviation the android button partial established
// (src/platform/android/button_handler.cpp's update_button_stroke) and the box_view partial documents:
// ONE android.graphics.drawable.GradientDrawable installed as the host MauiLayout's background, with
//   - i_border_view::stroke() paint color  → GradientDrawable.setStroke(widthPx, argb[, dashW, dashG])
//                                            (Stroke brush; the 4-arg overload when a dash array is set)
//   - i_border_view::stroke_thickness()     → that setStroke width (dp → px via ToPixels)
//   - i_border_view::stroke_dash_array()    → the dashW/dashG of the 4-arg setStroke (SetBorderDash: each
//                                            dash entry × strokeThickness px — a canvas-free DashPathEffect)
//   - the StrokeShape's rounded-rect corner → setCornerRadius(px)                          (Shape radius)
//   - the generic IView background paint    → setColor(argb) for a SolidPaint, OR setColors(int[]) +
//                                            setOrientation/setGradientType for a gradient brush (the fill)
// This is a FAITHFUL expression for the common Border AND now for the two facets GradientDrawable models
// natively: a DASHED stroke (setStroke's dashWidth/dashGap) and a GRADIENT FILL (setColors' multi-stop
// linear/radial ramp). It stays lossy where the framework GradientDrawable cannot reach — a GRADIENT
// STROKE brush (setStroke takes one solid color, so a gradient stroke renders as the paint's representative
// blended color), a dash array of more than two segments (collapsed to the first on/off pair — the single
// pair setStroke exposes), a gradient angle other than the eight cardinal Orientations, per-corner-distinct
// radii, non-miter joins, and arbitrary StrokeShape paths — exactly the surface MAUI routes through
// MauiDrawable's ShapeDrawable canvas, which this backend still lacks. Those remaining pieces funnel
// through update_border, keep the headless border_stroke_spec mirror current, and degrade gracefully on
// the native side (as the apple twin's CAShapeLayer is the only place caps/joins/gradient-strokes land).
// The dev.mauicpp.MauiLayout ctor is theme-independent (the same no-op ViewGroup content_page hosts into),
// so it constructs in the bare app_process testhost (LESSON 2 in docs/MACOS_ANDROID_RESUME.md — unlike
// the EditText / horizontal-ProgressBar ctors that resolve a theme style attr).
//
// HOW THE STROKE SHAPE / CORNER RADIUS REACHES THE HANDLER (the load-bearing mapping):
// the StrokeShape corner radius does NOT arrive as a value — the handler sees i_border_view, whose
// shape() is the control's i_shape (a RoundRectangle), and the union mapper's "stroke_shape" KEY
// funnels to update_border with no value. So the radius is recovered off the shape geometry exactly as
// box_view_handler.cpp's corner_radii_of does: shape->path_for_bounds(reference square) lays
// append_rounded_rectangle(x,y,w,h, tl,…) whose VERY FIRST point is move_to(x, y + tl), so the
// top-left radius is first_point().y - bounds.y. Exact for a uniform-radius RoundRectangle (the common
// Border shape); a conservative top-left approximation for the rare four-distinct case (still a rounded
// box). A square Rectangle or absent shape yields 0 → a sharp-cornered border, correct.
//
// THE HEADLESS MIRROR STAYS LIVE: every shared write the headless twin makes — hosted_content (the
// single content child) and the border_stroke_spec snapshot (make_border_stroke_spec) — is preserved
// here, so the android preset's PURE-NATIVE cross-platform suite (tools/android-emu-run.sh, no JavaVM)
// observes exactly the headless partial's behavior. The JNI GradientDrawable + MauiLayout pushes are
// layered ON TOP, behind scoped_env/app_context guards, and run only when a VM + a native host exist
// (the widget testhost + the app host).

#include "maui/core/border_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // The no-op-onLayout ViewGroup hosting the content child (the same host content_page_handler.cpp
    // uses) — also the surface carrying the border's GradientDrawable background. GetMethodID walks
    // superclasses, so the ViewGroup/View surface resolves through this class directly.
    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";
    constexpr const char* k_gradient_orientation_class = "android/graphics/drawable/GradientDrawable$Orientation";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // GradientDrawable.GradientType.RADIAL_GRADIENT (LINEAR_GRADIENT is the drawable's constructed default = 0).
    constexpr jint k_radial_gradient_type = 1;
    // A nominal px extent scaling a relative (0..1) radial radius into GradientDrawable's px-only radius (the
    // plain-drawable expression of a radial ramp, mirroring android_visual_ops's apply_background).
    constexpr float k_nominal_radial_extent = 100.0F;

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (restored after setContentDescription flips it).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec mode (PlatformArrange measures Exactly at the final size).
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    // ViewGroup.LayoutParams constants (the content fills the host; the host wraps content up front).
    constexpr jint k_match_parent = -1; // MATCH_PARENT — the content fills the border host
    constexpr jint k_wrap_content = -2; // WRAP_CONTENT — a parentless host's placeholder params

    [[nodiscard]] jobject host_of(const maui::core::border_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the partial must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back. Mirrors the
    // button/content_page partials' clear_pending.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, same channel the test host uses
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

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact (button partial).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The host's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read — exactly the button/content_page partials'
    // display_density, kept standalone so the partials stay independently buildable. 1.0 when any step
    // fails (failures are not memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject host)
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

    // The maui-managed GradientDrawable carrying the border stroke + corner radius + background fill (the
    // GradientDrawable stand-in for the ShapeDrawable canvas paint — see the header deviations). Returns
    // the installed one (View.getBackground() instanceof GradientDrawable identifies ours), installing a
    // fresh one only when `install` is set. An empty ref means "not installed and not asked to install"
    // (or a JNI failure). The button partial's maui_background_drawable, kept standalone here.
    [[nodiscard]] local_ref<jobject> maui_border_drawable(JNIEnv* env, jobject host, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID get_background =
            cache.method(env, k_maui_layout_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        if (gradient_class == nullptr || get_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> current{env, env->CallObjectMethod(host, get_background)};
        if (clear_pending(env))
        {
            return {};
        }
        if (current && env->IsInstanceOf(current.get(), gradient_class) == JNI_TRUE)
        {
            return current;
        }
        if (!install)
        {
            return {};
        }
        jmethodID ctor = cache.method(env, k_gradient_drawable_class, "<init>", "()V");
        jmethodID set_background =
            cache.method(env, k_maui_layout_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (ctor == nullptr || set_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> fresh{env, env->NewObject(gradient_class, ctor)};
        if (clear_pending(env) || !fresh)
        {
            return {};
        }
        env->CallVoidMethod(host, set_background, fresh.get());
        if (clear_pending(env))
        {
            return {};
        }
        return fresh;
    }

    // The ordered gradient stop colors as a jint[] (ARGB), for GradientDrawable.setColors(int[]). Mirrors
    // android_visual_ops's stop_color_array: setColors needs ≥ 2 colors, so a single-stop gradient widens to
    // a flat two-color ramp of that color (kept standalone so the border partial stays independently
    // buildable — the two share no TU). An empty ref on any JNI failure.
    [[nodiscard]] local_ref<jintArray> gradient_color_array(JNIEnv* env, const maui::graphics::gradient_paint& gradient)
    {
        const std::vector<maui::graphics::gradient_stop> stops = gradient.get_sorted_stops();
        const auto count = static_cast<jsize>(stops.size() < 2 ? 2 : stops.size());
        local_ref<jintArray> colors{env, env->NewIntArray(count)};
        if (!colors)
        {
            env->ExceptionClear();
            return {};
        }
        std::vector<jint> argb(static_cast<std::size_t>(count));
        if (stops.empty())
        {
            argb.assign(static_cast<std::size_t>(count), 0); // a deliberately-emptied paint → transparent ramp
        }
        else if (stops.size() == 1)
        {
            argb[0] = static_cast<jint>(stops[0].color().to_int());
            argb[1] = argb[0];
        }
        else
        {
            for (std::size_t i = 0; i < stops.size(); ++i)
            {
                argb[i] = static_cast<jint>(stops[i].color().to_int());
            }
        }
        env->SetIntArrayRegion(colors.get(), 0, count, argb.data());
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return {};
        }
        return colors;
    }

    // Paint the border fill onto the maui GradientDrawable. A SolidPaint (or any non-gradient) uses
    // setColor(argb) — the flat fill C#'s BorderHandler.MapBackground draws into the border layer. A
    // gradient paint installs the multi-stop ramp via setColors(int[]) + the orientation/type — the plain
    // GradientDrawable stand-in for MauiDrawable.SetBackground(LinearGradientPaint/RadialGradientPaint),
    // faithful for the common two-plus-stop linear/radial ramp, lossy for angles other than the eight
    // cardinal orientations (the documented no-MauiDrawable gap, same as android_visual_ops). The gradient
    // and stroke live on the SAME drawable, so the dashed/solid stroke set in push_border_to_host is
    // preserved. Returns true when the fill was applied (the caller skips the flat fallback).
    bool apply_border_fill(JNIEnv* env, jobject drawable, const maui::graphics::paint& paint)
    {
        auto& cache = default_jni_cache();
        const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(&paint);
        if (gradient == nullptr)
        {
            jmethodID set_color = cache.method(env, k_gradient_drawable_class, "setColor", "(I)V");
            if (set_color == nullptr)
            {
                return false;
            }
            env->CallVoidMethod(drawable, set_color, static_cast<jint>(paint.background_color().to_int()));
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
                return false;
            }
            return true;
        }

        const local_ref<jintArray> colors = gradient_color_array(env, *gradient);
        if (!colors)
        {
            return false;
        }
        jmethodID set_colors = cache.method(env, k_gradient_drawable_class, "setColors", "([I)V");
        if (set_colors == nullptr)
        {
            return false;
        }
        env->CallVoidMethod(drawable, set_colors, colors.get());
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return false;
        }
        // A radial gradient switches the drawable's gradient type + a representative px radius; a linear
        // gradient keeps the constructed LINEAR type (orientation set below). The orientation for a linear
        // ramp is derived from the start→end line, mapped onto the nearest cardinal Orientation.
        if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(gradient))
        {
            jmethodID set_type = cache.method(env, k_gradient_drawable_class, "setGradientType", "(I)V");
            jmethodID set_radius = cache.method(env, k_gradient_drawable_class, "setGradientRadius", "(F)V");
            if (set_type != nullptr)
            {
                env->CallVoidMethod(drawable, set_type, k_radial_gradient_type);
                env->ExceptionClear();
            }
            if (set_radius != nullptr)
            {
                env->CallVoidMethod(drawable, set_radius,
                                    static_cast<jfloat>(radial->radius()) * k_nominal_radial_extent);
                env->ExceptionClear();
            }
            return true;
        }

        // Linear: pick the cardinal GradientDrawable.Orientation nearest the start→end line (the
        // plain-drawable expression of the gradient angle — full angle support is the MauiDrawable gap).
        const auto* const linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(gradient);
        const char* orientation_name = "TOP_BOTTOM"; // GradientDrawable's own default
        if (linear != nullptr)
        {
            const maui::graphics::point start = linear->start_point();
            const maui::graphics::point end = linear->end_point();
            const double dx = end.x - start.x;
            const double dy = end.y - start.y;
            if (std::abs(dx) >= std::abs(dy))
            {
                orientation_name = dx >= 0 ? "LEFT_RIGHT" : "RIGHT_LEFT";
            }
            else
            {
                orientation_name = dy >= 0 ? "TOP_BOTTOM" : "BOTTOM_TOP";
            }
        }
        jclass orientation_class = cache.find_class(env, k_gradient_orientation_class);
        jmethodID set_orientation = cache.method(env, k_gradient_drawable_class, "setOrientation",
                                                 "(Landroid/graphics/drawable/GradientDrawable$Orientation;)V");
        if (orientation_class == nullptr || set_orientation == nullptr)
        {
            return true; // colors already applied; leave the drawable's default orientation
        }
        jfieldID orientation_field = env->GetStaticFieldID(orientation_class, orientation_name,
                                                           "Landroid/graphics/drawable/GradientDrawable$Orientation;");
        if (orientation_field == nullptr)
        {
            env->ExceptionClear();
            return true;
        }
        const local_ref<jobject> orientation{env, env->GetStaticObjectField(orientation_class, orientation_field)};
        if (env->ExceptionCheck() == JNI_TRUE || !orientation)
        {
            env->ExceptionClear();
            return true;
        }
        env->CallVoidMethod(drawable, set_orientation, orientation.get());
        env->ExceptionClear();
        return true;
    }

    // The border shape's uniform corner radius (dp), recovered off the i_shape bounds path. The
    // StrokeShape corner radius does NOT reach this seam as a value (the union mapper's "stroke_shape"
    // KEY funnels to update_border with no value — header). So the radius is read back out of the
    // geometry RoundRectangle.GetPath builds: PathForBounds lays append_rounded_rectangle(x,y,w,h,
    // tl,tr,bl,br) whose VERY FIRST point is move_to(x, y + tl) — the top-left radius is exactly
    // first_point().y - bounds.y. Exact for the uniform-radius RoundRectangle a Border almost always
    // carries; a conservative top-left approximation for the rare four-distinct case (still a rounded
    // box). Returns {0} when the shape is absent (a Rectangle / no StrokeShape), square-cornered, or its
    // geometry cannot be inspected — in which case the border still strokes correctly with sharp corners
    // (the color/stroke pushes are independent). The exact mirror of box_view_handler.cpp's
    // corner_radii_of, sharing nothing so the two partials stay independently buildable.
    [[nodiscard]] maui::graphics::corner_radius corner_radii_of(const maui::graphics::i_shape* shape)
    {
        if (shape == nullptr)
        {
            return {};
        }
        constexpr float k_ref = 100.0F; // a reference square; the recovered radius is bounds-independent
        const maui::graphics::path_f path = shape->path_for_bounds(maui::graphics::rect{0, 0, k_ref, k_ref});
        if (path.count() == 0)
        {
            return {};
        }
        const maui::graphics::rect_f bounds = path.bounds();
        if (bounds.width <= 0 || bounds.height <= 0)
        {
            return {}; // a collapsed path → no rounding
        }
        // append_rounded_rectangle's first emitted point is (min_x, min_y + tl); recover tl as the
        // vertical inset of that first point from the path's top edge.
        const maui::graphics::point_f first = path.first_point();
        const double tl = static_cast<double>(first.y - bounds.y);
        if (!(tl > 0))
        {
            return {}; // square corners (a Rectangle StrokeShape leaves the border sharp)
        }
        return maui::graphics::corner_radius(tl);
    }

    // The content's native android.view.View, via its view-handler's native_view() (C#'s ToPlatform()
    // = ContainerView ?? PlatformView). Null when the content is unattached. Mirrors
    // content_page_handler.cpp's native_child.
    [[nodiscard]] jobject native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return static_cast<jobject>(handler->native_view());
    }

    // Detach `child` from any ViewGroup parent (removeView), so addView never throws "already has a
    // parent" (the re-parent guard window/layout/content_page handlers share).
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

    // Add `child` to `host` with MATCH_PARENT/MATCH_PARENT layout params so it fills the border host (the
    // border sizes its single child to its content bounds; the content's own absolute platform_arrange
    // then frames it within the padding). The same call content_page_handler.cpp's add_filling_child
    // uses.
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

    // The native side of update_border: push the resolved stroke (color + width) + the shape's uniform
    // corner radius onto the maui GradientDrawable. Installs the drawable only when the border actually
    // carries a visible stroke or a rounded shape (an unstroked, square Border leaves the host's default
    // background — mirroring the button partial's lazy install and the headless "no stroke ⇒ no paint").
    // The generic IView background fill is pushed separately by update_background (so a Border with only a
    // BackgroundColor and no Stroke still fills, via the same lazily-installed drawable).
    void push_border_to_host(maui::core::border_platform& platform)
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
        jobject host = host_of(platform);
        const maui::core::border_stroke_spec& spec = platform.border;
        // GetStrokeProperties with the port's non-nullable color: thickness < 0 → width 0; the radius is
        // recovered from the shape geometry below. A border is "visible" once it strokes or rounds.
        const double thickness = spec.has_stroke && spec.thickness > 0 ? spec.thickness : 0;
        const maui::graphics::corner_radius radius = corner_radii_of(spec.shape);
        const bool rounds = radius.top_left > 0;
        const bool visible = thickness > 0 || rounds;
        const local_ref<jobject> drawable = maui_border_drawable(env.get(), host, /*install=*/visible);
        if (!drawable)
        {
            return; // unstroked + square + no installed drawable → keep the default background (no guess)
        }
        auto& cache = default_jni_cache();
        const float density = display_density(env.get(), host);
        const jint width_px = to_pixels(thickness, density);
        const jint argb = spec.has_stroke ? static_cast<jint>(spec.stroke_color.to_int()) : 0;
        // StrokeExtensions.UpdateStrokeColor/Thickness + MauiDrawable.SetBorderDash → the stroke outline.
        // When StrokeDashArray is set, MauiDrawable.SetBorderDash builds a DashPathEffect whose dash lengths
        // are each strokeDashArray[i] * strokeThickness (px) and whose phase is strokeDashOffset *
        // strokeThickness. The framework GradientDrawable expresses a dash via the 4-arg
        // setStroke(int width, int color, float dashWidth, float dashGap) — a single on/off pair, so the
        // port scales the first two dash entries by the px thickness (the same DashArray→px convention as
        // the C# oracle) and routes to that overload; longer/odd patterns collapse to that first pair (the
        // documented GradientDrawable lossiness — a canvas DashPathEffect would carry the full array).
        // A solid (empty-dash) border keeps the 2-arg setStroke, exactly matching C#'s null PathEffect.
        const std::vector<float>& dash = spec.dash_pattern;
        const bool dashed = thickness > 0 && dash.size() >= 2 && (dash[0] > 0 || dash[1] > 0);
        if (dashed)
        {
            jmethodID set_stroke_dashed = cache.method(env.get(), k_gradient_drawable_class, "setStroke", "(IIFF)V");
            if (set_stroke_dashed != nullptr)
            {
                // MauiDrawable.SetBorderDash: strokeDash[i] = strokeDashArray[i] * strokeThickness (px).
                const auto stroke_px = static_cast<float>(to_pixels(thickness, density));
                const auto dash_width = dash[0] * stroke_px;
                const auto dash_gap = dash[1] * stroke_px;
                env->CallVoidMethod(drawable.get(), set_stroke_dashed, width_px, argb, static_cast<jfloat>(dash_width),
                                    static_cast<jfloat>(dash_gap));
                clear_pending(env.get());
            }
        }
        else
        {
            // A zero-thickness/no-stroke border leaves a 0-width (invisible) solid stroke, matching C#.
            jmethodID set_stroke = cache.method(env.get(), k_gradient_drawable_class, "setStroke", "(II)V");
            if (set_stroke != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_stroke, width_px, argb);
                clear_pending(env.get());
            }
        }
        // The StrokeShape corner radius (uniform) → setCornerRadius(px); a 0 radius keeps it sharp.
        jmethodID set_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "setCornerRadius", "(F)V");
        if (set_corner_radius != nullptr)
        {
            env->CallVoidMethod(drawable.get(), set_corner_radius,
                                static_cast<jfloat>(to_pixels(radius.top_left, density)));
            clear_pending(env.get());
        }
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the dev.mauicpp.MauiLayout host (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView host here).
    border_platform::~border_platform()
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
    // VM-less cross-platform suite — then pushes to the real MauiLayout when one exists. Same override
    // set + dual-path the content_page android partial established (visibility/opacity/automation_id push
    // directly; transform/flow_direction/background/semantics push through the shared android ops; shadow/
    // clip/input_transparent keep ONLY the base mirror — no plain-ViewGroup analog, see below). ----

    void border_platform::update_visibility(maui::core::visibility value)
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
        // ViewExtensions.ToPlatformVisibility: Visible/Hidden/Collapsed → View.VISIBLE/INVISIBLE/GONE.
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

    void border_platform::update_opacity(double value)
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
            call_void_float(env.get(), host_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void border_platform::update_automation_id(std::string_view value)
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
        jobject host = host_of(*this);
        auto& cache = default_jni_cache();
        // PlatformInterop.setContentDescriptionForAutomationId: setting a ContentDescription flips
        // ImportantForAccessibility to YES; restore AUTO when that is what the host had.
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

    void border_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void border_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    // C#'s BorderHandler MapBackground draws the background INTO the border layer (clipped to the shape).
    // The port's GradientDrawable already clips the fill to its own corner radius, so the IView.Background
    // push lands on that same drawable (the one update_border installs for the stroke). A SolidPaint fills
    // with setColor(argb); a gradient paint fills with the multi-stop ramp (setColors + orientation/type)
    // — the plain-drawable stand-in for MauiDrawable.SetBackground(gradient). A null background leaves the
    // stroke drawable's fill alone (no clobber).
    void border_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        if (native == nullptr || value == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*this);
        // Install the maui GradientDrawable so the fill clips to the border's corner radius (rather than
        // the generic apply_background painting a flat rectangle behind the shape), matching the C#
        // background-into-the-border-layer behavior.
        const local_ref<jobject> drawable = maui_border_drawable(env.get(), host, /*install=*/true);
        if (!drawable)
        {
            return;
        }
        apply_border_fill(env.get(), drawable.get(), *value);
    }

    void border_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // Shadow / Clip / InputTransparent keep ONLY the base mirror on Android: C#'s ViewExtensions applies
    // them on a WrapperView, so an unwrapped plain ViewGroup host receives no shadow/clip/input-transparent
    // update in C# either (there is no apply_shadow / apply_input_transparent android op — the same scope
    // the content_page/button/layout partials document). No overrides are declared for them in the android
    // block of border_handler.hpp; the base view_platform_base bodies run. update_clip additionally must
    // not run because the border SHAPE owns the corner geometry on the GradientDrawable (apple block note).

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        // BorderHandler.CreatePlatformView → new ContentViewGroup(Context); the port's dev.mauicpp.MauiLayout
        // (the no-op-onLayout ViewGroup content_page hosts into; the GradientDrawable carries the border —
        // header deviations). The MauiLayout ctor is theme-independent, so it constructs in the bare
        // app_process testhost.
        jclass layout_class = cache.find_class(env.get(), k_maui_layout_class);
        jmethodID ctor = cache.method(env.get(), k_maui_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform; // MauiLayout is host-provided (java/MauiLayout.java)
        }
        const local_ref<jobject> host{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !host)
        {
            return platform;
        }
        // Wrap-content LayoutParams up front (the button/content_page rationale: a parentless host with
        // null LayoutParams trips relayout paths; the android container fan-out supplies them in C#).
        jclass layout_params_class = cache.find_class(env.get(), k_layout_params_class);
        jmethodID layout_params_ctor = cache.method(env.get(), k_layout_params_class, "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_maui_layout_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_wrap_content, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(host.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        platform->native = env->NewGlobalRef(host.get()); // released in ~border_platform
        return platform;
    }

    void border_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C#'s UpdateContent reads VirtualView.PresentedContent).
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
        // C# UpdateContent: handler.PlatformView.RemoveAllViews(), then AddView(content.ToPlatform()).
        // The same swap content_page_handler.cpp / window_handler.cpp do.
        jmethodID remove_all = default_jni_cache().method(env.get(), k_view_group_class, "removeAllViews", "()V");
        if (remove_all != nullptr)
        {
            env->CallVoidMethod(host, remove_all);
            clear_pending(env.get());
        }
        if (platform->hosted_content == nullptr)
        {
            return; // an empty border (the previous child was just removed)
        }
        if (jobject child = native_child(*platform->hosted_content))
        {
            add_filling_child(env.get(), host, child);
        }
    }

    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // Mirror the full resolved stroke surface (C# UpdateMauiCALayer pushes the same set of values),
        // then push the stroke + corner radius onto the host's GradientDrawable (the JNI half).
        platform->border = make_border_stroke_spec(*virtual_view());
        push_border_to_host(*platform);
    }

    void border_handler::arrange_native(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the host measures Exactly at the
        // final size (Android requires a measure pass before layout) and lays out. MauiLayout.onLayout is
        // a no-op so the single content child keeps the absolute frame its own platform_arrange set.
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

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
//
// THE CONTENT CLIP — MAUI's ½·strokeThickness gap (dispatchDraw CHILD-clip, NOT the ViewOutlineProvider):
// MAUI does NOT inset the content by the stroke thickness (content flush against the stroke). Its
// ContentViewGroup.GetClipPath insets the CONTENT CLIP by strokeThickness on every side and then
// ShapeExtensions.ToPlatform(innerPath:true) insets a FURTHER strokeThickness/2 — the non-IRoundRectangle
// branch yields Rect(1.5·st, 1.5·st, W−3·st, H−3·st), the IRoundRectangle branch nets an inset-by-st box with
// each corner radius reduced by st (RoundRectangle.InnerPathForBounds). Meanwhile the stroke, drawn by
// MauiDrawable/BorderDrawable, stays FULL width — so a ½·st band of the Border fill / page shows THROUGH
// between the stroke's inner edge (at st) and the content clip (at 1.5·st). border_content_inner_path_points
// reproduces that inner path exactly (in points); native_border_clip_path scales it to pixels.
//
// Android CANNOT express this with the ViewOutlineProvider + setClipToOutline the earlier convex-shape fix
// used: an Outline clip clips the BACKGROUND drawable TOGETHER with the children, so pulling the content
// inside the stroke would clip the stroke to the same inner edge (thinning the GradientDrawable stroke to
// half width). Only a canvas.clipPath INSIDE MauiLayout.dispatchDraw — applied AFTER the drawable background
// is drawn — clips the CHILDREN alone, leaving the GradientDrawable stroke full width. So the dispatchDraw
// Phase-2 children-clip that was built for arbitrary/canvas StrokeShapes (nativeBorderClipPath / borderPeer)
// is now REPURPOSED for the convex GradientDrawable path too: arrange_native installs the borderPeer for ANY
// shaped Border (not just canvas ones), and native_draw_border_fill / native_draw_border_stroke early-out for
// convex shapes (shape_needs_canvas == false) so the GradientDrawable keeps painting the stroke + fill while
// dispatchDraw drives ONLY the child clip (no double draw). The ViewOutlineProvider is thereby freed for the
// colored elevation shadow (clipToOutline stays OFF), so a shadowed Border now gets BOTH its shadow AND the
// content clip — resolving the old one-Outline-slot shadow-vs-clip deviation for GradientDrawable-shaped
// borders. Cites ContentViewGroup.GetClipPath + ShapeExtensions.ToPlatform(innerPath:true) + BorderDrawable.

#include "maui/core/border_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "android_canvas.hpp"
#include "android_clip_ops.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
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
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace
{
    using maui::platform::android::android_canvas;
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // The no-op-onLayout ViewGroup hosting the content child (the same host content_page_handler.cpp
    // uses) — also the surface carrying the border's GradientDrawable background. GetMethodID walks
    // superclasses, so the ViewGroup/View surface resolves through this class directly.
    constexpr const char* k_maui_layout_class = "dev/mauicpp/MauiLayout";
    constexpr const char* k_view_group_class = "android/view/ViewGroup";
    constexpr const char* k_layout_params_class = "android/view/ViewGroup$LayoutParams";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";
    constexpr const char* k_gradient_orientation_class = "android/graphics/drawable/GradientDrawable$Orientation";
    // The wrapper carrying the StrokeShape's own 0.5 DIP/side self-inset on the GradientDrawable route —
    // see border_drawable_self_inset_px + apply_border_drawable_inset below.
    constexpr const char* k_inset_drawable_class = "android/graphics/drawable/InsetDrawable";
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
    //
    // A STROKED border's drawable is wrapped in an InsetDrawable (apply_border_drawable_inset below), so
    // the background is EITHER the bare GradientDrawable or that wrapper — unwrap it, since every setter
    // below (setStroke / setCornerRadius / setColor) must reach the GradientDrawable itself.
    [[nodiscard]] local_ref<jobject> maui_border_drawable(JNIEnv* env, jobject host, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jclass inset_class = cache.find_class(env, k_inset_drawable_class);
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
        if (current && inset_class != nullptr && env->IsInstanceOf(current.get(), inset_class) == JNI_TRUE)
        {
            // DrawableWrapper.getDrawable() (API 23; minSdk is 24) — the wrapped GradientDrawable.
            jmethodID get_drawable =
                cache.method(env, k_inset_drawable_class, "getDrawable", "()Landroid/graphics/drawable/Drawable;");
            if (get_drawable == nullptr)
            {
                return {};
            }
            local_ref<jobject> wrapped{env, env->CallObjectMethod(current.get(), get_drawable)};
            if (clear_pending(env) || !wrapped)
            {
                return {};
            }
            return env->IsInstanceOf(wrapped.get(), gradient_class) == JNI_TRUE ? std::move(wrapped)
                                                                                : local_ref<jobject>{};
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

    // The StrokeShape's own 0.5 DIP/side self-inset, in whole PIXELS, for the GradientDrawable route.
    //
    // WHY THIS EXISTS SEPARATELY FROM border_shape_path_points. maui::core::shape_self_inset is already
    // applied on the CANVAS route (border_shape_path_points), but shape_needs_canvas() sends every
    // round-rect / rectangle / ellipse StrokeShape — i.e. every Border on the parity board — down the
    // GradientDrawable background instead, which that helper never touches. 4e93abf373 ("the shape
    // self-inset was missing on iOS/Catalyst/AppKit/Android too") therefore landed on a path the common
    // case does not take; the fix was INCOMPLETE on Android, not regressed (shape_needs_canvas predates
    // it: 9ec9a14240 / bdfaddea5a). A GradientDrawable draws its stroke centred on its bounds deflated by
    // strokeWidth/2, so its OUTER edge is flush with the view box; MAUI's MauiDrawable traces a path that
    // is deflated by the same strokeWidth/2 AND by the Shape's own 0.5 (Shape.PathForBounds with the
    // Controls Shape default StrokeThickness 1.0). Insetting the whole drawable by 0.5 DIP reproduces
    // that, and carries the fill + the corner-radius origin with it exactly as MAUI's single path does.
    //
    // MEASURED on the android board columns (density 2.75, 5 DIP stroke): MAUI's `border` box spans
    // 279.0 DIP outer edge to outer edge for a WidthRequest of 280 with the stroke centreline 3.0 DIP
    // (= 2.5 + 0.5) inside the box, where the port spanned the full 280.0 with the centreline at 2.5;
    // and on `border_stroke` MAUI leaves a 2 px white gap between vertically adjacent Grid Borders where
    // the port had them abutting (a merged 17 px red run vs MAUI's 3 + gap + 14).
    //
    // QUANTIZATION (a documented residual, not a derivation): Drawable bounds are integer px, so the
    // 1.375 px this is at density 2.75 rounds to 1 — 0.36 DIP instead of 0.5, leaving ~0.375 px/side.
    // ROUND, not the ContextExtensions.ToPixels CEIL `to_pixels` uses elsewhere: MAUI never converts this
    // inset to px at all (MauiDrawable strokes a float canvas path), so there is no ToPixels convention to
    // mirror here, and rounding is simply the nearest reachable geometry. Exactness on this route would
    // require abandoning the GradientDrawable for the canvas draw, which would also drop its dash,
    // gradient-fill and Outline-shadow support — see the header's deviation list.
    [[nodiscard]] jint border_drawable_self_inset_px(const maui::core::border_stroke_spec& spec, double thickness,
                                                     float density)
    {
        if (!spec.shape_self_insets)
        {
            return 0; // the Frame facade's synthesized StrokeShape — see i_border_stroke::shape_self_insets
        }
        if (thickness <= 0.0)
        {
            return 0; // Border.UpdateStrokeShape latched the shape's own thickness to 0 — shape_self_inset's gate
        }
        const maui::graphics::rect inset = maui::core::shape_self_inset(maui::graphics::rect{0.0, 0.0, 0.0, 0.0}, 1.0);
        return static_cast<jint>(std::lround(inset.x * static_cast<double>(density)));
    }

    // Ensure the host's background carries `inset_px` on every side, wrapping/unwrapping the maui
    // GradientDrawable in an InsetDrawable as needed. There is only ONE non-zero inset value (the constant
    // above), so "is it wrapped?" fully determines the current inset — no getPadding round-trip needed.
    // InsetDrawable's insets are fixed at construction before API 29, hence the re-install on a flip; the
    // flip only happens when a Border gains or loses its stroke, so it is not a per-push cost.
    void apply_border_drawable_inset(JNIEnv* env, jobject host, jobject drawable, jint inset_px)
    {
        auto& cache = default_jni_cache();
        jclass inset_class = cache.find_class(env, k_inset_drawable_class);
        jmethodID get_background =
            cache.method(env, k_maui_layout_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        jmethodID set_background =
            cache.method(env, k_maui_layout_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (inset_class == nullptr || get_background == nullptr || set_background == nullptr)
        {
            return;
        }
        const local_ref<jobject> current{env, env->CallObjectMethod(host, get_background)};
        if (clear_pending(env))
        {
            return;
        }
        const bool wrapped = current && env->IsInstanceOf(current.get(), inset_class) == JNI_TRUE;
        if (wrapped == (inset_px > 0))
        {
            return; // already in the wanted shape
        }
        if (inset_px <= 0)
        {
            env->CallVoidMethod(host, set_background, drawable); // unwrap: the bare GradientDrawable
            clear_pending(env);
            return;
        }
        jmethodID ctor =
            cache.method(env, k_inset_drawable_class, "<init>", "(Landroid/graphics/drawable/Drawable;IIII)V");
        if (ctor == nullptr)
        {
            return;
        }
        const local_ref<jobject> wrapper{
            env, env->NewObject(inset_class, ctor, drawable, inset_px, inset_px, inset_px, inset_px)};
        if (clear_pending(env) || !wrapper)
        {
            return;
        }
        env->CallVoidMethod(host, set_background, wrapper.get());
        clear_pending(env);
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

    // The border shape's FULL four per-corner radii (dp) — the per-corner counterpart to corner_radii_of.
    // A RoundRectangle StrokeShape can carry four DISTINCT corner radii (CornerRadius(TL,TR,BL,BR), e.g.
    // border_clip_playground's TL=60/TR=0/BL=0/BR=12); GradientDrawable.setCornerRadius is uniform-only, so
    // the four values are needed for setCornerRadii(float[8]) (below) and for the per-corner CONTENT clip
    // path (apply_outline_clip resolves the shape geometry itself, but the STROKE drawable must be told the
    // four radii explicitly). The port owns the round_rectangle shape the Border carries, so the exact four
    // values are read straight off it (dynamic_cast) — no geometry-recovery imprecision. For any OTHER
    // i_shape (a future custom RoundRectangleGeometry, or a shape the cast misses) it degrades to the
    // uniform top-left recovery (corner_radii_of), which stays exact for a uniform-radius shape and a
    // conservative rounded box otherwise — never a crash, never a wrong SHAPE kind. Returns {0} (square)
    // when the shape is absent / a Rectangle / an Ellipse (Ellipse rounds via the clip path, not the
    // rounded-rect drawable radii — its corner_radii are 0 so the drawable stays a sharp rect under the
    // ellipse clip, matching how a sharp GradientDrawable sits behind the outline clip).
    [[nodiscard]] maui::graphics::corner_radius corner_radii_all_of(const maui::graphics::i_shape* shape)
    {
        if (const auto* rr = dynamic_cast<const maui::graphics::shapes::round_rectangle*>(shape))
        {
            return rr->corner_radius();
        }
        // Non-round_rectangle shape: fall back to the uniform top-left recovery (a Rectangle/Ellipse yields
        // {0}, a uniform RoundRectangleGeometry yields its uniform radius on all four corners).
        return corner_radii_of(shape);
    }

    // Whether the four per-corner radii are all equal (within a sub-pixel epsilon). A uniform border keeps
    // the setCornerRadius(float) path (which the android_border headless tests read back via
    // getCornerRadius()); only a genuinely per-corner-distinct radius set switches to setCornerRadii.
    [[nodiscard]] bool corner_radii_are_uniform(const maui::graphics::corner_radius& r)
    {
        constexpr double k_eps = 0.01;
        return std::abs(r.top_left - r.top_right) < k_eps && std::abs(r.top_left - r.bottom_left) < k_eps &&
               std::abs(r.top_left - r.bottom_right) < k_eps;
    }

    // Whether the Border's StrokeShape needs the CANVAS draw (dispatchDraw + android_canvas) rather than
    // the GradientDrawable + Outline-clip fast path. The GradientDrawable expresses a rounded rect (incl. a
    // plain rectangle = 0 radius) and, with the outline clip, an ellipse — the convex shapes the last fix
    // handled. ANY other StrokeShape (a Polygon triangle, a PathGeometry, a Line) cannot be traced by a
    // rectangular drawable, so it routes to the canvas path (the port twin of MAUI's MauiDrawable canvas
    // draw). A null shape (a plain Border) is NOT arbitrary — it keeps the default background. The check is
    // by exclusion: a shape that is neither a round_rectangle nor an ellipse (the two the GradientDrawable +
    // clip already cover faithfully) is arbitrary. round_rectangle covers the plain-rectangle case too (a
    // rectangle StrokeShape is a round_rectangle with 0 radius on the port's shape hierarchy) — but the
    // graphics::shapes::rectangle primitive is a distinct type, so it is excluded explicitly as well.
    [[nodiscard]] bool shape_needs_canvas(const maui::graphics::i_shape* shape)
    {
        if (shape == nullptr)
        {
            return false; // no StrokeShape → default background, no canvas draw
        }
        if (dynamic_cast<const maui::graphics::shapes::round_rectangle*>(shape) != nullptr ||
            dynamic_cast<const maui::graphics::shapes::ellipse*>(shape) != nullptr ||
            dynamic_cast<const maui::graphics::shapes::rectangle*>(shape) != nullptr)
        {
            return false; // GradientDrawable + Outline clip already handle these convex shapes
        }
        return true; // Polygon / Path / Line / any other geometry → canvas draw
    }

    // The Border's shape path fitted to the STROKE bounds, in POINTS — the fill + stroke both trace this.
    // Mirrors MauiDrawable.Android.cs's shape bounds: the path is laid into Rect(sw/2, sw/2, fw-sw, fh-sw)
    // so a stroke of width sw centred on the path has its OUTER edge flush with the view boundary (fw/fh are
    // the view size in points; sw is the stroke thickness). An unstroked border fits the full box.
    [[nodiscard]] maui::graphics::path_f border_shape_path_points(const maui::core::border_stroke_spec& spec,
                                                                  double width_pt, double height_pt)
    {
        if (spec.shape == nullptr || width_pt <= 0.0 || height_pt <= 0.0)
        {
            return {};
        }
        const double sw = spec.has_stroke && spec.thickness > 0 ? spec.thickness : 0.0;
        const double x = sw / 2.0;
        const double y = sw / 2.0;
        const double w = width_pt - sw;
        const double h = height_pt - sw;
        if (w <= 0.0 || h <= 0.0)
        {
            return {};
        }
        // ...and then the default StrokeShape's OWN 0.5 pt/side self-inset on top: MauiDrawable.
        // UpdateClipPath hands those bounds to _shape.ToPlatform(...), whose non-innerPath branch is a bare
        // shape.PathForBounds (ShapeExtensions.cs:34) — so the Controls Rectangle's own StrokeThickness 1.0
        // deflates them again. Derivation + the `> 0` latch: shape_self_inset (core/border_handler.hpp).
        // Gated on `sw`, not on spec.thickness: this file already treats "thickness set but no Stroke brush"
        // as unstroked everywhere (MauiDrawable would not), so the inset follows the same convention rather
        // than introducing a second, inconsistent one.
        return spec.shape->path_for_bounds(maui::core::shape_self_inset(maui::graphics::rect{x, y, w, h}, sw));
    }

    // The Border CONTENT clip path (in POINTS, over the host's laid-out size) — the ONE mirror of C#'s
    // ContentViewGroup.GetClipPath + ShapeExtensions.ToPlatform(innerPath:true), driving BOTH the convex
    // GradientDrawable path (via dispatchDraw's Phase-2 children-clip, repurposed — see the header) and the
    // arbitrary canvas path. MAUI insets the CONTENT CLIP by 1.5·strokeThickness while the stroke stays full
    // width, so a ½·st band of the Border fill / page shows through between the stroke's inner edge (at st)
    // and this clip (at 1.5·st). Let st = the resolved stroke thickness (0 when unstroked). Per shape:
    //   • round_rectangle → RoundRectangle.InnerPathForBounds: an inset-by-st box (st, st, W−2st, H−2st) with
    //     each corner radius reduced by st (max(0, r−st)); the Fill-aspect transform nets the inset-by-st box.
    //   • ellipse → the non-IRoundRectangle ToPlatform branch: an ellipse in Rect(1.5st, 1.5st, W−3st, H−3st).
    //   • plain rectangle / polygon / path / any other → the same non-IRoundRectangle branch:
    //     shape->path_for_bounds(Rect(1.5st, 1.5st, W−3st, H−3st)); a bare rectangle is a rect inset 1.5·st
    //     (the border_stroke gap). Empty when the shape is null or the stroke swallows the box (W−3st ≤ 0).
    [[nodiscard]] maui::graphics::path_f border_content_inner_path_points(const maui::core::border_stroke_spec& spec,
                                                                          double width_pt, double height_pt)
    {
        maui::graphics::path_f path;
        const maui::graphics::i_shape* shape = spec.shape;
        if (shape == nullptr || width_pt <= 0.0 || height_pt <= 0.0)
        {
            return path;
        }
        const double st = spec.has_stroke && spec.thickness > 0 ? spec.thickness : 0.0;
        // ToPlatform's non-IRoundRectangle inset: bounds shrink by 1.5·st on every side (GetClipPath's st plus
        // ToPlatform's st/2). The IRoundRectangle branch nets an inset-by-st box, but a stroke wider than a
        // third of the box swallows even the round-rect content, so the uniform W−3st ≤ 0 guard bails on all.
        const double inner_w3 = width_pt - (3.0 * st);
        const double inner_h3 = height_pt - (3.0 * st);
        if (inner_w3 <= 0.0 || inner_h3 <= 0.0)
        {
            return path; // the stroke swallows the box → nothing to clip
        }
        if (dynamic_cast<const maui::graphics::shapes::round_rectangle*>(shape) != nullptr)
        {
            // RoundRectangle.InnerPathForBounds: inset by st on every side; each corner radius reduced by st.
            const maui::graphics::corner_radius r = corner_radii_all_of(shape);
            const auto reduce = [st](double radius) { return static_cast<float>(std::max(0.0, radius - st)); };
            const auto x = static_cast<float>(st);
            const auto y = static_cast<float>(st);
            const auto w = static_cast<float>(width_pt - (2.0 * st));
            const auto h = static_cast<float>(height_pt - (2.0 * st));
            path.append_rounded_rectangle(x, y, w, h, reduce(r.top_left), reduce(r.top_right), reduce(r.bottom_left),
                                          reduce(r.bottom_right));
            return path;
        }
        // The non-IRoundRectangle ToPlatform branch: pathBounds = Rect(1.5st, 1.5st, W−3st, H−3st) — and
        // then shape.PathForBounds(pathBounds) (ShapeExtensions.cs:34), so the default StrokeShape's own
        // 0.5 pt/side self-inset applies here too (shape_self_inset, core/border_handler.hpp). The
        // IRoundRectangle branch above does NOT get it: InnerPathForBounds is called directly, never
        // through PathForBounds. Measured on border_stroke: MAUI's orange content edge sits +0.50 pt
        // inward of the port's at StrokeThickness 1 and 5, the same constant as the stroke itself.
        const maui::graphics::rect inner =
            maui::core::shape_self_inset(maui::graphics::rect{1.5 * st, 1.5 * st, inner_w3, inner_h3}, st);
        if (dynamic_cast<const maui::graphics::shapes::ellipse*>(shape) != nullptr)
        {
            path.append_ellipse(static_cast<float>(inner.x), static_cast<float>(inner.y),
                                static_cast<float>(inner.width), static_cast<float>(inner.height));
            return path;
        }
        // A plain Rectangle / Polygon / Path / any other geometry: fit the shape path to the inset inner box.
        return shape->path_for_bounds(inner);
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
        // ARBITRARY StrokeShape (a Polygon triangle / a Path): the GradientDrawable can only draw a rounded
        // RECTANGLE, so it would paint a competing box behind the canvas-drawn triangle. Clear our drawable
        // background (setBackground(null)) — the MauiLayout.dispatchDraw canvas draws the fill + stroke along
        // the shape path instead (arrange_native installs the borderPeer). Nothing else to push here.
        if (shape_needs_canvas(spec.shape))
        {
            if (jmethodID set_background = default_jni_cache().method(env.get(), k_maui_layout_class, "setBackground",
                                                                      "(Landroid/graphics/drawable/Drawable;)V"))
            {
                // Only null OUR GradientDrawable, never a foreign background (guard with the same instanceof
                // maui_border_drawable uses): getBackground()==GradientDrawable ⇒ ours ⇒ clear it.
                const local_ref<jobject> ours = maui_border_drawable(env.get(), host, /*install=*/false);
                if (ours)
                {
                    env->CallVoidMethod(host, set_background, static_cast<jobject>(nullptr));
                    clear_pending(env.get());
                }
            }
            return;
        }
        // GetStrokeProperties with the port's non-nullable color: thickness < 0 → width 0; the radius is
        // recovered from the shape geometry below. A border is "visible" once it strokes or rounds.
        const double thickness = spec.has_stroke && spec.thickness > 0 ? spec.thickness : 0;
        // THE FILL INSET (MauiDrawable.UpdateClipPath): MAUI lays the border path into
        // Rect(sw/2, sw/2, fw-sw, fh-sw) — i.e. it insets the FILL by strokeThickness/2 on every side —
        // using StrokeThickness whether or not a Stroke BRUSH is set, and Border.StrokeThickness DEFAULTS
        // to 1 (Border.cs). GradientDrawable applies exactly the same half-stroke inset to its draw rect
        // whenever a stroke WIDTH is set, so the raw StrokeThickness is pushed as the width (with a
        // TRANSPARENT color when there is no brush, `argb` below) to reproduce MAUI's geometry. Using the
        // brush-gated `thickness` here instead left the fill covering the FULL bounds: varied_size_selector's
        // Wheat cells painted their whole 100dp pitch where MAUI leaves a ~4px gap between consecutive cells.
        const double geometry_thickness = spec.thickness > 0 ? spec.thickness : 0;
        // Recover the FULL per-corner radii (border_clip_playground carries TL=60/TR=0/BL=0/BR=12) — the
        // stroke drawable is told each corner explicitly (setCornerRadii) so the rounded rect matches the
        // per-corner CONTENT clip apply_outline_clip installs from the same shape (arrange_native).
        const maui::graphics::corner_radius radius = corner_radii_all_of(spec.shape);
        const bool rounds =
            radius.top_left > 0 || radius.top_right > 0 || radius.bottom_left > 0 || radius.bottom_right > 0;
        const bool visible = thickness > 0 || rounds;
        const local_ref<jobject> drawable = maui_border_drawable(env.get(), host, /*install=*/visible);
        if (!drawable)
        {
            return; // unstroked + square + no installed drawable → keep the default background (no guess)
        }
        auto& cache = default_jni_cache();
        const float density = display_density(env.get(), host);
        // The StrokeShape's own 0.5 DIP/side self-inset — the GradientDrawable-route twin of the
        // shape_self_inset border_shape_path_points takes on the canvas route (derivation + the measured
        // evidence: border_drawable_self_inset_px). Gated on the brush-gated `thickness`, the same
        // convention border_shape_path_points uses, so a "thickness set but no Stroke brush" border is
        // treated as unstroked here too.
        apply_border_drawable_inset(env.get(), host, drawable.get(),
                                    border_drawable_self_inset_px(spec, thickness, density));
        const jint width_px = to_pixels(geometry_thickness, density);
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
        // The StrokeShape corner radius → the GradientDrawable's rounded-rect corners.
        //   - UNIFORM (all four equal, incl. the common single-radius RoundRectangle and a sharp 0): keep
        //     setCornerRadius(px). The android_border headless tests read this back via getCornerRadius().
        //   - PER-CORNER DISTINCT (border_clip_playground's TL=60/TR=0/BL=0/BR=12): setCornerRadii(float[8]).
        //     The float[8] order is [TLx,TLy, TRx,TRy, BRx,BRy, BLx,BLy] — the same X==Y-per-corner circular
        //     radii MAUI's MauiDrawable builds for its per-corner stroke path (Android's addRoundRect radii
        //     array order). NOTE the C# CornerRadius / the port's corner_radius order is TL,TR,BL,BR, but the
        //     drawable's array order is TL,TR,BR,BL — BR and BL are swapped when packing.
        if (corner_radii_are_uniform(radius))
        {
            jmethodID set_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "setCornerRadius", "(F)V");
            if (set_corner_radius != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_corner_radius,
                                    static_cast<jfloat>(to_pixels(radius.top_left, density)));
                clear_pending(env.get());
            }
        }
        else
        {
            jmethodID set_corner_radii = cache.method(env.get(), k_gradient_drawable_class, "setCornerRadii", "([F)V");
            if (set_corner_radii != nullptr)
            {
                const auto tl = static_cast<jfloat>(to_pixels(radius.top_left, density));
                const auto tr = static_cast<jfloat>(to_pixels(radius.top_right, density));
                const auto br = static_cast<jfloat>(to_pixels(radius.bottom_right, density));
                const auto bl = static_cast<jfloat>(to_pixels(radius.bottom_left, density));
                // [TLx,TLy, TRx,TRy, BRx,BRy, BLx,BLy] — circular corners, so X==Y per corner.
                const std::array<jfloat, 8> radii{tl, tl, tr, tr, br, br, bl, bl};
                const local_ref<jfloatArray> radii_array{env.get(), env->NewFloatArray(8)};
                if (radii_array)
                {
                    env->SetFloatArrayRegion(radii_array.get(), 0, 8, radii.data());
                    env->CallVoidMethod(drawable.get(), set_corner_radii, radii_array.get());
                    clear_pending(env.get());
                }
            }
        }
    }

    // ── ARBITRARY-STROKESHAPE canvas draw (MauiLayout.dispatchDraw callbacks) ────────────────────────────
    // The port twin of MAUI's Android Border render for a StrokeShape the GradientDrawable cannot trace (a
    // Polygon triangle / a Path): MauiDrawable draws the fill + stroke ALONG the shape path on the canvas,
    // and ContentViewGroup.dispatchDraw clips the content to the shape path. These three JNI callbacks are
    // bound onto MauiLayout (RegisterNatives) and invoked from its dispatchDraw: fill behind the children,
    // clip the children, stroke over them. The peer is the border_platform*; it is cleared to 0 in
    // ~border_platform before the struct dies, so a late draw is a safe no-op.

    // The border host's display density from the canvas draw thread (memoized via display_density, keyed off
    // the host View). The dispatchDraw canvas is in PIXELS; the shape geometry + stroke width are in POINTS,
    // so the android_canvas applies canvas.scale(density) and every op below stays in point units.
    [[nodiscard]] float border_density(JNIEnv* env, jobject host)
    {
        return display_density(env, host);
    }

    // Phase 1 — fill the arbitrary shape path with the Border's background paint, behind the children.
    void JNICALL native_draw_border_fill(JNIEnv* env, jobject host, jlong peer, jobject canvas, jint width, jint height)
    {
        auto* platform = reinterpret_cast<maui::core::border_platform*>(peer);
        if (platform == nullptr || canvas == nullptr || env == nullptr || width <= 0 || height <= 0)
        {
            return;
        }
        // Convex shapes (rectangle / round-rect / ellipse) keep the GradientDrawable for their fill + stroke;
        // the borderPeer only drives the dispatchDraw CHILD-clip for them (header). Draw the canvas fill ONLY
        // for a shape the GradientDrawable cannot trace, so a convex border is not double-filled.
        if (!shape_needs_canvas(platform->border.shape))
        {
            return;
        }
        if (platform->background == nullptr)
        {
            return; // an unfilled border draws no fill (the stroke still traces the outline)
        }
        const float density = border_density(env, host);
        const float scale = density > 0 ? density : 1.0F;
        const auto w_pt = static_cast<double>(width) / scale;
        const auto h_pt = static_cast<double>(height) / scale;
        const maui::graphics::path_f path = border_shape_path_points(platform->border, w_pt, h_pt);
        if (path.count() == 0)
        {
            return;
        }
        android_canvas bridge(env, canvas);
        bridge.set_display_scale(scale);
        // The fill honours a solid OR gradient paint (set_fill_paint installs the shader for a gradient),
        // resolved over the path's bounds. winding non_zero matches a simple closed polygon/path fill.
        bridge.set_fill_paint(platform->background,
                              maui::graphics::rect_f(0.0F, 0.0F, static_cast<float>(w_pt), static_cast<float>(h_pt)));
        bridge.fill_path(path, maui::graphics::winding_mode::non_zero);
    }

    // Build the content clip Path (the 1.5·st-inset inner shape, in PIXELS) for MauiLayout.dispatchDraw to
    // clipPath the CHILDREN (Phase 2), for BOTH convex and arbitrary shapes — the ½·st gap MAUI's
    // ContentViewGroup.GetClipPath reveals (header). Returns null (→ no clip) when there is no shape or the
    // stroke swallows the box.
    jobject JNICALL native_border_clip_path(JNIEnv* env, jobject host, jlong peer, jint width, jint height)
    {
        auto* platform = reinterpret_cast<maui::core::border_platform*>(peer);
        if (platform == nullptr || env == nullptr || width <= 0 || height <= 0)
        {
            return nullptr;
        }
        const float density = border_density(env, host);
        const float scale = density > 0 ? density : 1.0F;
        const auto w_pt = static_cast<double>(width) / scale;
        const auto h_pt = static_cast<double>(height) / scale;
        const maui::graphics::path_f path = border_content_inner_path_points(platform->border, w_pt, h_pt);
        if (path.count() == 0)
        {
            return nullptr;
        }
        // Reuse the shared clip-path builder (points → android.graphics.Path scaled to pixels), returning a
        // LOCAL ref that JNI hands back to Java (Java owns it after return; do not release it here).
        local_ref<jobject> path_obj = maui::platform::android::detail::build_clip_path(env, path, scale);
        return path_obj.release();
    }

    // Phase 3 — stroke the arbitrary shape outline with the Border's stroke paint, over the children.
    void JNICALL native_draw_border_stroke(JNIEnv* env, jobject host, jlong peer, jobject canvas, jint width,
                                           jint height)
    {
        auto* platform = reinterpret_cast<maui::core::border_platform*>(peer);
        if (platform == nullptr || canvas == nullptr || env == nullptr || width <= 0 || height <= 0)
        {
            return;
        }
        const maui::core::border_stroke_spec& spec = platform->border;
        // Convex shapes stroke via the GradientDrawable (full width, behind the children); the borderPeer only
        // drives the dispatchDraw child-clip for them (header). Trace the canvas stroke ONLY for a shape the
        // GradientDrawable cannot express, so a convex border is not double-stroked.
        if (!shape_needs_canvas(spec.shape))
        {
            return;
        }
        if (!spec.has_stroke || spec.thickness <= 0)
        {
            return; // no stroke → nothing to trace
        }
        const float density = border_density(env, host);
        const float scale = density > 0 ? density : 1.0F;
        const auto w_pt = static_cast<double>(width) / scale;
        const auto h_pt = static_cast<double>(height) / scale;
        const maui::graphics::path_f path = border_shape_path_points(spec, w_pt, h_pt);
        if (path.count() == 0)
        {
            return;
        }
        android_canvas bridge(env, canvas);
        bridge.set_display_scale(scale);
        bridge.set_stroke_color(spec.stroke_color);
        bridge.set_stroke_size(static_cast<float>(spec.thickness));
        bridge.set_stroke_line_cap(spec.line_cap);
        bridge.set_stroke_line_join(spec.line_join);
        bridge.set_miter_limit(spec.miter_limit > 0 ? spec.miter_limit : 10.0F);
        if (spec.dash_pattern.size() >= 2)
        {
            bridge.set_stroke_dash_pattern(spec.dash_pattern);
            bridge.set_stroke_dash_offset(spec.dash_offset);
        }
        bridge.draw_path(path);
    }

    // Binds the three arbitrary-border draw callbacks onto MauiLayout (RegisterNatives — no Java_* export,
    // the same reflection-free recipe layout_handler's nativeArrange + box_view's nativeDraw use). Idempotent
    // (RegisterNatives replaces an existing binding), so it is safe to call on every border host creation.
    [[nodiscard]] bool register_border_draw_natives(JNIEnv* env)
    {
        jclass layout_class = default_jni_cache().find_class(env, k_maui_layout_class);
        if (layout_class == nullptr)
        {
            return false;
        }
        static const std::array<JNINativeMethod, 3> k_methods{
            JNINativeMethod{.name = const_cast<char*>("nativeDrawBorderFill"),
                            .signature = const_cast<char*>("(JLandroid/graphics/Canvas;II)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_draw_border_fill)},
            JNINativeMethod{.name = const_cast<char*>("nativeBorderClipPath"),
                            .signature = const_cast<char*>("(JII)Landroid/graphics/Path;"),
                            .fnPtr = reinterpret_cast<void*>(&native_border_clip_path)},
            JNINativeMethod{.name = const_cast<char*>("nativeDrawBorderStroke"),
                            .signature = const_cast<char*>("(JLandroid/graphics/Canvas;II)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_draw_border_stroke)},
        };
        const jint status = env->RegisterNatives(layout_class, k_methods.data(), static_cast<jint>(k_methods.size()));
        if (status != JNI_OK)
        {
            clear_pending(env);
            return false;
        }
        return true;
    }

    // Install (or clear, peer=0) the MauiLayout's borderPeer so its dispatchDraw runs the arbitrary-shape
    // canvas draw. call_void_long — the setBorderPeer(long) push.
    void set_border_peer(JNIEnv* env, jobject host, jlong peer)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, "setBorderPeer", "(J)V"))
        {
            env->CallVoidMethod(host, method, peer);
            clear_pending(env);
        }
    }

    // Invalidate the border host so its dispatchDraw re-runs (a shape/stroke/fill change — the MauiShapeView
    // invalidate twin). A plain View.invalidate().
    void invalidate_host(JNIEnv* env, jobject host)
    {
        if (jmethodID method = default_jni_cache().method(env, k_maui_layout_class, "invalidate", "()V"))
        {
            env->CallVoidMethod(host, method);
            clear_pending(env);
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
                // Clear the borderPeer FIRST so a late MauiLayout.dispatchDraw cannot dereference this
                // struct mid-teardown (the MauiShapeView.setNativePtr(0) doctrine).
                if (jmethodID clear_peer =
                        default_jni_cache().method(env.get(), k_maui_layout_class, "setBorderPeer", "(J)V"))
                {
                    env->CallVoidMethod(static_cast<jobject>(native), clear_peer, static_cast<jlong>(0));
                    if (env->ExceptionCheck() == JNI_TRUE)
                    {
                        env->ExceptionClear();
                    }
                }
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
        // ARBITRARY StrokeShape: the fill is drawn along the shape path by MauiLayout.dispatchDraw (which
        // reads this->background — already stored by the base body above), NOT by a rectangular
        // GradientDrawable. Skip the drawable install and just invalidate so the canvas re-fills.
        if (shape_needs_canvas(border.shape))
        {
            if (jmethodID inval = default_jni_cache().method(env.get(), k_maui_layout_class, "invalidate", "()V"))
            {
                env->CallVoidMethod(host, inval);
                clear_pending(env.get());
            }
            return;
        }
        // Install the maui GradientDrawable so the fill clips to the border's corner radius (rather than
        // the generic apply_background painting a flat rectangle behind the shape), matching the C#
        // background-into-the-border-layer behavior.
        const local_ref<jobject> drawable = maui_border_drawable(env.get(), host, /*install=*/true);
        if (!drawable)
        {
            return;
        }
        apply_border_fill(env.get(), drawable.get(), *value);
        // Re-push the stroke geometry now that the drawable EXISTS. push_border_to_host only ever mutates an
        // ALREADY-INSTALLED drawable when the border is not itself "visible" (no stroke brush, square corners),
        // and the stroke map runs BEFORE this background map — so for a Border whose only drawable trigger is
        // its Background (the common `<Border BackgroundColor=...>` with no Stroke), the stroke width never
        // landed and the fill covered the FULL bounds. MAUI insets the fill by strokeThickness/2 even with no
        // stroke brush (MauiDrawable.UpdateClipPath; Border.StrokeThickness defaults to 1), which is what
        // leaves the ~4px gap between varied_size_selector's stacked Wheat cells. Idempotent.
        push_border_to_host(*this);
    }

    void border_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // IView.Shadow on the border host → the native colored elevation shadow (android_visual_ops apply_shadow),
    // shaped by the border's corner radius so the glow follows the rounded box. Base mirror FIRST, then the
    // widget push. The host is 0×0 at map time (apply_shadow clears the elevation then); arrange_native
    // re-invokes apply_shadow at the live size + the recovered corner radius (the outline is bounds-dependent,
    // like update_border's stroke path). The corner radius is not on the shadow, so update_shadow uses 0 here
    // and arrange_native supplies the real radius via corner_radii_of(virtual_view()->shape()).
    void border_platform::update_shadow(const maui::core::i_shadow* value)
    {
        view_platform_base::update_shadow(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject host = host_of(*this);
        const float density = display_density(env.get(), host);
        const double width = maui::platform::android::detail::view_width_dp(env.get(), host, density);
        const double height = maui::platform::android::detail::view_height_dp(env.get(), host, density);
        maui::platform::android::apply_shadow(native, value, density, width, height, 0.0);
    }

    // Clip / InputTransparent keep ONLY the base mirror on Android: C#'s ViewExtensions applies them on a
    // WrapperView, so an unwrapped plain ViewGroup host receives no clip/input-transparent update in C#
    // either. No overrides are declared for them in the android block of border_handler.hpp; the base
    // view_platform_base bodies run. update_clip additionally must not run because the border SHAPE owns the
    // corner geometry on the GradientDrawable (apple block note).

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
        // Bind the arbitrary-StrokeShape draw callbacks onto MauiLayout (RegisterNatives, idempotent) before
        // any border host draws — the reflection-free recipe layout_handler/box_view share. A bind failure
        // (a stripped-down host without the border natives) degrades to no canvas draw, never a crash.
        static_cast<void>(register_border_draw_natives(env.get()));
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

        // ARBITRARY StrokeShape (a Polygon triangle / a Path): the GradientDrawable + Outline clip cannot
        // trace it, so the border draws through MauiLayout.dispatchDraw (fill + clip + stroke on the canvas —
        // the port twin of MAUI's MauiDrawable canvas draw + ContentViewGroup clip). Install the borderPeer
        // (the dispatchDraw activation) and clear any stale Outline clip (the dispatchDraw clipPath does the
        // clipping now). push_border_to_host already suppressed the GradientDrawable for an arbitrary shape,
        // so no rectangle chrome competes. A colored shadow is still expressed via the Outline (it wins the
        // one slot, same deviation as below); without a shadow the Outline clip is cleared here.
        if (shape_needs_canvas(platform->border.shape))
        {
            set_border_peer(env.get(), host, reinterpret_cast<jlong>(platform));
            if (platform->shadow != nullptr)
            {
                const maui::graphics::corner_radius radius = corner_radii_of(platform->border.shape);
                maui::platform::android::apply_shadow(host, platform->shadow, density, frame.width, frame.height,
                                                      radius.top_left);
            }
            else
            {
                // Clear any Outline clip so it does not contend with the dispatchDraw clipPath (an arbitrary
                // shape has no convex Outline clip anyway; pass an empty path → apply_outline_clip_path clears).
                maui::platform::android::apply_outline_clip_path(host, maui::graphics::path_f{}, density, frame.width,
                                                                 frame.height);
            }
            invalidate_host(env.get(), host); // redraw with the new size/shape
            return;
        }

        // A GradientDrawable-expressible shape (rounded rect / ellipse / plain rect): the GradientDrawable
        // (push_border_to_host) paints the stroke + fill FULL width; the CONTENT clip moves to
        // MauiLayout.dispatchDraw's Phase-2 CHILD-clip so the content is inset 1.5·st WITHOUT thinning the
        // stroke — the ½·st gap MAUI's ContentViewGroup reveals (header). So install the borderPeer for ANY
        // shaped Border (native_border_clip_path then runs; native_draw_border_fill/stroke early-out for these
        // convex shapes, so dispatchDraw drives ONLY the clip). A null shape (no StrokeShape at all) needs no
        // clip → leave the plain ViewGroup default (peer 0).
        if (platform->border.shape != nullptr)
        {
            set_border_peer(env.get(), host, reinterpret_cast<jlong>(platform));
        }
        else
        {
            set_border_peer(env.get(), host, 0);
        }

        // The ViewOutlineProvider is now FREE of the content clip (dispatchDraw owns it), so it carries ONLY
        // the colored elevation shadow — a shadowed Border keeps BOTH its shadow AND the dispatchDraw content
        // clip at once (resolving the old one-Outline-slot shadow-vs-clip deviation for these shapes). Re-
        // resolve the bounds-dependent shadow silhouette against the just-laid-out size + the recovered corner
        // radius (update_shadow ran before layout when the host was 0×0). With no shadow, tear down any stale
        // Outline clip a previous run installed (an empty path routes apply_outline_clip_path → clear), so
        // clipToOutline is OFF and the GradientDrawable stroke is never eaten by a lingering outline clip.
        if (platform->shadow != nullptr)
        {
            const maui::graphics::corner_radius radius = corner_radii_of(platform->border.shape);
            maui::platform::android::apply_shadow(host, platform->shadow, density, frame.width, frame.height,
                                                  radius.top_left);
        }
        else
        {
            maui::platform::android::apply_outline_clip_path(host, maui::graphics::path_f{}, density, frame.width,
                                                             frame.height);
        }
        invalidate_host(env.get(), host); // redraw so the dispatchDraw child-clip takes effect at the new size
    }
} // namespace maui::core

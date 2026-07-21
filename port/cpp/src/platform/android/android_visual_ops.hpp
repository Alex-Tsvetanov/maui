#pragma once
// Shared Android (JNI) operations for the generic-IView background — the platform side of the shared
// view_mapper's map_background (view_mapper.cpp). The JNI twin of apple_visual_ops.hpp's apply_background.
// Include only from the android partials. VM-less safe (acquires a scoped_env and returns when no
// JavaVM / no widget exists — the headless-mirror degradation the android partials document).
//
// apply_background ports Microsoft.Maui.Platform.ViewExtensions.UpdateBackground (Android,
// src/Core/src/Platform/Android/ViewExtensions.cs):
//   - a SolidPaint sets View.setBackgroundColor(color.ToPlatform()) — the ARGB int (color.to_int() is
//     already 0xAARRGGBB, the platform Color int);
//   - a gradient paint installs an android.graphics.drawable.GradientDrawable as the View background
//     (the plain-View analog of Paint.ToDrawable -> MauiDrawable). The drawable carries the ordered
//     stop colors (setColors(int[])) and, for a linear gradient, the orientation derived from the
//     start/end points; a radial gradient is set to RADIAL_GRADIENT with a representative gradient
//     radius. (C# routes gradients through MauiDrawable, which renders the same color ramp; the plain
//     GradientDrawable is the no-MauiDrawable stand-in — see the DEVIATIONS note.)
//   - a null paint clears OUR background drawable to null when one is installed (LayoutViewGroup /
//     ContentViewGroup clear to null in C#); a SolidPaint with no widget keeps the headless mirror.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each an infrastructure gap, not a behavior guess):
//   - C#'s UpdateBackground builds a MauiDrawable (a custom Drawable rendering MAUI's full paint model,
//     incl. multi-stop gradients with arbitrary geometry and the IBorderStroke layering). This APK-less
//     backend has no MauiDrawable; the gradient path uses the framework GradientDrawable, which models
//     a two-or-more-stop linear/radial ramp — faithful for the common gradient, lossy for paint features
//     GradientDrawable cannot express (sweep gradients, per-stop offsets other than even spacing).
//   - Clip has NO plain-android.view.View analog for an ARBITRARY (non-convex) shape: C#'s
//     ViewExtensions.UpdateClip sets WrapperView.Clip, a MAUI container that intercepts draw to apply an
//     arbitrary CAShapeLayer-style mask. The convex-shape subset is expressed via a ViewOutlineProvider +
//     setClipToOutline (android_clip_ops.hpp apply_outline_clip); the arbitrary-path case keeps the base
//     mirror (deferred — needs a WrapperView ViewGroup overriding dispatchDraw to canvas.clipPath()).
//   - Shadow (IView.Shadow) is expressed NATIVELY via apply_shadow below (setElevation + a rounded-rect
//     ViewOutlineProvider caster + setOutlineSpotShadowColor / setOutlineAmbientShadowColor, API 28+),
//     which produces the COLORED elevation drop shadow the shadow gallery pages expect. This is a faithful
//     APPROXIMATION of C#'s software WrapperView shadow (PlatformWrapperView.java draws a blurred, tinted,
//     OFFSET alpha-mask copy beneath the content): the color, a bounds+corner shape, and a
//     radius-proportional size/blur are reproduced; the arbitrary Shadow.Offset(x,y) DIRECTION and the
//     exact Gaussian blur falloff are NOT expressible via native elevation (documented on apply_shadow /
//     MauiShadowOutlineProvider.java). Before this the android partials kept ONLY the headless mirror for
//     shadow (the "needs PlatformWrapperView" deferral) — the API-28 colored-elevation path removed that
//     deferral for the common solid-color drop shadow.

#include <jni.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::platform::android
{
    namespace detail
    {
        inline constexpr const char* k_visual_view_class = "android/view/View";
        inline constexpr const char* k_gradient_drawable = "android/graphics/drawable/GradientDrawable";
        inline constexpr const char* k_gradient_orientation = "android/graphics/drawable/GradientDrawable$Orientation";

        // GradientDrawable.GradientType.RADIAL_GRADIENT (LINEAR_GRADIENT is the constructed default = 0).
        inline constexpr jint k_radial_gradient_type = 1;

        // GradientDrawable.Orientation enum-name for an L→R linear ramp; the port maps every linear
        // gradient onto LEFT_RIGHT (GradientDrawable orientation is one of eight cardinal directions —
        // the closest plain-drawable expression of the start→end line; full angle support is the
        // MauiDrawable gap above).
        inline constexpr const char* k_orientation_left_right = "LEFT_RIGHT";

        // Pull the stop colors as a freshly-allocated jint[] (ARGB ints) for setColors(int[]).
        [[nodiscard]] inline local_ref<jintArray> stop_color_array(JNIEnv* env,
                                                                   const maui::graphics::gradient_paint& gradient)
        {
            const std::vector<maui::graphics::gradient_stop> stops = gradient.get_sorted_stops();
            // GradientDrawable.setColors requires at least two colors; a single-stop gradient is widened
            // to a flat two-color ramp of that one color (the visible result a one-stop gradient yields).
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
                // No stops: a transparent ramp (GradientPaint defaults to white-to-white, so this only
                // hits a deliberately-emptied paint — render it transparent rather than crash).
                argb.assign(static_cast<std::size_t>(count), 0);
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
    } // namespace detail

    inline void apply_background(void* native, const maui::graphics::paint* p)
    {
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto* const view = static_cast<jobject>(native);
        auto& cache = default_jni_cache();

        // A null paint clears the background (the LayoutViewGroup / ContentViewGroup branch).
        if (p == nullptr)
        {
            jmethodID set_background = cache.method(env.get(), detail::k_visual_view_class, "setBackground",
                                                    "(Landroid/graphics/drawable/Drawable;)V");
            if (set_background != nullptr)
            {
                env->CallVoidMethod(view, set_background, static_cast<jobject>(nullptr));
                env->ExceptionClear();
            }
            return;
        }

        // A SolidPaint → setBackgroundColor(ARGB int).
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(p))
        {
            jmethodID set_background_color =
                cache.method(env.get(), detail::k_visual_view_class, "setBackgroundColor", "(I)V");
            if (set_background_color != nullptr)
            {
                env->CallVoidMethod(view, set_background_color, static_cast<jint>(solid->color().to_int()));
                env->ExceptionClear();
            }
            return;
        }

        // A gradient → a GradientDrawable installed as the background (the no-MauiDrawable stand-in).
        const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(p);
        if (gradient == nullptr)
        {
            return; // an unmodeled paint kind: leave the existing background (no guess)
        }

        const local_ref<jintArray> colors = detail::stop_color_array(env.get(), *gradient);
        if (!colors)
        {
            return;
        }

        // new GradientDrawable(Orientation.LEFT_RIGHT, int[] colors) — the orientation/colors ctor.
        // Orientation.LEFT_RIGHT is a STATIC enum field (the jni_cache::field helper resolves INSTANCE
        // fields via GetFieldID), so read the static field id directly via GetStaticFieldID.
        jclass orientation_class = cache.find_class(env.get(), detail::k_gradient_orientation);
        jmethodID drawable_ctor = cache.method(env.get(), detail::k_gradient_drawable, "<init>",
                                               "(Landroid/graphics/drawable/GradientDrawable$Orientation;[I)V");
        jclass drawable_class = cache.find_class(env.get(), detail::k_gradient_drawable);
        if (orientation_class == nullptr || drawable_ctor == nullptr || drawable_class == nullptr)
        {
            return;
        }
        jfieldID left_right = env->GetStaticFieldID(orientation_class, detail::k_orientation_left_right,
                                                    "Landroid/graphics/drawable/GradientDrawable$Orientation;");
        if (left_right == nullptr)
        {
            env->ExceptionClear(); // NoSuchFieldError -> leave the existing background (no guess)
            return;
        }
        const local_ref<jobject> orientation{env.get(), env->GetStaticObjectField(orientation_class, left_right)};
        if (env->ExceptionCheck() == JNI_TRUE || !orientation)
        {
            env->ExceptionClear();
            return;
        }
        const local_ref<jobject> drawable{
            env.get(), env->NewObject(drawable_class, drawable_ctor, orientation.get(), colors.get())};
        if (env->ExceptionCheck() == JNI_TRUE || !drawable)
        {
            env->ExceptionClear();
            return;
        }

        // A radial gradient switches the drawable's gradient type + sets a representative radius (the
        // GradientDrawable radius is in px; the paint radius is relative 0..1, so a non-zero radius maps
        // to a positive gradient radius — the plain-drawable expression of a radial ramp).
        if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(gradient))
        {
            jmethodID set_type = cache.method(env.get(), detail::k_gradient_drawable, "setGradientType", "(I)V");
            jmethodID set_radius = cache.method(env.get(), detail::k_gradient_drawable, "setGradientRadius", "(F)V");
            if (set_type != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_type, detail::k_radial_gradient_type);
                env->ExceptionClear();
            }
            if (set_radius != nullptr)
            {
                // A relative radius scaled to a nominal px extent (GradientDrawable has no "relative"
                // mode without a bounds pass; a positive value yields a visible radial ramp).
                constexpr float k_nominal_radial_extent = 100.0F;
                env->CallVoidMethod(drawable.get(), set_radius,
                                    static_cast<jfloat>(radial->radius()) * k_nominal_radial_extent);
                env->ExceptionClear();
            }
        }

        jmethodID set_background = cache.method(env.get(), detail::k_visual_view_class, "setBackground",
                                                "(Landroid/graphics/drawable/Drawable;)V");
        if (set_background != nullptr)
        {
            env->CallVoidMethod(view, set_background, drawable.get());
            env->ExceptionClear();
        }
    }

    // Clear any background TINT list (View.setBackgroundTintList(null)). The editor/entry handlers seed a gray
    // underline tint on their default EditText chrome at create time; when an EXPLICIT Background paint is then
    // applied via apply_background, that tint would recolor the explicit background (e.g. a green Entry
    // rendered gray — visual_states/clip_views regression). Call this right after apply_background so the
    // explicit background shows in its own color. No-op / best-effort when the tint or view is absent.
    inline void clear_background_tint(void* native)
    {
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_tint = cache.method(env.get(), detail::k_visual_view_class, "setBackgroundTintList",
                                          "(Landroid/content/res/ColorStateList;)V");
        if (set_tint != nullptr)
        {
            env->CallVoidMethod(static_cast<jobject>(native), set_tint, static_cast<jobject>(nullptr));
            env->ExceptionClear();
        }
    }

    namespace detail
    {
        inline constexpr const char* k_shadow_provider_class = "dev/mauicpp/MauiShadowOutlineProvider";

        // The default Android spot/ambient elevation-shadow tint: opaque black (0xFF000000). apply_shadow
        // restores this when a shadow is cleared so a shadowless view (or one whose shadow is removed) casts
        // no colored glow — and, once elevation is zeroed, no shadow at all. Mirrors the button
        // elevation-strip idiom (setElevation(0) leaves the view flat).
        inline constexpr jint k_default_shadow_tint = static_cast<jint>(0xFF000000);

        // Map Shadow.Radius (a blur radius, dp) + Offset magnitude to a native elevation Z (px). Native
        // elevation is not the same unit as a Gaussian blur radius, so this is a proportional mapping tuned
        // so the gallery's default radius (10) reads as a clearly-visible soft shadow: Z ≈ radius, nudged up
        // by a fraction of the offset magnitude (a larger requested offset reads as "further"/larger since
        // the native shadow has no per-view X/Y offset — see MauiShadowOutlineProvider.java). Clamped to a
        // small floor so even a radius-0 solid shadow still casts a visible glow (matching MAUI, whose
        // software shadow is visible at radius 0), and to a ceiling so an extreme radius does not blow the
        // shadow off-screen.
        [[nodiscard]] inline jfloat shadow_elevation_px(double radius, double offset_magnitude, float density)
        {
            constexpr double k_min_elevation_dp = 4.0;
            constexpr double k_max_elevation_dp = 48.0;
            constexpr double k_offset_weight = 0.25;
            const double base_dp = std::max(radius, 0.0) + (k_offset_weight * offset_magnitude);
            const double clamped_dp = std::clamp(base_dp, k_min_elevation_dp, k_max_elevation_dp);
            return static_cast<jfloat>(clamped_dp * static_cast<double>(density));
        }
    } // namespace detail

    // apply_shadow — the NATIVE-elevation expression of IView.Shadow (ViewHandler.MapShadow →
    // ViewExtensions.UpdateShadow), the android twin of apple_visual_ops.hpp apply_shadow (which sets the
    // CALayer shadow*). C# on Android draws the shadow in software (PlatformWrapperView); with no WrapperView
    // this backend uses the framework's own elevation drop shadow, tinted to Shadow.Color × Opacity via the
    // API-28 spot/ambient shadow-color setters, cast from a rounded-rect outline matching the view bounds +
    // corner radius. width/height/corner_radius are in POINTS (dp) — the view's CURRENT laid-out size; the
    // caller re-invokes from platform_arrange after layout because the outline is bounds-dependent (the view
    // is 0×0 at map time). VM-less safe (returns when no JavaVM / no widget exists — the headless mirror).
    //
    // Only a SolidPaint shadow is colorized (matching C#, which only honors SolidPaint on every platform and
    // treats gradient shadows as sugar); a gradient-paint shadow falls back to the paint's representative
    // background_color() so it still casts a visible colored glow. A null shadow / null paint clears the
    // shadow (elevation 0, default tint, background outline provider) — mirroring the button elevation strip.
    inline void apply_shadow(void* native, const maui::core::i_shadow* s, float density, double width, double height,
                             double corner_radius)
    {
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto* const view = static_cast<jobject>(native);
        auto& cache = default_jni_cache();

        jmethodID set_elevation = cache.method(env.get(), detail::k_visual_view_class, "setElevation", "(F)V");
        jmethodID set_provider = cache.method(env.get(), detail::k_visual_view_class, "setOutlineProvider",
                                              "(Landroid/view/ViewOutlineProvider;)V");
        jmethodID set_clip_to_outline =
            cache.method(env.get(), detail::k_visual_view_class, "setClipToOutline", "(Z)V");
        jmethodID invalidate_outline = cache.method(env.get(), detail::k_visual_view_class, "invalidateOutline", "()V");
        // API 28+ colored-shadow setters (the emulator + compileSdk are API 34). Absent on <28: the shadow
        // still casts (gray Material), just untinted — graceful degradation, no crash.
        jmethodID set_spot_color =
            cache.method(env.get(), detail::k_visual_view_class, "setOutlineSpotShadowColor", "(I)V");
        jmethodID set_ambient_color =
            cache.method(env.get(), detail::k_visual_view_class, "setOutlineAmbientShadowColor", "(I)V");
        if (set_elevation == nullptr || set_provider == nullptr)
        {
            return;
        }

        const maui::graphics::paint* const paint = (s != nullptr) ? s->paint() : nullptr;

        // ---- clear: null shadow / null paint / not-yet-laid-out view → flat, default tint ----
        if (paint == nullptr || width <= 0.0 || height <= 0.0)
        {
            env->CallVoidMethod(view, set_elevation, static_cast<jfloat>(0.0));
            env->ExceptionClear();
            if (set_spot_color != nullptr)
            {
                env->CallVoidMethod(view, set_spot_color, detail::k_default_shadow_tint);
                env->ExceptionClear();
            }
            if (set_ambient_color != nullptr)
            {
                env->CallVoidMethod(view, set_ambient_color, detail::k_default_shadow_tint);
                env->ExceptionClear();
            }
            // Restore the framework BACKGROUND outline provider (tracks the background drawable so a view
            // that legitimately elevates later still shadows correctly), leaving clipToOutline off.
            jclass provider_base = cache.find_class(env.get(), "android/view/ViewOutlineProvider");
            jfieldID background_field =
                provider_base != nullptr
                    ? env->GetStaticFieldID(provider_base, "BACKGROUND", "Landroid/view/ViewOutlineProvider;")
                    : nullptr;
            if (background_field == nullptr)
            {
                env->ExceptionClear();
            }
            else
            {
                const local_ref<jobject> background{env.get(),
                                                    env->GetStaticObjectField(provider_base, background_field)};
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    env->ExceptionClear();
                }
                else
                {
                    env->CallVoidMethod(view, set_provider, background.get());
                    env->ExceptionClear();
                }
            }
            if (set_clip_to_outline != nullptr)
            {
                env->CallVoidMethod(view, set_clip_to_outline, static_cast<jboolean>(false));
                env->ExceptionClear();
            }
            if (invalidate_outline != nullptr)
            {
                env->CallVoidMethod(view, invalidate_outline);
                env->ExceptionClear();
            }
            return;
        }

        // ---- install: a colored native elevation shadow shaped by a rounded-rect outline ----
        // Tint = Shadow.Color combined with Opacity, exactly as C# WrapperView.ShadowChanged:
        // color.WithAlpha(color.Alpha * Opacity). Only SolidPaint is colorized; any other paint uses its
        // representative background_color() (so a gradient shadow still glows in a sensible color).
        maui::graphics::color shadow_color = paint->background_color();
        const auto opacity = static_cast<float>(std::clamp(s->opacity(), 0.0, 1.0));
        shadow_color = shadow_color.with_alpha(shadow_color.alpha * opacity);
        const jint tint = static_cast<jint>(shadow_color.to_int());

        const maui::graphics::point offset = s->offset();
        const double offset_magnitude = std::hypot(offset.x, offset.y);
        const jfloat elevation = detail::shadow_elevation_px(s->radius(), offset_magnitude, density);

        // The rounded-rect caster outline in PIXELS (the view's live bounds + corner radius × density).
        const auto width_px = static_cast<jint>(std::lround(width * static_cast<double>(density)));
        const auto height_px = static_cast<jint>(std::lround(height * static_cast<double>(density)));
        const auto corner_px = static_cast<jfloat>(std::max(corner_radius, 0.0) * static_cast<double>(density));

        jclass provider_class = cache.find_class(env.get(), detail::k_shadow_provider_class);
        jmethodID provider_ctor = provider_class != nullptr
                                      ? cache.method(env.get(), detail::k_shadow_provider_class, "<init>", "(IIF)V")
                                      : nullptr;
        if (provider_class == nullptr || provider_ctor == nullptr)
        {
            return; // MauiShadowOutlineProvider is host-provided (java/MauiShadowOutlineProvider.java)
        }
        const local_ref<jobject> provider{
            env.get(), env->NewObject(provider_class, provider_ctor, width_px, height_px, corner_px)};
        if (env->ExceptionCheck() == JNI_TRUE || !provider)
        {
            env->ExceptionClear();
            return;
        }

        // Tint FIRST (API 28+), then shape + elevate. The outline shapes the shadow only — clipToOutline stays
        // OFF so the content is never clipped (a shadow needs the silhouette, not a content clip).
        if (set_spot_color != nullptr)
        {
            env->CallVoidMethod(view, set_spot_color, tint);
            env->ExceptionClear();
        }
        if (set_ambient_color != nullptr)
        {
            env->CallVoidMethod(view, set_ambient_color, tint);
            env->ExceptionClear();
        }
        env->CallVoidMethod(view, set_provider, provider.get());
        env->ExceptionClear();
        if (set_clip_to_outline != nullptr)
        {
            env->CallVoidMethod(view, set_clip_to_outline, static_cast<jboolean>(false));
            env->ExceptionClear();
        }
        env->CallVoidMethod(view, set_elevation, elevation);
        env->ExceptionClear();
        if (invalidate_outline != nullptr)
        {
            env->CallVoidMethod(view, invalidate_outline);
            env->ExceptionClear();
        }
    }
} // namespace maui::platform::android

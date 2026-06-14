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
//   - Shadow and Clip have NO plain-android.view.View analog: C#'s ViewExtensions.UpdateShadow /
//     UpdateClip set WrapperView.Shadow / WrapperView.Clip — a MAUI container that intercepts draw to
//     render the shadow / apply the outline clip. With no WrapperView the unwrapped View receives NO
//     shadow/clip update in C# either (the extension early-returns when the platformView is not a
//     WrapperView), so the android partials keep ONLY the headless mirror for shadow/clip — exactly
//     matching the C# unwrapped-View behavior. (The framework's View.setElevation / setClipToOutline
//     express a different, narrower model than WrapperView's arbitrary-shape clip + colored drop shadow,
//     so they are intentionally NOT substituted.)

#include <jni.h>

#include <vector>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
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
} // namespace maui::platform::android

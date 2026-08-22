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

#include "jni/app_context.hpp"
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
#include "maui/graphics/system_background_paint.hpp"

namespace maui::platform::android
{
    namespace detail
    {
        inline constexpr const char* k_visual_view_class = "android/view/View";
        inline constexpr const char* k_gradient_drawable = "android/graphics/drawable/GradientDrawable";
        inline constexpr const char* k_gradient_orientation = "android/graphics/drawable/GradientDrawable$Orientation";

        // Whether the app is in dark (night) mode, from the process Context's resource Configuration:
        // (Configuration.uiMode & UI_MODE_NIGHT_MASK) == UI_MODE_NIGHT_YES. Used to resolve
        // system_background_paint (MAUI's UIColor.systemBackground twin) to MAUI's dark surface. The capture
        // pipeline sets the emulator to night mode for dark shots, so this tracks the shot's theme.
        [[nodiscard]] inline bool is_night_mode(JNIEnv* env)
        {
            jobject context = app_context();
            if (env == nullptr || context == nullptr)
            {
                return false;
            }
            auto& cache = default_jni_cache();
            jmethodID get_resources =
                cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
            jmethodID get_config = cache.method(env, "android/content/res/Resources", "getConfiguration",
                                                "()Landroid/content/res/Configuration;");
            if (get_resources == nullptr || get_config == nullptr)
            {
                return false;
            }
            const local_ref<jobject> res{env, env->CallObjectMethod(context, get_resources)};
            if (env->ExceptionCheck() == JNI_TRUE || !res)
            {
                env->ExceptionClear();
                return false;
            }
            const local_ref<jobject> config{env, env->CallObjectMethod(res.get(), get_config)};
            if (env->ExceptionCheck() == JNI_TRUE || !config)
            {
                env->ExceptionClear();
                return false;
            }
            jfieldID ui_mode_field = cache.field(env, "android/content/res/Configuration", "uiMode", "I");
            if (ui_mode_field == nullptr)
            {
                return false;
            }
            constexpr jint k_ui_mode_night_mask = 0x30;
            constexpr jint k_ui_mode_night_yes = 0x20;
            const jint ui_mode = env->GetIntField(config.get(), ui_mode_field);
            env->ExceptionClear();
            return (ui_mode & k_ui_mode_night_mask) == k_ui_mode_night_yes;
        }

        // MAUI's Android system background: white in light, #121212 (Material dark surface, measured off the
        // shipped MAUI render) in dark — the resolution of system_background_paint's dynamic color.
        [[nodiscard]] inline jint system_background_argb(JNIEnv* env)
        {
            return is_night_mode(env) ? static_cast<jint>(0xFF121212U) : static_cast<jint>(0xFFFFFFFFU);
        }

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

        // Build the framework GradientDrawable that expresses a gradient_paint (the no-MauiDrawable
        // stand-in): the ordered stop colors as an L→R linear ramp, switched to RADIAL_GRADIENT with a
        // representative radius for a radial_gradient_paint. Returns an EMPTY ref on any JNI miss (the
        // caller then leaves the existing background — no guess). Factored VERBATIM out of apply_background
        // so its behavior is unchanged; apply_field_background reuses it for a gradient FILL layer.
        [[nodiscard]] inline local_ref<jobject> make_gradient_drawable(JNIEnv* env,
                                                                       const maui::graphics::gradient_paint& gradient)
        {
            auto& cache = default_jni_cache();
            const local_ref<jintArray> colors = stop_color_array(env, gradient);
            if (!colors)
            {
                return {};
            }
            // new GradientDrawable(Orientation.LEFT_RIGHT, int[] colors) — the orientation/colors ctor.
            // Orientation.LEFT_RIGHT is a STATIC enum field, read via GetStaticFieldID directly.
            jclass orientation_class = cache.find_class(env, k_gradient_orientation);
            jmethodID drawable_ctor = cache.method(env, k_gradient_drawable, "<init>",
                                                   "(Landroid/graphics/drawable/GradientDrawable$Orientation;[I)V");
            jclass drawable_class = cache.find_class(env, k_gradient_drawable);
            if (orientation_class == nullptr || drawable_ctor == nullptr || drawable_class == nullptr)
            {
                return {};
            }
            jfieldID left_right = env->GetStaticFieldID(orientation_class, k_orientation_left_right,
                                                        "Landroid/graphics/drawable/GradientDrawable$Orientation;");
            if (left_right == nullptr)
            {
                env->ExceptionClear(); // NoSuchFieldError -> leave the existing background (no guess)
                return {};
            }
            const local_ref<jobject> orientation{env, env->GetStaticObjectField(orientation_class, left_right)};
            if (env->ExceptionCheck() == JNI_TRUE || !orientation)
            {
                env->ExceptionClear();
                return {};
            }
            local_ref<jobject> drawable{env,
                                        env->NewObject(drawable_class, drawable_ctor, orientation.get(), colors.get())};
            if (env->ExceptionCheck() == JNI_TRUE || !drawable)
            {
                env->ExceptionClear();
                return {};
            }

            // A radial gradient switches the drawable's gradient type + sets a representative radius (the
            // GradientDrawable radius is in px; the paint radius is relative 0..1, so a non-zero radius maps
            // to a positive gradient radius — the plain-drawable expression of a radial ramp).
            if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(&gradient))
            {
                jmethodID set_type = cache.method(env, k_gradient_drawable, "setGradientType", "(I)V");
                jmethodID set_radius = cache.method(env, k_gradient_drawable, "setGradientRadius", "(F)V");
                if (set_type != nullptr)
                {
                    env->CallVoidMethod(drawable.get(), set_type, k_radial_gradient_type);
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
            return drawable;
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

        // system_background_paint (MAUI's UIColor.systemBackground twin — the default page/frame fill when
        // BackgroundColor is null) resolves DYNAMICALLY to the theme surface: white in light, MAUI's #121212
        // in dark. The base solid_paint carries only the static white fallback, so resolve it here from the
        // Context's night mode (the Android analog of the iOS trait-collection resolution in ios_visual_ops).
        // Checked BEFORE the plain solid_paint branch since it derives from solid_paint.
        if (dynamic_cast<const maui::graphics::system_background_paint*>(p) != nullptr)
        {
            jmethodID set_background_color =
                cache.method(env.get(), detail::k_visual_view_class, "setBackgroundColor", "(I)V");
            if (set_background_color != nullptr)
            {
                env->CallVoidMethod(view, set_background_color, detail::system_background_argb(env.get()));
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

        // Build the framework GradientDrawable (the ordered color ramp; radial switches type + radius) via
        // the shared detail helper — the same construction apply_field_background reuses for a gradient fill.
        const local_ref<jobject> drawable = detail::make_gradient_drawable(env.get(), *gradient);
        if (!drawable)
        {
            return; // a JNI miss during construction: leave the existing background (no guess)
        }

        jmethodID set_background = cache.method(env.get(), detail::k_visual_view_class, "setBackground",
                                                "(Landroid/graphics/drawable/Drawable;)V");
        if (set_background != nullptr)
        {
            env->CallVoidMethod(view, set_background, drawable.get());
            env->ExceptionClear();
        }
    }

    // apply_field_background — compose the framework EditText underline 9-patch OVER an explicit fill so the
    // at-rest underline SURVIVES a colored/gradient Background (the date/time picker fields). MAUI's Android
    // EditText composites its underline on top of the fill (a LayerDrawable); apply_background alone REPLACES
    // the 9-patch with the fill (setBackgroundColor / setBackground), so the underline vanishes over a
    // colored field. This op reads the current framework background (the underline 9-patch), tints it to the
    // measured at-rest underline color (a SEMI-TRANSPARENT tint via SRC_IN → out.rgb = tint.rgb, out.a =
    // ninepatch.a × tint.a, so only the line pixels stay opaque and they composite over whatever is beneath),
    // builds the fill drawable, and stacks {fill, underline} in a LayerDrawable (fill bottom, underline top).
    // underline_argb is the composited-at-rest underline: 0x99000000 (black@60%) light, 0xB3FFFFFF
    // (white@70%) dark — measured to match MAUI over white / #121212 / a blue fill. When there is NO existing
    // framework background (null), there is no underline to preserve, so it degrades to apply_background.
    //
    // This is a NEW composer kept SEPARATE from apply_background (which must not change behavior for its other
    // generic-IView callers): only the date/time picker update_background overrides call it, precisely because
    // those fields carry the defStyleRes underline chrome the plain apply_background would erase.
    // MEASURED GAP (2026-08-22): the underline DOES NOT DIM WHEN THE FIELD IS DISABLED, and MAUI's does.
    // time_picker_light, sampling the underline band of every row (implied alpha = 1 - level/255):
    //     y= 324  maui 0.60  port 0.59      y=1333  maui 0.60  port 0.59
    //     y=1721  maui 0.60  port 0.60      y=1915  maui 0.60  port 0.59
    //     y=1527  maui 0.26  port 0.59   <- the ONLY row that differs
    // y=1527 is `<TimePicker IsEnabled="False" />` (time_picker.xaml:33). So the 60% constant below is
    // RIGHT for the enabled states — nine of ten bands match to within 0.01 — and wrong only for disabled.
    //
    // It is NOT a missing setEnabled: `uiautomator dump` on both apps reads the same enabled flags in the
    // same order, with index 5 `false` in BOTH. The widget is disabled; the TEXT dims correctly in both
    // (darkest pixel 189 vs 189 on that row); only the underline stays dark.
    //
    // The mechanism to fix is right here: ColorStateList.valueOf() below builds a SINGLE-STATE list, which
    // by construction answers the same colour for state_enabled=false as for every other state. MAUI goes
    // through AppCompat's tinted background, whose ColorStateList carries a disabled entry. The fix is a
    // two-state ColorStateList — {-state_enabled: dimmed, default: underline_argb} — not a new constant.
    // Measured target for the disabled entry: 0.26 alpha against 0.60 enabled, i.e. ~43% of the enabled
    // alpha, which is consistent with Material dimming the control colour rather than a fixed value.
    // Worth ~0.6-0.9% on date_picker and time_picker, which are yellow on exactly this residual.
    // A TWO-STATE ColorStateList: {-state_enabled -> disabled_argb, default -> enabled_argb}.
    //
    // ColorStateList.valueOf() answers ONE colour for EVERY state, so a field tinted with it keeps its
    // enabled underline after setEnabled(false). MAUI does not: its AppCompat background tint carries a
    // disabled entry, and the difference is visible on the board. Measured on time_picker (android),
    // underline level at the disabled row against the enabled rows:
    //     light   enabled maui 102 / port 102      disabled maui 189 / port 102
    //     dark    enabled maui 184 / port 184      disabled maui  90 / port 184
    // The enabled constants are already right; only the disabled state was missing. Note MAUI moves the
    // colour TOWARD the page in both themes (lighter on white, darker on #121212) — it is reducing
    // contrast, not applying one fixed dim colour, which is why this takes two constants and not one.
    //
    // android.R.attr.state_enabled is 0x0101009e; NEGATING it (storing -attr) is how Android spells
    // "this state must be absent". The default row is an EMPTY int[], which matches anything and so must
    // come LAST — a ColorStateList is first-match-wins.
    [[nodiscard]] inline local_ref<jobject> make_enabled_disabled_tint(JNIEnv* env, jint enabled_argb,
                                                                       jint disabled_argb)
    {
        constexpr jint k_attr_state_enabled = 0x0101009e;
        auto& cache = default_jni_cache();
        jclass csl_class = cache.find_class(env, "android/content/res/ColorStateList");
        jclass int_array_class = env->FindClass("[I");
        if (csl_class == nullptr || int_array_class == nullptr)
        {
            env->ExceptionClear();
            return local_ref<jobject>{env, nullptr};
        }
        jmethodID ctor = cache.method(env, "android/content/res/ColorStateList", "<init>", "([[I[I)V");
        if (ctor == nullptr)
        {
            return local_ref<jobject>{env, nullptr};
        }
        // states[0] = {-state_enabled}  (disabled)   states[1] = {}  (everything else)
        local_ref<jintArray> disabled_state{env, env->NewIntArray(1)};
        if (!disabled_state)
        {
            env->ExceptionClear();
            return local_ref<jobject>{env, nullptr};
        }
        const jint negated = -k_attr_state_enabled;
        env->SetIntArrayRegion(disabled_state.get(), 0, 1, &negated);
        local_ref<jintArray> default_state{env, env->NewIntArray(0)};
        local_ref<jobjectArray> states{env, env->NewObjectArray(2, int_array_class, nullptr)};
        if (!default_state || !states)
        {
            env->ExceptionClear();
            return local_ref<jobject>{env, nullptr};
        }
        env->SetObjectArrayElement(states.get(), 0, disabled_state.get());
        env->SetObjectArrayElement(states.get(), 1, default_state.get());
        local_ref<jintArray> colors{env, env->NewIntArray(2)};
        if (!colors)
        {
            env->ExceptionClear();
            return local_ref<jobject>{env, nullptr};
        }
        const jint values[2] = {disabled_argb, enabled_argb};
        env->SetIntArrayRegion(colors.get(), 0, 2, values);
        local_ref<jobject> tint{env, env->NewObject(csl_class, ctor, states.get(), colors.get())};
        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
            return local_ref<jobject>{env, nullptr};
        }
        return tint;
    }

    inline void apply_field_background(void* native, const maui::graphics::paint* fill, jint underline_argb)
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

        // The framework underline 9-patch currently installed as the View background.
        jmethodID get_background = cache.method(env.get(), detail::k_visual_view_class, "getBackground",
                                                "()Landroid/graphics/drawable/Drawable;");
        const local_ref<jobject> old_bg{
            env.get(), get_background != nullptr ? env->CallObjectMethod(view, get_background) : nullptr};
        env->ExceptionClear();
        if (!old_bg)
        {
            // No framework chrome to preserve → just install the fill (the plain apply_background path).
            apply_background(native, fill);
            return;
        }

        // Tint the underline 9-patch DIRECTLY to the at-rest underline color. A SEMI-TRANSPARENT tint with
        // SRC_IN keeps the 9-patch shape (only the line pixels are opaque) and composites over the fill.
        // setTintList / setTintMode resolve on Drawable and dispatch to the 9-patch subclass instance.
        //
        // OPEN DEFECT (2026-08-22) — THIS DOES NOT PRODUCE MAUI'S UNDERLINE OVER A COLOURED FILL, and it
        // is the larger half of what keeps android date_picker / time_picker YELLOW in dark.
        // MEASURED on time_picker's `<TimePicker BackgroundColor="Blue" />` row, at the underline rows
        // (y=517-518; the field's geometry is IDENTICAL in both columns, blue band 432..540, so this is
        // purely colour):
        //     dark    MAUI (179,179,255)  port (0,0,0)
        //     light   MAUI (0,0,102)      port (0,0,0)
        // MAUI's dark value is EXACTLY 0xB3FFFFFF (white@70%) composited over pure blue, and its light
        // value is EXACTLY 0x99000000 (black@60%) over the same — i.e. THE CONSTANTS PASSED IN HERE AND
        // THE is_night_mode() SELECTION ARE BOTH ALREADY CORRECT. The port nevertheless paints an opaque
        // black line in BOTH themes, which is neither constant.
        //
        // REFUTED, so nobody spends the hour again: "SRC_IN drops the tint colour's alpha, so split it
        // into an opaque tint plus Drawable.setAlpha(a)". Implemented, built clean, deployed, measured —
        // the pixels did not move at all (still (0,0,0) at y=517). So the alpha is not what is lost, and
        // whatever paints that line is not this setTintList call taking effect with the wrong alpha.
        // WHAT old_bg ACTUALLY IS, logged from the device with a temporary __android_log_print probe:
        //     apply_field_background old_bg=android.graphics.drawable.InsetDrawable
        // i.e. the framework wraps @android:drawable/edit_text's 9-patch in an InsetDrawable for its
        // padding, so every setTintList/setTintMode/setAlpha here lands on a WRAPPER, not on the 9-patch.
        //
        // SECOND REFUTED HYPOTHESIS: "a Drawable inflated from a resource shares its ConstantState, so
        // tinting it without mutate() is the classic Android silent no-op". Implemented properly —
        // mutate() called on old_bg with the RETURNED handle replacing it, since tinting the pre-mutate
        // object would change nothing — built clean, deployed, re-measured. The pixels again did not move
        // by one level ((0,0,0) at y=517, (0,0,2) at y=518, identical to before).
        //
        // So BOTH "the tint is applied with the wrong alpha" and "the tint is dropped because the drawable
        // is shared" are dead. Two builds, two device measurements, zero pixels moved.
        //
        // PROVEN, and it closes the whole tint line of enquiry. Instrumented at the moment of the call:
        //     tint: value_of=1 set_tint_list=1 argb=b3ffffff   <- correct constant, so is_night_mode is fine
        //     tint: setTintList CALLED, threw=0                <- the call runs and does not throw
        // Then the decisive bisect: the tint was forced to OPAQUE RED (0xFFFF0000), rebuilt, redeployed.
        // The line stayed (0,0,0) — BLACK, unchanged, not one pixel of red.
        //
        // So THIS DRAWABLE IS NOT WHAT PAINTS THAT LINE — *as the call was ordered*. setTintList reached
        // the InsetDrawable and executed cleanly, and the pixels never moved.
        //
        // FOUND, and it was never the tint: IT WAS THE ORDER. The tint used to be applied HERE, and two
        // calls later (below) this function does `View.setBackgroundTintList(null)` to drop the create-time
        // view-level tint before installing the stack. At that moment `old_bg` is STILL the view's
        // background, and AOSP View.setBackgroundTintList(null) does not mean "no tint state" — it sets
        // mBackgroundTint.mHasTintList = true with a null list and then applyBackgroundTint() runs
        // `mBackground = mBackground.mutate(); mBackground.setTintList(null);`. So the null was pushed
        // straight back down into the very drawable this block had just tinted. One constraint explains
        // every earlier measurement at once: the clean call, the zero pixel movement, the failed mutate(),
        // the forced RED that never appeared, and a theme-INDEPENDENT (0,0,0) coming out of a
        // theme-dependent constant — what rendered was always the untinted stock 9-patch.
        //
        // Two consequences the fix has to respect, both AOSP mechanics rather than guesses:
        //   * mHasTintList can never be cleared again, so `setBackground(layer)` ALSO runs
        //     applyBackgroundTint() and pushes the null into the LayerDrawable — and LayerDrawable
        //     .setTintList forwards to every child. Tinting before the install therefore cannot survive
        //     it either. The tint must go on LAST, after setBackground.
        //   * that same applyBackgroundTint() calls mutate(), and LayerDrawable.mutate() rebuilds its
        //     children from their ConstantState (LayerState's ChildDrawable copy-ctor calls
        //     cs.newDrawable()), so `old_bg` is no longer the installed instance. The tint must go on the
        //     drawable RE-FETCHED from the view, via LayerDrawable.getDrawable(1) — see below.
        // The tint itself (a semi-transparent colour with SRC_IN, so out.a = ninepatch.a x tint.a) and the
        // constants passed in were correct all along; they are applied at the bottom of this function now.

        // Build the fill drawable beneath the underline. gradient → GradientDrawable (the shared helper);
        // solid / system_background → a flat ColorDrawable of the resolved argb (system_background_paint
        // derives from solid_paint, so it is checked FIRST, matching apply_background).
        local_ref<jobject> fill_dr;
        if (const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(fill))
        {
            fill_dr = detail::make_gradient_drawable(env.get(), *gradient);
        }
        else
        {
            jint fill_argb = 0;
            if (dynamic_cast<const maui::graphics::system_background_paint*>(fill) != nullptr)
            {
                fill_argb = detail::system_background_argb(env.get());
            }
            else if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(fill))
            {
                fill_argb = static_cast<jint>(solid->color().to_int());
            }
            else
            {
                return; // an unmodeled paint kind: leave the (now-tinted) underline in place (no guess)
            }
            jclass color_class = cache.find_class(env.get(), "android/graphics/drawable/ColorDrawable");
            jmethodID color_ctor = cache.method(env.get(), "android/graphics/drawable/ColorDrawable", "<init>", "(I)V");
            if (color_class == nullptr || color_ctor == nullptr)
            {
                return;
            }
            fill_dr = local_ref<jobject>{env.get(), env->NewObject(color_class, color_ctor, fill_argb)};
        }
        if (env->ExceptionCheck() == JNI_TRUE || !fill_dr)
        {
            env->ExceptionClear();
            return; // fill construction failed: leave the existing background (no guess)
        }

        // LayerDrawable(new Drawable[]{ fill, underline }) — fill on the bottom, tinted underline on top.
        jclass drawable_class = cache.find_class(env.get(), "android/graphics/drawable/Drawable");
        jclass layer_class = cache.find_class(env.get(), "android/graphics/drawable/LayerDrawable");
        jmethodID layer_ctor = cache.method(env.get(), "android/graphics/drawable/LayerDrawable", "<init>",
                                            "([Landroid/graphics/drawable/Drawable;)V");
        if (drawable_class == nullptr || layer_class == nullptr || layer_ctor == nullptr)
        {
            return;
        }
        const local_ref<jobjectArray> layers{env.get(), env->NewObjectArray(2, drawable_class, nullptr)};
        if (env->ExceptionCheck() == JNI_TRUE || !layers)
        {
            env->ExceptionClear();
            return;
        }
        env->SetObjectArrayElement(layers.get(), 0, fill_dr.get());
        env->SetObjectArrayElement(layers.get(), 1, old_bg.get());
        env->ExceptionClear();
        const local_ref<jobject> layer{env.get(), env->NewObject(layer_class, layer_ctor, layers.get())};
        if (env->ExceptionCheck() == JNI_TRUE || !layer)
        {
            env->ExceptionClear();
            return;
        }

        // Clear the create-time view-level underline tint (the plain-field STEP-A tint) so it does not
        // DOUBLE-tint the fill, then install the composed stack.
        jmethodID set_bg_tint = cache.method(env.get(), detail::k_visual_view_class, "setBackgroundTintList",
                                             "(Landroid/content/res/ColorStateList;)V");
        if (set_bg_tint != nullptr)
        {
            env->CallVoidMethod(view, set_bg_tint, static_cast<jobject>(nullptr));
            env->ExceptionClear();
        }
        jmethodID set_background = cache.method(env.get(), detail::k_visual_view_class, "setBackground",
                                                "(Landroid/graphics/drawable/Drawable;)V");
        if (set_background == nullptr)
        {
            return;
        }
        env->CallVoidMethod(view, set_background, layer.get());
        env->ExceptionClear();

        // ...AND ONLY NOW TINT THE UNDERLINE — on the child of the drawable the VIEW actually holds, not on
        // the `old_bg`/`layer` handles built above. Both of those are stale by this line: setBackground ran
        // applyBackgroundTint(), which mutate()s the LayerDrawable and rebuilds its children from their
        // ConstantState, and which would have wiped any tint applied earlier (the long note above). Layer 1
        // is the underline (index 0 = fill), by construction of the array a few lines up.
        const local_ref<jobject> installed{env.get(), env->CallObjectMethod(view, get_background)};
        env->ExceptionClear();
        if (!installed)
        {
            return;
        }
        jmethodID get_layer = cache.method(env.get(), "android/graphics/drawable/LayerDrawable", "getDrawable",
                                           "(I)Landroid/graphics/drawable/Drawable;");
        if (get_layer == nullptr)
        {
            return;
        }
        const local_ref<jobject> underline{env.get(), env->CallObjectMethod(installed.get(), get_layer, 1)};
        env->ExceptionClear();
        if (!underline)
        {
            return;
        }
        if (jclass csl_class = cache.find_class(env.get(), "android/content/res/ColorStateList"))
        {
            jmethodID value_of = cache.static_method(env.get(), "android/content/res/ColorStateList", "valueOf",
                                                     "(I)Landroid/content/res/ColorStateList;");
            jmethodID set_tint_list = cache.method(env.get(), "android/graphics/drawable/Drawable", "setTintList",
                                                   "(Landroid/content/res/ColorStateList;)V");
            if (value_of != nullptr && set_tint_list != nullptr)
            {
                const local_ref<jobject> tint{env.get(),
                                              env->CallStaticObjectMethod(csl_class, value_of, underline_argb)};
                if (env->ExceptionCheck() != JNI_TRUE && tint)
                {
                    env->CallVoidMethod(underline.get(), set_tint_list, tint.get());
                }
                env->ExceptionClear();
            }
        }
        if (jclass mode_class = cache.find_class(env.get(), "android/graphics/PorterDuff$Mode"))
        {
            jmethodID set_tint_mode = cache.method(env.get(), "android/graphics/drawable/Drawable", "setTintMode",
                                                   "(Landroid/graphics/PorterDuff$Mode;)V");
            jfieldID src_in = env->GetStaticFieldID(mode_class, "SRC_IN", "Landroid/graphics/PorterDuff$Mode;");
            if (set_tint_mode != nullptr && src_in != nullptr)
            {
                const local_ref<jobject> mode{env.get(), env->GetStaticObjectField(mode_class, src_in)};
                if (env->ExceptionCheck() != JNI_TRUE && mode)
                {
                    env->CallVoidMethod(underline.get(), set_tint_mode, mode.get());
                }
            }
            env->ExceptionClear();
        }
        // The tint lands after the view has already drawn its (untinted) first frame in some orderings, so
        // ask for a repaint rather than relying on the next layout pass to schedule one.
        jmethodID invalidate = cache.method(env.get(), detail::k_visual_view_class, "invalidate", "()V");
        if (invalidate != nullptr)
        {
            env->CallVoidMethod(view, invalidate);
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

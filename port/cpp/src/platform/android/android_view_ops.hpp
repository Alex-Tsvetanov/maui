#pragma once
// Shared Android (JNI) operations for the generic-IView render transform and flow direction — the
// platform side of the shared view_mapper's map_transform / map_flow_direction (view_mapper.cpp). The
// JNI twin of apple_view_ops.hpp: include only from the android partials (it reaches the jni_cache /
// scoped_env seam and android.view.View setters). VM-less safe (every helper acquires a scoped_env and
// quietly returns when no JavaVM / no widget exists — the headless-mirror degradation the android
// partials document).
//
// apply_transform ports Microsoft.Maui.Platform.TransformationExtensions
// (src/Core/src/Platform/Android/TransformationExtensions.cs) + ViewExtensions.Initialize's transform
// block. Unlike iOS — which rebuilds a single CATransform3D from all ten scalars — android.view.View
// has DIRECT setters for each transform property, so the C# Android extensions push each scalar straight
// onto the View. We push the whole transform_spec at once (the shared mapper always hands the complete
// bundle), but onto the same direct View setters the C# extensions use:
//   - TranslationX/Y  -> setTranslationX/Y(ToPixels(translation))   (the dp→px conversion, like C#)
//   - ScaleX          -> setScaleX(scale * scaleX)  (UpdateScaleX: NaN scale is a no-op — skipped)
//   - ScaleY          -> setScaleY(scale * scaleY)  (UpdateScaleY: NaN scale is a no-op — skipped)
//   - Rotation        -> setRotation(rotation)
//   - RotationX       -> setRotationX(rotationX)
//   - RotationY       -> setRotationY(rotationY)
//   - AnchorX/Y       -> setPivotX/Y(anchor * ToPixels(frame.size))  (UpdateAnchorX/Y)
// The pivot is the anchor-relative offset in pixels; C# multiplies AnchorX/Y by the laid-out frame
// width/height. The port's android partials lay the widget out via platform_arrange (View.layout),
// so the pivot reads the View's own getWidth()/getHeight() (the post-arrange analog of view.Frame).
//
// apply_flow_direction ports ViewExtensions.UpdateFlowDirection / GetLayoutDirection (Android): the
// View's LayoutDirection is set to INHERIT (MatchParent), LTR, or RTL. (C# also has a TextView fast path
// that recurses into TextViewExtensions; that path resolves to the same LayoutDirection assignment plus a
// TextAlignment refresh — the alignment refresh is part of the text-control fan-out, so the shared
// generic-IView op pushes only the LayoutDirection, exactly like the non-TextView branch.)

#include <jni.h>

#include <cmath>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/view_platform_base.hpp"

namespace maui::platform::android
{
    namespace detail
    {
        // All instance setters resolve through android/view/View (GetMethodID walks superclasses, so a
        // concrete widget like android.widget.Button resolves the View surface through this class too).
        inline constexpr const char* k_view_class = "android/view/View";

        // android.util.DisplayMetrics.density — the dp→px factor (ContextExtensions.ToPixels). 0 = not
        // read yet; failures are not memoized so a transient failure does not pin the fallback. Process-
        // wide, exactly like ContextExtensions' s_displayDensity cache (the JNI walk is four calls).
        [[nodiscard]] inline float view_display_density(JNIEnv* env, jobject view)
        {
            auto& cache = default_jni_cache();
            jmethodID get_context = cache.method(env, k_view_class, "getContext", "()Landroid/content/Context;");
            jmethodID get_resources =
                cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
            jmethodID get_display_metrics = cache.method(env, "android/content/res/Resources", "getDisplayMetrics",
                                                         "()Landroid/util/DisplayMetrics;");
            jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
            if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
                density_field == nullptr)
            {
                return 1.0F;
            }
            const local_ref<jobject> context{env, env->CallObjectMethod(view, get_context)};
            if (env->ExceptionCheck() == JNI_TRUE || !context)
            {
                env->ExceptionClear();
                return 1.0F;
            }
            const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
            if (env->ExceptionCheck() == JNI_TRUE || !resources)
            {
                env->ExceptionClear();
                return 1.0F;
            }
            const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
            if (env->ExceptionCheck() == JNI_TRUE || !metrics)
            {
                env->ExceptionClear();
                return 1.0F;
            }
            const jfloat density = env->GetFloatField(metrics.get(), density_field);
            if (env->ExceptionCheck() == JNI_TRUE || density == 0.0F)
            {
                env->ExceptionClear();
                return 1.0F;
            }
            return density;
        }

        // ContextExtensions.ToPixels (the float overload TransformationExtensions uses): dp * density.
        // (TranslationX/Y / pivots are floats, not ceiled ints — only padding/measure ceil; see the
        // float ToPixels in ContextExtensions.cs.)
        [[nodiscard]] inline jfloat view_to_pixels(double dp, float density)
        {
            return static_cast<jfloat>(dp * static_cast<double>(density));
        }

        inline void view_call_void_float(JNIEnv* env, jobject view, const char* name, jfloat value)
        {
            if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(F)V"))
            {
                env->CallVoidMethod(view, method, value);
                env->ExceptionClear(); // never leak pending JNI state into the cross-platform layer
            }
        }

        inline void view_call_void_int(JNIEnv* env, jobject view, const char* name, jint value)
        {
            if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(I)V"))
            {
                env->CallVoidMethod(view, method, value);
                env->ExceptionClear();
            }
        }

        // The View's laid-out size in dp (getWidth()/getHeight() px / density). The pivot multiplies the
        // anchor by this — the post-arrange analog of C#'s view.Frame.Width/Height.
        [[nodiscard]] inline double view_width_dp(JNIEnv* env, jobject view, float density)
        {
            jmethodID get_width = default_jni_cache().method(env, k_view_class, "getWidth", "()I");
            if (get_width == nullptr || density == 0.0F)
            {
                return 0;
            }
            const jint px = env->CallIntMethod(view, get_width);
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
                return 0;
            }
            return static_cast<double>(px) / static_cast<double>(density);
        }

        [[nodiscard]] inline double view_height_dp(JNIEnv* env, jobject view, float density)
        {
            jmethodID get_height = default_jni_cache().method(env, k_view_class, "getHeight", "()I");
            if (get_height == nullptr || density == 0.0F)
            {
                return 0;
            }
            const jint px = env->CallIntMethod(view, get_height);
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
                return 0;
            }
            return static_cast<double>(px) / static_cast<double>(density);
        }
    } // namespace detail

    // android.view.View layout-direction constants (Android.Views.LayoutDirection — the
    // GetLayoutDirection targets). LTR=0, RTL=1, INHERIT=2 (the platform integer values).
    inline constexpr jint k_layout_direction_ltr = 0;
    inline constexpr jint k_layout_direction_rtl = 1;
    inline constexpr jint k_layout_direction_inherit = 2;

    // Push the whole render transform onto the View's direct transform setters (TransformationExtensions).
    inline void apply_transform(void* native, const maui::core::transform_spec& t)
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
        const float density = detail::view_display_density(env.get(), view);

        // TranslationX/Y: dp → px (ContextExtensions.ToPixels), the float setters.
        detail::view_call_void_float(env.get(), view, "setTranslationX",
                                     detail::view_to_pixels(t.translation_x, density));
        detail::view_call_void_float(env.get(), view, "setTranslationY",
                                     detail::view_to_pixels(t.translation_y, density));

        // ScaleX/Y: scale * scaleX / scale * scaleY (UpdateScaleX/Y). A NaN uniform Scale is a no-op
        // (UpdateScaleX/Y return early on double.IsNaN(scale)).
        if (!std::isnan(t.scale))
        {
            detail::view_call_void_float(env.get(), view, "setScaleX", static_cast<jfloat>(t.scale * t.scale_x));
            detail::view_call_void_float(env.get(), view, "setScaleY", static_cast<jfloat>(t.scale * t.scale_y));
        }

        // Rotation / RotationX / RotationY: pushed directly (degrees, exactly as the View setters want).
        detail::view_call_void_float(env.get(), view, "setRotation", static_cast<jfloat>(t.rotation));
        detail::view_call_void_float(env.get(), view, "setRotationX", static_cast<jfloat>(t.rotation_x));
        detail::view_call_void_float(env.get(), view, "setRotationY", static_cast<jfloat>(t.rotation_y));

        // AnchorX/Y → pivot: anchor * the laid-out size in px (UpdateAnchorX/Y multiply by
        // ToPixels(view.Frame.Width/Height); the View's own getWidth/getHeight are the post-arrange size).
        const double width_dp = detail::view_width_dp(env.get(), view, density);
        const double height_dp = detail::view_height_dp(env.get(), view, density);
        detail::view_call_void_float(env.get(), view, "setPivotX",
                                     detail::view_to_pixels(t.anchor_x * width_dp, density));
        detail::view_call_void_float(env.get(), view, "setPivotY",
                                     detail::view_to_pixels(t.anchor_y * height_dp, density));
    }

    // Push FlowDirection onto the View's LayoutDirection (ViewExtensions.GetLayoutDirection).
    inline void apply_flow_direction(void* native, maui::core::flow_direction fd)
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
        jint direction = k_layout_direction_inherit; // MatchParent / default → Inherit
        if (fd == maui::core::flow_direction::left_to_right)
        {
            direction = k_layout_direction_ltr;
        }
        else if (fd == maui::core::flow_direction::right_to_left)
        {
            direction = k_layout_direction_rtl;
        }
        detail::view_call_void_int(env.get(), static_cast<jobject>(native), "setLayoutDirection", direction);
    }
} // namespace maui::platform::android

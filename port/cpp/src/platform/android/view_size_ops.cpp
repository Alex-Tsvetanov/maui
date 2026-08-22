// view_size_ops — Android (JNI): the REAL native minimum push, ViewHandler.cs:52-55 MapMinimumHeight /
// MapMinimumWidth -> ViewExtensions.cs:433-444 UpdateMinimumHeight / UpdateMinimumWidth:
//
//     var min = Dimension.ResolveMinimum(view.MinimumHeight);      // Unset -> 0 (Dimension.cs:24-32)
//     platformView.SetMinimumHeight((int)platformView.Context!.ToPixels(min));
//     PlatformInterop.RequestLayoutIfNeeded(platformView);
//
// This is the ONLY channel a Minimum*Request has on Android: ContextExtensions.CreateMeasureSpec
// (ContextExtensions.cs:418-434) folds it into the measure spec exclusively inside
// `if (IsExplicitSet(explicitSize))`, and GetDesiredSizeFromHandler (ViewHandlerExtensions.Android.cs
// :78-97) returns the platform measure with nothing applied afterwards. So a widget gets its floor only
// because its own onMeasure consults View.getSuggestedMinimumWidth/Height — which android.widget.TextView,
// Button and EditText do, and android.webkit.WebView does not. view<>::measure's
// measure_minimum_width/height stands the shared clamp down on this backend precisely so that this
// difference is the one that decides, exactly as it does in MAUI.
//
// The RequestLayoutIfNeeded half is deliberately NOT replicated: the port drives measure/arrange top-down
// from C++ rather than from Android's layout traversal (see maui::controls::view::invalidate_measure's
// header), and this push runs from the property mapper, i.e. inside a pass that is about to measure
// anyway. VM-less safe: no JavaVM / no native view / a widget with no Context is a quiet no-op, the
// android partials' standing degradation.

#include "maui/core/view_size_ops.hpp"

#include <jni.h>

#include <cmath>

#include "android_view_ops.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"

namespace maui::core
{
    namespace
    {
        // ContextExtensions.ToPixels: (int)(value * density + 0.5) is C#'s ToPixels rounding, applied
        // through the same DisplayMetrics.density the android partials read (android_view_ops).
        [[nodiscard]] jint to_pixels(double value, float density)
        {
            return static_cast<jint>(value * static_cast<double>(density) + 0.5);
        }

        // Dimension.ResolveMinimum (src/Core/src/Primitives/Dimension.cs:24-32): an unset minimum resolves
        // to Dimension.Minimum, i.e. 0. The port's unset sentinel is NaN / a negative request.
        [[nodiscard]] double resolve_minimum(double value)
        {
            return (std::isnan(value) || value < 0) ? 0.0 : value;
        }

        void push(JNIEnv* env, jobject view, const char* setter, jint px)
        {
            auto& cache = maui::platform::android::default_jni_cache();
            if (jmethodID method = cache.method(env, "android/view/View", setter, "(I)V"))
            {
                env->CallVoidMethod(view, method, px);
                if (env->ExceptionCheck() != JNI_FALSE)
                {
                    env->ExceptionClear();
                }
            }
        }
    } // namespace

    void apply_native_minimum_size(void* native_view, double minimum_width, double minimum_height)
    {
        if (native_view == nullptr)
        {
            return;
        }
        const maui::platform::android::scoped_env env;
        if (!env)
        {
            return;
        }
        auto view = static_cast<jobject>(native_view);
        const float density = maui::platform::android::detail::view_display_density(env.get(), view);
        push(env.get(), view, "setMinimumWidth", to_pixels(resolve_minimum(minimum_width), density));
        push(env.get(), view, "setMinimumHeight", to_pixels(resolve_minimum(minimum_height), density));
    }
} // namespace maui::core

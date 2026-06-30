// activity_indicator_handler — Android (JNI) platform partial, the M-android per-control fan-out
// replayed over JNI (the headless mirror's twin, the iOS .mm's sibling). The managed platform view
// is a REAL android.widget.ProgressBar constructed indeterminate (setIndeterminate(true)), held as a
// JNI global reference in activity_indicator_platform::native. The two mapped properties push through
// jni_cache'd method ids: map_is_running drives View visibility (the C# Visibility→MapIsRunning
// override) and map_color tints the IndeterminateDrawable. The control is display-only — no inbound
// channel, so there is no listener trampoline (unlike button_handler's click path).
//
// Ported DIRECTLY from ActivityIndicatorHandler.Android.cs + Platform/Android/
// {ActivityIndicatorExtensions.cs (UpdateIsRunning/GetActivityIndicatorVisibility/UpdateColor),
// ViewExtensions.cs (ToPlatformVisibility)}.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is an infrastructure gap, not a behavior guess):
//   - The widget is a plain android.widget.ProgressBar, NOT the internal MaterialActivityIndicator
//     (ActivityIndicatorHandler2, "TODO: material3 - make it public in .net 11"): the Material
//     Components library is a gradle/AAR dependency this APK-less backend does not carry. So the
//     public ActivityIndicatorHandler (plain ProgressBar) is the faithful target; the Material
//     subclass's centering PlatformArrange override is not ported (it belongs to that subclass only).
//   - The port's colors are non-nullable value types, so C#'s `color != null` branch collapses
//     exactly as it did in the ios/apple partials (see map_color): an unset (default-constructed)
//     color takes the ClearColorFilter path (restore the platform default tint), a set color takes
//     the SetColorFilter path. This is the same unset-color convention button_handler/the apple
//     partials use, derived from C#'s null-color semantics.
//   - The generic-IView pushes (opacity/automation_id/background/transform/flow_direction/semantics)
//     are NOT pushed to the real ProgressBar here: activity_indicator_handler.hpp declares no
//     `#ifdef MAUI_PLATFORM_ANDROID` override block on activity_indicator_platform (unlike
//     button_platform), and the deliverable touches only this .cpp + its test. They remain the
//     view_platform_base headless mirrors. This mirrors C# faithfully for the two ported keys:
//     ActivityIndicatorHandler.Android.cs implements ONLY MapIsRunning + MapColor; the remaining
//     IView properties flow through the base ViewHandler, which is the deferred android container
//     fan-out. Visibility is NOT a generic push here either — the mapper replaces the Visibility key
//     with MapIsRunning, so visibility reaches the widget through map_is_running (the C# override).
//     // TODO: verify against src/Core/src/Handlers/ActivityIndicator/ActivityIndicatorHandler.Android.cs
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the
// emulator where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly
// skips, while the headless mirrors (is_running/hidden/color) are ALWAYS maintained — so that suite
// observes exactly the headless partial's behavior, and the widget test host additionally observes
// the real ProgressBar.

#include "maui/core/activity_indicator_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <memory>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // Instance methods resolve through the widget's own class — GetMethodID walks the superclasses,
    // so the View surface (setVisibility/measure/getMeasured*/layout) resolves through ProgressBar.
    constexpr const char* k_progress_bar_class = "android/widget/ProgressBar";
    constexpr const char* k_drawable_class = "android/graphics/drawable/Drawable";
    constexpr const char* k_porter_duff_mode_class = "android/graphics/PorterDuff$Mode";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    [[nodiscard]] jobject widget_of(const maui::core::activity_indicator_platform& platform) noexcept
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
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, same channel the test host uses
        env->ExceptionClear();
        return true;
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "(Z)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_int(JNIEnv* env, jobject widget, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity
    // cache (the JNI walk is four calls). 1.0 when any step fails (failures are NOT memoized, so a
    // transient failure does not pin the fallback). Identical to button_handler's helper, kept local
    // to this TU rather than shared (the android backend has no common density helper header yet).
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_progress_bar_class, "getContext", "()Landroid/content/Context;");
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
        const local_ref<jobject> context{env, env->CallObjectMethod(widget, get_context)};
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
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.ProgressBar (the JNI shape of the
    // pimpl-owned-native-view doctrine: the ios twin CFReleases its UIActivityIndicatorView here).
    activity_indicator_platform::~activity_indicator_platform()
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

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass progress_bar_class = cache.find_class(env.get(), k_progress_bar_class);
        jmethodID ctor = cache.method(env.get(), k_progress_bar_class, "<init>", "(Landroid/content/Context;)V");
        if (progress_bar_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // ActivityIndicatorHandler.CreatePlatformView: new ProgressBar(Context) { Indeterminate = true }.
        const local_ref<jobject> widget{env.get(), env->NewObject(progress_bar_class, ctor, context)};
        if (clear_pending(env.get()) || !widget)
        {
            return platform;
        }
        // Indeterminate = true: the spinner-style ProgressBar (the indeterminate drawable is what
        // map_color tints). setIndeterminate(boolean) is a ProgressBar (not a base View) method.
        call_void_bool(env.get(), widget.get(), "setIndeterminate", JNI_TRUE);
        // Wrap-content LayoutParams up front (same parentless-measure NPE guard button_handler
        // documents): a parentless view with null LayoutParams can NPE in checkForRelayout once the
        // android container fan-out attaches it; the partial stands in for that attach.
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params = cache.method(env.get(), k_progress_bar_class, "setLayoutParams",
                                                   "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_wrap_content, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(widget.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        platform->native = env->NewGlobalRef(widget.get()); // released in ~activity_indicator_platform
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        // ActivityIndicatorExtensions.UpdateIsRunning → GetActivityIndicatorVisibility: when the view
        // is Visible, the widget is Visible while IsRunning else Invisible; otherwise the visibility
        // falls to ToPlatformVisibility (Hidden→Invisible, Collapsed→Gone). The mapper REPLACES the
        // generic Visibility key with this function (C#: "Visibility and IsRunning are dependent").
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Headless mirror FIRST — the VM-less cross-platform suite observes these (kept live exactly
        // as the headless partial writes them: is_running tracks IsRunning && Visible, hidden tracks
        // !Visible). The widget push below is additive.
        const bool visible = view.visibility() == visibility::visible;
        platform->is_running = view.is_running() && visible;
        platform->hidden = !visible;
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // GetActivityIndicatorVisibility resolved to one of the three View states.
        jint state = k_view_visible;
        if (view.visibility() == visibility::visible)
        {
            state = view.is_running() ? k_view_visible : k_view_invisible;
        }
        else if (view.visibility() == visibility::collapsed)
        {
            state = k_view_gone;
        }
        else // hidden
        {
            state = k_view_invisible;
        }
        call_void_int(env.get(), widget_of(*platform), "setVisibility", state);
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->color = view.color(); // ActivityIndicatorExtensions.UpdateColor (headless mirror)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ProgressBar.getIndeterminateDrawable() — the spinning drawable the color tints. A null
        // drawable (no indeterminate drawable yet) collapses both branches to a no-op, mirroring C#'s
        // `IndeterminateDrawable?.` null-conditional.
        jmethodID get_indeterminate_drawable = cache.method(env.get(), k_progress_bar_class, "getIndeterminateDrawable",
                                                            "()Landroid/graphics/drawable/Drawable;");
        if (get_indeterminate_drawable == nullptr)
        {
            return;
        }
        const local_ref<jobject> drawable{env.get(), env->CallObjectMethod(widget, get_indeterminate_drawable)};
        if (clear_pending(env.get()) || !drawable)
        {
            return;
        }
        // The port's color is a non-nullable value type, so C#'s `color != null` becomes
        // `color != default` (the unset-color convention — see the header and the ios/apple twins):
        //   set color   → IndeterminateDrawable.SetColorFilter(color.ToPlatform(), FilterMode.SrcIn)
        //   unset color → IndeterminateDrawable.ClearColorFilter()  (restore the platform default)
        if (view.color() != maui::graphics::color{})
        {
            // Drawable.setColorFilter(int color, PorterDuff.Mode mode) with PorterDuff.Mode.SRC_IN —
            // FilterMode.SrcIn in C#. (Deprecated in API 29 in favor of setColorFilter(ColorFilter),
            // but still functional and the exact oracle path; the prompt's setIndeterminateTintList
            // alternative is NOT what ActivityIndicatorExtensions.UpdateColor does.)
            jmethodID set_color_filter =
                cache.method(env.get(), k_drawable_class, "setColorFilter", "(ILandroid/graphics/PorterDuff$Mode;)V");
            jclass mode_class = cache.find_class(env.get(), k_porter_duff_mode_class);
            // SRC_IN is a STATIC field — jni_cache::field() is GetFieldID (instance) and returns null
            // for it, so resolve it directly with GetStaticFieldID.
            jfieldID src_in_field = mode_class != nullptr ? env->GetStaticFieldID(mode_class, "SRC_IN",
                                                                                  "Landroid/graphics/PorterDuff$Mode;")
                                                          : nullptr;
            clear_pending(env.get());
            if (set_color_filter == nullptr || src_in_field == nullptr || mode_class == nullptr)
            {
                return;
            }
            const local_ref<jobject> src_in{env.get(), env->GetStaticObjectField(mode_class, src_in_field)};
            if (clear_pending(env.get()) || !src_in)
            {
                return;
            }
            env->CallVoidMethod(drawable.get(), set_color_filter, static_cast<jint>(view.color().to_int()),
                                src_in.get());
            clear_pending(env.get());
        }
        else
        {
            jmethodID clear_color_filter = cache.method(env.get(), k_drawable_class, "clearColorFilter", "()V");
            if (clear_color_filter == nullptr)
            {
                return;
            }
            env->CallVoidMethod(drawable.get(), clear_color_filter);
            clear_pending(env.get());
        }
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double width_constraint,
                                                                      double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (the medium-style square),
            // so the backend-agnostic size-request suites see consistent numbers in the pure-native run.
            return {20.0, 20.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost
        // specs in pixels, infinite become Unspecified; View.measure, then the measured pixels come
        // back as dp (Context.FromPixels).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_progress_bar_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_progress_bar_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_progress_bar_class, "getMeasuredHeight", "()I");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || get_measured_width == nullptr ||
            get_measured_height == nullptr || measure_spec_class == nullptr)
        {
            return {0, 0};
        }
        const float density = display_density(env.get(), widget);
        const auto spec_for = [&](double constraint) -> jint {
            const jint size = std::isfinite(constraint) ? to_pixels(constraint, density) : 0;
            const jint mode = std::isfinite(constraint) ? k_measure_spec_at_most : k_measure_spec_unspecified;
            const jint spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, size, mode);
            return clear_pending(env.get()) ? 0 : spec;
        };
        const jint width_spec = spec_for(width_constraint);
        const jint height_spec = spec_for(height_constraint);
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        const jint measured_width = env->CallIntMethod(widget, get_measured_width);
        const jint measured_height = env->CallIntMethod(widget, get_measured_height);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        return {static_cast<double>(measured_width) / density, static_cast<double>(measured_height) / density};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless: no native layout to apply
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the view measures Exactly at the
        // final size (Android requires a measure pass before layout) and lays out. (The plain
        // ProgressBar uses the base PlatformArrange; only the Material subclass adds centering.)
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_progress_bar_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_progress_bar_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), widget);
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
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(widget, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

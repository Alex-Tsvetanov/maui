// slider_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed for the
// linear-value input. The managed platform view is a REAL android.widget.SeekBar (held as a JNI global
// reference in slider_platform::native): Minimum/Maximum/Value all recompute and push the integer
// Progress over a fixed Max = PlatformMaxValue range (SeekBar uses an integer scale), the three colors
// land on ProgressTintList (min track) / ProgressBackgroundTintList (max track) / ThumbTintList (thumb),
// and the native value change / drag flows back through ValueChanged → set_value, DragStarted /
// DragCompleted. SeekBar is interactive, so (unlike the determinate ProgressBar twin) IsEnabled IS
// pushed and there is a connect/disconnect channel.
//
// Ported DIRECTLY from SliderHandler.Android.cs + Platform/Android/SliderExtensions.cs +
// Platform/Android/{ViewExtensions.cs (UpdateVisibility/UpdateOpacity/UpdateIsEnabled/UpdateAutomationId),
// ContextExtensions.cs (ToPixels)}.
//
// EXACT-Max NOTE (read from the oracle, NOT guessed): SeekBar's range is
// SliderExtensions.PlatformMaxValue = int.MaxValue (NOT the ProgressBar's 10000). SliderExtensions
// scales the value with `seekBar.Progress = (int)((value - min) / (max - min) * PlatformMaxValue)`, and
// CreatePlatformView builds `new SeekBar(Context) { DuplicateParentStateEnabled = false, Max =
// (int)PlatformMaxValue }`. UpdateMinimum / UpdateMaximum on a SeekBar both just re-run UpdateValue (the
// integer Progress is recomputed from the new bounds) — so map_minimum / map_maximum / map_value share
// one update_value body, exactly as the SeekBar extension overloads do.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each a Material-library or infrastructure gap, not a
// behavior guess):
//   - C#'s SliderExtensions has TWO overload families: the plain android.widget.SeekBar extensions and a
//     Google.Android.Material `Slider` (the M3 range slider). The Material Components library is a
//     gradle/AAR dependency this APK-less backend does not carry, so the SeekBar overloads are the only
//     ones ported (the same branch C# takes when the platform view is a plain SeekBar) — including the
//     min→ProgressTintList / max→ProgressBackgroundTintList track-color split, which is the SeekBar
//     recipe (the Material `Slider` uses TrackActive/InactiveTintList).
//   - C# UpdateThumbColor calls `seekBar.Thumb?.SetColorFilter(slider.ThumbColor, FilterMode.SrcIn)` —
//     mutating the live thumb Drawable's color filter. The default SeekBar thumb Drawable is only present
//     once the widget is themed/attached; in the bare app_process test host the Thumb is frequently null
//     (the `?.` no-ops in C# too). The port pushes the thumb tint through the View-level ThumbTintList
//     (SeekBar.setThumbTintList(ColorStateList)) instead — the theme-independent equivalent that does not
//     depend on a live thumb Drawable, matching the ProgressTintList path the track colors use. A live
//     SetColorFilter would be the closer transcription, but it silently no-ops without a thumb Drawable;
//     ThumbTintList is the faithful, observable stand-in (parallels progress_bar's ColorStateList tints).
//   - The port's colors are non-nullable value types, so C#'s `MinimumTrackColor is not null` /
//     `MaximumTrackColor is not null` / `ThumbColor is not null` guards collapse to the `color != color{}`
//     sentinel the ios/apple partials use: an UNSET (default-constructed) color leaves the SeekBar's
//     native default tint (the C# null branch is a no-op — it never sets the tint), rather than forcing a
//     black/transparent fill. Only an explicitly-set color takes the ColorStateList.valueOf tint path.
//   - The PorterDuff tint *mode* C# sets alongside each tint (ProgressTintMode / ProgressBackgroundTintMode
//     = SrcIn) is the SeekBar default for tint lists, so it is left at the default rather than read through
//     a static PorterDuff.Mode field (no behavioral difference for an opaque tint; documented for fidelity).
//   - ThumbImageSource: SliderExtensions.UpdateThumbImageSourceAsync decodes the source to a Bitmap-backed
//     thumb Drawable and SetThumb()s it — that path needs the image-service decode + the thumb Drawable
//     surface (CompoundDrawable-style), which is part of the deferred android image/drawable fan-out (the
//     button partial defers its CompoundDrawables icon the same way). The per-backend primitives here keep
//     ONLY the headless-style mirror (thumb_image_set), so the android preset's pure-native cross-platform
//     suite still observes the load; the real JNI thumb-drawable push is deferred.
//   - UpdateOnTap is an iOS-Specific (UISlider tap-to-set workaround). SeekBar already jumps to the tapped
//     position on a track tap by default, so — like the apple twin — map_update_on_tap records the resolved
//     flag for parity only (no native gesture install needed; the SeekBar behavior already matches intent).
//   - The OnSeekBarChangeListener (OnProgressChanged → ValueChanged, OnStartTrackingTouch → DragStarted,
//     OnStopTrackingTouch → DragCompleted) requires a host-provided Java listener class bound via
//     RegisterNatives — the android test host carries only dev.mauicpp.NativeOnClickListener (for button),
//     NOT a SeekBar change listener. So the inbound channel is DEFERRED with the gesture fan-out, exactly
//     like button_handler.cpp's OnTouchListener: on_value_changed / on_drag_started / on_drag_completed
//     stay invokable C++ callbacks (the cross-platform suite drives them) carrying SliderHandler's
//     OnProgressChanged (the fromUser → Value write-back) / OnStartTrackingTouch / OnStopTrackingTouch.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly skips, while
// the headless mirrors (minimum/maximum/value/colors/thumb_image_set/update_on_tap + the base IView
// mirrors) are ALWAYS maintained — so that suite observes exactly the headless partial's behavior, and the
// widget test host additionally observes the real widget.

#include "maui/core/slider_handler.hpp"

#include <jni.h>

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
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_ios_slider_specifics.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // All instance methods are resolved through the widget's own class (GetMethodID walks the
    // superclasses, so the AbsSeekBar/ProgressBar/View surface resolves through android/widget/SeekBar).
    constexpr const char* k_seek_bar_class = "android/widget/SeekBar";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // SliderExtensions.PlatformMaxValue — MAUI scales [Minimum,Maximum] onto this fixed integer SeekBar
    // range. The oracle defines it as `int.MaxValue` (NOT the ProgressBar's 10000) — read, not guessed.
    constexpr jint k_platform_max_value = 2147483647; // = int.MaxValue

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (PlatformInterop restores it after
    // setContentDescription auto-flips the view to YES).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::slider_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back.
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

    void call_void_int(JNIEnv* env, jobject widget, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_seek_bar_class, name, "(Z)V"))
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

    // The widget's display density (Context.getResources().getDisplayMetrics().density). 1.0 when any
    // step fails. (The shared android view ops memoize this process-wide via ContextExtensions'
    // s_displayDensity cache; the measure/arrange seam reads it directly here, the same walk.)
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_seek_bar_class, "getContext", "()Landroid/content/Context;");
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
        return density;
    }

    // SliderExtensions.UpdateValue (the SeekBar overload, shared by UpdateMinimum/UpdateMaximum/UpdateValue):
    //   seekBar.Progress = (int)((value - min) / (max - min) * PlatformMaxValue).
    // A degenerate (zero-width or inverted) range would divide by zero / overflow; C# would NaN→0 the cast,
    // so guard it to a Progress of 0 (the same observable result).
    void push_value(JNIEnv* env, jobject widget, const maui::core::i_slider& view)
    {
        const double min = view.minimum();
        const double max = view.maximum();
        const double span = max - min;
        jint progress = 0;
        if (span > 0)
        {
            const double fraction = (view.value() - min) / span;
            progress = static_cast<jint>(fraction * static_cast<double>(k_platform_max_value));
        }
        call_void_int(env, widget, "setProgress", progress);
    }

    // The shared track-color push: an explicitly-set color installs ColorStateList.valueOf(argb) onto the
    // named tint property (setProgressTintList / setProgressBackgroundTintList / setThumbTintList); an unset
    // (default) color leaves the native default tint — the collapsed C# `is not null` guard. Mirrors
    // progress_bar_handler.cpp's ColorStateList tint push.
    void push_tint_list(JNIEnv* env, jobject widget, const char* setter, const maui::graphics::color& color)
    {
        if (color == maui::graphics::color{})
        {
            return; // unset → leave the SeekBar's native default tint (C# null branch is a no-op)
        }
        auto& cache = default_jni_cache();
        jmethodID value_of =
            cache.static_method(env, k_color_state_list_class, "valueOf", "(I)Landroid/content/res/ColorStateList;");
        jmethodID set_tint = cache.method(env, k_seek_bar_class, setter, "(Landroid/content/res/ColorStateList;)V");
        jclass color_state_list_class = cache.find_class(env, k_color_state_list_class);
        if (value_of == nullptr || set_tint == nullptr || color_state_list_class == nullptr)
        {
            return;
        }
        const auto argb = static_cast<jint>(color.to_int());
        const local_ref<jobject> tint_list{env, env->CallStaticObjectMethod(color_state_list_class, value_of, argb)};
        if (clear_pending(env) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_tint, tint_list.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.SeekBar (the JNI shape of the
    // pimpl-owned-native-view doctrine: the ios twin CFReleases its UISlider here).
    slider_platform::~slider_platform()
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
    // the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform suite (see
    // the header comment) — then pushes to the real widget when one exists.

    void slider_platform::update_visibility(maui::core::visibility value)
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
        // ViewExtensions.UpdateVisibility → ToPlatformVisibility: Visible/Hidden/Collapsed map to
        // View.VISIBLE/INVISIBLE/GONE.
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), widget_of(*this), "setVisibility", state);
    }

    void slider_platform::update_opacity(double value)
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
            call_void_float(env.get(), widget_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void slider_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateIsEnabled: platformView.Enabled = view.IsEnabled. A SeekBar is
            // interactive (unlike the determinate ProgressBar), so IsEnabled IS pushed.
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void slider_platform::update_automation_id(std::string_view value)
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
        jobject widget = widget_of(*this);
        auto& cache = default_jni_cache();
        // PlatformInterop.setContentDescriptionForAutomationId: setting a ContentDescription flips
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had, so the automation
        // id does not change the view's accessibility exposure.
        jmethodID get_important = cache.method(env.get(), k_seek_bar_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_seek_bar_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(widget, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(widget, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), widget, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    // Render transform + flow direction + background + semantics pushed to the real widget via the shared
    // android ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite observes
    // the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void slider_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void slider_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void slider_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // VisualElement.Background paints the SeekBar's View background via the shared android op
        // (ViewExtensions.UpdateBackground — behind the track). VM-less safe.
        maui::platform::android::apply_background(native, value);
    }

    void slider_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        auto platform = std::make_unique<slider_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass seek_bar_class = cache.find_class(env.get(), k_seek_bar_class);
        if (seek_bar_class == nullptr)
        {
            return platform;
        }
        // SliderHandler.CreatePlatformView: `new SeekBar(Context) { DuplicateParentStateEnabled = false,
        // Max = (int)PlatformMaxValue }`. The plain `new SeekBar(Context)` ctor resolves the seekBarStyle
        // theme attribute against the Context's THEME — which the app_process widget test host (a bare,
        // Activity-less Context) does NOT carry, so that ctor throws there (the agent flagged this exact
        // failure for ProgressBar). Use the theme-INDEPENDENT 3-arg `(Context, AttributeSet, int
        // defStyleAttr)` ctor with a null AttributeSet + 0 defStyleAttr (no theme-attr resolution), and
        // fall back to the plain (Context) ctor so the widget is never null (only relevant on a real,
        // themed Context, where it matches C#'s `new SeekBar(Context)`).
        jobject created = nullptr;
        // A horizontal SeekBar needs its STYLE for the thumb/track drawables (and thus a non-zero intrinsic
        // size); defStyleAttr=0 yields a degenerate, size-0, drawable-less SeekBar (measure → 0×0). The
        // plain SeekBar(Context) resolves the seekBarStyle theme attr, which the bare app_process testhost
        // lacks (throws). So construct via the theme-INDEPENDENT 4-arg ctor with defStyleRes =
        // android.R.style.Widget_SeekBar (a concrete style resource — like progress_bar's horizontal style;
        // read with GetStaticFieldID since it is a static field), then fall back to the 3-arg defStyleAttr=0
        // and finally the plain (Context) ctor so the widget is never null.
        jmethodID ctor_styled = cache.method(env.get(), k_seek_bar_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), "android/R$style");
        jfieldID seek_bar_style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, "Widget_SeekBar", "I") : nullptr;
        clear_pending(env.get());
        if (ctor_styled != nullptr && style_class != nullptr && seek_bar_style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, seek_bar_style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(seek_bar_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_themeless = cache.method(env.get(), k_seek_bar_class, "<init>",
                                                    "(Landroid/content/Context;Landroid/util/AttributeSet;I)V");
            if (ctor_themeless != nullptr)
            {
                created = env->NewObject(seek_bar_class, ctor_themeless, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0));
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain = cache.method(env.get(), k_seek_bar_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(seek_bar_class, ctor_plain, context);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            return platform;
        }
        const local_ref<jobject> widget{env.get(), created};
        // The object-initializer of CreatePlatformView: { DuplicateParentStateEnabled = false, Max =
        // PlatformMaxValue }.
        call_void_bool(env.get(), widget.get(), "setDuplicateParentStateEnabled", JNI_FALSE);
        call_void_int(env.get(), widget.get(), "setMax", k_platform_max_value);
        // Wrap-content LayoutParams up front (parentless View measure/layout safety — the android container
        // fan-out has not arrived; the partial stands in for the parent ViewGroup attach, exactly like
        // button_handler.cpp / progress_bar_handler.cpp do).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_seek_bar_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~slider_platform
        return platform;
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        // SliderHandler.ConnectHandler installs a SeekBarChangeListener (OnProgressChanged → the fromUser
        // Value write-back, OnStartTrackingTouch → DragStarted, OnStopTrackingTouch → DragCompleted). That
        // listener needs a host-provided Java class bound via RegisterNatives (like button's
        // dev.mauicpp.NativeOnClickListener); the android test host does NOT carry a SeekBar listener, so
        // the inbound JNI channel is DEFERRED (header note). The callbacks stay wired even VM-less so the
        // cross-platform suite can drive them — carrying SliderHandler.OnProgressChanged /
        // OnStartTrackingTouch / OnStopTrackingTouch.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                // OnProgressChanged (fromUser): VirtualView.Value = min + (max-min) * (progress/Max). The
                // platform mirror's `value` carries the user's resolved value (the headless seam writes it
                // there before invoking this), so write it straight back through i_range::set_value.
                view->set_value(platform_view->value);
            }
        };
        platform.on_drag_started = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_started(); // OnStartTrackingTouch → DragStarted
            }
        };
        platform.on_drag_completed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_completed(); // OnStopTrackingTouch → DragCompleted
            }
        };
        // The real SetOnSeekBarChangeListener install is deferred (no host listener class) — when the
        // SeekBar listener trampoline lands, wire it here exactly as button_handler.cpp wires its click
        // listener (RegisterNatives + NewObject(listener, peer) + setOnSeekBarChangeListener).
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        // SliderHandler.DisconnectHandler: ChangeListener.Handler = null; SetOnSeekBarChangeListener(null).
        // The native uninstall is deferred with the listener install (header note); drop the callbacks.
        platform.on_value_changed = nullptr;
        platform.on_drag_started = nullptr;
        platform.on_drag_completed = nullptr;
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->minimum = view.minimum(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SliderExtensions.UpdateMinimum(SeekBar) → UpdateValue: the integer Progress is recomputed
            // from the new bounds.
            push_value(env.get(), widget_of(*platform), view);
        }
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->maximum = view.maximum(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SliderExtensions.UpdateMaximum(SeekBar) → UpdateValue: recompute Progress from the new bounds.
            push_value(env.get(), widget_of(*platform), view);
        }
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // SliderExtensions.UpdateValue: write only when it differs (prevents the native ValueChanged echo
        // from looping — the headless mirror is also the deferred-listener's write-back source).
        if (platform->value != view.value())
        {
            platform->value = view.value(); // headless mirror first (VM-less suite)
        }
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SeekBar.Progress = (int)((value - min) / (max - min) * PlatformMaxValue).
            push_value(env.get(), widget_of(*platform), view);
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->minimum_track_color = view.minimum_track_color(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SliderExtensions.UpdateMinimumTrackColor(SeekBar): ProgressTintList = ColorStateList.ValueOf
            // (the filled side). The `is not null` guard collapses to the color{} sentinel.
            push_tint_list(env.get(), widget_of(*platform), "setProgressTintList", view.minimum_track_color());
        }
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->maximum_track_color = view.maximum_track_color(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SliderExtensions.UpdateMaximumTrackColor(SeekBar): ProgressBackgroundTintList =
            // ColorStateList.ValueOf (the unfilled side).
            push_tint_list(env.get(), widget_of(*platform), "setProgressBackgroundTintList",
                           view.maximum_track_color());
        }
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->thumb_color = view.thumb_color(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SliderExtensions.UpdateThumbColor(SeekBar): C# mutates the live Thumb Drawable's color filter;
            // the port pushes the theme-independent ThumbTintList instead (header deviation), so the tint is
            // observable without depending on a present thumb Drawable.
            push_tint_list(env.get(), widget_of(*platform), "setThumbTintList", view.thumb_color());
        }
    }

    // ---- per-backend ThumbImageSource primitives (the cross-platform map_thumb_image_source routes here) ----
    // The real SetThumb(bitmap-backed Drawable) push (SliderExtensions.UpdateThumbImageSourceAsync) needs
    // the image-service decode + thumb-Drawable surface that is part of the deferred android image/drawable
    // fan-out (the button partial defers its CompoundDrawables icon the same way). These keep only the
    // headless-style mirror (thumb_image_set) so the android preset's pure-native cross-platform suite still
    // observes the load; the real JNI thumb push is deferred.
    void slider_handler::configure_thumb_loader(image_source_loader& /*loader*/)
    {
    }

    void slider_handler::apply_thumb_image(slider_platform& platform, i_slider& /*view*/,
                                           const image_source_result& result)
    {
        // A loaded result records that a thumb image is set; an unloaded one clears the mirror (the
        // SetDefaultThumb else branch). `view` is unused (the iOS recipe tints the image with ThumbColor —
        // SliderExtensions ApplyTintColor — which the deferred android thumb-Drawable push will mirror).
        platform.thumb_image_set = result.loaded();
    }

    void slider_handler::clear_thumb_image(slider_platform& platform, i_slider& view)
    {
        // SliderExtensions.UpdateThumbImageSourceAsync else branch → SetDefaultThumb → UpdateThumbColor:
        // drop the image and restore the thumb color mirror.
        platform.thumb_image_set = false;
        platform.thumb_color = view.thumb_color();
    }

    void slider_handler::map_update_on_tap(slider_handler& handler, i_slider& view)
    {
        // Android deviation: UpdateOnTap is an iOS-Specific (a UISlider tap-to-set workaround). A SeekBar
        // already jumps to the tapped track position on a tap by default, so — like the apple twin — record
        // the resolved flag for parity; no native gesture install is needed.
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_slider_specifics*>(&view);
        if (platform == nullptr || specifics == nullptr)
        {
            return;
        }
        platform->update_on_tap = specifics->update_on_tap();
    }

    maui::graphics::size slider_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric, so the backend-agnostic
            // size-request suites see consistent numbers in the pure-native run (the UISlider natural ~31).
            return {100.0, 31.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost specs
        // in pixels, infinite become Unspecified; View.measure, then the measured pixels come back as dp
        // (Context.FromPixels).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_seek_bar_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_seek_bar_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_seek_bar_class, "getMeasuredHeight", "()I");
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

    void slider_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewExtensions/ViewHandler.PlatformArrange: the dp frame becomes pixels, the view measures
        // Exactly at the final size (Android requires a measure pass before layout) and lays out.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_seek_bar_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_seek_bar_class, "layout", "(IIII)V");
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

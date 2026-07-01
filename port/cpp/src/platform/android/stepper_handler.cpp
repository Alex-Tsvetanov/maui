// stepper_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed for the
// minus/plus numeric stepper. The managed platform view is a REAL MauiStepper — a horizontal
// android.widget.LinearLayout hosting TWO plain android.widget.Buttons ("－" down/minus, "＋" up/plus),
// held as a JNI global reference in stepper_platform::native (the two child Buttons are held as separate
// global references on the platform struct). Minimum/Maximum/Increment/Value/IsEnabled all funnel into
// ONE UpdateButtons pass that sets the two buttons' enabled state (the C# Android stepper has no slider
// thumb to push a value to — the value lives in the cross-platform view; the BUTTONS are what change),
// the background lands on the LinearLayout, and a minus/plus tap is the DEFERRED increment/decrement
// channel (kept as invokable C++ callbacks, like button's deferred touch listener / slider's deferred
// SeekBar listener).
//
// Ported DIRECTLY from StepperHandler.Android.cs + Platform/Android/{MauiStepper.cs (a bare LinearLayout
// subclass), StepperExtensions.cs (UpdateMinimum/Maximum/Increment/Value/IsEnabled all → UpdateButtons),
// StepperHandlerManager.cs (CreateStepperButtons + UpdateButtons + the StepperListener click → Value ±=
// Interval), ViewExtensions.cs (UpdateVisibility/UpdateOpacity/UpdateIsEnabled/UpdateAutomationId),
// ContextExtensions.cs (ToPixels)}.
//
// THE ENABLED LOGIC (read from StepperHandlerManager.UpdateButtons, NOT guessed):
//   downButton.Enabled = stepper.IsEnabled && stepper.Value >  stepper.Minimum;
//   upButton.Enabled   = stepper.IsEnabled && stepper.Value <  stepper.Maximum;
// i.e. the minus button is live only while there is room to go down, the plus only while there is room to
// go up, and BOTH die when the stepper itself is disabled. Every one of MapMinimum / MapMaximum /
// MapIncrement / MapValue / MapIsEnabled re-runs exactly this (StepperExtensions routes all five through
// UpdateButtons), so the port collapses them onto one update_buttons() helper called from each mapper —
// the faithful transcription of the five-extension-methods-one-body shape.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each an infrastructure gap, not a behavior guess):
//   - MauiStepper is a plain android.widget.LinearLayout (Orientation = Horizontal), exactly like C#'s
//     `MauiStepper : LinearLayout` (which overrides nothing). The Focusable / DescendantFocusability /
//     button NextFocusForwardId / Gravity = Center focus-traversal knobs CreatePlatformView and
//     CreateStepperButtons set are accessibility-focus niceties with no observable layout/parity effect;
//     they are applied where they map to a plain setter (setOrientation, the buttons' setGravity), and the
//     focus-id chain is skipped (the port has no id-allocation surface). The down button's
//     SetHeight(ToPixels(10.0)) floor (IAndroidStepperHandler.CreateButton) IS ported.
//   - The minus/plus glyphs are the C# oracle's EXACT characters, copied verbatim (the source's own
//     comment warns they are NOT the keyboard dash/plus): Text "－" (U+FF0D FULLWIDTH HYPHEN-MINUS) /
//     "＋" (U+FF0B FULLWIDTH PLUS), ContentDescription "−" (U+2212 MINUS SIGN) / "+" (U+002B). Faithful to
//     CreateStepperButtons. UpdateButtons in C# re-discovers the two buttons by matching child.Text ==
//     "－" / "＋"; the port holds direct references to the two created Buttons (down_button / up_button on
//     the platform struct), so no text-matching re-scan is needed — an implementation simplification with
//     identical behavior.
//   - The StepperListener click → `VirtualView.Value = Value ± Interval; UpdateButtons()` is the inbound
//     channel. Like button_handler.cpp's OnTouchListener and slider_handler.cpp's OnSeekBarChangeListener,
//     installing a REAL android.view.View.OnClickListener needs a host-provided Java listener class bound
//     via RegisterNatives — the android test host carries only dev.mauicpp.NativeOnClickListener (wired for
//     a single peer; the stepper would need TWO listeners or a per-button peer). So the inbound JNI click
//     is DEFERRED: on_minus / on_plus stay invokable C++ callbacks carrying the StepperListener body (down
//     subtracts Interval, up adds it, then UpdateButtons re-runs), and on_value_changed keeps the headless
//     mirror's write-back shape (StepperProxy.OnValueChanged) so the cross-platform suite drives the seam.
//     When the click trampoline lands, wire it here exactly as button_handler.cpp wires its click listener.
//   - The port's colors are non-nullable value types; the generic-IView background push reuses the shared
//     android apply_background (ViewExtensions.UpdateBackground) on the LinearLayout, the band behind the
//     two buttons — the same collapse the slider/button partials use.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly skips, while the
// headless mirrors (minimum/maximum/increment/value + the base IView mirrors) are ALWAYS maintained — so
// that suite observes exactly the headless partial's behavior, and the widget test host additionally
// observes the real widgets.

#include "maui/core/stepper_handler.hpp"

#include <jni.h>

#include <cmath>
#include <memory>
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
#include "maui/core/i_stepper.hpp"
#include "maui/core/semantics.hpp"
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
    using maui::platform::android::to_jstring;

    // All container instance methods resolve through the LinearLayout's own class (GetMethodID walks the
    // superclasses, so the ViewGroup/View surface — addView/measure/layout/setVisibility/setBackground —
    // resolves through android/widget/LinearLayout). The two child buttons are plain android.widget.Buttons.
    constexpr const char* k_linear_layout_class = "android/widget/LinearLayout";
    constexpr const char* k_button_class = "android/widget/Button";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // android.widget.LinearLayout.HORIZONTAL (setOrientation's target — MauiStepper sets Orientation =
    // AOrientation.Horizontal). LinearLayout.HORIZONTAL == 0.
    constexpr jint k_orientation_horizontal = 0;

    // android.view.Gravity.CENTER — CreateStepperButtons sets each button's Gravity = Center. == 17 (0x11).
    constexpr jint k_gravity_center = 17;

    // ViewGroup.LayoutParams sentinels: WRAP_CONTENT (-2) / MATCH_PARENT (-1). The C# AddView uses
    // `new LinearLayout.LayoutParams(WrapContent, MatchParent, 1)` for each button (weight 1).
    constexpr jint k_wrap_content = -2;
    constexpr jint k_match_parent = -1;

    // IAndroidStepperHandler.CreateButton: button.SetHeight(ToPixels(10.0)) — a 10dp minimum height floor.
    constexpr double k_button_min_height_dp = 10.0;

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

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // The C# oracle's EXACT minus/plus characters — copied verbatim from StepperHandlerManager
    // .CreateStepperButtons (whose own comment warns these are NOT the keyboard dash/plus):
    //   Text "－"  = U+FF0D FULLWIDTH HYPHEN-MINUS (a visually pleasing "minus")
    //   Desc "−"  = U+2212 MINUS SIGN (the phonetically correct minus)
    //   Text "＋"  = U+FF0B FULLWIDTH PLUS (a visually pleasing "plus")
    //   Desc "+"  = U+002B PLUS SIGN
    constexpr const char* k_down_text = "\xEF\xBC\x8D";        // U+FF0D
    constexpr const char* k_down_description = "\xE2\x88\x92"; // U+2212
    constexpr const char* k_up_text = "\xEF\xBC\x8B";          // U+FF0B
    constexpr const char* k_up_description = "+";              // U+002B

    [[nodiscard]] jobject panel_of(const maui::core::stepper_platform& platform) noexcept
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

    void call_void_int(JNIEnv* env, jobject panel, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_linear_layout_class, name, "(I)V"))
        {
            env->CallVoidMethod(panel, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject panel, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_linear_layout_class, name, "(F)V"))
        {
            env->CallVoidMethod(panel, method, value);
            clear_pending(env);
        }
    }

    // setEnabled on a plain Button (the enabled state IS what the stepper pushes — see update_buttons).
    void set_button_enabled(JNIEnv* env, jobject button, bool enabled)
    {
        if (jmethodID method = default_jni_cache().method(env, k_button_class, "setEnabled", "(Z)V"))
        {
            env->CallVoidMethod(button, method, static_cast<jboolean>(enabled));
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The panel's display density (Context.getResources().getDisplayMetrics().density). 1.0 when any step
    // fails. (The shared android view ops memoize this process-wide via ContextExtensions' s_displayDensity
    // cache; the measure/arrange seam reads it directly here, the same four-call walk as the sibling
    // partials.)
    [[nodiscard]] float display_density(JNIEnv* env, jobject panel)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_linear_layout_class, "getContext", "()Landroid/content/Context;");
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
        const local_ref<jobject> context{env, env->CallObjectMethod(panel, get_context)};
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

    // StepperHandlerManager.CreateStepperButtons + IAndroidStepperHandler.CreateButton, collapsed: create a
    // plain android.widget.Button(Context) (Button's (Context) ctor is theme-independent — TextView/Button
    // OK per the android lessons), set its 10dp min-height floor, center gravity, glyph Text +
    // ContentDescription, and a WRAP_CONTENT/MATCH_PARENT/weight-1 LinearLayout.LayoutParams. Returns a
    // local ref (the caller globalizes the kept buttons and adds them to the panel). Empty on any failure.
    [[nodiscard]] local_ref<jobject> create_stepper_button(JNIEnv* env, jobject context, const char* text,
                                                           const char* description, float density)
    {
        auto& cache = default_jni_cache();
        jclass button_class = cache.find_class(env, k_button_class);
        jmethodID ctor = cache.method(env, k_button_class, "<init>", "(Landroid/content/Context;)V");
        if (button_class == nullptr || ctor == nullptr)
        {
            return {};
        }
        local_ref<jobject> button{env, env->NewObject(button_class, ctor, context)};
        if (clear_pending(env) || !button)
        {
            return {};
        }
        // IAndroidStepperHandler.CreateButton: button.SetHeight(ToPixels(10.0)).
        if (jmethodID set_height = cache.method(env, k_button_class, "setHeight", "(I)V"))
        {
            env->CallVoidMethod(button.get(), set_height, to_pixels(k_button_min_height_dp, density));
            clear_pending(env);
        }
        // CreateStepperButtons: button.Gravity = GravityFlags.Center.
        if (jmethodID set_gravity = cache.method(env, k_button_class, "setGravity", "(I)V"))
        {
            env->CallVoidMethod(button.get(), set_gravity, k_gravity_center);
            clear_pending(env);
        }
        // The visually-pleasing glyph Text + the phonetic ContentDescription (the oracle's exact chars).
        if (jmethodID set_text = cache.method(env, k_button_class, "setText", "(Ljava/lang/CharSequence;)V"))
        {
            const local_ref<jstring> raw = to_jstring(env, text);
            env->CallVoidMethod(button.get(), set_text, raw.get());
            clear_pending(env);
        }
        if (jmethodID set_description =
                cache.method(env, k_button_class, "setContentDescription", "(Ljava/lang/CharSequence;)V"))
        {
            const local_ref<jstring> raw = to_jstring(env, description);
            env->CallVoidMethod(button.get(), set_description, raw.get());
            clear_pending(env);
        }
        // new LinearLayout.LayoutParams(WrapContent, MatchParent, 1) — each button gets equal weight.
        jclass params_class = cache.find_class(env, "android/widget/LinearLayout$LayoutParams");
        jmethodID params_ctor = cache.method(env, "android/widget/LinearLayout$LayoutParams", "<init>", "(IIF)V");
        jmethodID set_layout_params =
            cache.method(env, k_button_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (params_class != nullptr && params_ctor != nullptr && set_layout_params != nullptr)
        {
            const local_ref<jobject> params{
                env, env->NewObject(params_class, params_ctor, k_wrap_content, k_match_parent, 1.0F)};
            if (!clear_pending(env) && params)
            {
                env->CallVoidMethod(button.get(), set_layout_params, params.get());
                clear_pending(env);
            }
        }
        return button;
    }

    // StepperHandlerManager.UpdateButtons (the enabled logic — read, not guessed):
    //   downButton.Enabled = stepper.IsEnabled && Value >  Minimum;
    //   upButton.Enabled   = stepper.IsEnabled && Value <  Maximum;
    // Re-run on every Map{Minimum,Maximum,Increment,Value,IsEnabled} (StepperExtensions routes all five
    // here). Taken in scalar form so BOTH the view-driven mappers (which read the live i_stepper) and the
    // struct's update_is_enabled (which has only the headless mirrors — IsEnabled never crosses the struct
    // boundary as an i_stepper) can call it. VM-less safe: a no-op when there is no env or no native buttons.
    void update_buttons(maui::core::stepper_platform& platform, bool is_enabled, double value, double minimum,
                        double maximum)
    {
        if (platform.down_button == nullptr && platform.up_button == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        if (platform.down_button != nullptr)
        {
            set_button_enabled(env.get(), static_cast<jobject>(platform.down_button), is_enabled && value > minimum);
        }
        if (platform.up_button != nullptr)
        {
            set_button_enabled(env.get(), static_cast<jobject>(platform.up_button), is_enabled && value < maximum);
        }
    }

    // The view-driven overload (the five stepper-property mappers): read the live i_stepper.
    void update_buttons(maui::core::stepper_platform& platform, const maui::core::i_stepper& view)
    {
        update_buttons(platform, view.is_enabled(), view.value(), view.minimum(), view.maximum());
    }
} // namespace

namespace maui::core
{
    // Releases the global references pinning the MauiStepper LinearLayout AND its two child Buttons (the
    // JNI shape of the pimpl-owned-native-view doctrine: the apple twin CFReleases its NSStepper here). The
    // buttons are removeAllViews-free because deleting the global refs after the panel drops is sufficient
    // (the panel itself is going away).
    stepper_platform::~stepper_platform()
    {
        if (native == nullptr && down_button == nullptr && up_button == nullptr)
        {
            return;
        }
        const scoped_env env; // any-thread teardown, exactly like global_ref::reset
        if (env)
        {
            if (down_button != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(down_button));
            }
            if (up_button != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(up_button));
            }
            if (native != nullptr)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
        }
        down_button = nullptr;
        up_button = nullptr;
        native = nullptr;
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each calls
    // the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform suite (see
    // the header comment) — then pushes to the real LinearLayout when one exists.

    void stepper_platform::update_visibility(maui::core::visibility value)
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
        call_void_int(env.get(), panel_of(*this), "setVisibility", state);
    }

    void stepper_platform::update_opacity(double value)
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
            call_void_float(env.get(), panel_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void stepper_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // StepperHandler.MapIsEnabled → StepperExtensions.UpdateIsEnabled → UpdateButtons: IsEnabled does
        // NOT land on the LinearLayout — it re-derives the two child buttons' enabled state. The port's
        // shared view_mapper drives the generic "is_enabled" key through THIS override (the stepper handler
        // has no separate map_is_enabled key), so re-run UpdateButtons here, the exact MapIsEnabled landing.
        // Only the headless mirrors are in scope (IsEnabled never crosses as an i_stepper), and they are the
        // values the five stepper mappers keep current — so this is the faithful UpdateButtons input.
        update_buttons(*this, value, this->value, this->minimum, this->maximum);
    }

    void stepper_platform::update_automation_id(std::string_view value)
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
        jobject panel = panel_of(*this);
        auto& cache = default_jni_cache();
        // PlatformInterop.setContentDescriptionForAutomationId: setting a ContentDescription flips
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had, so the automation
        // id does not change the view's accessibility exposure.
        jmethodID get_important = cache.method(env.get(), k_linear_layout_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_linear_layout_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(panel, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(panel, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), panel, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    // Render transform + flow direction + background + semantics pushed to the real LinearLayout via the
    // shared android ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite
    // observes the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void stepper_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void stepper_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void stepper_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // VisualElement.Background paints the MauiStepper LinearLayout's View background via the shared
        // android op (the band behind the two buttons). VM-less safe.
        maui::platform::android::apply_background(native, value);
    }

    void stepper_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<stepper_platform> stepper_handler::create_platform_view()
    {
        auto platform = std::make_unique<stepper_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass layout_class = cache.find_class(env.get(), k_linear_layout_class);
        // LinearLayout(Context) is theme-independent (no style-attr resolution), unlike SeekBar/EditText —
        // so the plain (Context) ctor is safe in the bare app_process test host (the layout_handler partial
        // relies on the same property for its MauiLayout ViewGroup).
        jmethodID ctor = cache.method(env.get(), k_linear_layout_class, "<init>", "(Landroid/content/Context;)V");
        if (layout_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // StepperHandler.CreatePlatformView: new MauiStepper(Context) { Orientation = Horizontal, … }. The
        // port's MauiStepper IS a plain LinearLayout (C#'s adds nothing).
        const local_ref<jobject> panel{env.get(), env->NewObject(layout_class, ctor, context)};
        if (clear_pending(env.get()) || !panel)
        {
            return platform;
        }
        call_void_int(env.get(), panel.get(), "setOrientation", k_orientation_horizontal);
        const float density = display_density(env.get(), panel.get());
        // StepperHandlerManager.CreateStepperButtons: the "－" down + "＋" up buttons.
        const local_ref<jobject> down =
            create_stepper_button(env.get(), context, k_down_text, k_down_description, density);
        const local_ref<jobject> up = create_stepper_button(env.get(), context, k_up_text, k_up_description, density);
        // StepperHandler.CreatePlatformView: stepperLayout.AddView(downButton, …); AddView(upButton, …).
        jmethodID add_view = cache.method(env.get(), k_linear_layout_class, "addView", "(Landroid/view/View;)V");
        if (add_view != nullptr)
        {
            if (down)
            {
                env->CallVoidMethod(panel.get(), add_view, down.get());
                clear_pending(env.get());
            }
            if (up)
            {
                env->CallVoidMethod(panel.get(), add_view, up.get());
                clear_pending(env.get());
            }
        }
        // Wrap-content LayoutParams on the panel up front (parentless View measure/layout safety — the
        // android container fan-out has not arrived; the partial stands in for the parent ViewGroup attach,
        // exactly like the sibling leaf partials).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params = cache.method(env.get(), k_linear_layout_class, "setLayoutParams",
                                                   "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_wrap_content, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(panel.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        // Globalize the kept references (released in ~stepper_platform). The buttons are pinned separately
        // because update_buttons + the deferred click channel address them directly.
        if (down)
        {
            platform->down_button = env->NewGlobalRef(down.get());
        }
        if (up)
        {
            platform->up_button = env->NewGlobalRef(up.get());
        }
        platform->native = env->NewGlobalRef(panel.get());
        return platform;
    }

    void stepper_handler::on_connect_handler(stepper_platform& platform)
    {
        // StepperProxy.OnValueChanged: write the native value back to the virtual view. The headless mirror
        // keeps `value` as the resolved native value; the cross-platform suite drives this seam.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                view->set_value(platform_view->value);
            }
        };
        // StepperHandlerManager.StepperListener.OnClick, split per button — the DEFERRED inbound click
        // channel (no host listener class; see the header). Down subtracts Interval, up adds it, then
        // UpdateButtons re-runs. The callbacks stay invokable even VM-less so the cross-platform suite can
        // drive them (the documented stand-in for the real OnClickListener trampoline).
        platform.on_minus = [this] {
            auto* view = virtual_view();
            if (view != nullptr)
            {
                view->set_value(view->value() - view->interval()); // increment = -Interval (down button)
                if (auto* platform_view = typed_platform_view())
                {
                    update_buttons(*platform_view, *view);
                }
            }
        };
        platform.on_plus = [this] {
            auto* view = virtual_view();
            if (view != nullptr)
            {
                view->set_value(view->value() + view->interval()); // increment = +Interval (up button)
                if (auto* platform_view = typed_platform_view())
                {
                    update_buttons(*platform_view, *view);
                }
            }
        };
        // The real SetOnClickListener install on each button is deferred (no host listener class) — when the
        // stepper click trampoline lands, wire it here exactly as button_handler.cpp wires its click
        // listener (RegisterNatives + NewObject(listener, peer) + setOnClickListener on each button).
    }

    void stepper_handler::on_disconnect_handler(stepper_platform& platform)
    {
        // StepperListener uninstall is deferred with the listener install (header note); drop the callbacks.
        platform.on_value_changed = nullptr;
        platform.on_minus = nullptr;
        platform.on_plus = nullptr;
    }

    void stepper_handler::map_increment(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateIncrement → UpdateButtons. Only a positive increment is meaningful on the
        // native step (the headless mirror keeps the same positive-only gate), and the buttons' enabled
        // state is then re-derived.
        if (auto* platform = handler.typed_platform_view())
        {
            if (view.interval() > 0)
            {
                platform->increment = view.interval(); // headless mirror first (VM-less suite)
            }
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_minimum(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateMinimum → UpdateButtons.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum = view.minimum(); // headless mirror first (VM-less suite)
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_maximum(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateMaximum → UpdateButtons.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum = view.maximum(); // headless mirror first (VM-less suite)
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_value(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateValue → UpdateButtons. The headless mirror keeps the UpdateValue body
        // (refresh the minimum mirror first so a stale higher minimum does not skew the comparison, then
        // write when the value differs), and the buttons' enabled state is re-derived.
        if (auto* platform = handler.typed_platform_view())
        {
            if (platform->minimum != view.minimum())
            {
                platform->minimum = view.minimum();
            }
            if (platform->value != view.value())
            {
                platform->value = view.value();
            }
            update_buttons(*platform, view);
        }
    }

    void stepper_handler::map_flow_direction(stepper_handler& handler, i_stepper& view)
    {
        // StepperHandler.MapFlowDirection: apply the RESOLVED direction (the MatchParent → parent-IView
        // fallback) to the MauiStepper LinearLayout's userInterfaceLayoutDirection (apply_flow_direction)
        // and mirror it. The android LinearLayout honors the layout direction directly (the iOS-26 subview
        // re-application has no android counterpart). Mirrors the slider/progress_bar android twins.
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved; // headless mirror first (VM-less suite)
        if (platform->native != nullptr)
        {
            maui::platform::android::apply_flow_direction(platform->native, resolved);
        }
    }

    maui::graphics::size stepper_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (the UIStepper natural 94x32),
            // so the backend-agnostic size-request suites see consistent numbers in the pure-native run.
            return {94.0, 32.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject panel = panel_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost specs
        // in pixels, infinite become Unspecified; View.measure (the LinearLayout measures its two buttons),
        // then the measured pixels come back as dp (Context.FromPixels).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_linear_layout_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_linear_layout_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_linear_layout_class, "getMeasuredHeight", "()I");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || get_measured_width == nullptr ||
            get_measured_height == nullptr || measure_spec_class == nullptr)
        {
            return {0, 0};
        }
        const float density = display_density(env.get(), panel);
        const auto spec_for = [&](double constraint) -> jint {
            const jint size = std::isfinite(constraint) ? to_pixels(constraint, density) : 0;
            const jint mode = std::isfinite(constraint) ? k_measure_spec_at_most : k_measure_spec_unspecified;
            const jint spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, size, mode);
            return clear_pending(env.get()) ? 0 : spec;
        };
        const jint width_spec = spec_for(width_constraint);
        const jint height_spec = spec_for(height_constraint);
        env->CallVoidMethod(panel, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        const jint measured_width = env->CallIntMethod(panel, get_measured_width);
        const jint measured_height = env->CallIntMethod(panel, get_measured_height);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        return {static_cast<double>(measured_width) / density, static_cast<double>(measured_height) / density};
    }

    void stepper_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless / VM-less: no native panel to position
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject panel = panel_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange (ViewHandlerExtensions.PlatformArrangeHandler, Android): the dp frame
        // becomes pixels and the platform view is LAID OUT at the final size — MAUI calls **only**
        // `platformView.Layout(left, top, right, bottom)`; it does NOT re-`measure()` here. The measure state
        // it lays out from is the prior AtMost/wrap pass (GetDesiredSizeFromHandler → CreateMeasureSpec), so
        // the MauiStepper's two `WrapContent`-width, weight-1 buttons keep their NATURAL widths and the
        // LinearLayout left-packs them inside the (Fill) frame — the red BackgroundColor then shows in the
        // remaining space. On-device (uiautomator) proof: the stepper LinearLayout is 1014px (Fill) yet each
        // button is only ~242px (the Button minWidth), not stretched.
        //
        // The port previously re-`measure()`d the LinearLayout **Exactly** at the frame width right before
        // layout (the generic leaf-handler two-step). For a plain View that is byte-identical, but a
        // LinearLayout distributes its weight EXCESS during an EXACTLY measure — so the Exactly re-measure
        // stretched the two weight-1 buttons to 50/50 and buried the red band (the parity RED). Mirror MAUI:
        // measure AtMost (which does NOT distribute weight-excess — same as the natural GetDesiredSize pass),
        // guaranteeing a valid measured state without stretching, then layout at the full frame.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_linear_layout_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_linear_layout_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), panel);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        // AtMost (NOT Exactly): the LinearLayout measures its two buttons at their natural WrapContent widths
        // and does not spread the weight excess across them — the buttons stay compact, exactly like MAUI's
        // GetDesiredSize AtMost pass that PlatformArrangeHandler lays out from.
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_at_most);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_at_most);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(panel, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(panel, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core

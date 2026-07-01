// radio_button_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed for
// the mutually-exclusive radio button. The managed platform view is a REAL android.widget.RadioButton
// (held as a JNI global reference in radio_button_platform::native). A RadioButton IS a CompoundButton
// (the check_box's superclass) AND a TextView, so it carries BOTH seams: IsChecked maps onto
// setChecked(bool) / isChecked read-back (the CompoundButton toggle, exactly as check_box), while
// Content / TextColor / Font / CharacterSpacing map onto the TextView text surface (setText /
// setTextColor / setTypeface+setTextSize / setLetterSpacing, exactly as label). The native CheckedChange
// channel is DEFERRED — the C++ on_select hook stays invokable so the VM-less cross-platform suite drives
// it, but no real OnCheckedChangeListener is installed (see below).
//
// Ported DIRECTLY from RadioButtonHandler.Android.cs + Platform/Android/{RadioButtonExtensions.cs
// (UpdateIsChecked/UpdateContent/UpdateBackground/UpdateStroke*/UpdateCornerRadius/UpdateBorderDrawable),
// TextViewExtensions.cs (UpdateTextColor/UpdateFont/UpdateCharacterSpacing), ViewExtensions.cs
// (UpdateVisibility/UpdateOpacity/UpdateIsEnabled/UpdateAutomationId/UpdateBackground), ContextExtensions.cs
// (ToPixels)} + Fonts/FontManager.Android.cs (the CreateTypeface / GetFontSize tail shared verbatim with
// label/button).
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each a library or infrastructure gap, not a behavior
// guess — the button/label/check_box partials document the same set):
//   - The widget is a plain android.widget.RadioButton, not AndroidX.AppCompat.Widget.AppCompatRadioButton
//     (the C# CreatePlatformView's `new AppCompatRadioButton(Context)`): the AppCompat library is a
//     gradle/AAR dependency this APK-less backend does not carry — exactly the plain-widget deviation
//     button_handler.cpp documents for the AppCompatButton / check_box_handler.cpp for the MaterialCheckBox.
//     A plain RadioButton is itself a CompoundButton + TextView, so every mapped seam below is on the View
//     directly (no AppCompat shim). C#'s `SoundEffectsEnabled = false` is ported (a plain View property).
//   - map_text_color: TextViewExtensions.UpdateTextColor's `textColor != null` guard collapses (the port's
//     color is a non-nullable value type), exactly as in the label/button partials — the valueOf/ToPlatform
//     path is always taken. RadioButton's text-color is the TextView SetTextColor (the same one label uses).
//   - PARITY (the selectable-circle glyph color): RadioButtonExtensions.cs NEVER tints the buttonDrawable —
//     the dot/ring color comes entirely from the theme (colorControlActivated / colorControlNormal). Under
//     this AAR-less host's framework Theme.DeviceDefault.Light that resolves to the DeviceDefault INDIGO
//     (checked dot navy #495D92, unchecked ring bluish #45464F); real MAUI's AppCompatRadioButton under
//     Theme.MaterialComponents.DayNight resolves to the Material light GRAYS (checked dot #E0E0E0, unchecked
//     ring #666666 — measured off the maui-compare baseline). create_platform_view therefore seeds a
//     two-state ColorStateList (checked→#E0E0E0, unchecked→#666666) on setButtonTintList
//     (seed_default_material_button_tint) so the UNSET glyph reproduces MAUI's RENDERED default. This is NOT
//     a semantic deviation — MAUI leaves the native default *because that default is already these grays*;
//     the port pins the equivalent grays because its host theme's default is not. Nothing overrides it (no
//     dot-color property); the stroke/corner/background mappers touch the SEPARATE background drawable, so
//     they compose independently. The radio twin of the slider fix (bbb632f301).
//   - The CheckedChange listener (CompoundButton.CheckedChange += OnCheckChanged → VirtualView.IsChecked =
//     e.IsChecked) is DEFERRED with the gesture/event fan-out, exactly as check_box: there is no
//     host-provided OnCheckedChangeListener Java class (the test host ships only dev.mauicpp's button click
//     listener). on_select stays a wired, invokable C++ callback carrying OnCheckChanged's body — the
//     cross-platform suite drives it directly, exactly as the headless mirror does — but the real widget's
//     setOnCheckedChangeListener is not installed. A programmatic setChecked still reaches the widget
//     (virtual→native) and reads back through isChecked. A radio tap SELECTS (send_is_checked(true)); the
//     group's mutual exclusion unchecks the others at the Controls layer (radio_button_group), never here.
//   - The stroke / corner-radius / background BORDER (RadioButtonExtensions.UpdateBorderDrawable: a custom
//     Microsoft.Maui.Platform.BorderDrawable set as the widget Background, carrying SetBackground /
//     SetBorderBrush / SetBorderWidth / SetCornerRadius) is expressed with the SAME GradientDrawable
//     stand-in the button partial uses (src/platform/android/button_handler.cpp's update_button_stroke):
//     ONE android.graphics.drawable.GradientDrawable installed as the RadioButton's BACKGROUND, with
//       - the generic IView background paint → GradientDrawable.setColor(argb)        (the fill)
//       - i_radio_button::stroke_color()     → GradientDrawable.setStroke(widthPx, argb) (the border brush)
//       - i_radio_button::stroke_thickness() → that setStroke width (dp → px via ToPixels)
//       - i_radio_button::corner_radius()    → GradientDrawable.setCornerRadius(px)    (the rounded corner)
//     The RadioButton's selectable-circle glyph is its buttonDrawable (a SEPARATE slot from background), so
//     installing a GradientDrawable as the background draws the border around the whole widget WITHOUT
//     removing the circle — exactly the iOS reference (a colored rounded border around the whole option
//     row, the circle glyph intact). The drawable is installed LAZILY — only once the radio carries a
//     visible stroke, a positive corner radius, or a background paint — so an untouched radio keeps its
//     default theme background. Lossy only for the BorderDrawable surface at large (gradient brushes, dash
//     patterns, per-corner-distinct radii) the GradientDrawable cannot express, the same scope the button /
//     border partials document. The headless mirrors (stroke_color / stroke_thickness / corner_radius) are
//     kept ALWAYS live so the VM-less cross-platform suite still observes them.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly skips, while
// the headless mirrors (is_checked / content / text_color / text_font / character_spacing / stroke_color /
// stroke_thickness / corner_radius + the base IView mirrors) are ALWAYS maintained — so that suite observes
// exactly the headless partial's behavior, and the widget test host additionally observes the real widget.

#include "maui/core/radio_button_handler.hpp"

#include <jni.h>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::core::font;
    using maui::core::font_slant;
    using maui::core::font_weight;
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // All instance methods are resolved through the widget's own class (GetMethodID walks the
    // superclasses, so the View/TextView/CompoundButton surface resolves through android/widget/RadioButton).
    constexpr const char* k_radio_button_class = "android/widget/RadioButton";
    constexpr const char* k_style_class = "android/R$style";
    // android.R.style.Widget_CompoundButton_RadioButton — the concrete platform style that carries the
    // radio buttonDrawable (the selectable-circle glyph). Resolved theme-independently as a defStyleRes so
    // the bare app_process testhost (and the app host) construct a radio button that actually HAS its
    // circle glyph (defStyleAttr=0 with no defStyleRes resolves no buttonDrawable → an invisible,
    // drawable-less circle, label only).
    constexpr const char* k_radio_button_style_field = "Widget_CompoundButton_RadioButton";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";
    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    // android.R.attr — the attribute-id namespace; state_checked keys the CHECKED row of the buttonDrawable
    // tint ColorStateList (see seed_default_material_button_tint). Read theme-independently with
    // GetStaticFieldID, exactly like k_style_field above.
    constexpr const char* k_attr_class = "android/R$attr";
    constexpr const char* k_state_checked_field = "state_checked";

    // The Material light-theme RadioButton default buttonDrawable colors (PARITY, the radio twin of the
    // slider's seed_default_material_tints fix, commit bbb632f301). The radio's selectable-circle glyph
    // (buttonDrawable) has NO explicit tint in RadioButtonExtensions.cs — its color comes ENTIRELY from the
    // theme: real .NET MAUI renders an AppCompatRadioButton under Theme.MaterialComponents.DayNight, whose
    // buttonDrawable resolves through colorControlNormal (unchecked ring) / colorControlActivated (checked
    // dot) to the Material light GRAYS — measured off the maui-compare baseline
    // (docs/comparison/android/maui/radio_button_border.png): the CHECKED inner dot is #E0E0E0 and the
    // UNCHECKED ring is #666666. This AAR-less app host uses the framework Theme.DeviceDefault.Light, whose
    // colorControlActivated is the DeviceDefault INDIGO and whose colorControlNormal is a bluish dark gray —
    // so a bare RadioButton draws its checked dot in dark navy (#495D92 measured) and its unchecked ring in
    // a bluish #45464F, NOT MAUI's neutral grays. That accent mismatch is the sole cause of the "selected
    // radio dot solid navy/blue vs MAUI's light gray dot" parity diff (radio_button_border / input_controls
    // and every radio page). Since RadioButtonExtensions never tints the buttonDrawable, seeding a two-state
    // ColorStateList (checked→#E0E0E0, unchecked→#666666) on setButtonTintList reproduces MAUI's RENDERED
    // default without any semantic deviation — MAUI leaves the native default *because that default is
    // already these grays*; the port pins the equivalent grays because its host theme's default is not.
    // Nothing overrides this (the radio exposes no dot-color property); the stroke/corner/background mappers
    // touch the separate BACKGROUND drawable, not the buttonDrawable, so they compose independently.
    constexpr int k_material_radio_dot_gray = 0xE0;  // #E0E0E0 — MAUI default checked dot (measured)
    constexpr int k_material_radio_ring_gray = 0x66; // #666666 — MAUI default unchecked ring (measured)
    // The maui-managed border drawable class (the GradientDrawable stand-in for the custom BorderDrawable
    // — header deviation; the same one button_handler.cpp / border_handler.cpp install).
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";

    constexpr float k_default_font_size = 14.0F;         // FontManager.DefaultFontSize (14sp)
    constexpr float k_em_coefficient = 0.0624F;          // UnitExtensions.EmCoefficient (CharacterSpacing.ToEm)
    constexpr double k_to_pixels_epsilon = 0.0000000001; // GeometryUtil.Epsilon (ContextExtensions.ToPixels)

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (restored after setContentDescription).
    constexpr jint k_important_for_accessibility_auto = 0;

    constexpr jint k_typeface_normal = 0; // android.graphics.Typeface styles
    constexpr jint k_typeface_bold = 1;
    constexpr jint k_typeface_italic = 2;
    constexpr jint k_complex_unit_dip = 1; // android.util.TypedValue complex units
    constexpr jint k_complex_unit_sp = 2;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::radio_button_platform& platform) noexcept
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
        if (jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_radio_button_class, name, "(Z)V"))
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
    // step fails. (The shared android view ops memoize this process-wide; the measure/arrange seam reads
    // it directly here, the same walk check_box/label/button use.)
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_radio_button_class, "getContext", "()Landroid/content/Context;");
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

    // The maui-managed GradientDrawable carrying background fill + stroke + corner radius (the plain-widget
    // stand-in for the custom BorderDrawable — header deviation). Returns the installed one (the
    // RadioButton's getBackground() instanceof GradientDrawable identifies OURS; the default theme
    // background is not a GradientDrawable), installing a fresh one only when `install` is set. An empty
    // ref means "not installed and not asked to install" (or a JNI failure). The exact mirror of
    // button_handler.cpp's maui_background_drawable, kept standalone so the partials stay independently
    // buildable.
    [[nodiscard]] local_ref<jobject> maui_border_drawable(JNIEnv* env, jobject widget, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID get_background =
            cache.method(env, k_radio_button_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        if (gradient_class == nullptr || get_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> current{env, env->CallObjectMethod(widget, get_background)};
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
            cache.method(env, k_radio_button_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (ctor == nullptr || set_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> fresh{env, env->NewObject(gradient_class, ctor)};
        if (clear_pending(env) || !fresh)
        {
            return {};
        }
        env->CallVoidMethod(widget, set_background, fresh.get());
        if (clear_pending(env))
        {
            return {};
        }
        return fresh;
    }

    // Seed the just-created RadioButton's buttonDrawable (the selectable circle) with a two-state Material
    // ColorStateList — checked→#E0E0E0 (the light inner dot), unchecked→#666666 (the neutral ring) — via
    // CompoundButton.setButtonTintList, so its UNSET glyph matches real MAUI instead of the DeviceDefault
    // navy accent / bluish normal (see the k_material_radio_* note). The RadioButton is a CompoundButton,
    // so setButtonTintList is on the widget directly (no CompoundButtonCompat shim — the same plain-widget
    // path check_box_handler.cpp's UpdateForeground takes). Called once at construction; nothing overrides
    // it (the radio has no dot-color property). state_checked is read theme-independently with
    // GetStaticFieldID. Best-effort: any JNI resolution failure leaves the theme default (the widget still
    // renders — only the accent-vs-gray tint would remain).
    void seed_default_material_button_tint(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        // state_checked (android.R.attr.state_checked) keys the CHECKED row; the empty {} row is the
        // catch-all default (unchecked). A ColorStateList matches the FIRST row whose states are all set on
        // the view, so [ {state_checked}, {} ] gives checked→dot, everything-else→ring.
        jclass attr_class = cache.find_class(env, k_attr_class);
        jfieldID state_checked_field =
            attr_class != nullptr ? env->GetStaticFieldID(attr_class, k_state_checked_field, "I") : nullptr;
        clear_pending(env);
        jclass color_state_list_class = cache.find_class(env, k_color_state_list_class);
        jmethodID csl_ctor = cache.method(env, k_color_state_list_class, "<init>", "([[I[I)V");
        jmethodID set_button_tint =
            cache.method(env, k_radio_button_class, "setButtonTintList", "(Landroid/content/res/ColorStateList;)V");
        if (attr_class == nullptr || state_checked_field == nullptr || color_state_list_class == nullptr ||
            csl_ctor == nullptr || set_button_tint == nullptr)
        {
            return;
        }
        const jint state_checked = env->GetStaticIntField(attr_class, state_checked_field);
        if (clear_pending(env))
        {
            return;
        }
        // states = int[2][]: row 0 = { state_checked }, row 1 = {} (empty = default/unchecked).
        jclass int_array_class = cache.find_class(env, "[I");
        if (int_array_class == nullptr)
        {
            return;
        }
        const local_ref<jobjectArray> states{env, env->NewObjectArray(2, int_array_class, nullptr)};
        if (clear_pending(env) || !states)
        {
            return;
        }
        const local_ref<jintArray> checked_row{env, env->NewIntArray(1)};
        const local_ref<jintArray> default_row{env, env->NewIntArray(0)};
        if (clear_pending(env) || !checked_row || !default_row)
        {
            return;
        }
        env->SetIntArrayRegion(checked_row.get(), 0, 1, &state_checked);
        if (clear_pending(env))
        {
            return;
        }
        env->SetObjectArrayElement(states.get(), 0, checked_row.get());
        env->SetObjectArrayElement(states.get(), 1, default_row.get());
        if (clear_pending(env))
        {
            return;
        }
        // colors = int[2]: { dot (checked), ring (unchecked) } — parallel to the states rows.
        const jint dot =
            static_cast<jint>(maui::graphics::color::from_rgb(k_material_radio_dot_gray, k_material_radio_dot_gray,
                                                              k_material_radio_dot_gray)
                                  .to_int());
        const jint ring =
            static_cast<jint>(maui::graphics::color::from_rgb(k_material_radio_ring_gray, k_material_radio_ring_gray,
                                                              k_material_radio_ring_gray)
                                  .to_int());
        const std::array<jint, 2> color_values = {dot, ring};
        const local_ref<jintArray> colors{env, env->NewIntArray(2)};
        if (clear_pending(env) || !colors)
        {
            return;
        }
        env->SetIntArrayRegion(colors.get(), 0, 2, color_values.data());
        if (clear_pending(env))
        {
            return;
        }
        const local_ref<jobject> tint_list{
            env, env->NewObject(color_state_list_class, csl_ctor, states.get(), colors.get())};
        if (clear_pending(env) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_button_tint, tint_list.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.RadioButton (the JNI shape of the
    // pimpl-owned-native-view doctrine: the apple twin releases its NSButton/UIButton here).
    radio_button_platform::~radio_button_platform()
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

#ifdef MAUI_PLATFORM_ANDROID
    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform
    // suite (header comment) — then pushes to the real widget when one exists.

    void radio_button_platform::update_visibility(maui::core::visibility value)
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

    void radio_button_platform::update_opacity(double value)
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

    void radio_button_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateIsEnabled: platformView.Enabled = view.IsEnabled.
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void radio_button_platform::update_automation_id(std::string_view value)
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
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had, so the
        // automation id does not change the view's accessibility exposure.
        jmethodID get_important = cache.method(env.get(), k_radio_button_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_radio_button_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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
    // android ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite
    // observes the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void radio_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void radio_button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void radio_button_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // RadioButtonExtensions.UpdateBackground → UpdateBorderDrawable installs ONE drawable carrying BOTH
        // the background fill AND the stroke/corner border. The port expresses that drawable as the shared
        // GradientDrawable stand-in (header deviation): the IView background paint lands as the drawable's
        // setColor, the SAME drawable the stroke mappers push setStroke/setCornerRadius onto — so a Border
        // radio's yellow fill and its red/green rounded stroke compose in one drawable. A null paint clears
        // OUR drawable's fill to transparent when one is installed; an untouched radio never installs one.
        const local_ref<jobject> drawable =
            maui_border_drawable(env.get(), widget_of(*this), /*install=*/value != nullptr);
        if (!drawable)
        {
            return;
        }
        jmethodID set_color = default_jni_cache().method(env.get(), k_gradient_drawable_class, "setColor", "(I)V");
        if (set_color != nullptr)
        {
            const jint argb = value != nullptr ? static_cast<jint>(value->background_color().to_int()) : 0;
            env->CallVoidMethod(drawable.get(), set_color, argb);
            clear_pending(env.get());
        }
    }

    void radio_button_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }
#endif

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass radio_button_class = cache.find_class(env.get(), k_radio_button_class);
        if (radio_button_class == nullptr)
        {
            return platform;
        }
        // RadioButtonHandler.CreatePlatformView wants `new AppCompatRadioButton(Context)` — a plain
        // RadioButton here (AppCompat deviation). `new RadioButton(Context)` resolves the
        // `radioButtonStyle` theme attr against the Context's THEME, which the app_process widget test host
        // (a bare, Activity-less Context) does not carry, so that ctor may THROW → null widget (the same
        // shape check_box / progress_bar document).
        //
        // The 3-arg (Context, AttributeSet, int defStyleAttr) ctor with defStyleAttr=0 constructs fine, but
        // with NO defStyleRes it resolves NO buttonDrawable — so the radio renders its label but the
        // selectable-CIRCLE glyph is INVISIBLE (the exact missing-glyph bug the slider's thumb/track hit,
        // fixed there with defStyleRes=Widget_SeekBar; identical to the check_box/switch fix in this wave).
        // So construct THEME-INDEPENDENTLY via the 4-arg (Context, AttributeSet, int defStyleAttr, int
        // defStyleRes) ctor with defStyleAttr=0 and defStyleRes =
        // android.R.style.Widget_CompoundButton_RadioButton (a concrete style resource that CARRIES the
        // buttonDrawable — read with GetStaticFieldID since it is a static field). Then fall back to the
        // 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so the widget is never null.
        jobject created = nullptr;
        jmethodID ctor_styled = cache.method(env.get(), k_radio_button_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        jfieldID style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_radio_button_style_field, "I") : nullptr;
        clear_pending(env.get());
        if (ctor_styled != nullptr && style_class != nullptr && style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(radio_button_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_attr = cache.method(env.get(), k_radio_button_class, "<init>",
                                               "(Landroid/content/Context;Landroid/util/AttributeSet;I)V");
            if (ctor_attr != nullptr)
            {
                created = env->NewObject(radio_button_class, ctor_attr, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0));
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain =
                cache.method(env.get(), k_radio_button_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(radio_button_class, ctor_plain, context);
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
        // CreatePlatformView's object-initializer { SoundEffectsEnabled = false } (a plain-View property —
        // the AppCompat wrapper is the only deviation).
        call_void_bool(env.get(), widget.get(), "setSoundEffectsEnabled", JNI_FALSE);
        // Wrap-content LayoutParams up front (parentless View measure/layout safety — the android
        // container fan-out has not arrived; the partial stands in for the parent ViewGroup attach,
        // exactly like button_handler.cpp / check_box_handler.cpp do).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params = cache.method(env.get(), k_radio_button_class, "setLayoutParams",
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
        // PARITY: seed the buttonDrawable (the selectable circle) with MAUI's two-state Material grays
        // (checked dot #E0E0E0 / unchecked ring #666666) so the UNSET glyph matches real MAUI instead of the
        // DeviceDefault navy accent this bare-framework host inherits (see the k_material_radio_* /
        // seed_default_material_button_tint note). Nothing overrides it — the radio exposes no dot-color
        // property. The radio twin of the slider fix (bbb632f301).
        seed_default_material_button_tint(env.get(), widget.get());
        platform->native = env->NewGlobalRef(widget.get()); // released in ~radio_button_platform
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        // RadioButtonHandler.OnCheckChanged: a native tap CHECKS the button → VirtualView.IsChecked = true
        // (a radio tap SELECTS; it never unchecks itself — the group unchecks the others at the Controls
        // layer). The callback stays wired even VM-less so the cross-platform suite (and the headless
        // mirror) can drive it; the real CompoundButton.CheckedChange listener install is DEFERRED with the
        // event fan-out (no host-provided listener Java class exists for it — header deviations).
        platform.on_select = [this] {
            if (auto* view = virtual_view())
            {
                view->send_is_checked(true);
            }
        };
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        platform.on_select = nullptr;
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked(); // headless mirror first (VM-less suite) + the read-back state
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // RadioButtonExtensions.UpdateIsChecked: platformRadioButton.Checked = radioButton.IsChecked.
            call_void_bool(env.get(), widget_of(*platform), "setChecked", static_cast<jboolean>(view.is_checked()));
        }
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->content = std::string(view.content_as_string()); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // RadioButtonExtensions.UpdateContent: platformRadioButton.Text = $"{radioButton.Content}". The
        // port's content is already the ContentAsString() string (the string-content deviation), so the
        // interpolation is the identity — set it on the TextView surface (setText resolves through
        // RadioButton).
        jmethodID set_text =
            default_jni_cache().method(env.get(), k_radio_button_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr)
        {
            const local_ref<jstring> text = to_jstring(env.get(), view.content_as_string());
            env->CallVoidMethod(widget_of(*platform), set_text, text.get());
            clear_pending(env.get());
        }
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // TextViewExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()). C#'s null guard
            // collapses (non-nullable color), exactly as in the label/button/apple partials.
            call_void_int(env.get(), widget_of(*platform), "setTextColor",
                          static_cast<jint>(view.text_color().to_int()));
        }
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // TextViewExtensions.UpdateCharacterSpacing: LetterSpacing = CharacterSpacing.ToEm().
            call_void_float(env.get(), widget_of(*platform), "setLetterSpacing",
                            static_cast<jfloat>(view.character_spacing()) * k_em_coefficient);
        }
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font(); // headless mirror first (VM-less suite)
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
        const font value = view.font();

        // FontManager.GetTypeface → CreateTypeface (the non-registered-family tail; header): base =
        // Typeface.create(family, ToTypefaceStyle(weight, italic)), then the API-28+ refinement
        // Typeface.create(base, weight, italic). Identical to the label/button partials.
        const bool italic = value.slant() != font_slant::normal;
        const bool bold = value.weight() >= font_weight::bold;
        jint style = k_typeface_normal;
        if (bold && italic)
        {
            style = k_typeface_bold | k_typeface_italic;
        }
        else if (bold)
        {
            style = k_typeface_bold;
        }
        else if (italic)
        {
            style = k_typeface_italic;
        }
        jmethodID create_named = cache.static_method(env.get(), k_typeface_class, "create",
                                                     "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
        jmethodID create_weighted = cache.static_method(env.get(), k_typeface_class, "create",
                                                        "(Landroid/graphics/Typeface;IZ)Landroid/graphics/Typeface;");
        jmethodID set_typeface =
            cache.method(env.get(), k_radio_button_class, "setTypeface", "(Landroid/graphics/Typeface;)V");
        jclass typeface_class = cache.find_class(env.get(), k_typeface_class);
        if (create_named == nullptr || create_weighted == nullptr || set_typeface == nullptr ||
            typeface_class == nullptr)
        {
            return;
        }
        local_ref<jstring> family; // empty family() ⇒ C# null Family ⇒ Typeface.create(null, …) = default
        if (!value.family().empty())
        {
            family = to_jstring(env.get(), value.family());
        }
        const local_ref<jobject> base{env.get(),
                                      env->CallStaticObjectMethod(typeface_class, create_named, family.get(), style)};
        if (clear_pending(env.get()) || !base)
        {
            return;
        }
        const local_ref<jobject> typeface{
            env.get(), env->CallStaticObjectMethod(typeface_class, create_weighted, base.get(),
                                                   static_cast<jint>(value.weight()), static_cast<jboolean>(italic))};
        if (clear_pending(env.get()) || !typeface)
        {
            return;
        }
        env->CallVoidMethod(widget, set_typeface, typeface.get());
        if (clear_pending(env.get()))
        {
            return;
        }

        // FontManager.GetFontSize: size ≤ 0 / NaN → DefaultFontSize; AutoScalingEnabled picks Sp vs Dip.
        auto size = static_cast<float>(value.size());
        if (!(size > 0) || std::isnan(size))
        {
            size = k_default_font_size;
        }
        const jint unit = value.auto_scaling_enabled() ? k_complex_unit_sp : k_complex_unit_dip;
        jmethodID set_text_size = cache.method(env.get(), k_radio_button_class, "setTextSize", "(IF)V");
        if (set_text_size != nullptr)
        {
            env->CallVoidMethod(widget, set_text_size, unit, static_cast<jfloat>(size));
            clear_pending(env.get());
        }
    }

    namespace
    {
        // RadioButtonExtensions.UpdateStroke*/UpdateCornerRadius all funnel into UpdateBorderDrawable — the
        // three properties are intertwined in one drawable (the stroke needs the radius for its rounded
        // outline, the radius needs the stroke width). So C# routes MapStrokeColor / MapStrokeThickness /
        // MapCornerRadius through this ONE update; the plain-widget cut pushes setStroke(width, color) +
        // setCornerRadius onto the maui GradientDrawable (the same drawable update_background fills),
        // installing it only once a stroke or radius is actually visible (header deviations; mirrors
        // button_handler.cpp's update_button_stroke). The headless mirrors are written by the mappers FIRST
        // (the VM-less suite observes them) — this is the native half only.
        void update_radio_stroke(radio_button_handler& handler, i_radio_button& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->native == nullptr)
            {
                return;
            }
            const scoped_env env;
            if (!env)
            {
                return;
            }
            jobject widget = widget_of(*platform);
            // GetStrokeProperties with the port's non-nullable color: thickness < 0 → width 0; radius < 0 →
            // 0. A border is "visible" once it strokes or rounds; an unstroked, square radio installs no
            // drawable (keeps its default theme background — the lazy-install gate).
            const double thickness = view.stroke_thickness() >= 0 ? view.stroke_thickness() : 0;
            const int radius = view.corner_radius() >= 0 ? view.corner_radius() : 0;
            const bool visible = thickness > 0 || radius > 0;
            const local_ref<jobject> drawable = maui_border_drawable(env.get(), widget, /*install=*/visible);
            if (!drawable)
            {
                return;
            }
            auto& cache = default_jni_cache();
            jmethodID set_stroke = cache.method(env.get(), k_gradient_drawable_class, "setStroke", "(II)V");
            jmethodID set_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "setCornerRadius", "(F)V");
            if (set_stroke == nullptr || set_corner_radius == nullptr)
            {
                return;
            }
            const float density = display_density(env.get(), widget);
            // StrokeExtensions.UpdateStrokeColor/Thickness → setStroke(widthPx, argb). A 0-thickness stroke
            // is an invisible 0-width border, matching C#'s DefaultStrokeThicknessNoColor.
            env->CallVoidMethod(drawable.get(), set_stroke, to_pixels(thickness, density),
                                static_cast<jint>(view.stroke_color().to_int()));
            if (clear_pending(env.get()))
            {
                return;
            }
            env->CallVoidMethod(drawable.get(), set_corner_radius,
                                static_cast<jfloat>(to_pixels(static_cast<double>(radius), density)));
            clear_pending(env.get());
        }
    } // namespace

    // The stroke / corner-radius mappers mirror the property into the headless slot FIRST (the VM-less
    // suite observes it), then push the GradientDrawable border via update_radio_stroke (the JNI half).

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_color = view.stroke_color();
            update_radio_stroke(handler, view); // RadioButtonExtensions.UpdateStrokeColor → UpdateBorderDrawable
        }
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_thickness = view.stroke_thickness();
            update_radio_stroke(handler, view); // RadioButtonExtensions.UpdateStrokeThickness → UpdateBorderDrawable
        }
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->corner_radius = view.corner_radius();
            update_radio_stroke(handler, view); // RadioButtonExtensions.UpdateCornerRadius → UpdateBorderDrawable
        }
    }

    maui::graphics::size radio_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial reports {0,0} (no native control), so the
            // backend-agnostic size-request suites see the consistent headless number.
            return {0, 0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost
        // specs in pixels, infinite become Unspecified; View.measure, then the measured pixels come back
        // as dp (Context.FromPixels). RadioButtonHandler.Android.PlatformArrange's PrepareForTextViewArrange
        // adjustment is on the ARRANGE path (deferred — see platform_arrange), not the measure path.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_radio_button_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_radio_button_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_radio_button_class, "getMeasuredHeight", "()I");
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

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // RadioButtonHandler.Android.PlatformArrange = PrepareForTextViewArrange(frame) + base.PlatformArrange.
        // The TextView gravity/min-height pre-adjustment (PrepareForTextViewArrange) is DEFERRED; the base
        // arrange (the dp frame → pixels, measure Exactly at the final size — Android requires a measure
        // pass before layout — then layout) is ported, exactly as check_box/label.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_radio_button_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_radio_button_class, "layout", "(IIII)V");
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

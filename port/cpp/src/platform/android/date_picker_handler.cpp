// date_picker_handler — Android (JNI) platform partial: the date-selection field of the M-android
// per-control fan-out (the headless mirror in src/platform/headless/date_picker_handler.cpp is the
// VM-less twin, src/platform/apple/date_picker_handler.mm is the native-AppKit twin which translates to
// NSDatePicker, and src/platform/ios/date_picker_handler.mm is the real iOS native render). The managed
// platform view is a REAL android.widget.EditText made non-editable (held as a JNI global reference in
// date_picker_platform::native): the field DISPLAYS the FORMATTED date and is read-only — the
// DatePickerDialog interaction is DEFERRED (documented below, exactly as the picker partial defers its
// MaterialAlertDialog and the editor partial its TextWatcher). The displayed string is produced by the
// SAME cross-platform formatter the headless/ios mirrors use, so MAUI ┃ C++ ┃ C++&XAML render identical
// text. The date_selected commit (the dialog's OnDateSet → VirtualView.Date) is DEFERRED but on_done
// stays a live C++ callback for portable drives + a future dialog trampoline.
//
// Ported DIRECTLY from DatePickerHandler.Android.cs + Platform/Android/{DatePickerExtensions.cs
// (UpdateFormat/UpdateDate both → SetText: Text ← Date?.ToString(Format) ?? string.Empty; UpdateTextColor
// → SetTextColor; UpdateMinimumDate/UpdateMaximumDate set the DIALOG's DatePicker.Min/MaxDate, NOT the
// field), MauiDatePicker.cs (the AppCompatEditText subclass made read-only via PickerManager.Init +
// SetOnClickListener(this) → ShowPicker), the shared EditTextExtensions / TextViewExtensions / FontManager
// (font / character-spacing surface) and ViewExtensions / ContextExtensions (ToPixels)}.
//
// The picker partial (picker_handler.cpp) is the STRUCTURAL TEMPLATE — it reuses the SAME EditText widget,
// the SAME read-only construction (setKeyListener(null)/setFocusable(false)/setClickable(true)/
// setCursorVisible(false)/setSingleLine(true)), the SAME scoped_env/app_context VM-less guards, the SAME
// call_void_* / to_pixels / display_density / global-ref-lifecycle / clear_pending discipline, and the
// SAME generic-IView pushes — all copied verbatim where they apply. The date_picker is the SIMPLER case:
// one formatted-text string (no items / no title-hint / no selection index), so push_display reduces to a
// single setText, and there is no setHint / title-color surface.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is a library / infrastructure gap, not a behavior guess):
//   - The widget is a plain android.widget.EditText, not MauiDatePicker (which subclasses
//     AndroidX.AppCompat.Widget.AppCompatEditText): the AndroidX AppCompat library is a gradle/AAR
//     dependency this APK-less backend does not carry, exactly like the picker / editor partials'
//     plain-android.widget.EditText stand-in. EditText extends TextView, so the whole TextView surface
//     MauiDatePicker relies on (Text / SetTextColor / Typeface / LetterSpacing / TextSize) resolves
//     through android/widget/EditText (GetMethodID walks the superclasses). MauiDatePicker's read-only
//     intent (DefaultMovementMethod = null + PickerManager.Init, which makes the field non-focusable and
//     click-only) is reproduced on the plain widget with setKeyListener(null) + setFocusable(false) +
//     setClickable(true) (see create_platform_view) — the picker partial's exact recipe.
//   - The DatePicker DIALOG is DEFERRED. C#'s MauiDatePicker.Initialize installs SetOnClickListener(this)
//     → OnClick → ShowPicker → ShowPickerDialog, which builds an android.app.DatePickerDialog whose
//     OnDateSet sets VirtualView.Date (then UpdateDate re-renders the field). The android click/dialog
//     trampoline fan-out has not arrived (same class of gap as the picker's MaterialAlertDialog and the
//     editor's TextWatcher), so no android.view.View.OnClickListener is installed yet; the field renders
//     the current formatted date (the capture target) and on_done stays a live C++ callback (the
//     cross-platform suite + a future dialog trampoline drive it → set_date → UpdateDate). MapIsOpen /
//     ShowPickerDialog / HidePickerDialog have no dialog to present and are no-ops for the same reason —
//     the control-level is_open()/Opened/Closed are the observable result.
//     // TODO: verify against src/Core/src/Handlers/DatePicker/DatePickerHandler.Android.cs
//     (ShowPickerDialog → DatePickerDialog.OnDateSet → VirtualView.Date) + the android click/dialog
//     trampoline seam when it lands.
//   - Minimum/MaximumDate are kept ONLY as headless mirrors (no field push). In C# MapMinimumDate /
//     MapMaximumDate write the DIALOG's DatePicker.MinDate / MaxDate (UpdateMinimumDate/UpdateMaximumDate
//     take a DatePickerDialog? and no-op when it is null) — they NEVER touch the field. With the dialog
//     deferred there is nothing native to set; the mirrors keep the bounds for the future dialog and for
//     the control's own date coercion. Byte-for-byte the headless partial's map_minimum_date /
//     map_maximum_date.
//   - The native EditText color setter takes a ColorStateList (C# routes through
//     PlatformInterop.CreateEditTextColorStateList in UpdateTextColor). The plain-widget cut uses the
//     single-int overload setTextColor(int) — the ColorStateList path's enabled-state coloring is an
//     AppCompat/PlatformInterop nicety, not part of the IDatePicker contract, and the int overload lands
//     the same ARGB. This is byte-for-byte the picker / editor partials' text-color cut.
//   - The port's colors are non-nullable value types, so C#'s `textColor is not null` guard in
//     UpdateTextColor (restore the theme default when null) has no value-type analog; an unset color is a
//     real value here, pushed through to setTextColor like every other color. Identical to the picker /
//     editor / ios / apple partials.
//   - FontManager's registrar/asset/file lookups and the "-light"/"-medium" suffix map are skipped (no
//     font registrar yet, on any backend): family → Typeface.create(family, style), then the API-28+
//     Typeface.create(base, weight, italic) refinement — the exact CreateTypeface tail. Byte-for-byte the
//     picker / editor / button partials' map_font.
//
// VM-less degradation (identical to the picker / editor / button partials): the android preset also runs
// the PURE-NATIVE cross-platform suite on the emulator (tools/android-emu-run.sh) where no Java VM exists.
// Every JNI path here checks scoped_env/app_context() and quietly skips, while the headless mirrors
// (date / text / minimum_date / maximum_date / text_color / font / character_spacing) are ALWAYS
// maintained — so that suite observes exactly the headless partial's behavior, and the widget test host
// (tools/android-testhost-run.sh) / the android APP HOST additionally observes the real widget.

#include "maui/core/date_picker_handler.hpp"
#include "maui/core/bindable_object.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "android_clip_ops.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_date_picker.hpp"
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

    // All instance methods resolve through the widget's own class (GetMethodID walks the superclasses, so
    // the View/TextView surface resolves through android/widget/EditText too) — identical to the picker /
    // editor partials, which use the very same widget.
    constexpr const char* k_edit_text_class = "android/widget/EditText";
    constexpr const char* k_style_class = "android/R$style";
    // The concrete platform style that carries the EditText's box/underline CHROME (the read-only field
    // showing the selected date), resolved theme-independently as a defStyleRes so the bare app_process
    // testhost (and the app host) construct a field that actually HAS its chrome. A defStyleAttr=0 ctor
    // with NO defStyleRes resolves no background drawable → a chrome-less field (the missing-chrome bug the
    // switch/checkbox glyph waves hit). The gallery's light Activity theme makes
    // Widget_EditText the matching field chrome: it carries @android:drawable/edit_text (a framework-res
    // 9-patch underline) that renders in the bare app_process host; the Material_Light variant resolves its
    // background to a theme attr the host can't satisfy and paints NOTHING (verified in wave 14), so it is
    // only a fallback. The *_alt fields are tried in turn if absent. (GetStaticFieldID — static fields.)
    constexpr const char* k_edit_text_style_field = "Widget_EditText";
    constexpr const char* k_edit_text_style_field_alt = "Widget_Material_EditText";
    constexpr const char* k_edit_text_style_field_alt2 = "Widget_Material_Light_EditText";
    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // FontManager.DefaultFontSize (Android) — 14sp. (FontManager.GetFontSize fallback.)
    constexpr float k_default_font_size = 14.0F;

    // UnitExtensions.EmCoefficient — TextView.LetterSpacing speaks em, CharacterSpacing speaks pt.
    constexpr float k_em_coefficient = 0.0624F;

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (PlatformInterop restores it after
    // setContentDescription auto-flips the view to YES).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.graphics.Typeface styles (FontManager.ToTypefaceStyle's targets).
    constexpr jint k_typeface_normal = 0;
    constexpr jint k_typeface_bold = 1;
    constexpr jint k_typeface_italic = 2;

    // android.util.TypedValue complex unit types (FontManager.GetFontSize's units).
    constexpr jint k_complex_unit_dip = 1;
    constexpr jint k_complex_unit_sp = 2;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::date_picker_platform& platform) noexcept
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
        if (jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "(Z)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // EditTextExtensions/TextViewExtensions: set a CharSequence property (setText).
    void call_void_char_sequence(JNIEnv* env, jobject widget, const char* name, std::string_view value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_edit_text_class, name, "(Ljava/lang/CharSequence;)V"))
        {
            const local_ref<jstring> str = to_jstring(env, value);
            env->CallVoidMethod(widget, method, str.get());
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
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity cache
    // (and the picker / editor / button partials' twin). 1.0 when any step fails (failures are NOT
    // memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_edit_text_class, "getContext", "()Landroid/content/Context;");
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

    // The view's CURRENT size in POINTS (getWidth/getHeight pixels / density). {0,0} before the first
    // layout (apply_outline_clip skips a 0-sized view; platform_arrange re-resolves once laid out). The
    // generic-IView clip op resolves the geometry against rect{0,0,w,h} in points (the WrapperView.SetClip
    // convention), so the clip lands in the same point space the apple apply_clip twin uses.
    struct point_size
    {
        double width;
        double height;
    };
    [[nodiscard]] point_size view_point_size(JNIEnv* env, jobject widget, float density)
    {
        auto& cache = default_jni_cache();
        jmethodID get_width = cache.method(env, k_edit_text_class, "getWidth", "()I");
        jmethodID get_height = cache.method(env, k_edit_text_class, "getHeight", "()I");
        if (get_width == nullptr || get_height == nullptr || density == 0.0F)
        {
            return {.width = 0.0, .height = 0.0};
        }
        const jint w = env->CallIntMethod(widget, get_width);
        const jint h = env->CallIntMethod(widget, get_height);
        if (clear_pending(env))
        {
            return {.width = 0.0, .height = 0.0};
        }
        return {.width = static_cast<double>(w) / density, .height = static_cast<double>(h) / density};
    }

    // DatePickerExtensions.SetText: Text ← Date?.ToString(Format) ?? string.Empty. The cross-platform
    // formatter (format_date_time, the SAME helper the headless/ios mirrors call) produces the string; a
    // null Date renders empty. Pushed to the REAL widget. (Run only when a widget exists; the headless
    // mirror's text/date are written by the maui::core update_date helper below, kept identical.)
    void push_display(JNIEnv* env, jobject widget, const maui::core::i_date_picker& view)
    {
        const auto date = view.date();
        if (!date.has_value())
        {
            // Text ← string.Empty. setText("") clears (CharSequence overload); the empty C++ string is a
            // benign empty CharSequence — not null — matching `?? string.Empty`.
            call_void_char_sequence(env, widget, "setText", std::string_view{});
            return;
        }
        // Date?.ToString(Format): the standard "d"/"D"/empty patterns and custom DateTime patterns route
        // through the shared formatter (see date_time.hpp). The headless mirror computed the SAME string.
        const std::string_view format = view.format();
        const std::string text = (format.empty() || format == "d" || format == "D")
                                     ? maui::core::format_date_time(*date, format == "D" ? "D" : "d")
                                     : maui::core::format_date_time(*date, format);
        call_void_char_sequence(env, widget, "setText", text);
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // DatePickerExtensions.UpdateDate / UpdateFormat (both → SetText): refresh the headless mirror's
        // date + display text, then push the formatted Text to the real widget. The mirror half is
        // byte-for-byte the headless partial's update_date (the task: "Keep every headless-mirror write");
        // the native half follows DatePickerExtensions.SetText.
        void update_date(date_picker_handler& handler, i_date_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            // --- headless mirror (identical to src/platform/headless/date_picker_handler.cpp) ---
            const auto date = view.date();
            platform->date = date.value_or(date_time::today());
            if (!date.has_value())
            {
                platform->text.clear();
            }
            else
            {
                const std::string_view format = view.format();
                if (format.empty() || format == "d" || format == "D")
                {
                    platform->text = format_date_time(*date, format == "D" ? "D" : "d");
                }
                else
                {
                    platform->text = format_date_time(*date, format);
                }
            }

            // --- native push (DatePickerExtensions.SetText against the real EditText) ---
            if (platform->native == nullptr)
            {
                return;
            }
            const scoped_env env;
            if (env)
            {
                push_display(env.get(), widget_of(*platform), view);
            }
        }
    } // namespace

    // Releases the global reference pinning the android.widget.EditText (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSDatePicker here).
    date_picker_platform::~date_picker_platform()
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
    // the header comment) — then pushes to the real widget when one exists. Copied from the picker partial;
    // only the widget class name differs (both are android/widget/EditText).

    void date_picker_platform::update_visibility(maui::core::visibility value)
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

    void date_picker_platform::update_opacity(double value)
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

    void date_picker_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // ViewExtensions.UpdateIsEnabled: Enabled = view.IsEnabled. The field is intentionally
        // non-focusable / click-driven (the read-only construction), so — like the picker partial — we
        // push Enabled + Clickable (so a disabled field stops responding to the deferred dialog tap) and
        // keep Focusable false.
        jobject widget = widget_of(*this);
        call_void_bool(env.get(), widget, "setEnabled", static_cast<jboolean>(value));
        call_void_bool(env.get(), widget, "setClickable", static_cast<jboolean>(value));
    }

    void date_picker_platform::update_automation_id(std::string_view value)
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
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had.
        jmethodID get_important = cache.method(env.get(), k_edit_text_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_edit_text_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
        // DatePickerHandler.MapBackground (an Android-specific override) delegates to PlatformView
        // .UpdateBackground(datePicker) — the shared android view op pushes the solid/gradient/image
        // background to the View (VM-less safe). The headless mirror keeps the base body.
        view_platform_base::update_background(value);
        // WAVE 14: a NULL paint must NOT call apply_background — setBackground(null) would ERASE the
        // defStyleRes field chrome (the underline the styled ctor installed). MAUI leaves the native
        // default when Background is null; the EditText's styled background IS that default.
        if (value != nullptr)
        {
            maui::platform::android::apply_background(native, value);
        }
    }

    // Render transform + flow direction + semantics pushed to the real widget via the shared android ops.
    // Each calls the view_platform_base body FIRST — the VM-less cross-platform suite observes the headless
    // mirror — then the shared op (itself VM-less safe) pushes to the View.
    void date_picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void date_picker_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void date_picker_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // VisualElement.Clip on a generic view (wave 24): install a ViewOutlineProvider + setClipToOutline so
    // the framework clips the whole EditText to the convex shape (the clip_views EllipseGeometry). The base
    // mirror runs FIRST (the VM-less cross-platform suite observes it). The borrow is stashed so
    // platform_arrange can re-resolve the bounds-dependent geometry after layout (the view is 0×0 at map
    // time — apply_outline_clip clears the clip then, and the arrange pass rebuilds it at the live size).
    void date_picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        view_platform_base::update_clip(value);
        clip_shape = value;
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*this);
        const float density = display_density(env.get(), widget);
        const point_size size = view_point_size(env.get(), widget, density);
        maui::platform::android::apply_outline_clip(native, value, density, size.width, size.height);
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass edit_text_class = cache.find_class(env.get(), k_edit_text_class);
        if (edit_text_class == nullptr)
        {
            return platform;
        }
        // DatePickerHandler.CreatePlatformView: new MauiDatePicker(Context). MauiDatePicker →
        // AppCompatEditText(Context). The plain android.widget.EditText stand-in (header deviations);
        // EditText(Context) chains to the (Context, AttributeSet, int) ctor with defStyleAttr =
        // editTextStyle, a THEME attr that throws in the bare, Activity-less app_process testhost (the same
        // trap progress_bar's styled ctor + the picker / editor partials hit). The defStyleAttr=0 3-arg
        // ctor constructs fine BUT resolves NO background drawable, so the read-only field renders with NO
        // box/underline chrome (the missing-chrome bug the switch/checkbox glyph waves hit). So construct
        // THEME-INDEPENDENTLY via the 4-arg (Context, AttributeSet, int defStyleAttr, int defStyleRes) ctor
        // with defStyleAttr=0 and defStyleRes = android.R.style.Widget_Material_Light_EditText (a concrete
        // style resource that CARRIES the field chrome — read with GetStaticFieldID since it is a static
        // field). Then fall back to the 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so
        // the widget is never null.
        jobject created = nullptr;
        jmethodID ctor_styled = cache.method(env.get(), k_edit_text_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        jfieldID style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_edit_text_style_field, "I") : nullptr;
        clear_pending(env.get()); // a missing-field lookup raises NoSuchFieldError — clear it, then try alts
        if (style_class != nullptr && style_field == nullptr)
        {
            style_field = env->GetStaticFieldID(style_class, k_edit_text_style_field_alt, "I");
            clear_pending(env.get());
        }
        if (style_class != nullptr && style_field == nullptr)
        {
            style_field = env->GetStaticFieldID(style_class, k_edit_text_style_field_alt2, "I");
            clear_pending(env.get());
        }
        if (ctor_styled != nullptr && style_class != nullptr && style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(edit_text_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_unstyled = cache.method(env.get(), k_edit_text_class, "<init>",
                                                   "(Landroid/content/Context;Landroid/util/AttributeSet;I)V");
            if (ctor_unstyled != nullptr)
            {
                created = env->NewObject(edit_text_class, ctor_unstyled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0));
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain = cache.method(env.get(), k_edit_text_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(edit_text_class, ctor_plain, context);
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

        // MauiDatePicker makes the field NON-EDITABLE: it is a read-only field that opens the
        // DatePickerDialog on click, never accepting keyboard input (DefaultMovementMethod = null +
        // PickerManager.Init's focus/cursor suppression). Reproduce the read-only intent on the plain
        // widget (header deviations), the picker partial's exact recipe: setKeyListener(null) makes it
        // ignore key input (Android's canonical "read-only EditText"), setFocusable(false) keeps the
        // keyboard from opening, and setClickable(true) keeps it tappable for the (deferred) dialog. The
        // capture target — the field SHOWING the formatted date — needs only these; the dialog Click
        // listener is the deferred half.
        jmethodID set_key_listener =
            cache.method(env.get(), k_edit_text_class, "setKeyListener", "(Landroid/text/method/KeyListener;)V");
        if (set_key_listener != nullptr)
        {
            env->CallVoidMethod(widget.get(), set_key_listener, static_cast<jobject>(nullptr));
            clear_pending(env.get());
        }
        call_void_bool(env.get(), widget.get(), "setFocusable", JNI_FALSE);
        call_void_bool(env.get(), widget.get(), "setFocusableInTouchMode", JNI_FALSE);
        call_void_bool(env.get(), widget.get(), "setClickable", JNI_TRUE);
        call_void_bool(env.get(), widget.get(), "setCursorVisible", JNI_FALSE);
        // SetSingleLine(true): the date field shows the single formatted line (a date string is never
        // multi-line) — the picker partial's choice (unlike the editor, which sets it false).
        call_void_bool(env.get(), widget.get(), "setSingleLine", JNI_TRUE);

        // Wrap-content LayoutParams up front: a parentless TextView with null LayoutParams NPEs in
        // checkForRelayout on any setText AFTER the first measure (TextView.java reads
        // getLayoutParams().width). The android container fan-out has not arrived, so the partial stands in
        // for the parent ViewGroup attach (identical to the picker / editor / button partials' note).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_edit_text_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~date_picker_platform
        return platform;
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        // DatePickerHandler.Android ConnectHandler installs MauiDatePicker.ShowPicker = ShowPickerDialog
        // (fired by the field's OnClick), which builds the android.app.DatePickerDialog whose OnDateSet
        // sets VirtualView.Date. That dialog and the android click trampoline are deferred (header
        // deviations), EXACTLY like the picker partial defers its MaterialAlertDialog and the editor its
        // TextWatcher — but on_done stays a live C++ callback so the VM-less cross-platform suite (and a
        // future dialog trampoline) can drive a commit: OnDateSet → VirtualView.Date = the dialog's value,
        // which the headless mirror holds in platform.date; set_date re-runs UpdateDate (re-rendering the
        // field). The control's own coercion clamps the committed date into [MinimumDate, MaximumDate].
        platform.on_done = [this] {
            auto* view = virtual_view();
            auto* typed = typed_platform_view();
            if (view == nullptr || typed == nullptr)
            {
                return;
            }
            view->set_date(typed->date);
        };
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        // DisconnectHandler: ShowPicker/HidePicker = null; ResetDialog disposes the DatePickerDialog (the
        // native click/dialog uninstall lands with the deferred trampoline fan-out).
        platform.on_done = nullptr;
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // UpdateFormat → SetText (re-render the text), same as UpdateDate
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // UpdateDate → SetText
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        // MapMinimumDate writes the DIALOG's DatePicker.MinDate (UpdateMinimumDate no-ops with a null
        // DatePickerDialog and NEVER touches the field). The dialog is deferred (header deviations), so
        // there is nothing native to set — keep the headless mirror for the future dialog + the control's
        // own coercion. Byte-for-byte the headless partial's map_minimum_date.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum_date = view.minimum_date();
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        // MapMaximumDate: the DIALOG's DatePicker.MaxDate; mirror-only here (see map_minimum_date).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum_date = view.maximum_date();
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // DatePickerExtensions.UpdateTextColor: `if (textColor != null) SetTextColor(...)`. Mirror the
            // C# null guard via BindableObject.IsSet: pushing an UNSET color (value-type default = opaque
            // black) via the single-int setTextColor REPLACES the native EditText ColorStateList that carries
            // the disabled-state dim, so a disabled field never dims (and an unset color paints flat black
            // instead of the theme default). Leave the native ColorStateList intact when unset.
            const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            if (bindable != nullptr && bindable->is_property_set("text_color"))
            {
                call_void_int(env.get(), widget_of(*platform), "setTextColor",
                              static_cast<jint>(view.text_color().to_int()));
            }
            else if (maui::platform::android::detail::is_night_mode(env.get()))
            {
                // DARK: the DeviceDefault dark value text is a dim blue-gray (~#40484D, near-invisible);
                // MAUI's Material dark renders the value WHITE. Seed white (light keeps the native default).
                call_void_int(env.get(), widget_of(*platform), "setTextColor", static_cast<jint>(0xFFFFFFFFU));
            }
        }
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font(); // headless mirror
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

        // FontManager.GetTypeface → CreateTypeface (the non-registered-family tail; header deviations,
        // identical to the picker / editor / button partials' map_font): base = family ?
        // Typeface.create(family, ToTypefaceStyle(weight, italic)) : Typeface.DEFAULT, then the API-28+
        // refinement Typeface.create(base, weight, italic).
        const bool italic = value.slant() != font_slant::normal;
        const bool bold = value.weight() >= font_weight::bold; // ToTypefaceStyle: bold = weight >= Bold
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
            cache.method(env.get(), k_edit_text_class, "setTypeface", "(Landroid/graphics/Typeface;)V");
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

        // FontManager.GetFontSize: size ≤ 0 / NaN → DefaultFontSize; AutoScalingEnabled picks Sp
        // (user-scaled) vs Dip. TextViewExtensions.UpdateFont: SetTextSize(unit, value).
        auto size = static_cast<float>(value.size());
        if (!(size > 0) || std::isnan(size))
        {
            size = k_default_font_size;
        }
        const jint unit = value.auto_scaling_enabled() ? k_complex_unit_sp : k_complex_unit_dip;
        jmethodID set_text_size = cache.method(env.get(), k_edit_text_class, "setTextSize", "(IF)V");
        if (set_text_size != nullptr)
        {
            env->CallVoidMethod(widget, set_text_size, unit, static_cast<jfloat>(size));
            clear_pending(env.get());
        }
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing(); // headless mirror
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

    void date_picker_handler::map_is_open(date_picker_handler& /*handler*/, i_date_picker& /*view*/)
    {
        // DatePickerHandler.MapIsOpen → ShowPickerDialog/HidePickerDialog (the android.app.DatePickerDialog
        // Show/Hide). The dialog is deferred with the click trampoline (header deviations), so there is no
        // native dialog to present/dismiss; this is a genuine no-op — the control-level
        // is_open()/Opened/Closed are the observable result.
        // TODO: verify against DatePickerHandler.Android.cs (ShowPickerDialog/HidePickerDialog) when the
        // dialog trampoline lands.
    }

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (a single-line field ~150pt
            // wide, clamped to a finite width constraint, one line tall), so the backend-agnostic
            // size-request suites see consistent numbers in the pure-native run.
            double width = 150.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 22.0};
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
        // (Context.FromPixels). Identical to the picker / editor / button partials.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_edit_text_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_edit_text_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_edit_text_class, "getMeasuredHeight", "()I");
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

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the view measures Exactly at the final
        // size (Android requires a measure pass before layout) and lays out. Identical to the picker /
        // editor / button partials (the date field is a TextView-derived field, so the
        // PrepareForTextViewArrange nicety would apply; it is deferred with the editor's, the base
        // measure/arrange stands in).
        // TODO: verify against PrepareForTextViewArrange when the text-view arrange-prep seam lands.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_edit_text_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_edit_text_class, "layout", "(IIII)V");
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

        // Re-resolve the clip against the just-laid-out bounds (the iOS reapply_clip analog): the outline
        // geometry is resolved against the live frame size in points, and update_clip may have run before
        // the first layout when the view was 0×0 (apply_outline_clip cleared it then). frame is in points.
        if (platform->clip_shape != nullptr)
        {
            maui::platform::android::apply_outline_clip(platform->native, platform->clip_shape, density, frame.width,
                                                        frame.height);
        }
    }
} // namespace maui::core

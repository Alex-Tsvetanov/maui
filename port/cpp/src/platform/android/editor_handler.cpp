// editor_handler — Android (JNI) platform partial, the multi-line text-editor control of the
// M-android per-control fan-out (M6's iOS Rosetta Stone replayed over JNI; the headless mirror in
// src/platform/headless/editor_handler.cpp is the VM-less twin). The managed platform view is a REAL
// android.widget.EditText (held as a JNI global reference in editor_platform::native): every map_*
// pushes its property through the jni_cache'd method ids, and a native edit / end-of-edit flows back
// through C++ callbacks (on_text_changed / on_completed) into i_editor::send_text_changed /
// send_completed — exactly the inbound channel the headless mirror exposes.
//
// Ported DIRECTLY from EditorHandler.Android.cs + Platform/Android/{EditTextExtensions.cs,
// TextViewExtensions.cs, TextAlignmentExtensions.cs, AlignmentExtensions.cs, KeyboardExtensions.cs,
// ViewExtensions.cs, ContextExtensions.cs (ToPixels), UnitExtensions.cs (ToEm)} + Fonts/
// FontManager.Android.cs. The button partial (button_handler.cpp) is the structural template — the
// scoped_env/app_context VM-less guards, the call_void_* helpers, to_pixels/display_density,
// clear_pending-after-every-call discipline, the global-ref lifecycle, and the per-backend
// view_platform_base overrides are copied from it verbatim where they apply.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is a library / infrastructure gap, not a behavior
// guess):
//   - The widget is a plain android.widget.EditText, not MauiAppCompatEditText: the AndroidX AppCompat
//     library (which MauiAppCompatEditText subclasses) is a gradle/AAR dependency this APK-less backend
//     does not carry, exactly like the button partial's plain-android.widget.Button stand-in. EditText
//     extends TextView, so the whole TextView surface MapText/Font/CharacterSpacing/alignment reaches
//     resolves through android/widget/EditText (GetMethodID walks the superclasses). The AppCompat-only
//     construction knobs (ImeOptions = ImeAction.Done, the SelectionChanged hook) have no plain-widget
//     analog and are skipped; SetSingleLine(false) + Gravity = Top + TextAlignment = ViewStart + the
//     TYPE_TEXT_FLAG_MULTI_LINE bit (the multi-line knobs the task calls out) ARE ported, as they are
//     plain TextView/EditText properties.
//   - MapBackground rides the shared view_mapper (no per-handler override needed); the maui background
//     drawable / corner-radius / stroke machinery the button partial carries does not apply to an editor.
//   - The port's colors are non-nullable value types (default color{} = opaque BLACK), so C#'s null-color
//     branches have no direct value-type analog. UpdateTextColor's "restore the theme default" else-branch
//     is dropped (an unset TextColor is pushed through to setTextColor like every other color — TextColor
//     is not one of the parity yellows). UpdatePlaceholderColor's null branch (resolve
//     android.R.attr.textColorHint, SetHintTextColor to it) IS honored: map_placeholder_color discriminates
//     on BindableObject.IsSet and, when UNSET, asserts the measured native hint gray (#666666) instead of
//     black — the unset-color-default family fix (see map_placeholder_color), so an empty editor shows a
//     gray placeholder matching MAUI, not solid black. An explicit PlaceholderColor still overrides.
//   - The native EditText color setters take a ColorStateList (C# routes through
//     PlatformInterop.CreateEditTextColorStateList). The plain-widget cut uses the single-int overloads
//     setTextColor(int) / setHintTextColor(int) — the ColorStateList path's enabled-state coloring is an
//     AppCompat/PlatformInterop nicety, not part of the IEditor contract, and the int overloads land the
//     same ARGB the tests read back via getCurrentTextColor / getCurrentHintTextColor.
//   - FontManager's registrar/asset/file lookups and the "-light"/"-medium" suffix map are skipped (the
//     port has no font registrar yet, on any backend): family goes straight to Typeface.create(family,
//     style), then the API-28+ Typeface.create(base, weight, italic) refinement — the exact CreateTypeface
//     tail for a non-registered family. This is byte-for-byte the button partial's map_font.
//   - max_length is enforced control-side (editor::set_max_length truncates the stored text, like entry),
//     so the native InputFilter[] LengthFilter (PlatformInterop.UpdateMaxLength) push is a documented
//     no-op here: the value is mirrored into editor_platform::max_length and the truncation already
//     happened before the text reached the widget. // TODO: verify against
//     src/Core/src/Platform/Android/EditTextExtensions.cs (UpdateMaxLength → SetLengthFilter) when the
//     native InputFilter seam lands.
//   - The TextChanged / FocusChange listeners (EditorHandler.Android.cs ConnectHandler:
//     platformView.TextChanged += OnTextChanged; FocusChange += OnFocusChange → Completed) are deferred
//     with the gesture/text-watcher fan-out, EXACTLY like the button partial defers its OnTouchListener.
//     on_text_changed / on_completed stay invokable C++ callbacks (the cross-platform suite drives them);
//     no android.text.TextWatcher / View.OnFocusChangeListener trampoline is installed yet.
//     // TODO: verify against src/Core/src/Handlers/Editor/EditorHandler.Android.cs (OnTextChanged /
//     OnFocusChange) when the android TextWatcher/focus trampoline arrives.
//   - SelectionChanged → MapCursorPosition / MapSelectionLength: the native selection round-trip
//     (EditTextExtensions.UpdateCursorPosition/UpdateSelectionLength via SetSelection, posted on the
//     looper) is deferred with the same listener fan-out; map_cursor_position / map_selection_length
//     mirror the values and push setSelection best-effort. // TODO: verify against EditTextExtensions.cs
//     (UpdateCursorSelection's editText.Post looper hop) when the focus/looper seam lands.
//
// VM-less degradation (identical to the button partial): the android preset also runs the PURE-NATIVE
// cross-platform suite on the emulator (tools/android-emu-run.sh) where no Java VM exists. Every JNI
// path here checks scoped_env/app_context() and quietly skips, while the headless mirrors (text/
// placeholder/text_color/…) are ALWAYS maintained — so that suite observes exactly the headless
// partial's behavior, and the widget test host (tools/android-testhost-run.sh) additionally observes
// the real widget.

#include "maui/core/editor_handler.hpp"

#include <jni.h>

#include <cmath>
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
#include "maui/core/bindable_object.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/keyboard_flags.hpp"
#include "maui/core/text_alignment.hpp"
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

    // All instance methods resolve through the widget's own class (GetMethodID walks the superclasses,
    // so the View/TextView surface resolves through android/widget/EditText too).
    constexpr const char* k_edit_text_class = "android/widget/EditText";
    constexpr const char* k_style_class = "android/R$style";
    // The concrete platform style that carries the EditText's box/underline CHROME, resolved
    // theme-independently as a defStyleRes so the bare app_process testhost (and the app host) construct an
    // EditText that actually HAS its field chrome. The 3-arg (Context, AttributeSet, int defStyleAttr) ctor
    // with defStyleAttr=0 and NO defStyleRes resolves NO background drawable, so an EMPTY EditText renders
    // as invisible nothing (no box, no underline) — the exact missing-chrome bug the switch/checkbox glyph
    // waves hit, fixed there with a defStyleRes (Widget_*). The gallery's light Activity theme makes
    // android.R.style.Widget_EditText the matching field chrome: it carries @android:drawable/edit_text, a
    // framework-res state-list 9-patch underline that does NOT depend on an app/AppCompat theme, so it
    // renders in the bare app_process host. The Material_Light variant resolves its background to a theme
    // attr the host can't satisfy and paints NOTHING (verified empirically in wave 14), so it is only a
    // fallback. The *_alt fields are tried in turn if the primary is absent on some API level. (Read with
    // GetStaticFieldID — these are static fields.)
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

    // android.view.View.TEXT_ALIGNMENT_* (AlignmentExtensions.ToTextAlignment's targets — the EditText
    // TextAlignment path, taken on RTL-capable Android, which is every device the port targets):
    // ToTextAlignment maps Center → Center, End → ViewEnd, else → ViewStart.
    constexpr jint k_text_alignment_center = 4;     // View.TEXT_ALIGNMENT_CENTER
    constexpr jint k_text_alignment_view_start = 5; // View.TEXT_ALIGNMENT_VIEW_START
    constexpr jint k_text_alignment_view_end = 6;   // View.TEXT_ALIGNMENT_VIEW_END

    // android.view.Gravity bits (TextAlignmentExtensions' vertical-gravity masking + the multi-line
    // Gravity = Top construction knob). Editor's vertical alignment defaults to Top (Start).
    constexpr jint k_gravity_top = 0x30;
    constexpr jint k_gravity_bottom = 0x50;
    constexpr jint k_gravity_center_vertical = 0x10;
    constexpr jint k_gravity_vertical_mask = k_gravity_top | k_gravity_bottom | k_gravity_center_vertical;

    // android.text.InputType flags (KeyboardExtensions.ToInputType + SetInputType). Ported verbatim
    // from android.text.InputType so the computed type matches the oracle bit-for-bit.
    constexpr jint k_type_class_text = 0x00000001;
    constexpr jint k_type_class_number = 0x00000002;
    constexpr jint k_type_class_phone = 0x00000003;
    constexpr jint k_type_class_datetime = 0x00000004;
    constexpr jint k_type_text_variation_normal = 0x00000000;
    constexpr jint k_type_text_variation_uri = 0x00000010;
    constexpr jint k_type_text_variation_email = 0x00000020;
    constexpr jint k_type_text_variation_password = 0x00000080;
    constexpr jint k_type_text_flag_cap_characters = 0x00001000;
    constexpr jint k_type_text_flag_cap_words = 0x00002000;
    constexpr jint k_type_text_flag_cap_sentences = 0x00004000;
    constexpr jint k_type_text_flag_auto_correct = 0x00008000;
    constexpr jint k_type_text_flag_auto_complete = 0x00010000;
    constexpr jint k_type_text_flag_multi_line = 0x00020000;
    constexpr jint k_type_text_flag_no_suggestions = 0x00080000;
    constexpr jint k_type_number_flag_signed = 0x00001000;
    constexpr jint k_type_number_flag_decimal = 0x00002000;
    constexpr jint k_type_datetime_variation_normal = 0x00000000;
    constexpr jint k_type_datetime_variation_time = 0x00000020;

    [[nodiscard]] jobject widget_of(const maui::core::editor_platform& platform) noexcept
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

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity
    // cache (and the button partial's twin). 1.0 when any step fails (failures are NOT memoized).
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

    // AlignmentExtensions.ToTextAlignment: Center → Center, End → ViewEnd, else ViewStart. (The EditText
    // path UpdateHorizontalAlignment takes on RTL-capable Android.)
    [[nodiscard]] jint to_text_alignment(maui::core::text_alignment alignment)
    {
        switch (alignment)
        {
            case maui::core::text_alignment::center:
                return k_text_alignment_center;
            case maui::core::text_alignment::end:
                return k_text_alignment_view_end;
            default:
                return k_text_alignment_view_start;
        }
    }

    // AlignmentExtensions.ToVerticalGravityFlags: Start → Top, End → Bottom, else CenterVertical.
    // (Editor's vertical default is Start = Top — see editor_platform's vertical_alignment default.)
    [[nodiscard]] jint to_vertical_gravity(maui::core::text_alignment alignment)
    {
        switch (alignment)
        {
            case maui::core::text_alignment::start:
                return k_gravity_top;
            case maui::core::text_alignment::end:
                return k_gravity_bottom;
            default:
                return k_gravity_center_vertical;
        }
    }

    // KeyboardExtensions.ToInputType — the named-keyboard / CustomKeyboard branch table, ported
    // verbatim. The result is the BASE input type before SetInputType's editor multi-line / prediction
    // / spellcheck overlay (apply_input_type below).
    [[nodiscard]] jint keyboard_to_input_type(maui::core::keyboard keyboard)
    {
        using kind = enum maui::core::keyboard::kind; // disambiguate the nested enum from keyboard::kind()
        switch (keyboard.kind())
        {
            case kind::default_:
                return k_type_class_text | k_type_text_variation_normal;
            case kind::chat:
            case kind::text:
                return k_type_class_text | k_type_text_flag_cap_sentences | k_type_text_flag_auto_complete;
            case kind::email:
                return k_type_class_text | k_type_text_variation_email;
            case kind::numeric:
                return k_type_class_number | k_type_number_flag_decimal | k_type_number_flag_signed;
            case kind::telephone:
                return k_type_class_phone;
            case kind::url:
                return k_type_class_text | k_type_text_variation_uri;
            case kind::date:
                return k_type_class_datetime | k_type_datetime_variation_normal;
            case kind::time:
                return k_type_class_datetime | k_type_datetime_variation_time;
            case kind::password:
                return k_type_class_text | k_type_text_variation_password;
            case kind::plain:
            case kind::custom: {
                // CustomKeyboard branch: ClassText | (flag-derived bits). Plain is Create(None) in C#.
                const maui::core::keyboard_flags flags = keyboard.flags();
                jint result = k_type_class_text;
                if (maui::core::has_flag(flags, maui::core::keyboard_flags::capitalize_sentence))
                {
                    result |= k_type_text_flag_cap_sentences;
                }
                if (!maui::core::has_flag(flags, maui::core::keyboard_flags::spellcheck))
                {
                    result |= k_type_text_flag_no_suggestions;
                }
                if (maui::core::has_flag(flags, maui::core::keyboard_flags::suggestions))
                {
                    result |= k_type_text_flag_auto_correct;
                }
                if (flags != maui::core::keyboard_flags::all)
                {
                    if (maui::core::has_flag(flags, maui::core::keyboard_flags::capitalize_word))
                    {
                        result |= k_type_text_flag_cap_words;
                    }
                    if (maui::core::has_flag(flags, maui::core::keyboard_flags::capitalize_character))
                    {
                        result |= k_type_text_flag_cap_characters;
                    }
                }
                return result;
            }
        }
        return k_type_text_variation_normal; // "Should never happen" (KeyboardExtensions' else).
    }

    // EditTextExtensions.SetInputType (the editor slice): compute the keyboard base type, OR-in the
    // prediction/spellcheck bits (the non-CustomKeyboard branch always re-applies both — they are also
    // their own mappers below, each re-running this), then OR-in TextFlagMultiLine because the virtual
    // view is an IEditor. setInputType is on TextView. Recomputed from i_editor each call so any of the
    // three intertwined properties (keyboard / prediction / spellcheck) lands the full type.
    void apply_input_type(JNIEnv* env, jobject widget, const maui::core::i_editor& view)
    {
        jint input_type = keyboard_to_input_type(view.keyboard());

        // UpdateIsTextPredictionEnabled / UpdateIsSpellCheckEnabled (not gated by CustomKeyboard here —
        // SetInputType only skips them for a CustomKeyboard, which the editor surface never carries).
        if (view.keyboard().kind() != maui::core::keyboard::kind::custom &&
            view.keyboard().kind() != maui::core::keyboard::kind::plain)
        {
            if (view.is_text_prediction_enabled())
            {
                input_type |= k_type_text_flag_auto_correct;
            }
            else
            {
                input_type &= ~k_type_text_flag_auto_correct;
            }
            if (!view.is_spell_check_enabled())
            {
                input_type |= k_type_text_flag_no_suggestions;
            }
            else
            {
                input_type &= ~k_type_text_flag_no_suggestions;
            }
        }

        // SetInputType tail: `if (textInput is IEditor) editText.InputType |= TextFlagMultiLine;`.
        input_type |= k_type_text_flag_multi_line;

        call_void_int(env, widget, "setInputType", input_type);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.EditText (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin releases its NSScrollView/NSTextView here).
    editor_platform::~editor_platform()
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

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform
    // suite (see the header comment) — then pushes to the real widget when one exists. Copied from the
    // button partial; only the widget class name differs.

    void editor_platform::update_visibility(maui::core::visibility value)
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

    void editor_platform::update_opacity(double value)
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

    void editor_platform::update_is_enabled(bool value)
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

    void editor_platform::update_automation_id(std::string_view value)
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

    void editor_platform::update_background(const maui::graphics::paint* value)
    {
        // MapBackground rides the shared view_mapper in C# (EditorHandler.Android delegates straight to
        // PlatformView.UpdateBackground). The headless mirror keeps the base body; the android view op
        // pushes the solid/gradient/image background to the View (VM-less safe).
        view_platform_base::update_background(value);
        // WAVE 14: a NULL paint must NOT call apply_background — that path does setBackground(null), which
        // would ERASE the defStyleRes field chrome (the underline 9-patch the styled ctor installed),
        // leaving the field box-less again. MAUI's UpdateBackground leaves the native default when the
        // virtual Background is null; the EditText's styled background IS that native default.
        if (value != nullptr)
        {
            maui::platform::android::apply_background(native, value);
            // An explicit Background overrides the field chrome, so drop the create-time underline tint —
            // otherwise it recolors the explicit background (e.g. a green Editor rendered gray).
            maui::platform::android::clear_background_tint(native);
        }
    }

    // Render transform + flow direction + semantics pushed to the real widget via the shared android
    // ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite observes the
    // headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void editor_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void editor_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void editor_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // VisualElement.Clip on a generic view (wave 24): install a ViewOutlineProvider + setClipToOutline so
    // the framework clips the whole EditText to the convex shape (the clip_views EllipseGeometry). The base
    // mirror runs FIRST (the VM-less cross-platform suite observes it). The borrow is stashed so
    // platform_arrange can re-resolve the bounds-dependent geometry after layout (the view is 0×0 at map
    // time — apply_outline_clip clears the clip then, and the arrange pass rebuilds it at the live size).
    void editor_platform::update_clip(const maui::graphics::i_shape* value)
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

    std::unique_ptr<editor_platform> editor_handler::create_platform_view()
    {
        auto platform = std::make_unique<editor_platform>();
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
        // EditorHandler.CreatePlatformView: new MauiAppCompatEditText(Context) { TextAlignment = ViewStart,
        // Gravity = Top, … }; editText.SetSingleLine(false); editText.SetHorizontallyScrolling(false).
        // EditText(Context) chains to the (Context, AttributeSet, int) ctor with defStyleAttr = the theme
        // attr editTextStyle, which it resolves against the Context's THEME — the bare, Activity-less
        // app_process testhost has no such theme, so that ctor throws (the same trap progress_bar's styled
        // ctor hit). The 3-arg (Context, AttributeSet, int defStyleAttr) ctor with defStyleAttr=0 constructs
        // fine BUT resolves NO background drawable, so an empty EditText renders with NO box/underline chrome
        // (the missing-chrome bug the switch/checkbox glyph waves hit). So construct THEME-INDEPENDENTLY via
        // the 4-arg (Context, AttributeSet, int defStyleAttr, int defStyleRes) ctor with defStyleAttr=0 and
        // defStyleRes = android.R.style.Widget_Material_Light_EditText (a concrete style resource that
        // CARRIES the field chrome — read with GetStaticFieldID since it is a static field). Then fall back
        // to the 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so the widget is never
        // null. The ImeOptions = Done is AppCompat-only (header deviations); the multi-line knobs ARE ported
        // below.
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
        // SetSingleLine(false): the multi-line construction the task calls out (without it EditText is
        // a single-line field that ignores TextFlagMultiLine on the input type).
        call_void_bool(env.get(), widget.get(), "setSingleLine", JNI_FALSE);
        // SetHorizontallyScrolling(false): wrap instead of scrolling horizontally (multi-line wrap).
        call_void_bool(env.get(), widget.get(), "setHorizontallyScrolling", JNI_FALSE);
        // TextAlignment = ViewStart + Gravity = Top: the editor's default leading/top alignment.
        call_void_int(env.get(), widget.get(), "setTextAlignment", k_text_alignment_view_start);
        call_void_int(env.get(), widget.get(), "setGravity", k_gravity_top);

        // Tint the framework EditText's underline 9-patch to MAUI's rendered gray. The @android:drawable/
        // edit_text 9-patch is an alpha mask tinted by the host theme's colorControlNormal; under the bare
        // Theme.DeviceDefault app_process host that resolves to a dark blue-gray (~#40484D), but real MAUI's
        // AppCompat/Material EditText renders the at-rest underline at #666666. Seed the background tint so
        // the underline matches (SRC_IN so the tint replaces the mask color). Same idiom as the search_bar
        // underline tint + the check_box glyph seed; best-effort (a JNI miss leaves the framework chrome).
        // DARK: MAUI's Material dark underline is the brighter #B8B8B8 (measured), so fork on night mode
        // (mirrors the entry underline dark seed).
        const jint k_editor_underline_tint = maui::platform::android::detail::is_night_mode(env.get())
                                                 ? static_cast<jint>(0xFFB8B8B8U)
                                                 : static_cast<jint>(0xFF666666U);
        if (jclass csl_class = cache.find_class(env.get(), "android/content/res/ColorStateList"))
        {
            jmethodID value_of = cache.static_method(env.get(), "android/content/res/ColorStateList", "valueOf",
                                                     "(I)Landroid/content/res/ColorStateList;");
            jmethodID set_bg_tint = cache.method(env.get(), k_edit_text_class, "setBackgroundTintList",
                                                 "(Landroid/content/res/ColorStateList;)V");
            if (value_of != nullptr && set_bg_tint != nullptr)
            {
                const local_ref<jobject> tint{
                    env.get(), env->CallStaticObjectMethod(csl_class, value_of, k_editor_underline_tint)};
                if (!clear_pending(env.get()) && tint)
                {
                    env->CallVoidMethod(widget.get(), set_bg_tint, tint.get());
                    clear_pending(env.get());
                }
            }
        }

        // Wrap-content LayoutParams up front: a parentless TextView with null LayoutParams NPEs in
        // checkForRelayout on any setText AFTER the first measure (TextView.java reads
        // getLayoutParams().width). The android container fan-out has not arrived, so the partial stands
        // in for the parent ViewGroup attach (identical to the button partial's note).
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~editor_platform
        return platform;
    }

    void editor_handler::on_connect_handler(editor_platform& platform)
    {
        // EditorHandler.Android ConnectHandler installs platformView.TextChanged += OnTextChanged and
        // FocusChange += OnFocusChange (→ Completed). The android TextWatcher / OnFocusChangeListener
        // trampolines are deferred with the gesture/text-watcher fan-out (header deviations), EXACTLY
        // like the button partial defers its OnTouchListener — but the C++ callbacks stay wired so the
        // VM-less cross-platform suite (and a future trampoline) can drive them, and they keep the
        // headless mirror's last_known_text live so an inbound edit can supply the (old, new) pair.
        platform.on_text_changed = [this](const std::string& old_value, const std::string& new_value) {
            if (auto* platform_view = typed_platform_view())
            {
                platform_view->last_known_text = new_value;
            }
            if (auto* view = virtual_view())
            {
                view->send_text_changed(old_value, new_value);
            }
        };
        platform.on_completed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_completed();
            }
        };
    }

    void editor_handler::on_disconnect_handler(editor_platform& platform)
    {
        // DisconnectHandler: TextChanged -= OnTextChanged; FocusChange -= OnFocusChange; (the native
        // trampoline uninstall lands with the deferred listener fan-out).
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
    }

    void editor_handler::map_text(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text());
        platform->last_known_text = platform->text;
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
        // EditTextExtensions.UpdateText(IEditor): editText.Text = editor.Text; SetSelection(Text.Length).
        // to_jstring goes through the real-UTF-8 path (supplementary-plane safe — see jni_string.hpp).
        jmethodID set_text = cache.method(env.get(), k_edit_text_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr)
        {
            const local_ref<jstring> text = to_jstring(env.get(), view.text());
            env->CallVoidMethod(widget, set_text, text.get());
            clear_pending(env.get());
        }
        // SetSelection(Text.Length): keep the caret at the end after a programmatic set (the comment in
        // UpdateText: setting the text resets the cursor to position zero).
        jmethodID set_selection = cache.method(env.get(), k_edit_text_class, "setSelection", "(I)V");
        if (set_selection != nullptr)
        {
            env->CallVoidMethod(widget, set_selection, static_cast<jint>(view.text().size()));
            clear_pending(env.get());
        }
    }

    void editor_handler::map_text_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // EditTextExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()). The null branch
            // (restore the theme default) collapses for the port's non-nullable color (header deviations);
            // the ColorStateList path is replaced by the int overload (header deviations).
            //
            // The port's non-nullable TextColor default (color{}) is opaque BLACK — correct on the LIGHT
            // white field, but invisible on MAUI's #121212 DARK surface where MAUI leaves the EditText's
            // WHITE textColorPrimary. Discriminate on BindableObject.IsSet (C#'s `!= null` stand-in) and
            // seed white when unset + night; light + explicit-color paths unchanged. Mirrors label_handler.
            const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            const jint argb = (!color_is_set && maui::platform::android::detail::is_night_mode(env.get()))
                                  ? static_cast<jint>(0xFFFFFFFFU)
                                  : static_cast<jint>(view.text_color().to_int());
            call_void_int(env.get(), widget_of(*platform), "setTextColor", argb);
        }
    }

    void editor_handler::map_placeholder(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->placeholder = std::string(view.placeholder());
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // EditTextExtensions.UpdatePlaceholder: editText.Hint = textInput.Placeholder (the Hint == guard
        // is a redundant-set optimization, not behavior; the unconditional set lands the same value).
        jmethodID set_hint =
            default_jni_cache().method(env.get(), k_edit_text_class, "setHint", "(Ljava/lang/CharSequence;)V");
        if (set_hint != nullptr)
        {
            const local_ref<jstring> hint = to_jstring(env.get(), view.placeholder());
            env->CallVoidMethod(widget_of(*platform), set_hint, hint.get());
            clear_pending(env.get());
        }
    }

    void editor_handler::map_placeholder_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->placeholder_color = view.placeholder_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // EditTextExtensions.UpdatePlaceholderColor discriminates on `placeholderTextColor is null`:
            // SET → SetHintTextColor(color); UNSET → resolve android.R.attr.textColorHint from the theme
            // and SetHintTextColor to THAT (the native EditText hint gray). The port's Color is a
            // NON-nullable value type (default color{} = opaque BLACK), so a `!= color{}` compare cannot
            // stand in for `!= null` — it misreads an explicit PlaceholderColor=Black as unset AND (the bug
            // this fix closes) rendered every unset placeholder solid BLACK, indistinguishable from real
            // text. Discriminate instead on BindableObject.IsSet (is_property_set("placeholder_color")) —
            // the faithful stand-in for `!= null`, exactly the sentinel activity_indicator_handler::map_color
            // / button_handler::map_text_color use. On the UNSET branch, positively assert the measured
            // native EditText hint gray (#666666, the android textColorHint the maui-compare reference
            // samples) instead of black — reproducing C#'s theme-textColorHint *result* deterministically
            // (this AAR-less backend cannot rely on the force-styled Context resolving the theme attr to
            // that gray). An explicit PlaceholderColor still overrides via the SET branch. (ColorStateList
            // → int overload — header deviations.)
            constexpr jint k_native_default_hint_color = static_cast<jint>(0xFF666666U);
            const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("placeholder_color");
            const jint argb =
                color_is_set ? static_cast<jint>(view.placeholder_color().to_int()) : k_native_default_hint_color;
            call_void_int(env.get(), widget_of(*platform), "setHintTextColor", argb);
        }
    }

    void editor_handler::map_is_read_only(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_read_only = view.is_read_only();
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
        // EditTextExtensions.UpdateIsReadOnly(IEditor): bool editable = !editor.IsReadOnly;
        // FocusableInTouchMode = editable; Focusable = editable; SetCursorVisible(editable). (NOTE: the
        // C# editor overload reuses the variable name `isReadOnly` but assigns `!editor.IsReadOnly` to
        // it — it is the EDITABLE flag; the entry overload SetInputType too, but the editor overload
        // does not.)
        const jboolean editable = static_cast<jboolean>(!view.is_read_only());
        call_void_bool(env.get(), widget, "setFocusableInTouchMode", editable);
        call_void_bool(env.get(), widget, "setFocusable", editable);
        call_void_bool(env.get(), widget, "setCursorVisible", editable);
    }

    void editor_handler::map_max_length(editor_handler& handler, i_editor& view)
    {
        // EditTextExtensions.UpdateMaxLength → PlatformInterop.UpdateMaxLength sets an InputFilter[]
        // LengthFilter. The port enforces max_length control-side (editor::set_max_length truncates the
        // stored text, like entry), so the native filter is a documented no-op; the value is mirrored and
        // the text already arrived truncated (header deviations). // TODO: verify against
        // src/Core/src/Platform/Android/EditTextExtensions.cs (UpdateMaxLength → SetLengthFilter).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->max_length = view.max_length();
        }
    }

    void editor_handler::map_font(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
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
        // identical to the button partial's map_font): base = family ? Typeface.create(family,
        // ToTypefaceStyle(weight, italic)) : Typeface.DEFAULT, then the API-28+ refinement
        // Typeface.create(base, weight, italic).
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

    void editor_handler::map_character_spacing(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
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

    void editor_handler::map_horizontal_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
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
        // TextViewExtensions.UpdateHorizontalTextAlignment → UpdateHorizontalAlignment (EditText): on
        // RTL-capable Android (every device the port targets) it sets BOTH TextAlignment and the
        // horizontal gravity bits — "text alignment does not work at runtime, so we also need gravity".
        // The horizontal-gravity re-masking on top of the existing Gravity is done native-side by the
        // single setTextAlignment here (the gravity half needs a getGravity round-trip; deferred with the
        // gravity-mask helper — the TextAlignment push is the runtime-effective one).
        // TODO: verify against src/Core/src/Platform/Android/TextAlignmentExtensions.cs
        // (UpdateHorizontalAlignment's Gravity re-mask) when the getGravity round-trip lands.
        jmethodID set_text_alignment = cache.method(env.get(), k_edit_text_class, "setTextAlignment", "(I)V");
        if (set_text_alignment != nullptr)
        {
            env->CallVoidMethod(widget, set_text_alignment, to_text_alignment(view.horizontal_text_alignment()));
            clear_pending(env.get());
        }
    }

    void editor_handler::map_vertical_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
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
        // TextAlignmentExtensions.UpdateVerticalAlignment(EditText): Gravity = (Gravity & ~VerticalMask)
        // | ToVerticalGravityFlags(alignment). Read-modify-write the existing gravity so the horizontal
        // bits set at construction (Top includes vertical Top) survive.
        jmethodID get_gravity = cache.method(env.get(), k_edit_text_class, "getGravity", "()I");
        jmethodID set_gravity = cache.method(env.get(), k_edit_text_class, "setGravity", "(I)V");
        if (get_gravity == nullptr || set_gravity == nullptr)
        {
            return;
        }
        const jint current = env->CallIntMethod(widget, get_gravity);
        if (clear_pending(env.get()))
        {
            return;
        }
        const jint updated = (current & ~k_gravity_vertical_mask) | to_vertical_gravity(view.vertical_text_alignment());
        env->CallVoidMethod(widget, set_gravity, updated);
        clear_pending(env.get());
    }

    void editor_handler::map_is_text_prediction_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // EditTextExtensions.UpdateIsTextPredictionEnabled toggles InputType's TextFlagAutoCorrect.
            // The port recomputes the FULL input type (keyboard base + prediction + spellcheck + the
            // editor multi-line bit) so the three intertwined properties stay consistent — SetInputType's
            // shape (header deviations).
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void editor_handler::map_is_spell_check_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // EditTextExtensions.UpdateIsSpellCheckEnabled toggles InputType's TextFlagNoSuggestions;
            // recompute the full type (see map_is_text_prediction_enabled).
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void editor_handler::map_keyboard(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // EditTextExtensions.UpdateKeyboard(IEditor) → SetInputType: keyboard base type + prediction/
            // spellcheck + the editor multi-line bit. (MapKeyboard in C# also re-pushes Text first to
            // restore the caret after the input-type change; the caret restore is the deferred SetSelection
            // round-trip — header deviations — and map_text already lands the text.)
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void editor_handler::map_cursor_position(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // EditTextExtensions.UpdateCursorPosition → SetSelection(start) (clamped to the text length). The
        // full UpdateCursorSelection logic (the looper Post when focused, the RTL-selection preservation)
        // is deferred with the focus/selection-changed seam (header deviations); the best-effort
        // SetSelection here lands the clamped caret synchronously.
        // TODO: verify against src/Core/src/Platform/Android/EditTextExtensions.cs (UpdateCursorSelection).
        const auto length = static_cast<int>(view.text().size());
        const jint start = static_cast<jint>(std::max(0, std::min(view.cursor_position(), length)));
        call_void_int(env.get(), widget_of(*platform), "setSelection", start);
    }

    void editor_handler::map_selection_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
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
        // EditTextExtensions.UpdateSelectionLength → SetSelection(start, end). start = clamp(cursor),
        // end = clamp(start + selectionLength). The native-RTL / looper-Post nuances are deferred (header
        // deviations); the two-arg SetSelection lands the clamped range synchronously.
        // TODO: verify against EditTextExtensions.cs (GetSelectionStart/GetSelectionEnd) with the focus seam.
        const auto length = static_cast<int>(view.text().size());
        const int start = std::max(0, std::min(view.cursor_position(), length));
        const int end = std::max(start, std::min(length, start + view.selection_length()));
        jmethodID set_selection = default_jni_cache().method(env.get(), k_edit_text_class, "setSelection", "(II)V");
        if (set_selection != nullptr)
        {
            env->CallVoidMethod(widget, set_selection, static_cast<jint>(start), static_cast<jint>(end));
            clear_pending(env.get());
        }
    }

    maui::graphics::size editor_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (a multi-line view ~150pt
            // wide, clamped to a finite width constraint, a few lines tall), so the backend-agnostic
            // size-request suites see consistent numbers in the pure-native run.
            double width = 150.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 66.0};
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
        // as dp (Context.FromPixels). Identical to the button partial.
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

    void editor_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange (EditorHandler.Android wraps it with PrepareForTextViewArrange, a
        // text-view re-layout nicety that needs the measured-min-height cache; deferred): the dp frame
        // becomes pixels, the view measures Exactly at the final size (Android requires a measure pass
        // before layout) and lays out. Identical to the button partial.
        // TODO: verify against src/Core/src/Platform/Android/.../PrepareForTextViewArrange when the
        // text-view arrange-prep seam lands.
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

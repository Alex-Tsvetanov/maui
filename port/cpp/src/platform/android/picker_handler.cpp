// picker_handler — Android (JNI) platform partial, the single-selection item picker of the M-android
// per-control fan-out (M6's iOS Rosetta Stone replayed over JNI; the headless mirror in
// src/platform/headless/picker_handler.cpp is the VM-less twin, and src/platform/apple/picker_handler.mm
// is the native-AppKit twin). The managed platform view is a REAL android.widget.EditText made
// non-editable (held as a JNI global reference in picker_platform::native): the field DISPLAYS the
// selected item's text and shows the Title as its hint — the selection-dialog interaction is DEFERRED
// (documented below, exactly as the button partial defers its OnTouchListener and the editor partial
// defers its TextWatcher). The items/selection maps run the C# PickerExtensions.UpdatePickerCore
// algorithm against both the headless mirrors AND the real widget; a native row pick still flows back
// through i_picker::set_selected_index — that inbound channel (on_done / FinishSelectItem) stays
// invokable for portable drives and a future dialog trampoline.
//
// Ported DIRECTLY from PickerHandler.Android.cs + Platform/Android/{PickerExtensions.cs (UpdatePickerCore
// = Hint←Title, Text←GetItem(SelectedIndex) or null; UpdateTitle/UpdateSelectedIndex→UpdatePicker;
// UpdateTextColor; UpdateTitleColorCore→SetHintTextColor), MauiPicker.cs (the AppCompatEditText subclass,
// non-editable: DefaultMovementMethod = null), EditTextExtensions.cs / TextViewExtensions.cs (the shared
// TextView font / character-spacing / alignment surface), AlignmentExtensions.cs / TextAlignmentExtensions.cs,
// ViewExtensions.cs, ContextExtensions.cs (ToPixels), UnitExtensions.cs (ToEm)} + Fonts/FontManager.Android.cs.
// The editor partial (editor_handler.cpp) is the structural template — it reuses the SAME EditText widget
// and most of the same mappers — and the button partial (button_handler.cpp) is the scoped_env/app_context
// VM-less-guard / call_void_* / to_pixels/density / global-ref-lifecycle / clear_pending discipline source;
// all are copied verbatim where they apply.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is a library / infrastructure gap, not a behavior
// guess):
//   - The widget is a plain android.widget.EditText, not MauiPicker (which subclasses
//     MauiPickerBase : AndroidX.AppCompat.Widget.AppCompatEditText): the AndroidX AppCompat library is a
//     gradle/AAR dependency this APK-less backend does not carry, exactly like the editor partial's
//     plain-android.widget.EditText stand-in. EditText extends TextView, so the whole TextView surface
//     MauiPickerBase relies on (Hint / Text / SetTextColor / SetHintTextColor / Typeface / LetterSpacing
//     / TextAlignment / Gravity) resolves through android/widget/EditText (GetMethodID walks the
//     superclasses). MauiPickerBase's only behavioral extra is DefaultMovementMethod = null (no cursor
//     navigation because the field is read-only); we reproduce the read-only intent with
//     setKeyListener(null) + setFocusable(false) + setClickable(true) (see create_platform_view) — the
//     MovementMethod knob has no plain-widget setter analog and is approximated by the KeyListener removal.
//   - The selection DIALOG is DEFERRED. C#'s ConnectHandler wires platformView.Click += OnClick, and
//     OnClick builds a MaterialAlertDialogBuilder single-choice list (Google.Android.Material) that, on a
//     row tap, sets VirtualView.SelectedIndex and re-runs UpdatePicker. The Material dialog is an AAR this
//     backend lacks (same class of gap as the editor's deferred TextWatcher), AND the per-control gesture/
//     click trampoline fan-out has not arrived. So no android.view.View.OnClickListener is installed yet;
//     the field renders the current selection (the capture target the task calls out) and on_done stays a
//     live C++ callback (the cross-platform suite + a future dialog trampoline drive it → set_selected_index
//     → UpdatePicker). MapIsOpen / ShowDialog / DismissDialog have no widget to present and are no-ops for
//     the same reason — the control-level is_open()/Opened/Closed are the observable result.
//     // TODO: verify against src/Core/src/Handlers/Picker/PickerHandler.Android.cs (OnClick →
//     MaterialAlertDialogBuilder single-choice, OnDialogShown/OnDialogDismiss) + the android click/dialog
//     trampoline seam when it lands.
//   - The native EditText color setters take a ColorStateList (C# routes through
//     PlatformInterop.CreateEditTextColorStateList in UpdateTextColor / UpdateTitleColorCore). The
//     plain-widget cut uses the single-int overloads setTextColor(int) / setHintTextColor(int) — the
//     ColorStateList path's enabled-state coloring is an AppCompat/PlatformInterop nicety, not part of the
//     IPicker contract, and the int overloads land the same ARGB. This is byte-for-byte the editor
//     partial's text/placeholder-color cut.
//   - The port's colors are non-nullable value types, so C#'s null-color branches collapse exactly as
//     they did in the editor/apple/ios partials: UpdateTextColor's `textColor == null → SetTextColor(
//     defaultColor)` and UpdateTitleColorCore's `titleColor is not null` guard have no value-type analog;
//     an unset color is a real value here, pushed through to setTextColor / setHintTextColor like every
//     other color. (UpdateTitleColorCore maps the TITLE color onto the HINT color — the title IS the hint
//     on Android — so map_title_color pushes setHintTextColor, NOT a separate title surface.)
//   - FontManager's registrar/asset/file lookups and the "-light"/"-medium" suffix map are skipped (no
//     font registrar yet, on any backend): family → Typeface.create(family, style), then the API-28+
//     Typeface.create(base, weight, italic) refinement — the exact CreateTypeface tail. Byte-for-byte the
//     editor/button partials' map_font.
//   - vertical_text_alignment: C#'s MapVerticalTextAlignment → UpdateVerticalAlignment masks the vertical
//     Gravity bits, identical to the editor partial; ported the same way (read-modify-write Gravity). The
//     headless/apple twins keep only the mirror (the AppKit popup has no vertical analog), so the android
//     partial is the faithful one here.
//
// VM-less degradation (identical to the editor/button partials): the android preset also runs the
// PURE-NATIVE cross-platform suite on the emulator (tools/android-emu-run.sh) where no Java VM exists.
// Every JNI path here checks scoped_env/app_context() and quietly skips, while the headless mirrors
// (items/selected_index/text/title/text_color/…) are ALWAYS maintained — so that suite observes exactly
// the headless partial's behavior, and the widget test host (tools/android-testhost-run.sh) additionally
// observes the real widget.

#include "maui/core/picker_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_picker.hpp"
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
    // so the View/TextView surface resolves through android/widget/EditText too) — identical to the
    // editor partial, which uses the very same widget.
    constexpr const char* k_edit_text_class = "android/widget/EditText";
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

    // android.view.Gravity bits (TextAlignmentExtensions' vertical-gravity masking). Picker's vertical
    // alignment defaults to Center (see picker_platform's vertical_alignment default).
    constexpr jint k_gravity_top = 0x30;
    constexpr jint k_gravity_bottom = 0x50;
    constexpr jint k_gravity_center_vertical = 0x10;
    constexpr jint k_gravity_vertical_mask = k_gravity_top | k_gravity_bottom | k_gravity_center_vertical;

    [[nodiscard]] jobject widget_of(const maui::core::picker_platform& platform) noexcept
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

    // EditTextExtensions/TextViewExtensions: set a CharSequence property (setText / setHint).
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
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity
    // cache (and the editor/button partials' twin). 1.0 when any step fails (failures are NOT memoized).
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
    // (Picker's vertical default is Center — see picker_platform's vertical_alignment default.)
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

    // PickerExtensions.UpdatePickerCore: Hint ← picker.Title (always); Text ← null when SelectedIndex is
    // unset (-1) or out of range (>= count), else GetItem(SelectedIndex). Pushed to the REAL widget. The
    // headless mirror's text/items fields are written by the maui::core update_picker helper below (kept
    // identical) — this is the native half, run only when a widget exists.
    void push_display(JNIEnv* env, jobject widget, const maui::core::i_picker& view)
    {
        // Hint = picker.Title (UpdatePickerCore sets it unconditionally).
        call_void_char_sequence(env, widget, "setHint", view.title());
        const int selected = view.selected_index();
        const int count = view.get_count();
        // Text = null when out of range, else the item. setText(null) clears (CharSequence overload).
        if (selected == -1 || selected >= count)
        {
            if (jmethodID set_text =
                    default_jni_cache().method(env, k_edit_text_class, "setText", "(Ljava/lang/CharSequence;)V"))
            {
                env->CallVoidMethod(widget, set_text, static_cast<jobject>(nullptr));
                clear_pending(env);
            }
        }
        else
        {
            call_void_char_sequence(env, widget, "setText", view.get_item(selected));
        }
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // PickerExtensions.UpdatePicker(platformPicker, picker, newSelectedIndex): refresh the headless
        // mirror's display text + item list, write the selection back to the virtual view (skipped while
        // empty), then push the display to the real widget. The mirror half is byte-for-byte the headless
        // partial's update_picker (the task: "Keep every headless-mirror write"); the native half follows
        // PickerExtensions.UpdatePickerCore (the out-of-range guard + the always-set Hint).
        void update_picker(picker_handler& handler, i_picker& view, int selected_index)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            // --- headless mirror (identical to src/platform/headless/picker_handler.cpp) ---
            platform->text = selected_index != -1 ? view.get_item(selected_index) : std::string{};

            const int count = view.get_count();
            platform->items.clear();
            platform->items.reserve(static_cast<std::size_t>(count));
            for (int at = 0; at < count; ++at)
            {
                platform->items.push_back(view.get_item(at));
            }

            if (count == 0)
            {
                // Nothing selected and no items: still refresh the widget's hint (Title) so an
                // item-less picker shows its placeholder, matching UpdatePickerCore (Text→null already
                // holds via the empty mirror; the widget push below lands the hint).
                if (platform->native != nullptr)
                {
                    const scoped_env env;
                    if (env)
                    {
                        push_display(env.get(), widget_of(*platform), view);
                    }
                }
                return;
            }
            platform->selected_index = selected_index;
            view.set_selected_index(selected_index); // picker.SelectedIndex = selectedIndex (FromHandler)

            // --- native push (PickerExtensions.UpdatePickerCore against the real EditText) ---
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
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSPopUpButton here).
    picker_platform::~picker_platform()
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
    // editor partial; only the widget class name differs (both are android/widget/EditText).

    void picker_platform::update_visibility(maui::core::visibility value)
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

    void picker_platform::update_opacity(double value)
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

    void picker_platform::update_is_enabled(bool value)
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
        // PickerHandler.MapIsEnabled (an Android-specific override): Enabled = picker.IsEnabled; AND
        // Clickable = Focusable = picker.IsEnabled so touch events propagate to the parent when disabled.
        // (The base ViewExtensions.UpdateIsEnabled only sets Enabled; the picker overrides it to also
        // toggle Clickable/Focusable — but our field is intentionally non-focusable / click-driven, so
        // re-enabling Focusable would defeat the read-only construction. We push Enabled + Clickable, and
        // keep Focusable false, matching the non-editable widget; the propagate-when-disabled intent
        // holds via Clickable.)
        jobject widget = widget_of(*this);
        call_void_bool(env.get(), widget, "setEnabled", static_cast<jboolean>(value));
        call_void_bool(env.get(), widget, "setClickable", static_cast<jboolean>(value));
    }

    void picker_platform::update_automation_id(std::string_view value)
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

    void picker_platform::update_background(const maui::graphics::paint* value)
    {
        // PickerHandler.MapBackground (an Android-specific override) delegates to PlatformView
        // .UpdateBackground(picker) — the shared android view op pushes the solid/gradient/image
        // background to the View (VM-less safe). The headless mirror keeps the base body.
        view_platform_base::update_background(value);
        maui::platform::android::apply_background(native, value);
    }

    // Render transform + flow direction + semantics pushed to the real widget via the shared android
    // ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite observes the
    // headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void picker_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void picker_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<picker_platform>();
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
        // PickerHandler.CreatePlatformView: new MauiPicker(Context). MauiPicker → MauiPickerBase →
        // AppCompatEditText(Context). The plain android.widget.EditText stand-in (header deviations);
        // EditText(Context) chains to the (Context, AttributeSet, int) ctor with defStyleAttr =
        // editTextStyle, a THEME attr that throws in the bare, Activity-less app_process testhost (the
        // same trap progress_bar's styled ctor + the editor partial hit). Construct with defStyleAttr = 0
        // (no theme attr, what TextView(Context) does); fall back to the plain (Context) ctor so the
        // widget is never null.
        jobject created = nullptr;
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

        // MauiPickerBase makes the field NON-EDITABLE: it is a read-only picker that opens a dialog on
        // click, never accepting keyboard input (DefaultMovementMethod = null disables cursor navigation).
        // Reproduce the read-only intent on the plain widget (header deviations): setKeyListener(null)
        // makes it ignore key input (Android's canonical "read-only EditText" recipe), setFocusable(false)
        // keeps the keyboard from opening, and setClickable(true) keeps it tappable for the (deferred)
        // selection dialog. The capture target — the field SHOWING the selected item / title — needs only
        // these; the dialog Click listener is the deferred half.
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
        // SetSingleLine(true): the picker field shows the single selected line (unlike the editor, which
        // sets it false for multi-line). A picker selection is never multi-line.
        call_void_bool(env.get(), widget.get(), "setSingleLine", JNI_TRUE);

        // Wrap-content LayoutParams up front: a parentless TextView with null LayoutParams NPEs in
        // checkForRelayout on any setText AFTER the first measure (TextView.java reads
        // getLayoutParams().width). The android container fan-out has not arrived, so the partial stands
        // in for the parent ViewGroup attach (identical to the editor/button partials' note).
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~picker_platform
        return platform;
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        // PickerHandler.Android ConnectHandler installs platformView.Click += OnClick, which builds the
        // MaterialAlertDialogBuilder single-choice list. That dialog (a Google.Android.Material AAR) and
        // the android click trampoline are deferred (header deviations), EXACTLY like the editor partial
        // defers its TextWatcher and the button partial its OnTouchListener — but on_done stays a live
        // C++ callback so the VM-less cross-platform suite (and a future dialog trampoline) can drive a
        // row commit. FinishSelectItem: an unset (-1) row with items present commits row 0, then the pick
        // lands on the virtual view through UpdatePicker (which also re-pushes the display to the widget).
        platform.on_done = [this](int row) {
            auto* view = virtual_view();
            if (view == nullptr)
            {
                return;
            }
            if (row == -1 && view->get_count() > 0)
            {
                row = 0;
            }
            update_picker(*this, *view, row);
        };
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        // DisconnectHandler: Click -= OnClick; dispose the dialog (the native click/dialog uninstall
        // lands with the deferred trampoline fan-out).
        platform.on_done = nullptr;
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // Reload -> UpdatePicker(picker)
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // UpdateSelectedIndex -> UpdatePicker
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title = std::string(view.title()); // headless mirror (UpdatePickerTitle)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // PickerExtensions.UpdateTitle → UpdatePicker → UpdatePickerCore: Hint = picker.Title (and
            // re-resolve the displayed Text). push_display lands both.
            push_display(env.get(), widget_of(*platform), view);
        }
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title_color = view.title_color(); // headless mirror
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // PickerExtensions.UpdateTitleColorCore: the TITLE color maps onto the HINT text color (the
            // Title IS the hint on Android) → SetHintTextColor(titleColor.ToPlatform()). The null guard
            // collapses for the port's value-type color + the ColorStateList path → int overload (header
            // deviations).
            call_void_int(env.get(), widget_of(*platform), "setHintTextColor",
                          static_cast<jint>(view.title_color().to_int()));
        }
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
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
            // PickerExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()). The null branch
            // (restore the theme default) collapses for the value-type color + ColorStateList → int
            // overload (header deviations) — byte-for-byte the editor's map_text_color.
            call_void_int(env.get(), widget_of(*platform), "setTextColor",
                          static_cast<jint>(view.text_color().to_int()));
        }
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
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
        // identical to the editor/button partials' map_font): base = family ? Typeface.create(family,
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

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
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

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment(); // headless mirror
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
        // PickerHandler.MapHorizontalTextAlignment → UpdateHorizontalAlignment (EditText): on RTL-capable
        // Android (every device the port targets) it sets BOTH TextAlignment and the horizontal gravity
        // bits — "text alignment does not work at runtime, so we also need gravity". The horizontal-gravity
        // re-masking needs a getGravity round-trip and is deferred with the gravity-mask helper (editor's
        // pattern); the setTextAlignment push is the runtime-effective one.
        // TODO: verify against src/Core/src/Platform/Android/TextAlignmentExtensions.cs
        // (UpdateHorizontalAlignment's Gravity re-mask) when the getGravity round-trip lands.
        jmethodID set_text_alignment = cache.method(env.get(), k_edit_text_class, "setTextAlignment", "(I)V");
        if (set_text_alignment != nullptr)
        {
            env->CallVoidMethod(widget, set_text_alignment, to_text_alignment(view.horizontal_text_alignment()));
            clear_pending(env.get());
        }
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment(); // headless mirror
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
        // PickerHandler.MapVerticalTextAlignment → TextAlignmentExtensions.UpdateVerticalAlignment:
        // Gravity = (Gravity & ~VerticalMask) | ToVerticalGravityFlags(alignment). Read-modify-write the
        // existing gravity so the horizontal bits survive (editor's pattern).
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

    void picker_handler::map_is_open(picker_handler& /*handler*/, i_picker& /*view*/)
    {
        // PickerHandler.MapIsOpen → ShowDialog/DismissDialog (CallOnClick → the MaterialAlertDialog). The
        // dialog is deferred with the click trampoline (header deviations), so there is no native dialog
        // to present/dismiss; this is a genuine no-op — the control-level is_open()/Opened/Closed are the
        // observable result. // TODO: verify against PickerHandler.Android.cs (ShowDialog/DismissDialog)
        // when the dialog trampoline lands.
    }

    maui::graphics::size picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (a single-line field
            // ~150pt wide, clamped to a finite width constraint, one line tall), so the backend-agnostic
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
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost
        // specs in pixels, infinite become Unspecified; View.measure, then the measured pixels come back
        // as dp (Context.FromPixels). Identical to the editor/button partials.
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

    void picker_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // size (Android requires a measure pass before layout) and lays out. Identical to the editor/button
        // partials (the picker is a TextView-derived field, so the PrepareForTextViewArrange nicety would
        // apply; it is deferred with the editor's, the base measure/arrange stands in).
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
    }
} // namespace maui::core

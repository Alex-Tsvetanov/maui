// search_bar_handler — Android (JNI) platform partial, the single-line search-input control of the
// M-android per-control fan-out (M6's iOS Rosetta Stone replayed over JNI; the headless mirror in
// src/platform/headless/search_bar_handler.cpp is the VM-less twin, and the apple/NSSearchField partial
// in src/platform/apple/search_bar_handler.mm is the real-native twin). The managed platform view is a
// plain android.widget.EditText (held as a JNI global reference in search_bar_platform::native): every
// map_* pushes its property through the jni_cache'd method ids, and a native edit / search-key press
// flows back through the C++ callbacks (on_text_changed / on_search_button_pressed) into
// i_search_bar::send_text_changed / send_search_button_pressed — exactly the inbound channel the headless
// mirror exposes.
//
// Ported DIRECTLY from SearchBarHandler.Android.cs + Platform/Android/{SearchViewExtensions.cs,
// EditTextExtensions.cs, TextViewExtensions.cs, TextAlignmentExtensions.cs, AlignmentExtensions.cs,
// KeyboardExtensions.cs, ViewExtensions.cs, ContextExtensions.cs (ToPixels), UnitExtensions.cs (ToEm)} +
// Fonts/FontManager.Android.cs. The editor partial (editor_handler.cpp) is the structural template — a
// search bar is just a single-line EditText stand-in — so the scoped_env/app_context VM-less guards, the
// call_void_* helpers, to_pixels/display_density, clear_pending-after-every-call discipline, the global-
// ref lifecycle, and the per-backend view_platform_base overrides are copied from it verbatim where they
// apply, with the editor-only multi-line knobs removed.
//
// THE EDITTEXT STAND-IN (the central documented deviation): MAUI's Android SearchBar is a MauiSearchView
// (androidx.appcompat.widget.SearchView), whose visible text field is the inner EditText queryEditor
// (MauiSearchView._queryEditor = this.GetFirstChildOfType<EditText>()). The whole Android text surface of
// SearchBarHandler — MapText/Placeholder/PlaceholderColor/TextColor/Font/CharacterSpacing/Horizontal+
// VerticalTextAlignment/IsReadOnly/IsTextPrediction/IsSpellCheck/Keyboard/MaxLength/Cursor/Selection —
// is delegated by SearchViewExtensions onto THAT inner EditText (via UpdateText/UpdatePlaceholder/… and
// the EditTextExtensions/TextViewExtensions the editor partial already ports). Because this APK-less
// backend deliberately carries NO AndroidX AppCompat AAR (the same gradle/AAR gap the button and editor
// partials document for MauiAppCompatButton / MauiAppCompatEditText), there is no SearchView to host;
// the port stands the search bar up as the inner EditText DIRECTLY — exactly the picker/editor EditText
// stand-in approach — and addresses every text map against android/widget/EditText. This is faithful:
// the inner queryEditor IS an EditText, and the C# maps target it, not the SearchView chrome.
//
// THE SEARCHVIEW CHROME (magnifier + inset + clear-X), reproduced with FRAMEWORK compound drawables:
// MAUI's visible SearchView chrome is a LEFT magnifier (search_mag_icon), an inset/indented query field,
// and a RIGHT clear-X (search_close_btn) that appears only with text. The EditText stand-in reproduces
// all three WITHOUT any SearchView by hanging framework drawables (android.R.drawable.ic_menu_search /
// ic_menu_close_clear_cancel) on the EditText as compound drawables
// (setCompoundDrawablesWithIntrinsicBounds) plus a setCompoundDrawablePadding inset. FRAMEWORK drawables
// are the deliberate choice: they resolve theme-independently (via Context.getDrawable(int)) so they load
// in the bare, Activity-less app_process testhost too — unlike the AppCompat abc_ic_* set the entry
// partial documents as unavailable (no AAR), and unlike the framework android.widget.SearchView itself
// (in android.jar since API 11 but inflating a fragile themed layout — its SearchAutoComplete needs
// ?attr/autoCompleteTextViewStyle etc. — that is prone to crash in the theme-less host, so the real
// SearchView is DEFERRED). This gives the SAME visual (loupe + inset + text-gated clear-X) in BOTH the
// bare testhost and the real app host. Mirrored C#: MauiSearchView.Initialize's SetIconifiedByDefault
// (false) → the always-shown magnifier; SearchViewExtensions.UpdateSearchIconColor / UpdateCancelButton
// Color → the LEFT / RIGHT drawable tints (see map_search_icon_color / map_cancel_button_color); the
// search_close_btn's text-gated visibility → the clear-X-shown-only-with-text rule (see map_text).
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is a library / infrastructure gap, not a behavior
// guess):
//   - The widget is a plain android.widget.EditText, not a SearchView's inner SearchAutoComplete: the
//     AndroidX AppCompat library is a gradle/AAR dependency this backend does not carry (as above), and
//     the framework android.widget.SearchView is DEFERRED (fragile themed inflation in the Activity-less
//     host). The SearchView's magnifier + clear-X ARE rendered — as framework compound drawables on the
//     EditText (see the SEARCHVIEW CHROME note above) — so cancel_button_color / search_icon_color are NO
//     LONGER mirror-only: UpdateCancelButtonColor tints the RIGHT clear-X drawable and UpdateSearchIcon
//     Color tints the LEFT magnifier drawable (map_cancel_button_color / map_search_icon_color). Still
//     DEFERRED to mirror-only (no plain-EditText / framework-drawable analog):
//       * return_type (UpdateReturnType — sets SearchView.ImeOptions + the inner EditText.ImeOptions and
//                      RestartInput); the IME-options push has no observable surface on the bare testhost
//                      (no soft keyboard) and there is no SearchView ImeOptions target.
//     map_return_type therefore records the cross-platform mirror only, like the apple/AppKit twin.
//     // TODO: verify against src/Core/src/Platform/Android/SearchViewExtensions.cs (UpdateReturnType)
//     when a soft-keyboard-aware app host lands, and re-point the icon tints at the real search_mag_icon /
//     search_close_btn ImageViews if an AppCompat-equivalent SearchView shell ever lands.
//   - MaxWidth = int.MaxValue / the queryEditor LayoutParams FillVertical tweaks / the search_close_btn
//     min-width (MauiSearchView.Initialize) are SearchView-shell setup with no plain-EditText analog;
//     skipped. SetIconifiedByDefault(false) — the expand-to-show-the-magnifier call — is reproduced not
//     by a SearchView method but by the always-installed LEFT magnifier compound drawable (above).
//   - MapBackground rides the shared view_mapper (the C# MapBackground delegates straight to
//     PlatformView.UpdateBackground); update_background pushes the solid/gradient/image background to the
//     View via the shared android op (VM-less safe), like the editor partial.
//   - The port's colors are non-nullable value types (default color{} = opaque BLACK), so C#'s null-color
//     branches have no direct value-type analog. UpdateTextColor's "restore the theme default"
//     (TryGetDefaultStateColor) else-branch is dropped (an unset TextColor is pushed through setTextColor
//     like every other color — TextColor is not one of the parity yellows). UpdatePlaceholderColor's null
//     branch (resolve android.R.attr.textColorHint, SetHintTextColor to it) IS honored: map_placeholder_
//     color discriminates on BindableObject.IsSet and, when UNSET, asserts the measured native EditText
//     hint gray (#666666) instead of black — the unset-color-default family fix (see map_placeholder_color
//     / entry_handler / editor_handler), so an empty search bar shows a gray query-hint matching MAUI, not
//     solid black. An explicit PlaceholderColor still overrides. The magnifier IS present now (a compound
//     drawable), but its co-tint inside UpdateTextColor / UpdatePlaceholderColor (C# tints search_mag_icon
//     to the text/placeholder colour as a fallback) is NOT applied here — the loupe tint is driven by
//     SearchIconColor (map_search_icon_color) instead; the text/placeholder-colour co-tint is a minor
//     fallback the ISearchBar contract does not require. // TODO: verify against SearchViewExtensions.cs
//     (UpdateTextColor / UpdatePlaceholderColor search_mag_icon co-tint) if that fallback tint is ever needed.
//   - The native EditText color setters take a ColorStateList (C# routes through
//     PlatformInterop.CreateEditTextColorStateList). The plain-widget cut uses the single-int overloads
//     setTextColor(int) / setHintTextColor(int) — the ColorStateList path's enabled-state coloring is an
//     AppCompat/PlatformInterop nicety, not part of the ISearchBar contract, and the int overloads land
//     the same ARGB. (Identical to the editor partial.)
//   - FontManager's registrar/asset/file lookups and the "-light"/"-medium" suffix map are skipped (the
//     port has no font registrar yet, on any backend): family → Typeface.create(family, style), then the
//     API-28+ Typeface.create(base, weight, italic) refinement — byte-for-byte the editor partial's
//     map_font (which is byte-for-byte the button partial's).
//   - max_length is enforced control-side (search_bar::set_max_length / the InputView truncation, like
//     entry/editor), so the native InputFilter[] LengthFilter + the SetQuery(trimmedQuery) re-push
//     (SearchViewExtensions.UpdateMaxLength) are a documented no-op here: the value is mirrored into
//     search_bar_platform::max_length and the truncation already happened before the text reached the
//     widget. // TODO: verify against src/Core/src/Platform/Android/SearchViewExtensions.cs
//     (UpdateMaxLength → SetLengthFilter / TrimToMaxLength) when the native InputFilter seam lands.
//   - SINGLE-LINE: unlike the editor partial (a multi-line IEditor → SetSingleLine(false) + the InputType
//     TextFlagMultiLine bit + Gravity = Top), a search bar is single-line. apply_input_type here does NOT
//     OR-in TextFlagMultiLine, the widget keeps EditText's default single-line behaviour, and the
//     vertical alignment defaults to CENTER (search_bar_platform::vertical_alignment = center), matching
//     SearchViewExtensions.UpdateVerticalTextAlignment's TextAlignment.Center.ToVerticalGravityFlags()
//     fallback and the inner SearchAutoComplete's centered single line.
//   - The QueryTextChange / QueryTextSubmit / focus listeners (SearchBarHandler.Android ConnectHandler:
//     platformView.QueryTextChange += OnQueryTextChange → VirtualView.UpdateText; QueryTextSubmit +=
//     OnQueryTextSubmit → VirtualView.SearchButtonPressed; SetOnQueryTextFocusChangeListener; the
//     QueryEditorTouch/KeyListener cursor/selection trampolines) are DEFERRED with the gesture/text-
//     watcher fan-out, EXACTLY like the editor partial defers its TextWatcher/FocusChange. on_text_changed
//     / on_search_button_pressed stay invokable C++ callbacks (the cross-platform suite drives them); no
//     android.text.TextWatcher / View.OnFocusChangeListener / OnTouchListener / OnKeyListener trampoline
//     is installed yet. // TODO: verify against src/Core/src/Handlers/SearchBar/SearchBarHandler.Android.cs
//     (OnQueryTextChange / OnQueryTextSubmit / OnQueryEditorSelectionChanged) when the android
//     TextWatcher/focus/key trampolines arrive.
//   - Cursor/selection (MapCursorPosition / MapSelectionLength → EditTextExtensions
//     UpdateCursorPosition/UpdateSelectionLength via SetSelection, posted on the looper) are deferred with
//     the same listener fan-out; map_cursor_position / map_selection_length mirror the values and push
//     setSelection best-effort, identical to the editor partial. // TODO: verify against
//     EditTextExtensions.cs (UpdateCursorSelection's editText.Post looper hop) when the focus/looper seam
//     lands.
//
// VM-less degradation (identical to the editor/button partials): the android preset also runs the
// PURE-NATIVE cross-platform suite on the emulator (tools/android-emu-run.sh) where no Java VM exists.
// Every JNI path here checks scoped_env/app_context() and quietly skips, while the headless mirrors
// (text/placeholder/text_color/…) are ALWAYS maintained — so that suite observes exactly the headless
// partial's behavior, and the Android app host (a real Activity) additionally observes the real widget
// (the app_process widget testhost CANNOT construct an EditText — its Editor eagerly queries a Settings
// ContentProvider the shell-uid process may not reach → SecurityException — so the search bar, like the
// editor/entry/switch, is app-host-only; see docs/MACOS_ANDROID_RESUME.md lesson 3).

#include "maui/core/search_bar_handler.hpp"

#include <jni.h>

#include <algorithm>
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
#include "maui/core/i_search_bar.hpp"
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
    // The concrete platform style that carries the EditText's box/underline CHROME (the search field
    // stand-in for the SearchView's inner queryEditor), resolved theme-independently as a defStyleRes so
    // the bare app_process testhost (and the app host) construct a field that actually HAS its chrome. A
    // defStyleAttr=0 ctor with NO defStyleRes resolves no background drawable → a chrome-less field (the
    // missing-chrome bug the switch/checkbox glyph waves hit). The gallery's light Activity theme makes
    // Widget_EditText the matching field chrome: it carries @android:drawable/edit_text (a framework-res
    // 9-patch underline) that renders in the bare app_process host; the Material_Light variant resolves its
    // background to a theme attr the host can't satisfy and paints NOTHING (verified in wave 14), so it is
    // only a fallback. The *_alt fields are tried in turn if absent. (GetStaticFieldID — static fields.)
    // The SearchView's magnifier + clear-X are reproduced as FRAMEWORK compound drawables on this EditText
    // (see k_r_drawable_class below); the search-field rounded background remains DEFERRED (header
    // deviation: no SearchView shell here).
    constexpr const char* k_edit_text_style_field = "Widget_EditText";
    constexpr const char* k_edit_text_style_field_alt = "Widget_Material_EditText";
    constexpr const char* k_edit_text_style_field_alt2 = "Widget_Material_Light_EditText";
    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // FRAMEWORK drawables (android.R.drawable.*) reproducing MAUI's SearchView chrome on the EditText
    // stand-in: the LEFT magnifier (SearchView's search_mag_icon, always shown once
    // MauiSearchView.Initialize's SetIconifiedByDefault(false) expands the field) and the RIGHT clear-X
    // (SearchView's search_close_btn, gated on non-empty text — SearchViewExtensions' text-driven
    // UpdateCancelButtonState). Both are android.R.drawable resources (NOT AppCompat), so they resolve
    // theme-independently in the bare app_process host too — the whole reason framework drawables are
    // chosen over the unavailable AppCompat abc_ic_* set. Read with GetStaticFieldID on android/R$drawable
    // (static ints), exactly like the style fields above. // TODO: verify against
    // src/Core/src/Platform/Android/{MauiSearchView.cs (SetIconifiedByDefault(false) → the always-shown
    // magnifier), SearchViewExtensions.cs (UpdateSearchIconColor/UpdateCancelButtonColor tints, the
    // search_close_btn text-gated visibility)} when an AppCompat-equivalent SearchView (with the real
    // search_mag_icon/search_close_btn ImageViews) lands.
    constexpr const char* k_r_drawable_class = "android/R$drawable";
    constexpr const char* k_search_icon_field = "ic_menu_search";            // the LEFT magnifier
    constexpr const char* k_clear_icon_field = "ic_menu_close_clear_cancel"; // the RIGHT clear-X
    constexpr const char* k_drawable_class = "android/graphics/drawable/Drawable";
    // The icon-to-text inset (CompoundDrawablePadding), a modest gap matching the SearchView's
    // magnifier-to-query indent so the text sits inset from the loupe like MAUI's expanded field.
    constexpr double k_compound_drawable_padding_dp = 8.0;
    // MAUI's Material SearchView icons render ~17dp (magnifier/clear-X); the framework ic_menu_* drawables'
    // intrinsic bounds are ~24dp, which over-inflates the plain EditText row. Set explicit 18dp bounds.
    constexpr double k_search_icon_size_dp = 18.0;
    // MAUI's SearchView plate is a fixed 48dp min-height (Material). The AAR-less DeviceDefault EditText has no
    // such plate, so once the icon no longer inflates it the row measures short; floor it to 48dp (same
    // Material-height-floor pattern as slider/switch) so a page of stacked search bars doesn't drift.
    constexpr double k_material_search_height_dp = 48.0;
    // Default magnifier / clear-X tint when SearchIconColor / CancelButtonColor are UNSET. The framework
    // ic_menu_search / ic_menu_close_clear_cancel drawables render a mid-GRAY (~#868686) untinted, but
    // MAUI's SearchView renders its icons at the theme textColorPrimary (~87% black = #DE000000, measured
    // [33,33,33] on the light board vs the port's gray [134,134,134]). Tint to that dark default on unset
    // so the icons match the reference; an explicit SearchIconColor / CancelButtonColor still overrides.
    constexpr jint k_default_icon_tint = static_cast<jint>(0xDE000000U);
    // MAUI's SearchView plate shows only a FAINT gray field underline (measured rgb ~217,217,217); the
    // DeviceDefault EditText's default 9-patch underline is a dark accent (~rgb 64,72,77). Tint the field
    // background to the faint gray so the underline matches MAUI (the dark line is a per-row structural diff).
    constexpr jint k_underline_tint = static_cast<jint>(0xFFD9D9D9U);

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

    // android.view.Gravity bits (TextAlignmentExtensions' vertical-gravity masking). The search bar's
    // vertical alignment defaults to Center (search_bar_platform::vertical_alignment = center).
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
    constexpr jint k_type_text_flag_no_suggestions = 0x00080000;
    constexpr jint k_type_number_flag_signed = 0x00001000;
    constexpr jint k_type_number_flag_decimal = 0x00002000;
    constexpr jint k_type_datetime_variation_normal = 0x00000000;
    constexpr jint k_type_datetime_variation_time = 0x00000020;

    [[nodiscard]] jobject widget_of(const maui::core::search_bar_platform& platform) noexcept
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
    // cache (and the editor partial's twin). 1.0 when any step fails (failures are NOT memoized).
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
    // (The search bar's vertical default is Center — see search_bar_platform's vertical_alignment default
    // and SearchViewExtensions.UpdateVerticalTextAlignment's TextAlignment.Center fallback.)
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

    // KeyboardExtensions.ToInputType — the named-keyboard / CustomKeyboard branch table, ported verbatim
    // from the editor partial. The result is the BASE input type before SetInputType's prediction /
    // spellcheck overlay (apply_input_type below). UNLIKE the editor, there is NO multi-line bit: a
    // search bar is single-line.
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

    // EditTextExtensions.SetInputType (the search-bar slice): compute the keyboard base type, then OR-in
    // the prediction/spellcheck bits (UpdateIsTextPredictionEnabled / UpdateIsSpellCheckEnabled are
    // SearchViewExtensions methods that toggle exactly these two bits on the inner EditText). NO
    // TextFlagMultiLine — the virtual view is an ISearchBar, not an IEditor (the editor partial's tail
    // `if (textInput is IEditor) InputType |= TextFlagMultiLine` does not apply). Recomputed from
    // i_search_bar each call so any of the three intertwined properties (keyboard / prediction /
    // spellcheck) lands the full type.
    void apply_input_type(JNIEnv* env, jobject widget, const maui::core::i_search_bar& view)
    {
        jint input_type = keyboard_to_input_type(view.keyboard());

        // UpdateIsTextPredictionEnabled / UpdateIsSpellCheckEnabled (skipped for a CustomKeyboard, which
        // already derives these bits from its flags — matching SetInputType's CustomKeyboard guard).
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

        call_void_int(env, widget, "setInputType", input_type);
    }

    // Resolve a framework drawable (android.R.drawable.<field_name>) to a live android.graphics.drawable
    // .Drawable via context.getDrawable(int) (API 21+, in android.jar), tinting it to `tint` when
    // `apply_tint` is set. Returns a MOVED local ref (null on any failure — every id/lookup is guarded,
    // and the caller skips cleanly, mirroring the VM-less degradation discipline the rest of the file
    // uses). Framework resources need no theme, so this loads in the bare app_process host too. The tint
    // uses Drawable.mutate() first so the shared constant-state drawable is not tinted process-wide
    // (SearchViewExtensions.SafeSetTint's "mutate before tinting" rule).
    [[nodiscard]] local_ref<jobject> resolve_framework_drawable(JNIEnv* env, jobject widget, const char* field_name,
                                                                bool apply_tint, jint tint)
    {
        auto& cache = default_jni_cache();
        jclass drawable_r_class = cache.find_class(env, k_r_drawable_class);
        if (drawable_r_class == nullptr)
        {
            return {};
        }
        jfieldID drawable_field = env->GetStaticFieldID(drawable_r_class, field_name, "I");
        if (clear_pending(env) || drawable_field == nullptr)
        {
            return {}; // a missing-field lookup raises NoSuchFieldError — cleared above
        }
        const jint drawable_res = env->GetStaticIntField(drawable_r_class, drawable_field);
        if (clear_pending(env))
        {
            return {};
        }
        // Context.getDrawable(int) — the widget's own Context resolves the framework resource.
        jmethodID get_context = cache.method(env, k_edit_text_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_drawable =
            cache.method(env, "android/content/Context", "getDrawable", "(I)Landroid/graphics/drawable/Drawable;");
        if (get_context == nullptr || get_drawable == nullptr)
        {
            return {};
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(widget, get_context)};
        if (clear_pending(env) || !context)
        {
            return {};
        }
        local_ref<jobject> drawable{env, env->CallObjectMethod(context.get(), get_drawable, drawable_res)};
        if (clear_pending(env) || !drawable)
        {
            return {};
        }
        if (apply_tint)
        {
            // SafeSetTint: Drawable.mutate() (copy the shared constant state) then setTint(argb). mutate()
            // returns the (possibly new) drawable to tint; guard it, fall back to the original on failure.
            jmethodID mutate = cache.method(env, k_drawable_class, "mutate", "()Landroid/graphics/drawable/Drawable;");
            if (mutate != nullptr)
            {
                local_ref<jobject> mutated{env, env->CallObjectMethod(drawable.get(), mutate)};
                if (!clear_pending(env) && mutated)
                {
                    drawable = std::move(mutated);
                }
            }
            jmethodID set_tint = cache.method(env, k_drawable_class, "setTint", "(I)V");
            if (set_tint != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_tint, tint);
                clear_pending(env);
            }
        }
        return drawable;
    }

    // Reproduce MAUI's SearchView chrome on the EditText: the LEFT magnifier (always shown, mirroring
    // MauiSearchView.Initialize's SetIconifiedByDefault(false)) + the RIGHT clear-X (only when there is
    // text — SearchViewExtensions' text-gated UpdateCancelButtonState) via TextView compound drawables,
    // plus the icon-to-text inset. `left_tint`/`right_tint` are applied only when the corresponding
    // *_is_set flag is true (the BindableObject.IsSet discriminator, so an unset icon keeps the framework
    // drawable's own dark tint — which already matches the reference's dark magnifier/X — rather than
    // being forced to the default sentinel value). Every JNI id is guarded; on any failure the icon is
    // simply omitted (the VM-less / lookup-miss degradation the rest of the file uses).
    void set_search_compound_icons(JNIEnv* env, jobject widget, bool has_text, bool left_is_set, jint left_tint,
                                   bool right_is_set, jint right_tint, float density)
    {
        auto& cache = default_jni_cache();
        // setCompoundDrawables (NOT …WithIntrinsicBounds) so the manually-set 18dp bounds below are honored;
        // the framework ic_menu_* intrinsic bounds are ~24dp and over-inflate the EditText row.
        jmethodID set_compound = cache.method(env, k_edit_text_class, "setCompoundDrawables",
                                              "(Landroid/graphics/drawable/Drawable;Landroid/graphics/drawable/"
                                              "Drawable;Landroid/graphics/drawable/Drawable;Landroid/graphics/"
                                              "drawable/Drawable;)V");
        if (set_compound == nullptr)
        {
            return;
        }
        // Always tint: an explicit color when set, else the dark default (the untinted framework drawable
        // is a mid-gray that doesn't match MAUI's textColorPrimary icons — see k_default_icon_tint).
        const jint left_eff = left_is_set ? left_tint : k_default_icon_tint;
        const jint right_eff = right_is_set ? right_tint : k_default_icon_tint;
        const local_ref<jobject> left =
            resolve_framework_drawable(env, widget, k_search_icon_field, /*apply_tint=*/true, left_eff);
        local_ref<jobject> right;
        if (has_text)
        {
            right = resolve_framework_drawable(env, widget, k_clear_icon_field, /*apply_tint=*/true, right_eff);
        }
        // Size both icons to MAUI's ~18dp Material search icon via explicit setBounds (setCompoundDrawables
        // honors these; …WithIntrinsicBounds would reset to the oversized ~24dp framework intrinsics).
        const jint icon_px = to_pixels(k_search_icon_size_dp, density);
        if (jmethodID set_bounds = cache.method(env, k_drawable_class, "setBounds", "(IIII)V"))
        {
            if (left)
            {
                env->CallVoidMethod(left.get(), set_bounds, 0, 0, icon_px, icon_px);
                clear_pending(env);
            }
            if (right)
            {
                env->CallVoidMethod(right.get(), set_bounds, 0, 0, icon_px, icon_px);
                clear_pending(env);
            }
        }
        env->CallVoidMethod(widget, set_compound, left.get(), static_cast<jobject>(nullptr), right.get(),
                            static_cast<jobject>(nullptr));
        clear_pending(env);
        // CompoundDrawablePadding: the magnifier-to-text inset (dp → px). A modest gap so the query text
        // sits indented from the loupe, matching the SearchView's expanded-field inset.
        call_void_int(env, widget, "setCompoundDrawablePadding", to_pixels(k_compound_drawable_padding_dp, density));
    }

    // The single DRY entry point every icon-touching map/create calls: recompute the LEFT magnifier (always
    // on) + the RIGHT clear-X (text-gated) + their tints straight from the virtual view. The tints follow
    // SearchIconColor / CancelButtonColor, applied only when BindableObject.IsSet marks them explicit (an
    // unset icon keeps the framework drawable's own dark tint, which already matches the reference). The
    // set-ness discriminator is the same dynamic_cast<bindable_object> + is_property_set idiom the button /
    // ios partials use for the unset-color sentinel collision.
    void refresh_search_compound_icons(JNIEnv* env, jobject widget, const maui::core::i_search_bar& view)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool left_is_set = bindable != nullptr && bindable->is_property_set("search_icon_color");
        const bool right_is_set = bindable != nullptr && bindable->is_property_set("cancel_button_color");
        const auto left_tint = static_cast<jint>(view.search_icon_color().to_int());
        const auto right_tint = static_cast<jint>(view.cancel_button_color().to_int());
        const bool has_text = !view.text().empty();
        const float density = display_density(env, widget);
        set_search_compound_icons(env, widget, has_text, left_is_set, left_tint, right_is_set, right_tint, density);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.EditText (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin releases its NSSearchField here).
    search_bar_platform::~search_bar_platform()
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
    // editor partial; only the widget class name differs.

    void search_bar_platform::update_visibility(maui::core::visibility value)
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

    void search_bar_platform::update_opacity(double value)
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

    void search_bar_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SearchBarHandler.MapIsEnabled → SearchViewExtensions.UpdateIsEnabled sets the inner
            // EditText.Enabled = searchBar.IsEnabled. With the EditText stand-in that IS the platform
            // view, so this is platformView.Enabled = value (the shared ViewExtensions.UpdateIsEnabled).
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void search_bar_platform::update_automation_id(std::string_view value)
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

    void search_bar_platform::update_background(const maui::graphics::paint* value)
    {
        // MapBackground rides the shared view_mapper in C# (SearchBarHandler.Android.MapBackground
        // delegates straight to PlatformView.UpdateBackground). The headless mirror keeps the base body;
        // the android view op pushes the solid/gradient/image background to the View (VM-less safe).
        view_platform_base::update_background(value);
        // WAVE 14: a NULL paint must NOT call apply_background — setBackground(null) would ERASE the
        // defStyleRes field chrome (the underline the styled ctor installed). MAUI leaves the native
        // default when Background is null; the EditText's styled background IS that default.
        if (value != nullptr)
        {
            maui::platform::android::apply_background(native, value);
        }
    }

    // Render transform + flow direction + semantics pushed to the real widget via the shared android
    // ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite observes the
    // headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void search_bar_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void search_bar_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void search_bar_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // VisualElement.Clip on a generic view (wave 24): install a ViewOutlineProvider + setClipToOutline so
    // the framework clips the whole EditText to the convex shape (the clip_views EllipseGeometry). The base
    // mirror runs FIRST (the VM-less cross-platform suite observes it). The borrow is stashed so
    // platform_arrange can re-resolve the bounds-dependent geometry after layout (the view is 0×0 at map
    // time — apply_outline_clip clears the clip then, and the arrange pass rebuilds it at the live size).
    void search_bar_platform::update_clip(const maui::graphics::i_shape* value)
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

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
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
        // SearchBarHandler.CreatePlatformView builds a MauiSearchView (a SearchView whose inner field is
        // an EditText). The AppCompat SearchView is not carried here (header deviation), so the EditText
        // stand-in IS the platform view. EditText(Context) chains to the (Context, AttributeSet, int)
        // ctor with defStyleAttr = the theme attr editTextStyle, which it resolves against the Context's
        // THEME — the bare, Activity-less app_process testhost has no such theme, so that ctor throws
        // (the trap the editor partial documents). The defStyleAttr=0 3-arg ctor constructs fine BUT
        // resolves NO background drawable, so an empty search field renders with NO box/underline chrome
        // (the missing-chrome bug the switch/checkbox glyph waves hit). So construct THEME-INDEPENDENTLY
        // via the 4-arg (Context, AttributeSet, int defStyleAttr, int defStyleRes) ctor with defStyleAttr=0
        // and defStyleRes = android.R.style.Widget_Material_Light_EditText (a concrete style resource that
        // CARRIES the field chrome — read with GetStaticFieldID since it is a static field). Then fall back
        // to the 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so the widget is never
        // null. Single-line (the EditText default) — NO SetSingleLine(false) / multi-line knobs.
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
        // TextAlignment = ViewStart + Gravity = CenterVertical: the search bar's default leading,
        // vertically-centered single line (vs the editor's Top). The vertical centre matches
        // SearchViewExtensions.UpdateVerticalTextAlignment's TextAlignment.Center fallback.
        call_void_int(env.get(), widget.get(), "setTextAlignment", k_text_alignment_view_start);
        call_void_int(env.get(), widget.get(), "setGravity", k_gravity_center_vertical);

        // Tint the field's 9-patch underline to MAUI's faint gray (see k_underline_tint) via
        // setBackgroundTintList(ColorStateList.valueOf(...)) — the framework default is a dark accent that
        // reads as a per-row structural diff vs MAUI's barely-visible SearchView plate underline.
        if (jclass csl_class = cache.find_class(env.get(), "android/content/res/ColorStateList"))
        {
            jmethodID value_of = cache.static_method(env.get(), "android/content/res/ColorStateList", "valueOf",
                                                     "(I)Landroid/content/res/ColorStateList;");
            jmethodID set_bg_tint = cache.method(env.get(), k_edit_text_class, "setBackgroundTintList",
                                                 "(Landroid/content/res/ColorStateList;)V");
            if (value_of != nullptr && set_bg_tint != nullptr)
            {
                const local_ref<jobject> tint{env.get(),
                                              env->CallStaticObjectMethod(csl_class, value_of, k_underline_tint)};
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
        // in for the parent ViewGroup attach (identical to the editor partial's note).
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

        // Install the SearchView chrome up front: the LEFT magnifier (always shown, mirroring
        // MauiSearchView.Initialize's SetIconifiedByDefault(false)) + the icon-to-text inset, with the
        // RIGHT clear-X omitted (the field starts empty — SearchViewExtensions gates it on non-empty
        // text). No virtual view is available in CreatePlatformView (the maps push properties later), so
        // this is the default-tint / no-text initial state; map_text re-evaluates the clear-X and
        // map_search_icon_color / map_cancel_button_color re-tint from the real view.
        const float density = display_density(env.get(), widget.get());
        set_search_compound_icons(env.get(), widget.get(), /*has_text=*/false, /*left_is_set=*/false, /*left_tint=*/0,
                                  /*right_is_set=*/false, /*right_tint=*/0, density);

        platform->native = env->NewGlobalRef(widget.get()); // released in ~search_bar_platform
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        // SearchBarHandler.Android ConnectHandler installs platformView.QueryTextChange += OnQueryTextChange
        // (→ VirtualView.UpdateText) and QueryTextSubmit += OnQueryTextSubmit (→ SearchButtonPressed), plus
        // the focus / cursor-selection listeners. The android trampolines are deferred with the gesture/
        // text-watcher fan-out (header deviations), EXACTLY like the editor partial defers its TextWatcher
        // — but the C++ callbacks stay wired so the VM-less cross-platform suite (and a future trampoline)
        // can drive them, and on_text_changed keeps the headless mirror's last_known_text live so an
        // inbound edit can supply the (old, new) pair.
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
        platform.on_search_button_pressed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_search_button_pressed();
            }
        };
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        // DisconnectHandler: QueryTextChange -= OnQueryTextChange; QueryTextSubmit -= OnQueryTextSubmit;
        // SetOnQueryTextFocusChangeListener(null); … (the native trampoline uninstall lands with the
        // deferred listener fan-out).
        platform.on_text_changed = nullptr;
        platform.on_search_button_pressed = nullptr;
    }

    void search_bar_handler::map_text(search_bar_handler& handler, i_search_bar& view)
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
        // SearchViewExtensions.UpdateText → SearchView.SetQuery(Text, false); the inner-EditText overload
        // (SearchViewExtensions.UpdateText(EditText)) is editText.Text = newText with an equality guard.
        // With the EditText stand-in: setText(Text), then SetSelection(Text.Length) to keep the caret at
        // the end after a programmatic set (setting text resets the cursor to 0). to_jstring goes through
        // the real-UTF-8 path (supplementary-plane safe — see jni_string.hpp).
        jmethodID set_text = cache.method(env.get(), k_edit_text_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr)
        {
            const local_ref<jstring> text = to_jstring(env.get(), view.text());
            env->CallVoidMethod(widget, set_text, text.get());
            clear_pending(env.get());
        }
        jmethodID set_selection = cache.method(env.get(), k_edit_text_class, "setSelection", "(I)V");
        if (set_selection != nullptr)
        {
            env->CallVoidMethod(widget, set_selection, static_cast<jint>(view.text().size()));
            clear_pending(env.get());
        }
        // Re-evaluate the RIGHT clear-X: SearchViewExtensions shows search_close_btn only with text (an
        // empty field hides it). Recomputes both icons + their tints from the view so the magnifier and
        // any explicit tints survive the text change.
        refresh_search_compound_icons(env.get(), widget, view);
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
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
            // SearchViewExtensions.UpdateTextColor → inner EditText.SetTextColor(color). The null branch
            // (TryGetDefaultStateColor + the search_mag_icon co-tint) collapses for the port's non-nullable
            // color + the deferred SearchView chrome (header deviations); the ColorStateList path is
            // replaced by the int overload (header deviations).
            call_void_int(env.get(), widget_of(*platform), "setTextColor",
                          static_cast<jint>(view.text_color().to_int()));
        }
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
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
        // SearchViewExtensions.UpdatePlaceholder → SearchView.QueryHint = Placeholder. The QueryHint is
        // surfaced on the inner EditText as its Hint, so with the EditText stand-in this is setHint(Placeholder).
        jmethodID set_hint =
            default_jni_cache().method(env.get(), k_edit_text_class, "setHint", "(Ljava/lang/CharSequence;)V");
        if (set_hint != nullptr)
        {
            const local_ref<jstring> hint = to_jstring(env.get(), view.placeholder());
            env->CallVoidMethod(widget_of(*platform), set_hint, hint.get());
            clear_pending(env.get());
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
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
            // SearchViewExtensions.UpdatePlaceholderColor → EditTextExtensions.UpdatePlaceholderColor on the
            // inner EditText, which discriminates on `placeholderTextColor is null`: SET →
            // SetHintTextColor(color); UNSET → resolve android.R.attr.textColorHint from the theme and
            // SetHintTextColor to THAT (the native EditText hint gray). The port's Color is a NON-nullable
            // value type (default color{} = opaque BLACK), so a `!= color{}` compare cannot stand in for
            // `!= null` — it misreads an explicit PlaceholderColor=Black as unset AND (the bug this fix
            // closes) rendered every unset query-hint solid BLACK, indistinguishable from real text.
            // Discriminate instead on BindableObject.IsSet (is_property_set("placeholder_color")) — the
            // faithful stand-in for `!= null`, exactly as editor_handler / entry_handler::map_placeholder_color
            // do. On the UNSET branch, positively assert the measured native EditText hint gray (#666666, the
            // android textColorHint the maui-compare reference samples) instead of black — reproducing C#'s
            // theme-textColorHint result deterministically. An explicit PlaceholderColor still overrides via
            // the SET branch. (The search_mag_icon co-tint stays deferred to SearchIconColor — header
            // deviations; ColorStateList → int overload — header deviations.)
            constexpr jint k_native_default_hint_color = static_cast<jint>(0xFF666666U);
            const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("placeholder_color");
            const jint argb =
                color_is_set ? static_cast<jint>(view.placeholder_color().to_int()) : k_native_default_hint_color;
            call_void_int(env.get(), widget_of(*platform), "setHintTextColor", argb);
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
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
        // SearchViewExtensions.UpdateIsReadOnly(EditText): bool isReadOnly = !searchBar.IsReadOnly;
        // FocusableInTouchMode = isReadOnly; Focusable = isReadOnly; SetCursorVisible(isReadOnly). (NOTE:
        // the C# variable named `isReadOnly` is actually the EDITABLE flag = !IsReadOnly; the search
        // overload, like the editor overload, does NOT touch InputType. The SearchView's
        // UpdateCancelButtonState co-call is deferred SearchView chrome — header deviations.)
        const jboolean editable = static_cast<jboolean>(!view.is_read_only());
        call_void_bool(env.get(), widget, "setFocusableInTouchMode", editable);
        call_void_bool(env.get(), widget, "setFocusable", editable);
        call_void_bool(env.get(), widget, "setCursorVisible", editable);
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        // SearchViewExtensions.UpdateMaxLength → editText.SetLengthFilter + SetQuery(TrimToMaxLength). The
        // port enforces max_length control-side (the InputView truncation, like entry/editor), so the
        // native filter + re-query is a documented no-op; the value is mirrored and the text already
        // arrived truncated (header deviations). // TODO: verify against
        // src/Core/src/Platform/Android/SearchViewExtensions.cs (UpdateMaxLength → SetLengthFilter).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->max_length = view.max_length();
        }
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
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

        // SearchViewExtensions.UpdateFont → inner EditText.UpdateFont (FontManager.GetTypeface →
        // CreateTypeface; the non-registered-family tail; header deviations, identical to the editor
        // partial's map_font): base = family ? Typeface.create(family, ToTypefaceStyle(weight, italic)) :
        // Typeface.DEFAULT, then the API-28+ refinement Typeface.create(base, weight, italic).
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

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
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
            // SearchViewExtensions.MapCharacterSpacing → inner EditText.UpdateCharacterSpacing →
            // TextViewExtensions: LetterSpacing = CharacterSpacing.ToEm().
            call_void_float(env.get(), widget_of(*platform), "setLetterSpacing",
                            static_cast<jfloat>(view.character_spacing()) * k_em_coefficient);
        }
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
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
        // SearchViewExtensions.MapHorizontalTextAlignment → inner EditText.UpdateHorizontalTextAlignment →
        // UpdateHorizontalAlignment (EditText): on RTL-capable Android (every device the port targets) it
        // sets BOTH TextAlignment and the horizontal gravity bits — "text alignment does not work at
        // runtime, so we also need gravity". The horizontal-gravity re-masking is done native-side by the
        // single setTextAlignment here (the gravity half needs a getGravity round-trip; deferred with the
        // gravity-mask helper, identical to the editor partial — the TextAlignment push is the
        // runtime-effective one). // TODO: verify against
        // src/Core/src/Platform/Android/TextAlignmentExtensions.cs (UpdateHorizontalAlignment's Gravity
        // re-mask) when the getGravity round-trip lands.
        jmethodID set_text_alignment = cache.method(env.get(), k_edit_text_class, "setTextAlignment", "(I)V");
        if (set_text_alignment != nullptr)
        {
            env->CallVoidMethod(widget, set_text_alignment, to_text_alignment(view.horizontal_text_alignment()));
            clear_pending(env.get());
        }
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
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
        // SearchViewExtensions.UpdateVerticalTextAlignment → inner EditText.UpdateVerticalAlignment:
        // Gravity = (Gravity & ~VerticalMask) | ToVerticalGravityFlags(alignment). Read-modify-write the
        // existing gravity so the horizontal bits survive (identical to the editor partial; the search
        // bar's vertical default is Center, not Top).
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

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
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
            // SearchViewExtensions.UpdateIsTextPredictionEnabled toggles the inner EditText InputType's
            // TextFlagAutoCorrect. The port recomputes the FULL input type (keyboard base + prediction +
            // spellcheck) so the three intertwined properties stay consistent — SetInputType's shape
            // (header deviations). NO multi-line bit (single-line search bar).
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
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
            // SearchViewExtensions.UpdateIsSpellCheckEnabled toggles the inner EditText InputType's
            // TextFlagNoSuggestions; recompute the full type (see map_is_text_prediction_enabled).
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
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
            // SearchBarHandler.MapKeyboard re-pushes Text first (UpdateValue(nameof(Text)) — to restore
            // the caret after the input-type change) then UpdateKeyboard → SetInputType: keyboard base
            // type + prediction/spellcheck. The caret restore is the deferred SetSelection round-trip
            // (header deviations); map_text already lands the text. NO multi-line bit (single-line).
            apply_input_type(env.get(), widget_of(*platform), view);
        }
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
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
        // full UpdateCursorSelection logic (the looper Post when focused, the OnQueryEditorSelectionChanged
        // round-trip) is deferred with the focus/selection-changed seam (header deviations); the
        // best-effort SetSelection here lands the clamped caret synchronously (identical to the editor
        // partial). // TODO: verify against EditTextExtensions.cs (UpdateCursorSelection).
        const auto length = static_cast<int>(view.text().size());
        const jint start = static_cast<jint>(std::max(0, std::min(view.cursor_position(), length)));
        call_void_int(env.get(), widget_of(*platform), "setSelection", start);
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
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
        // EditTextExtensions.UpdateSelectionLength → SetSelection(start, end). start = clamp(cursor), end =
        // clamp(start + selectionLength). The native-RTL / looper-Post nuances are deferred (header
        // deviations); the two-arg SetSelection lands the clamped range synchronously (identical to the
        // editor partial). // TODO: verify against EditTextExtensions.cs (GetSelectionStart/End).
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

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        // SearchViewExtensions.UpdateCancelButtonColor tints the search_close_btn ImageView's drawable via
        // SafeSetTint. On the EditText stand-in the search_close_btn is the RIGHT clear-X compound drawable
        // (framework ic_menu_close_clear_cancel), so this tints it — CancelButtonColor set → the clear-X
        // takes that colour (the reference's "Cancel is red" row's red X); unset → the framework drawable's
        // own dark tint. (The apple/AppKit twin stays mirror-only: NSSearchFieldCell's cancel cell has no
        // public tint.)
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cancel_button_color = view.cancel_button_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            refresh_search_compound_icons(env.get(), widget_of(*platform), view);
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        // SearchViewExtensions.UpdateSearchIconColor tints the search_mag_icon ImageView's drawable via
        // SafeSetTint. On the EditText stand-in the search_mag_icon is the LEFT magnifier compound drawable
        // (framework ic_menu_search), so this tints it — SearchIconColor set → the loupe takes that colour;
        // unset → the framework drawable's own dark tint (which matches the reference). (The apple twin
        // stays mirror-only.)
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->search_icon_color = view.search_icon_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            refresh_search_compound_icons(env.get(), widget_of(*platform), view);
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        // SearchViewExtensions.UpdateReturnType sets SearchView.ImeOptions + the inner EditText.ImeOptions
        // (ReturnType.ToPlatform) and RestartInput. The IME-options push has no observable surface on the
        // bare app_process testhost (no soft keyboard) and the SearchView ImeOptions has no stand-in
        // target; DEFERRED to mirror-only (header deviations — like the apple twin's hardware-keyboard
        // collapse). // TODO: verify against SearchViewExtensions.cs (UpdateReturnType → ImeOptions +
        // RestartInput) when a soft-keyboard-aware app host lands.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->bar_return_type = view.return_type();
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (a single-line bar ~200pt
            // wide, clamped to a finite width constraint, fixed line height), so the backend-agnostic
            // size-request suites see consistent numbers in the pure-native run.
            double width = 200.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 30.0};
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
        // as dp (Context.FromPixels). Identical to the editor partial.
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
        // Floor the row to MAUI's fixed 48dp SearchView plate (Material) — the DeviceDefault EditText has no
        // plate, so with the icon no longer inflating it the row measures short and a stack of search bars
        // drifts. Same Material-height-floor pattern as slider/switch.
        const double height_dp = std::max(static_cast<double>(measured_height) / density, k_material_search_height_dp);
        return {static_cast<double>(measured_width) / density, height_dp};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange (the dp frame becomes pixels, the view measures Exactly at the final
        // size — Android requires a measure pass before layout — and lays out). Identical to the editor
        // partial; the text-view re-layout nicety (PrepareForTextViewArrange) is deferred.
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

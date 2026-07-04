// entry_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.TextBox. The
// windows twin of src/platform/apple/entry_handler.mm (editable NSTextField) / the android JNI partial,
// and the real-native sibling of the headless mirror partial (src/platform/headless/entry_handler.cpp).
// Every map_* pushes its property onto the control, and the native TextChanged / KeyUp(Enter) events
// flow back through the platform callbacks into i_entry::send_text_changed / send_completed.
//
// Ported DIRECTLY from EntryHandler.Windows.cs + Platform/Windows/{TextBoxExtensions.cs,
// ControlExtensions.cs (UpdateFont/UpdateIsEnabled), ViewExtensions.cs, AlignmentExtensions.cs,
// FontExtensions.cs, CharacterSpacingExtensions.cs} + Fonts/FontManager.Windows.cs.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - The control is a stock TextBox, not MauiPasswordTextBox (EntryHandler.Windows.CreatePlatformView's
//     obfuscating subclass with IsObfuscationDelayed): the password composition is a template-level
//     custom control the port has not brought up. map_is_password/map_text reproduce the ESSENCE of
//     MauiPasswordTextBox.Obfuscate — the visible Text is a run of U+25CF ('●') while the real value
//     stays in the virtual view / mirror (Password). The live delayed-obfuscation timer + key-
//     interception machinery (IsObfuscationDelayed, obfuscate-on-type) stay deferred.
//   - Text/placeholder colors + Background land on the DIRECT dependency properties (Foreground /
//     PlaceholderForeground / Background). C# writes the themed resource keys (TextControlForeground* /
//     TextControlPlaceholderForeground* / TextControlBackground* + RefreshThemeResources) so the
//     PointerOver/Focused/Disabled visual states track the pushed value — deferred: the direct property
//     covers the rest state; the state-brush resources need the port's resource-dictionary seam. The C#
//     null branches (RemoveKeys / ClearValue) map to ClearValue, discriminated through
//     BindableObject.IsSet where the port's color type has no null.
//   - UpdateInputScope's IsTextPredictionEnabled/IsSpellCheckEnabled push (incl. the CustomKeyboard
//     flags override) is ported; its InputScope NAMES tail (InputScopeNameValue.Search for
//     ReturnType.Search + Keyboard.ToInputScopeName) is deferred — no InputScopeNameValue map yet, so
//     map_keyboard / map_return_type mirror + push only the prediction/spellcheck half.
//   - MauiTextBox's attached-property template walks are deferred: SetVerticalTextAlignment (the
//     ScrollViewer "ContentElement" + placeholder alignment), SetIsDeleteButtonEnabled (the
//     "DeleteButton" grid-column collapse behind UpdateClearButtonVisibility), and the SizeChanged →
//     InvalidateAttachedProperties re-run. map_vertical_text_alignment / map_clear_button_visibility
//     keep the headless mirrors.
//   - The SelectionChanged → cursor/selection write-back (OnPlatformSelectionChanged) and the
//     ReturnType.Next → TryMoveFocus(FocusNavigationDirection.Next) hop are deferred with the focus
//     seam; map_cursor_position / map_selection_length still push SelectionStart/SelectionLength
//     outward per TextBoxExtensions.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14 (ControlContentThemeFontSize).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// the on_text_changed/on_completed callbacks stay invokable C++ callbacks (the cross-platform suite
// drives them) — so that suite observes exactly the headless partial's behavior.

#include "maui/core/entry_handler.hpp"
#include "maui/core/i_ios_entry_specifics.hpp" // --- platform configuration (W2-24) ---

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/i_text_input.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/keyboard_flags.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxi = winrt::Microsoft::UI::Xaml::Input;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host). Same constant as the
    // label/button partials.
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::TextBox text_box_of(const maui::core::entry_platform& platform)
    {
        return wnative::borrow<muxc::TextBox>(platform.native);
    }

    // MauiPasswordTextBox.Obfuscate: the visible Text becomes `new string('●', Password.Length)` — a run
    // of U+25CF BLACK CIRCLE, one glyph per UTF-16 code unit of the real value (C# string.Length). The
    // real value never lives in the TextBox while masked; it stays in the virtual view / mirror. Using the
    // projected hstring's length reproduces C#'s UTF-16 Length exactly (incl. the astral-pair 2-dots quirk).
    [[nodiscard]] winrt::hstring obfuscate(std::string_view real)
    {
        const std::size_t units = wnative::to_hstring_utf8(real).size(); // UTF-16 code units == C# Length
        constexpr wchar_t k_mask_glyph = static_cast<wchar_t>(0x25CF);   // U+25CF BLACK CIRCLE '●'
        return winrt::hstring{std::wstring(units, k_mask_glyph)};
    }

    // FontExtensions.ToFontStyle: Slant → FontStyle (Italic / Oblique / Normal).
    [[nodiscard]] wut::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return wut::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return wut::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return wut::FontStyle::Normal;
        }
    }

    // FontExtensions.ToFontWeight — the port's font_weight enum values ARE the numeric OpenType
    // weights C#'s switch resolves to, so the numeric FontWeight constructor is the whole mapping.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    // AlignmentExtensions.ToPlatform(alignment, isLtr: true) — the C# body carries the same
    // "no FlowDirection yet" TODO, so the LTR branch is the faithful port (the label partial's twin).
    [[nodiscard]] mux::TextAlignment to_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return mux::TextAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::TextAlignment::Right;
            case maui::core::text_alignment::justify:
                return mux::TextAlignment::Justify;
            case maui::core::text_alignment::start:
            default:
                return mux::TextAlignment::Left;
        }
    }

    // TextBoxExtensions.UpdateInputScope (the head half): a CustomKeyboard's Suggestions/Spellcheck
    // flags override the ITextInput properties; otherwise the properties push directly. deferred: the
    // InputScope NAMES tail — InputScopeNameValue.Search when an IEntry's ReturnType is Search, plus
    // KeyboardExtensions.ToInputScopeName for the keyboard kind (no InputScopeNameValue map yet).
    void apply_input_scope(const muxc::TextBox& box, const maui::core::i_text_input& view)
    {
        const maui::core::keyboard kb = view.keyboard();
        if (kb.kind() == maui::core::keyboard::kind::custom)
        {
            box.IsTextPredictionEnabled((kb.flags() & maui::core::keyboard_flags::suggestions) !=
                                        maui::core::keyboard_flags::none);
            box.IsSpellCheckEnabled((kb.flags() & maui::core::keyboard_flags::spellcheck) !=
                                    maui::core::keyboard_flags::none);
        }
        else
        {
            box.IsTextPredictionEnabled(view.is_text_prediction_enabled());
            box.IsSpellCheckEnabled(view.is_spell_check_enabled());
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the TextBox (the wnative shape of the pimpl-owned-native
    // doctrine). The event tokens are plain ints — on_disconnect_handler revokes them.
    entry_platform::~entry_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real TextBox when one exists.

    void entry_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void entry_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void entry_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled → ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto box = text_box_of(*this))
        {
            box.IsEnabled(value);
        }
    }

    void entry_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void entry_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto box = text_box_of(*this);
        if (box == nullptr)
        {
            return;
        }
        // EntryHandler.Windows.MapBackground → TextBoxExtensions.UpdateBackground: a null brush removes
        // the TextControlBackground* resource keys (→ theme default), a value sets them all +
        // RefreshThemeResources. The port pushes the DIRECT Background property (deferred: the per-state
        // resource keys — see the header) and ClearValue for the null branch.
        if (value == nullptr)
        {
            box.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        box.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<entry_platform> entry_handler::create_platform_view()
    {
        auto platform = std::make_unique<entry_platform>();
        try
        {
            // EntryHandler.Windows.CreatePlatformView: new MauiPasswordTextBox { IsObfuscationDelayed =
            // DeviceInfo.Idiom != Desktop } — a stock TextBox in this first cut (deferred: the
            // MauiPasswordTextBox obfuscation subclass; header deviations).
            const muxc::TextBox box;
            platform->native = wnative::store(box); // released in ~entry_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void entry_handler::on_connect_handler(entry_platform& platform)
    {
        // The headless twin's inbound callbacks — ALWAYS wired, even XAML-less, so the cross-platform
        // suite can drive them directly (the android partial's shape). on_text_changed updates
        // last_known_text and forwards to i_entry::send_text_changed; on_completed → send_completed.
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
        auto box = text_box_of(platform);
        if (box == nullptr)
        {
            return;
        }
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers).
        auto* peer = &platform;
        // ConnectHandler: platformView.TextChanged += OnPlatformTextChanged — which pushes the native
        // text back onto the virtual view (VirtualView.UpdateText(PlatformView.Text)). The port's seam
        // carries (old, new); the old value is last_known_text, and an unchanged text is skipped (the
        // dedup C# gets from InputView's Text setter no-oping on an equal value).
        const winrt::event_token text_token = box.TextChanged(
            [peer](const winrt::Windows::Foundation::IInspectable& sender, const muxc::TextChangedEventArgs&) {
                const auto sender_box = sender.try_as<muxc::TextBox>();
                if (sender_box == nullptr || !peer->on_text_changed)
                {
                    return;
                }
                const std::string new_text = wnative::to_utf8(sender_box.Text());
                if (new_text != peer->last_known_text)
                {
                    peer->on_text_changed(peer->last_known_text, new_text);
                }
            });
        platform.text_changed_token = text_token.value;
        // ConnectHandler: platformView.KeyUp += OnPlatformKeyUp — Enter → VirtualView.Completed().
        // deferred: the ReturnType.Next branch (TryMoveFocus(FocusNavigationDirection.Next)) rides the
        // focus seam that has not landed on this backend.
        const winrt::event_token key_token = box.KeyUp(
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxi::KeyRoutedEventArgs& args) {
                if (args.Key() != winrt::Windows::System::VirtualKey::Enter)
                {
                    return;
                }
                if (peer->on_completed)
                {
                    peer->on_completed();
                }
            });
        platform.key_up_token = key_token.value;
        // deferred: SelectionChanged += OnPlatformSelectionChanged (the native cursor/selection
        // write-back onto the virtual view) and SizeChanged += OnPlatformViewSizeChanged
        // (MauiTextBox.InvalidateAttachedProperties) — header deviations.
    }

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        // DisconnectHandler: KeyUp -= OnPlatformKeyUp; TextChanged -= OnPlatformTextChanged. The C++
        // callbacks are cleared like the headless twin.
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
        if (auto box = text_box_of(platform))
        {
            if (platform.text_changed_token != 0)
            {
                box.TextChanged(winrt::event_token{platform.text_changed_token});
            }
            if (platform.key_up_token != 0)
            {
                box.KeyUp(winrt::event_token{platform.key_up_token});
            }
        }
        platform.text_changed_token = 0;
        platform.key_up_token = 0;
    }

    void entry_handler::map_text(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text()); // mirror keeps the REAL value (== MauiPasswordTextBox.Password)
        platform->last_known_text = platform->text;
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateText: skip when equal; set Text; a non-empty programmatic set parks
        // the cursor at the end with no selection (Select(Text.Length, 0)). When IsPassword, the VISIBLE
        // Text is the obfuscated run (MauiPasswordTextBox.Obfuscate) — the mirror above keeps the real value.
        const auto new_text = view.is_password() ? obfuscate(view.text()) : wnative::to_hstring_utf8(view.text());
        if (box.Text() == new_text)
        {
            return;
        }
        box.Text(new_text);
        if (!box.Text().empty())
        {
            box.Select(static_cast<std::int32_t>(box.Text().size()), 0);
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->placeholder = std::string(view.placeholder());
        // TextBoxExtensions.UpdatePlaceholder: PlaceholderText = placeholder ?? string.Empty.
        if (auto box = text_box_of(*platform))
        {
            box.PlaceholderText(wnative::to_hstring_utf8(view.placeholder()));
        }
    }

    void entry_handler::map_placeholder_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->placeholder_color = view.placeholder_color();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdatePlaceholderColor: null → RemoveKeys(TextControlPlaceholderForeground*)
        // + ClearValue(PlaceholderForegroundProperty); value → SetValueForAllKey + PlaceholderForeground
        // = brush. The port pushes the direct PlaceholderForeground (the UniversalApiContract >= 5
        // branch; deferred: the per-state resource keys — header), the null branch discriminated through
        // BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("placeholder_color");
        if (color_is_set)
        {
            box.PlaceholderForeground(wnative::to_brush(view.placeholder_color()));
        }
        else
        {
            box.ClearValue(muxc::TextBox::PlaceholderForegroundProperty());
        }
    }

    void entry_handler::map_is_password(entry_handler& handler, i_entry& view)
    {
        // TextBoxExtensions.UpdateIsPassword → MauiPasswordTextBox.IsPassword: the real value stays in the
        // mirror / virtual view (Password); the visible Text is masked with U+25CF when true and shown clear
        // when false (Obfuscate / UpdateVisibleText). map_text runs BEFORE this in the mapper table, and the
        // page toggle re-invokes ONLY this mapper, so it re-renders the visible text self-sufficiently. The
        // live delayed-obfuscation / key-interception machinery of MauiPasswordTextBox remains deferred.
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_password = view.is_password();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        const auto visible =
            view.is_password() ? obfuscate(view.text()) : wnative::to_hstring_utf8(view.text());
        if (box.Text() == visible)
        {
            return;
        }
        box.Text(visible);
        if (!box.Text().empty())
        {
            box.Select(static_cast<std::int32_t>(box.Text().size()), 0);
        }
    }

    void entry_handler::map_is_read_only(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_read_only = view.is_read_only();
        // TextBoxExtensions.UpdateIsReadOnly: IsReadOnly = textInput.IsReadOnly.
        if (auto box = text_box_of(*platform))
        {
            box.IsReadOnly(view.is_read_only());
        }
    }

    void entry_handler::map_max_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->max_length = view.max_length();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateMaxLength: 0 forces read-only (else re-derive from IsReadOnly); -1
        // widens to int.MaxValue; push MaxLength; truncate the current native text past the cap.
        int max_length = view.max_length();
        box.IsReadOnly(max_length == 0 || view.is_read_only());
        if (max_length == -1)
        {
            max_length = std::numeric_limits<int>::max();
        }
        box.MaxLength(max_length);
        const winrt::hstring current = box.Text();
        if (max_length >= 0 && current.size() > static_cast<std::uint32_t>(max_length))
        {
            box.Text(winrt::hstring{std::wstring_view{current}.substr(0, static_cast<std::size_t>(max_length))});
        }
    }

    void entry_handler::map_text_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateTextColor: null → RemoveKeys(TextControlForeground*); value →
        // SetValueForAllKey + RefreshThemeResources. The port pushes the direct Foreground (deferred:
        // the per-state resource keys — header), the null branch discriminated through
        // BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        if (color_is_set)
        {
            box.Foreground(wnative::to_brush(view.text_color()));
        }
        else
        {
            box.ClearValue(muxc::Control::ForegroundProperty());
        }
    }

    void entry_handler::map_font(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateFont → ControlExtensions.UpdateFont: FontSize + FontFamily + FontStyle
        // + FontWeight + IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily — registrar
        // skipped, header).
        const font value = view.font();
        const double size = value.size();
        box.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            box.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            box.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family → the default family
        }
        box.FontStyle(to_font_style(value.slant()));
        box.FontWeight(to_font_weight(value.weight()));
        box.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void entry_handler::map_character_spacing(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // TextBoxExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm(). deferred: the
        // ApplyCharacterSpacingToPlaceholder walk ("PlaceholderTextContentPresenter" TextBlock, applied
        // on Loaded) — the template-descendant seam has not landed on this backend.
        if (auto box = text_box_of(*platform))
        {
            box.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        // TextBoxExtensions.UpdateHorizontalTextAlignment: TextAlignment = alignment.ToPlatform(true)
        // (the C# body's FlowDirection TODO applies equally here — LTR assumed).
        if (auto box = text_box_of(*platform))
        {
            box.TextAlignment(to_text_alignment(view.horizontal_text_alignment()));
        }
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& handler, i_entry& view)
    {
        // deferred: TextBoxExtensions.UpdateVerticalTextAlignment → MauiTextBox.SetVerticalTextAlignment
        // — an attached property whose changed-handler walks the template descendants (the ScrollViewer
        // "ContentElement" + the placeholder TextBlock) and re-runs on SizeChanged; the
        // template-descendant seam has not landed on this backend, so only the headless mirror records
        // the value.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void entry_handler::map_is_text_prediction_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        // TextBoxExtensions.UpdateIsTextPredictionEnabled → UpdateInputScope (see apply_input_scope).
        if (auto box = text_box_of(*platform))
        {
            apply_input_scope(box, view);
        }
    }

    void entry_handler::map_is_spell_check_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        // TextBoxExtensions.UpdateIsSpellCheckEnabled → UpdateInputScope (see apply_input_scope).
        if (auto box = text_box_of(*platform))
        {
            apply_input_scope(box, view);
        }
    }

    void entry_handler::map_keyboard(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        // TextBoxExtensions.UpdateKeyboard → UpdateInputScope: the prediction/spellcheck half is pushed
        // (incl. the CustomKeyboard flags override); deferred: the InputScope NAMES tail
        // (Keyboard.ToInputScopeName) — see apply_input_scope.
        if (auto box = text_box_of(*platform))
        {
            apply_input_scope(box, view);
        }
    }

    void entry_handler::map_return_type(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->entry_return_type = view.return_type();
        // TextBoxExtensions.UpdateReturnType → UpdateInputScope: deferred — the ReturnType.Search →
        // InputScopeNameValue.Search name push rides the InputScope tail that has not landed; the
        // prediction/spellcheck half still refreshes (C# re-runs the whole UpdateInputScope here).
        if (auto box = text_box_of(*platform))
        {
            apply_input_scope(box, view);
        }
    }

    void entry_handler::map_clear_button_visibility(entry_handler& handler, i_entry& view)
    {
        // deferred: TextBoxExtensions.UpdateClearButtonVisibility → MauiTextBox.SetIsDeleteButtonEnabled
        // — an attached property whose changed-handler walks the template ("DeleteButton" + its grid
        // column) and re-runs on SizeChanged; the template-descendant seam has not landed on this
        // backend, so only the headless mirror records the value.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->clear_button = view.clear_button_visibility();
        }
    }

    void entry_handler::map_cursor_position(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateCursorPosition: the TextBox does not clamp CursorPosition to
        // Text.Length natively — clamp, write the clamped value back onto the virtual view, then push
        // SelectionStart when different.
        const auto text_length = static_cast<int>(box.Text().size());
        const int position = std::min(view.cursor_position(), text_length);
        if (view.cursor_position() != position)
        {
            view.set_cursor_position(position);
            platform->cursor_position = position;
        }
        if (static_cast<int>(box.SelectionStart()) != position)
        {
            box.SelectionStart(position);
        }
    }

    void entry_handler::map_selection_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
        auto box = text_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // TextBoxExtensions.UpdateSelectionLength: the TextBox does not clamp SelectionLength to
        // Text.Length natively — clamp against the text past SelectionStart, write the clamped value
        // back, then push when different.
        const auto text_length = static_cast<int>(box.Text().size());
        const int length = std::min(view.selection_length(), text_length - static_cast<int>(box.SelectionStart()));
        if (view.selection_length() != length)
        {
            view.set_selection_length(length);
            platform->selection_length = length;
        }
        if (static_cast<int>(box.SelectionLength()) != length)
        {
            box.SelectionLength(length);
        }
    }

    // --- platform configuration (W2-24): the iOSSpecific Entry.CursorColor map — an iOS knob
    // (UITextField.tintColor) with no TextBox analog, so Windows keeps the headless mirror only,
    // guarded by the IsSet probe like TextExtensions.UpdateCursorColor (a documented no-op natively).
    void entry_handler::map_cursor_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_entry_specifics*>(&view);
        if (platform == nullptr || specifics == nullptr || !specifics->cursor_color_set())
        {
            return;
        }
        platform->cursor_color = specifics->cursor_color();
    }

    // The iOSSpecific Entry.AdjustsFontSizeToFitWidth map — an iOS knob
    // (UITextField.adjustsFontSizeToFitWidth) with no TextBox analog; the mirror records the knob
    // UNCONDITIONALLY like TextExtensions.UpdateAdjustsFontSizeToFitWidth (no IsSet guard). A
    // documented no-op natively on Windows.
    void entry_handler::map_adjusts_font_size_to_fit_width(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_entry_specifics*>(&view);
        if (platform == nullptr || specifics == nullptr)
        {
            return;
        }
        platform->adjusts_font_size_to_fit_width = specifics->adjusts_font_size_to_fit_width();
    }

    maui::graphics::size entry_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (~150pt single-line field,
            // 22pt line) so the backend-agnostic size-request suites see consistent numbers.
            double width = 150.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 22.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (the value C#'s MapWidth/MapHeight would have pushed — see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void entry_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the TextBox to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core

// search_bar_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.AutoSuggestBox, the
// same native type SearchBarHandler.Windows.cs creates (NOT a TextBox — the class comment on
// i_search_bar.hpp's include list is a reminder worth repeating here). Ported from
// SearchBarHandler.Windows.cs + SearchBarExtensions.cs (Windows) + MauiAutoSuggestBox.cs.
//
// DOCUMENTED DEVIATIONS against C#, every one forced by the same missing piece — see entry_handler.cpp's
// header note and slider_handler.cpp's ThumbImageSource note for the identical class of gap on this
// backend: this port has NO VisualTreeHelper descendant-by-name/by-type walk yet
// (GetDescendantByName<T>(name) / GetFirstDescendant<T>() / GetChildren<T>()). AutoSuggestBox's own
// visible surface (Text, PlaceholderText, Resources, Foreground, CharacterSpacing, FontXxx,
// Horizontal/VerticalContentAlignment, QueryIcon) needs none of that and is ported for real below; every
// oracle member that reaches INSIDE the control template for the internal query TextBox or "DeleteButton"
// cancel button is mirror-only (value recorded on search_bar_platform, no native push), listed here so a
// later descendant-walk addition knows exactly what to wire up:
//
//   - map_is_read_only: UpdateIsReadOnly -> MauiAutoSuggestBox.SetIsReadOnly (an ATTACHED property whose
//     change handler does GetDescendantByName<TextBox>("TextBox")). Mirror only.
//   - map_max_length: the TextBox.MaxLength push (over GetChildren<TextBox>()) and the SetIsReadOnly
//     forcing on MaxLength==0 share the same gap as above and stay mirror-only; the OTHER half of
//     UpdateMaxLength — clamping the AutoSuggestBox's own `Text` when it exceeds the new max — needs no
//     descendant and IS pushed for real.
//   - map_is_text_prediction_enabled / map_is_spell_check_enabled / map_keyboard / map_return_type: all
//     four route through `platformControl.GetFirstDescendant<TextBox>()` before pushing onto the inner
//     TextBox (UpdateIsTextPredictionEnabled/UpdateIsSpellCheckEnabled/UpdateInputScope/UpdateReturnType).
//     Mirror only. (SearchBar's ReturnType, unlike Entry's, has no OTHER effect in C# either — there is no
//     OnPlatformKeyUp Tab-to-next wiring for AutoSuggestBox — so this is a complete, not partial, mirror.)
//   - map_cancel_button_color: UpdateCancelButtonColor's `GetDescendantByName<Button>("DeleteButton")`.
//     Mirror only. (MauiCancelButton.cs, part of the requested oracle reading, turns out to be DEAD for
//     this path: nothing in SearchBarHandler.Windows.cs / SearchBarExtensions.cs references it — grep
//     across src/ turns up only its own file and the PublicAPI surface list. The live cancel-button reach
//     is the plain GetDescendantByName<Button> call mirrored above, not the MauiCancelButton class.)
//   - map_cursor_position / map_selection_length: MapCursorPosition/MapSelectionLength already no-op on
//     the oracle itself unless `SearchBarHandler._queryTextBox` (populated by OnLoaded's
//     GetFirstDescendant<TextBox>()) is non-null. This backend never populates that handle, so it always
//     takes the SAME no-op branch the oracle takes when the lookup fails — an honest mirror of the actual
//     C# code path, not a shortfall invented here. The same gap silences the OTHER direction too: C#'s
//     `_queryTextBox.SelectionChanged -> OnPlatformSelectionChanged` (the live caret round-trip) has
//     nothing to subscribe to and is not wired.
//   - map_character_spacing: the AutoSuggestBox's own `CharacterSpacing` IS pushed for real; only
//     ApplyCharacterSpacing's follow-up reach into the "PlaceholderTextContentPresenter" descendant (the
//     placeholder TextBlock's spacing) is skipped, exactly as entry_handler.cpp documents for the same
//     property on TextBox.
//
// ONE further documented simplification, unrelated to the descendant-walk gap and matching an existing
// port-wide precedent (button/entry's own identical note on their `update_background`): `update_background`
// rides the shared generic-IView push (winui_visual_ops::apply_background) instead of
// SearchBarExtensions.UpdateBackground's TextControlBackground* resource-key override. Identical AT REST
// (what a parity screenshot captures); diverges only across the hover/focused/disabled visual states.
//
// QuerySubmitted's `if (e.QueryText != VirtualView.Text) VirtualView.Text = e.QueryText;` has no literal
// equivalent here: i_search_bar/i_text_input expose no `set_text` (see i_text_input.hpp — CursorPosition/
// SelectionLength are the only mutable inbound fields on the contract). The sync below reuses the same
// `on_text_changed` hook TextChanged already uses — the net effect (Text tracks the submitted query, then
// SearchButtonPressed fires) matches the oracle without inventing a new channel.

#include "maui/core/search_bar_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
// IVector/IMap members: Resources() is an IMap<IInspectable, IInspectable>, and the resource-key dance
// below calls .Insert(...)/.Remove(...) on it — the C++/WinRT include rule (see winui_interop.hpp): the
// impl/*.0.h headers that arrive transitively only forward-declare those, and calling them without this
// full header fails with C3779, an error that does not read as "add an include".
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using auto_suggest_box = winui::Controls::AutoSuggestBox;
    using IInspectable = winrt::Windows::Foundation::IInspectable;

    auto_suggest_box as_auto_suggest_box(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<auto_suggest_box>();
    }

    // "Was this property explicitly set?" — see button_handler.cpp / entry_handler.cpp's identical twin
    // for why this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // MAUI's Windows FontManager defaults (see label_handler.cpp/entry_handler.cpp's identical
    // constants/rationale): the WinUI theme's ContentControlThemeFontFamily / ControlContentThemeFontSize,
    // hard-coded because the port's Windows i_font_manager doesn't exist yet.
    constexpr double k_default_font_size = 14.0;
    constexpr std::wstring_view k_default_font_family = L"Segoe UI Variable Text";

    winrt::Windows::UI::Text::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return winrt::Windows::UI::Text::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    winrt::Windows::UI::Text::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return winrt::Windows::UI::Text::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return winrt::Windows::UI::Text::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return winrt::Windows::UI::Text::FontStyle::Normal;
        }
    }

    // AlignmentExtensions.ToPlatformHorizontalAlignment / ToPlatformVerticalAlignment, ported verbatim
    // (including Justify's Stretch case, the one member Center/Start/End don't share).
    winui::HorizontalAlignment to_platform_horizontal(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::HorizontalAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::HorizontalAlignment::Right;
            case maui::core::text_alignment::justify:
                return winui::HorizontalAlignment::Stretch;
            case maui::core::text_alignment::start:
            default:
                return winui::HorizontalAlignment::Left;
        }
    }

    winui::VerticalAlignment to_platform_vertical(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::VerticalAlignment::Bottom;
            case maui::core::text_alignment::start:
            default:
                return winui::VerticalAlignment::Top;
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden (see button_handler.cpp's identical twin).
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    void set_resources(const auto_suggest_box& box, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            box.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const auto_suggest_box& box, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            box.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // SearchBarExtensions' resource-key sets — the same TextControl* theme keys the base TextBox-derived
    // controls override, since AutoSuggestBox's template rebinds to these exactly like a plain TextBox's
    // does.
    constexpr std::array<std::wstring_view, 4> k_placeholder_color_keys{
        L"TextControlPlaceholderForeground", L"TextControlPlaceholderForegroundPointerOver",
        L"TextControlPlaceholderForegroundFocused", L"TextControlPlaceholderForegroundDisabled"};
    constexpr std::array<std::wstring_view, 4> k_text_color_keys{
        L"TextControlForeground", L"TextControlForegroundPointerOver", L"TextControlForegroundFocused",
        L"TextControlForegroundDisabled"};
} // namespace

namespace maui::core
{
    search_bar_platform::~search_bar_platform()
    {
        // Revoke exactly what on_connect_handler registered, even if disconnect never ran (mirrors
        // entry_platform's / button_platform's detach-in-destructor safety net).
        search_bar_handler::on_disconnect_handler(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
        // SearchBarHandler.Windows.CreatePlatformView: AutoMaximizeSuggestionArea = false (this port has
        // no suggestion-list feature to maximize a popup for), QueryIcon = SymbolIcon(Symbol.Find) — the
        // magnifying-glass glyph MapSearchIconColor below tints.
        auto_suggest_box box;
        box.AutoMaximizeSuggestionArea(false);
        box.QueryIcon(winui::Controls::SymbolIcon{winui::Controls::Symbol::Find});
        platform->native = maui::platform::windows::take<winui::UIElement>(box);
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        // Cross-platform hooks: forward to the virtual view (headless tests invoke these directly; the
        // native events wired below call them instead of send_* directly, matching entry_handler's
        // on_text_changed/on_completed indirection convention on this backend).
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
        if (platform.native == nullptr)
        {
            return;
        }
        const auto_suggest_box box = as_auto_suggest_box(platform.native);

        // TextChanged: SearchBarHandler.OnTextChanged — ignore ProgrammaticChange (MapText's own pushes
        // loop back here; without this filter a bound Text update would re-report itself as a user edit).
        // A genuine user edit routes through the same on_text_changed hook every TextBox-based backend
        // uses.
        platform.text_changed_token =
            box.TextChanged(
                   [this](const auto_suggest_box&, const winui::Controls::AutoSuggestBoxTextChangedEventArgs& args) {
                       if (args.Reason() == winui::Controls::AutoSuggestionBoxTextChangeReason::ProgrammaticChange)
                       {
                           return;
                       }
                       auto* platform_view = typed_platform_view();
                       if (platform_view == nullptr || platform_view->native == nullptr)
                       {
                           return;
                       }
                       const auto_suggest_box native_box = as_auto_suggest_box(platform_view->native);
                       const std::string new_value = maui::platform::windows::to_utf8(native_box.Text());
                       if (platform_view->on_text_changed)
                       {
                           platform_view->on_text_changed(platform_view->last_known_text, new_value);
                       }
                   })
                .value;

        // QuerySubmitted: SearchBarHandler.OnQuerySubmitted — sync Text to the submitted query first (see
        // the header note on why this goes through on_text_changed rather than a direct setter), THEN
        // raise the search action, unconditionally.
        platform.query_submitted_token =
            box.QuerySubmitted(
                   [this](const auto_suggest_box&, const winui::Controls::AutoSuggestBoxQuerySubmittedEventArgs& args) {
                       auto* platform_view = typed_platform_view();
                       if (platform_view == nullptr)
                       {
                           return;
                       }
                       const std::string query_text = maui::platform::windows::to_utf8(args.QueryText());
                       if (auto* view = virtual_view(); view != nullptr && query_text != view->text())
                       {
                           if (platform_view->on_text_changed)
                           {
                               platform_view->on_text_changed(platform_view->last_known_text, query_text);
                           }
                       }
                       if (platform_view->on_search_button_pressed)
                       {
                           platform_view->on_search_button_pressed();
                       }
                   })
                .value;

        // GotFocus/LostFocus: SearchBarHandler.Windows special-cases these itself rather than relying on
        // whatever generic FocusManager-based tracking other Windows controls get — the oracle's own
        // comment explains why: tapping an AutoSuggestBox focuses its INTERNAL TextBox, not the
        // AutoSuggestBox itself, so a comparison against PlatformView fails for this control specifically.
        // Ported directly; UpdateIsFocused(bool) is i_view::set_is_focused here.
        platform.got_focus_token = box.GotFocus([this](const IInspectable&, const winui::RoutedEventArgs&) {
                                          if (auto* view = virtual_view())
                                          {
                                              view->set_is_focused(true);
                                          }
                                      })
                                       .value;
        platform.lost_focus_token = box.LostFocus([this](const IInspectable&, const winui::RoutedEventArgs&) {
                                           if (auto* view = virtual_view())
                                           {
                                               view->set_is_focused(false);
                                           }
                                       })
                                        .value;
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        if (platform.native != nullptr)
        {
            const auto_suggest_box box = as_auto_suggest_box(platform.native);
            box.TextChanged(winrt::event_token{platform.text_changed_token});
            box.QuerySubmitted(winrt::event_token{platform.query_submitted_token});
            box.GotFocus(winrt::event_token{platform.got_focus_token});
            box.LostFocus(winrt::event_token{platform.lost_focus_token});
        }
        platform.text_changed_token = 0;
        platform.query_submitted_token = 0;
        platform.got_focus_token = 0;
        platform.lost_focus_token = 0;
        platform.on_text_changed = nullptr;
        platform.on_search_button_pressed = nullptr;
    }

    void search_bar_handler::map_text(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text());
        platform->last_known_text = platform->text;
        as_auto_suggest_box(platform->native).Text(maui::platform::windows::to_hstring(platform->text));
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->placeholder = std::string(view.placeholder());
        as_auto_suggest_box(platform->native)
            .PlaceholderText(maui::platform::windows::to_hstring(platform->placeholder));
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->placeholder_color = view.placeholder_color();
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        // C#'s UpdateColors: a null PlaceholderColor removes the resource override; the Universal API
        // Contract < 5 fallback branch (which this doesn't have) is legacy-OS back-compat this
        // MSVC-19.44/std:c++latest target doesn't need — see entry_handler.cpp's identical note on the
        // twin PlaceholderForeground property.
        if (!is_set(view, "placeholder_color"))
        {
            remove_resources(box, k_placeholder_color_keys);
            refresh_theme_resources(box);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->placeholder_color)};
        set_resources(box, k_placeholder_color_keys, brush);
        refresh_theme_resources(box);
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note (MauiAutoSuggestBox.SetIsReadOnly's descendant-by-name reach).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_read_only = view.is_read_only();
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->max_length = view.max_length();
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        int max_length = platform->max_length;
        if (max_length == -1) // defensive: the port's own max_length default is already int::max
        {
            max_length = std::numeric_limits<int>::max();
        }
        // The one half of UpdateMaxLength reachable without a descendant walk: clamp the box's OWN Text.
        // The TextBox.MaxLength push and the MaxLength==0-forces-read-only coupling stay mirror-only — see
        // the header note.
        const winrt::hstring current = box.Text();
        if (static_cast<int>(current.size()) > max_length)
        {
            box.Text(winrt::hstring{current.c_str(), static_cast<std::uint32_t>(max_length)});
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        // As in button/entry: an UNSET TextColor must leave the theme brush alone rather than paint
        // transparent black — see [[cpp-unset-color-sentinel-collision]].
        if (!is_set(view, "text_color"))
        {
            remove_resources(box, k_text_color_keys);
            box.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(box);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->text_color)};
        // BOTH the theme-resource overrides AND the direct Foreground, exactly as
        // SearchBarExtensions.UpdateTextColor does — Foreground alone reverts the moment the control
        // enters a different visual state.
        set_resources(box, k_text_color_keys, brush);
        box.Foreground(brush);
        refresh_theme_resources(box);
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign (ControlExtensions.UpdateFont(Control, Font, IFontManager) is unconditional,
        // matching label/entry's map_font — NOT button's skip-if-unset): fontManager.GetFontSize/
        // GetFontFamily resolve the FRAMEWORK default when the font is unset.
        box.FontSize(f.size() > 0 ? f.size() : k_default_font_size);
        box.FontFamily(f.family().empty() ? winui::Media::FontFamily{k_default_font_family}
                                          : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        box.FontStyle(to_font_style(f.slant()));
        box.FontWeight(to_font_weight(f.weight()));
        // Unlike TextBox (entry_handler.cpp's note): AutoSuggestBox IS a Control, and
        // ControlExtensions.UpdateFont pushes IsTextScaleFactorEnabled straight onto it.
        box.IsTextScaleFactorEnabled(f.auto_scaling_enabled());
    }

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units.
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_auto_suggest_box(platform->native).CharacterSpacing(em);
        // ApplyCharacterSpacing's follow-up reach into the "PlaceholderTextContentPresenter" descendant
        // (the placeholder TextBlock's own spacing) needs the same VisualTreeHelper name-lookup this
        // backend doesn't have yet — see the header note; the placeholder keeps its own default spacing.
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        as_auto_suggest_box(platform->native)
            .HorizontalContentAlignment(to_platform_horizontal(platform->horizontal_alignment));
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
        as_auto_suggest_box(platform->native)
            .VerticalContentAlignment(to_platform_vertical(platform->vertical_alignment));
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note (GetFirstDescendant<TextBox>() reach).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note (GetFirstDescendant<TextBox>() reach).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note (GetFirstDescendant<TextBox>() -> UpdateInputScope reach).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->keyboard = view.keyboard();
        }
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — the oracle's own MapCursorPosition already no-ops unless `_queryTextBox` (found by
        // OnLoaded's GetFirstDescendant<TextBox>() walk) is non-null; this backend never populates that
        // handle, so it always takes the SAME no-op branch the oracle takes when the lookup fails. See the
        // header note.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cursor_position = view.cursor_position();
        }
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — same as map_cursor_position above.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selection_length = view.selection_length();
        }
    }

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note (GetDescendantByName<Button>("DeleteButton") reach;
        // MauiCancelButton.cs is NOT the live path here, despite being part of the requested oracle set).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cancel_button_color = view.cancel_button_color();
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->search_icon_color = view.search_icon_color();
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        // UpdateSearchIconColor: QueryIcon is a plain property on AutoSuggestBox itself (set in
        // create_platform_view) — no descendant walk needed here, unlike the cancel button.
        if (const auto icon = box.QueryIcon().try_as<winui::Controls::SymbolIcon>())
        {
            if (!is_set(view, "search_icon_color"))
            {
                icon.ClearValue(winui::Controls::IconElement::ForegroundProperty());
            }
            else
            {
                icon.Foreground(
                    winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->search_icon_color)});
            }
        }
        refresh_theme_resources(box);
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        // Mirror only — see the header note. UpdateReturnType's sole effect for SearchBar (unlike Entry)
        // is UpdateInputScope's software-keyboard key LABEL on the descendant TextBox; there is no
        // OnPlatformKeyUp-style Tab-to-next behavior for AutoSuggestBox in the oracle either, so this is a
        // complete mirror, not a partial one.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->bar_return_type = view.return_type();
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (generalised from image_button_handler.cpp, commit a2444f94ba): pin
        // Width/Height to the view's own explicit request instead of clearing to NaN unconditionally,
        // then only WIDEN the incoming constraint at measure time — see the oracle at
        // ViewHandlerExtensions.Windows.cs:56-74 GetDesiredSizeFromHandler + :91-105 AdjustForExplicitSize,
        // and image_button_handler.cpp's get_desired_size for the full writeup. platform_arrange's OWN
        // stamp (below) is UNTOUCHED.
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        box.Width(explicit_width);
        box.Height(explicit_height);
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        box.Measure(
            winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(adjusted_width_constraint),
                                             maui::platform::windows::measure_constraint(adjusted_height_constraint)});
        const auto desired = box.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite (see label/button/entry_handler.cpp's
        // identical note on the stowed-exception 0xC000027B otherwise).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const auto_suggest_box box = as_auto_suggest_box(platform->native);
        winui::Controls::Canvas::SetLeft(box, frame.x);
        winui::Controls::Canvas::SetTop(box, frame.y);
        box.Width(frame.width);
        box.Height(frame.height);
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void search_bar_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void search_bar_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void search_bar_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void search_bar_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void search_bar_platform::update_background(const maui::graphics::paint* value)
    {
        // SearchBarHandler.Windows's dedicated MapBackground (-> SearchBarExtensions.UpdateBackground's
        // TextControlBackground resource-key dance) is NOT replicated: this port's search_bar_handler.hpp
        // routes Background through the generic-IView push instead (matching button/entry, which take the
        // identical documented simplification for their own dedicated-vs-generic background maps).
        // Identical AT REST; diverges only in the hover/focused/disabled visual states.
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core

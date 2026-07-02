// search_bar_handler — Windows (WinUI 3) platform partial: a REAL
// Microsoft.UI.Xaml.Controls.AutoSuggestBox (SearchBarHandler.Windows.CreatePlatformView's control,
// with AutoMaximizeSuggestionArea = false and a Find SymbolIcon QueryIcon). The windows twin of
// src/platform/apple/search_bar_handler.mm (NSSearchField) / the android JNI partial, and the
// real-native sibling of the headless mirror partial (src/platform/headless/search_bar_handler.cpp).
// Every map_* pushes its property onto the control where the WinUI surface is direct; the native
// TextChanged / QuerySubmitted events flow back through the platform callbacks into
// i_search_bar::send_text_changed / send_search_button_pressed.
//
// Ported DIRECTLY from SearchBarHandler.Windows.cs + Platform/Windows/{SearchBarExtensions.cs,
// ControlExtensions.cs (UpdateFont), ViewExtensions.cs, AlignmentExtensions.cs, FontExtensions.cs,
// CharacterSpacingExtensions.cs} + Fonts/FontManager.Windows.cs.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - Several SearchBarExtensions bodies reach INTO the AutoSuggestBox's template (the inner query
//     TextBox via GetFirstDescendant<TextBox>, the "DeleteButton", the placeholder ContentControl) or
//     the themed resource keys (TextControlBackground*/TextControlForeground*/
//     TextControlPlaceholderForeground* + RefreshThemeResources) — the template-descendant and
//     resource-dictionary seams have not landed on this backend, so those maps keep the headless mirror
//     with an honest // deferred: placeholder_color (resource keys only), is_read_only + the inner-
//     TextBox half of max_length (MauiAutoSuggestBox.SetIsReadOnly attached), is_text_prediction /
//     is_spell_check / keyboard / return_type (all walk to the query TextBox's InputScope),
//     cursor_position / selection_length (the _queryTextBox), cancel_button_color (the DeleteButton).
//   - map_text_color pushes the direct Foreground (C# sets BOTH the TextControlForeground* keys AND
//     Foreground); the per-state keys are deferred with the resource seam. Null branches ride
//     ClearValue, discriminated through BindableObject.IsSet (the port's color has no null).
//   - map_search_icon_color IS pushed (SymbolIcon QueryIcon Foreground — the one template-free color).
//   - The Loaded → re-push pass (OnLoaded re-runs the color/readonly/keyboard updates once the template
//     exists, then hooks the query TextBox's SelectionChanged) and the GotFocus/LostFocus →
//     UpdateIsFocused pair are deferred with the same template/focus seams.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14 (ControlContentThemeFontSize).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// the on_text_changed/on_search_button_pressed callbacks stay invokable C++ callbacks (the
// cross-platform suite drives them) — so that suite observes exactly the headless partial's behavior.

#include "maui/core/search_bar_handler.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_search_bar.hpp"
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
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host). Same constant as the
    // label/button/entry partials.
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::AutoSuggestBox suggest_box_of(const maui::core::search_bar_platform& platform)
    {
        return wnative::borrow<muxc::AutoSuggestBox>(platform.native);
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

    // AlignmentExtensions.ToPlatformHorizontalAlignment: Center → Center, End → Right, Justify →
    // Stretch, else Left.
    [[nodiscard]] mux::HorizontalAlignment to_horizontal_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return mux::HorizontalAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::HorizontalAlignment::Right;
            case maui::core::text_alignment::justify:
                return mux::HorizontalAlignment::Stretch;
            case maui::core::text_alignment::start:
            default:
                return mux::HorizontalAlignment::Left;
        }
    }

    // AlignmentExtensions.ToPlatformVerticalAlignment: Center → Center, End → Bottom, else Top.
    [[nodiscard]] mux::VerticalAlignment to_vertical_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return mux::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::VerticalAlignment::Bottom;
            default:
                return mux::VerticalAlignment::Top;
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the AutoSuggestBox (the wnative shape of the
    // pimpl-owned-native doctrine). The event tokens are plain ints — on_disconnect_handler revokes them.
    search_bar_platform::~search_bar_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real AutoSuggestBox when one exists.

    void search_bar_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void search_bar_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void search_bar_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // SearchBarHandler.Windows.MapIsEnabled → SearchBarExtensions.UpdateIsEnabled:
        // platformControl.IsEnabled = searchBar.IsEnabled.
        if (auto box = suggest_box_of(*this))
        {
            box.IsEnabled(value);
        }
    }

    void search_bar_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void search_bar_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto box = suggest_box_of(*this);
        if (box == nullptr)
        {
            return;
        }
        // SearchBarHandler.Windows.MapBackground → SearchBarExtensions.UpdateBackground: a null brush
        // removes the TextControlBackground* resource keys (→ theme default), a value sets them all +
        // RefreshThemeResources. The port pushes the DIRECT Background property (deferred: the per-state
        // resource keys — see the header) and ClearValue for the null branch.
        if (value == nullptr)
        {
            box.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            box.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
        try
        {
            // SearchBarHandler.Windows.CreatePlatformView: new AutoSuggestBox {
            // AutoMaximizeSuggestionArea = false, QueryIcon = new SymbolIcon(Symbol.Find) }.
            const muxc::AutoSuggestBox box;
            box.AutoMaximizeSuggestionArea(false);
            box.QueryIcon(muxc::SymbolIcon{muxc::Symbol::Find});
            platform->native = wnative::store(box); // released in ~search_bar_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        // The headless twin's inbound callbacks — ALWAYS wired, even XAML-less, so the cross-platform
        // suite can drive them directly (the android partial's shape). on_text_changed updates
        // last_known_text and forwards to i_search_bar::send_text_changed; on_search_button_pressed →
        // send_search_button_pressed.
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
        auto box = suggest_box_of(platform);
        if (box == nullptr)
        {
            return;
        }
        // The native events route through the platform callbacks (the peer is the platform struct,
        // whose heap address is stable until disconnect revokes these handlers).
        auto* peer = &platform;
        // ConnectHandler: platformView.TextChanged += OnTextChanged — a ProgrammaticChange is skipped
        // (the map_text push must not echo back); a user edit pushes the native text onto the virtual
        // view (VirtualView.Text = sender.Text). The port's seam carries (old, new) with the old value
        // from last_known_text.
        const winrt::event_token text_token = box.TextChanged(
            [peer](const muxc::AutoSuggestBox& sender, const muxc::AutoSuggestBoxTextChangedEventArgs& args) {
                if (args.Reason() == muxc::AutoSuggestionBoxTextChangeReason::ProgrammaticChange)
                {
                    return;
                }
                if (sender == nullptr || !peer->on_text_changed)
                {
                    return;
                }
                const std::string new_text = wnative::to_utf8(sender.Text());
                if (new_text != peer->last_known_text)
                {
                    peer->on_text_changed(peer->last_known_text, new_text);
                }
            });
        platform.text_changed_token = text_token.value;
        // ConnectHandler: platformView.QuerySubmitted += OnQuerySubmitted — the query text may lag the
        // delayed TextChanged (C# syncs VirtualView.Text = e.QueryText first), then SearchButtonPressed.
        const winrt::event_token query_token = box.QuerySubmitted(
            [peer](const muxc::AutoSuggestBox&, const muxc::AutoSuggestBoxQuerySubmittedEventArgs& args) {
                const std::string query = wnative::to_utf8(args.QueryText());
                if (query != peer->last_known_text && peer->on_text_changed)
                {
                    peer->on_text_changed(peer->last_known_text, query);
                }
                if (peer->on_search_button_pressed)
                {
                    peer->on_search_button_pressed();
                }
            });
        platform.query_submitted_token = query_token.value;
        // deferred: Loaded += OnLoaded (the template-ready re-push of colors/readonly/keyboard + the
        // query TextBox's SelectionChanged hookup) and GotFocus/LostFocus += UpdateIsFocused (the focus
        // seam) — header deviations.
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        // DisconnectHandler: QuerySubmitted -= OnQuerySubmitted; TextChanged -= OnTextChanged (the
        // Loaded/focus/selection unhooks belong to the deferred seams above). The C++ callbacks are
        // cleared like the headless twin.
        platform.on_text_changed = nullptr;
        platform.on_search_button_pressed = nullptr;
        if (auto box = suggest_box_of(platform))
        {
            if (platform.text_changed_token != 0)
            {
                box.TextChanged(winrt::event_token{platform.text_changed_token});
            }
            if (platform.query_submitted_token != 0)
            {
                box.QuerySubmitted(winrt::event_token{platform.query_submitted_token});
            }
        }
        platform.text_changed_token = 0;
        platform.query_submitted_token = 0;
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
        // SearchBarExtensions.UpdateText: platformControl.Text = searchBar.Text (the native TextChanged
        // this raises reports ProgrammaticChange and is skipped by the connect handler).
        if (auto box = suggest_box_of(*platform))
        {
            box.Text(wnative::to_hstring_utf8(view.text()));
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
        // SearchBarExtensions.UpdatePlaceholder: PlaceholderText = placeholder ?? string.Empty.
        if (auto box = suggest_box_of(*platform))
        {
            box.PlaceholderText(wnative::to_hstring_utf8(view.placeholder()));
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdatePlaceholderColor writes ONLY the themed resource keys
        // (TextControlPlaceholderForeground* + RefreshThemeResources) — the AutoSuggestBox has no direct
        // placeholder-foreground property (unlike TextBox.PlaceholderForeground), so the push needs the
        // resource-dictionary seam that has not landed on this backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->placeholder_color = view.placeholder_color();
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateIsReadOnly → MauiAutoSuggestBox.SetIsReadOnly(platformControl,
        // searchBar.MaxLength == 0 || searchBar.IsReadOnly) — an attached property that walks to the
        // template's query TextBox; the template-descendant seam has not landed on this backend.
        // Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_read_only = view.is_read_only();
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->max_length = view.max_length();
        auto box = suggest_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateMaxLength: -1 widens to int.MaxValue; the current text past the cap
        // is truncated. deferred: the GetChildren<TextBox> MaxLength push and the
        // MauiAutoSuggestBox.SetIsReadOnly(0 → read-only) half — both reach the template's query TextBox
        // (template-descendant seam).
        int max_length = view.max_length();
        if (max_length == -1)
        {
            max_length = std::numeric_limits<int>::max();
        }
        const winrt::hstring current = box.Text();
        if (max_length >= 0 && current.size() > static_cast<std::uint32_t>(max_length))
        {
            box.Text(winrt::hstring{std::wstring_view{current}.substr(0, static_cast<std::size_t>(max_length))});
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        auto box = suggest_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateTextColor: null → RemoveKeys(TextControlForeground*) + Foreground =
        // null; value → SetValueForAllKey + Foreground = tintBrush (C# sets BOTH). The port pushes the
        // direct Foreground half (deferred: the per-state resource keys — header), the null branch
        // discriminated through BindableObject.IsSet (the port's color has no null).
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

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        auto box = suggest_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateFont → ControlExtensions.UpdateFont: FontSize + FontFamily +
        // FontStyle + FontWeight + IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily —
        // registrar skipped, header).
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

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // SearchBarExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm(). deferred: the
        // ApplyCharacterSpacing walk ("PlaceholderTextContentPresenter" ContentControl's TextBlock,
        // applied on Loaded) — the template-descendant seam has not landed on this backend.
        if (auto box = suggest_box_of(*platform))
        {
            box.CharacterSpacing(wnative::to_em(view.character_spacing()));
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
        // SearchBarExtensions.UpdateHorizontalTextAlignment: HorizontalContentAlignment =
        // alignment.ToPlatformHorizontalAlignment().
        if (auto box = suggest_box_of(*platform))
        {
            box.HorizontalContentAlignment(to_horizontal_alignment(view.horizontal_text_alignment()));
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
        // SearchBarExtensions.UpdateVerticalTextAlignment: VerticalContentAlignment =
        // alignment.ToPlatformVerticalAlignment().
        if (auto box = suggest_box_of(*platform))
        {
            box.VerticalContentAlignment(to_vertical_alignment(view.vertical_text_alignment()));
        }
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateIsTextPredictionEnabled walks to the template's query
        // TextBox (GetFirstDescendant<TextBox>) and re-runs TextBoxExtensions.UpdateIsTextPredictionEnabled
        // on it — the template-descendant seam has not landed on this backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateIsSpellCheckEnabled walks to the template's query TextBox
        // (GetFirstDescendant<TextBox>) — the template-descendant seam has not landed on this backend.
        // Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateKeyboard walks to the template's query TextBox and runs
        // TextBoxExtensions.UpdateInputScope on it — the template-descendant seam has not landed on this
        // backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->keyboard = view.keyboard();
        }
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarHandler.Windows.MapCursorPosition targets the _queryTextBox captured in
        // OnLoaded (the template's query TextBox) — the Loaded/template seam has not landed on this
        // backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cursor_position = view.cursor_position();
        }
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarHandler.Windows.MapSelectionLength targets the _queryTextBox captured in
        // OnLoaded — the Loaded/template seam has not landed on this backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selection_length = view.selection_length();
        }
    }

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateCancelButtonColor walks to the template's "DeleteButton"
        // (GetDescendantByName<Button>) and writes the TextControlButtonForeground* keys — the
        // template-descendant + resource-dictionary seams have not landed on this backend. Headless
        // mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cancel_button_color = view.cancel_button_color();
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->search_icon_color = view.search_icon_color();
        auto box = suggest_box_of(*platform);
        if (box == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateSearchIconColor: the QueryIcon SymbolIcon's Foreground — null →
        // ClearValue(SymbolIcon.ForegroundProperty), value → Foreground = brush (the RefreshThemeResources
        // tail rides the deferred resource seam). The null branch is discriminated through
        // BindableObject.IsSet (the port's color has no null).
        const auto icon = box.QueryIcon();
        const auto query_icon = icon != nullptr ? icon.try_as<muxc::SymbolIcon>() : muxc::SymbolIcon{nullptr};
        if (query_icon == nullptr)
        {
            return;
        }
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("search_icon_color");
        if (color_is_set)
        {
            query_icon.Foreground(wnative::to_brush(view.search_icon_color()));
        }
        else
        {
            query_icon.ClearValue(muxc::IconElement::ForegroundProperty());
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        // deferred: SearchBarExtensions.UpdateReturnType walks to the template's query TextBox and runs
        // TextBoxExtensions.UpdateReturnType (→ UpdateInputScope) on it — the template-descendant seam
        // has not landed on this backend. Headless mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->bar_return_type = view.return_type();
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint,
                                                              double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (~200pt single-line bar,
            // 30pt line) so the backend-agnostic size-request suites see consistent numbers.
            double width = 200.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 30.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (the value C#'s MapWidth/MapHeight would have pushed — see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the AutoSuggestBox
        // to the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout
        // model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core

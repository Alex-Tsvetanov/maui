// editor_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.TextBox, the same base
// native type EditorHandler.Windows.cs creates — with AcceptsReturn=true + TextWrapping=Wrap, the two
// flags that make it multi-line (the only structural difference from entry_handler.cpp's single-line
// TextBox; the two controls otherwise share TextBoxExtensions.cs almost entirely). Ported from
// EditorHandler.Windows.cs + TextBoxExtensions.cs.
//
// DOCUMENTED DEVIATION from EditorHandler.Windows.cs's own event wiring — Completed rides LostFocus
// (OnLostFocus -> VirtualView?.Completed()), NOT KeyUp/Enter: a multi-line editor's Enter key inserts a
// newline (TextBox.AcceptsReturn=true above), it does not submit, so entry_handler.cpp's KeyUp(Enter)
// path has no analog here — this control has no ReturnType at all (IEditor is not IEntry). SelectionChanged
// is wired directly in on_connect_handler rather than a separate SetVirtualView override: the C# oracle
// hand-rolls a `_set` bool specifically so SelectionChanged survives ConnectHandler/DisconnectHandler
// running more than once across virtual-view swaps, but this port's view_handler::set_virtual_view only
// ever calls on_connect_handler ONCE per handler (the "first_setup" guard in view_handler.hpp) — so the
// once-only semantics the C# bool exists to enforce by hand are already the port's default behavior.
//
// Not ported (documented mirror-only — Editor and Entry share TextBoxExtensions.cs, so these are the same
// shortcuts entry_handler.cpp's header documents for the same properties):
//   - VerticalTextAlignment: MauiTextBox.SetVerticalTextAlignment reaches into the control template for
//     the ScrollViewer + placeholder TextBlock (a descendant-by-name walk this backend has no helper for
//     yet); this pushes Control.VerticalContentAlignment instead, a plain public property most of the same
//     template binds its content presenter to.
//   - OnPlatformViewSizeChanged -> MauiTextBox.InvalidateAttachedProperties: re-applies the
//     ScrollViewer/placeholder-descendant VerticalTextAlignment whenever the box resizes. Since this
//     backend substitutes VerticalContentAlignment (a property that does not need re-applying on resize),
//     the SizeChanged subscription itself would have nothing to do — skipped, not silently dropped.
//   - ApplyCharacterSpacingToPlaceholder (the "PlaceholderTextContentPresenter" descendant): same
//     descendant-lookup gap; the placeholder keeps its own default character spacing.
//   - UpdateInputScope's InputScope object (the soft/touch-keyboard layout hint) has no visible effect on
//     a desktop hardware keyboard; the two BOOL side effects it also carries (IsTextPredictionEnabled /
//     IsSpellCheckEnabled) ARE pushed. Unlike Entry, IEditor is not IEntry, so UpdateInputScope's
//     `ReturnType.Search` InputScopeName branch never applied here in the first place — nothing lost.
//   - ClearButtonVisibility / IsPassword / ReturnType / CursorColor / AdjustsFontSizeToFitWidth: Entry-only
//     surface; IEditor has none of them, so entry_handler.cpp's equivalents have no counterpart here.

#include "maui/core/editor_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): you need the FULL header for every
// namespace whose MEMBERS you call. Without this one, IVector<T>::Append and friends are only
// forward-declared and every call fails with "error C3779: a function that returns 'auto'
// cannot be used before it is defined" - which does not read as "add an include".
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using text_box = winui::Controls::TextBox;
    using IInspectable = winrt::Windows::Foundation::IInspectable;

    text_box as_text_box(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<text_box>();
    }

    // "Was this property explicitly set?" — see the twin in entry_handler.cpp / label_handler.cpp /
    // button_handler.cpp for why this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // TextAlignmentExtensions.ToPlatform(isLtr: true) — same rationale as entry_handler.cpp (no
    // FlowDirection wired through here yet).
    winui::TextAlignment to_platform(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::TextAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::TextAlignment::Right;
            case maui::core::text_alignment::justify:
                return winui::TextAlignment::Justify;
            case maui::core::text_alignment::start:
            default:
                return winui::TextAlignment::Left;
        }
    }

    // VerticalAlignmentExtensions.ToPlatformVerticalAlignment, pushed onto Control.VerticalContentAlignment
    // — see the header note on why this substitutes for MauiTextBox's ScrollViewer-descendant reach.
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

    // TextBoxExtensions.UpdateInputScope's two BOOL side effects (IsTextPredictionEnabled /
    // IsSpellCheckEnabled) — the InputScope object itself (the touch-keyboard hint) is skipped, see the
    // header note. Shared by map_is_text_prediction_enabled / map_is_spell_check_enabled / map_keyboard,
    // exactly as C#'s three mappers all funnel into the one UpdateInputScope. Unlike entry_handler.cpp's
    // twin, there is no `textInput is IEntry entry && entry.ReturnType == ReturnType.Search` branch to
    // reproduce: IEditor is not IEntry, so that branch never ran for this control in the oracle either.
    void apply_input_scope_flags(const text_box& box, const maui::core::i_editor& view)
    {
        const maui::core::keyboard kb = view.keyboard();
        if (kb.kind() == maui::core::keyboard::kind::custom)
        {
            box.IsTextPredictionEnabled(maui::core::has_flag(kb.flags(), maui::core::keyboard_flags::suggestions));
            box.IsSpellCheckEnabled(maui::core::has_flag(kb.flags(), maui::core::keyboard_flags::spellcheck));
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
    editor_platform::~editor_platform()
    {
        // Revoke exactly what on_connect_handler registered, even if disconnect never ran (mirrors
        // entry_platform's identical destructor safety net).
        editor_handler::on_disconnect_handler(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<editor_platform> editor_handler::create_platform_view()
    {
        auto platform = std::make_unique<editor_platform>();
        text_box box;
        // EditorHandler.Windows.CreatePlatformView: the two flags that make a plain TextBox multi-line
        // (the only structural difference from entry_handler.cpp's create_platform_view).
        box.AcceptsReturn(true);
        box.TextWrapping(winui::TextWrapping::Wrap);
        platform->native = maui::platform::windows::take<winui::UIElement>(box);
        return platform;
    }

    void editor_handler::on_connect_handler(editor_platform& platform)
    {
        // Cross-platform hooks: forward to the virtual view (headless tests invoke these directly; the
        // native events wired below call them instead of send_* directly, matching entry_handler's
        // identical indirection convention on this backend).
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
        if (platform.native == nullptr)
        {
            return;
        }
        const text_box box = as_text_box(platform.native);

        // TextChanged: EditorHandler.Windows's OnTextChanged -> ITextInputExtensions.UpdateText — an
        // unconditional "the native Text IS the new real value" (no password obfuscation on this control,
        // unlike entry_handler.cpp's MauiPasswordTextBox dance).
        platform.text_changed_token =
            box.TextChanged([this](const IInspectable&, const winui::Controls::TextChangedEventArgs&) {
                   auto* platform_view = typed_platform_view();
                   if (platform_view == nullptr || platform_view->native == nullptr)
                   {
                       return;
                   }
                   const text_box native_box = as_text_box(platform_view->native);
                   const std::string new_value = maui::platform::windows::to_utf8(native_box.Text());
                   if (new_value == platform_view->last_known_text)
                   {
                       // Same real value (e.g. the recursive bounce from our own map_text re-assigning
                       // Text below) — do not re-report a no-op edit.
                       return;
                   }
                   if (platform_view->on_text_changed)
                   {
                       platform_view->on_text_changed(platform_view->last_known_text, new_value);
                   }
               })
                .value;

        // LostFocus: EditorHandler.Windows's OnLostFocus -> VirtualView?.Completed(). A multi-line editor
        // has no KeyUp/Enter-submits behavior (Enter inserts a newline, per AcceptsReturn above), so
        // end-of-edit is end-of-focus here — unlike entry_handler.cpp's KeyUp(Enter) path, which has no
        // analog on this control.
        platform.lost_focus_token = box.LostFocus([this](const IInspectable&, const winui::RoutedEventArgs&) {
                                           if (auto* platform_view = typed_platform_view();
                                               platform_view != nullptr && platform_view->on_completed)
                                           {
                                               platform_view->on_completed();
                                           }
                                       })
                                        .value;

        // SelectionChanged: EditorHandler.Windows's OnSelectionChanged — the user moving the caret writes
        // CursorPosition/SelectionLength back onto the virtual view. No cross-platform hook exists for
        // this (i_editor has no send_cursor_changed — cursor/selection are plain inbound setters), so this
        // calls the virtual view directly, matching entry_handler.cpp's identical SelectionChanged wiring
        // (see the header note on why on_connect_handler alone gives the once-only semantics the C#
        // SetVirtualView override's `_set` bool otherwise has to enforce by hand).
        platform.selection_changed_token =
            box.SelectionChanged([this](const IInspectable&, const winui::RoutedEventArgs&) {
                   auto* platform_view = typed_platform_view();
                   if (platform_view == nullptr || platform_view->native == nullptr)
                   {
                       return;
                   }
                   const text_box native_box = as_text_box(platform_view->native);
                   const auto cursor = std::max<std::int32_t>(0, native_box.SelectionStart());
                   const auto selection_length = native_box.SelectionLength();
                   if (auto* view = virtual_view())
                   {
                       if (view->cursor_position() != cursor)
                       {
                           view->set_cursor_position(cursor);
                       }
                       if (view->selection_length() != selection_length)
                       {
                           view->set_selection_length(selection_length);
                       }
                   }
               })
                .value;
    }

    void editor_handler::on_disconnect_handler(editor_platform& platform)
    {
        if (platform.native != nullptr)
        {
            const text_box box = as_text_box(platform.native);
            box.TextChanged(winrt::event_token{platform.text_changed_token});
            box.LostFocus(winrt::event_token{platform.lost_focus_token});
            box.SelectionChanged(winrt::event_token{platform.selection_changed_token});
        }
        platform.text_changed_token = 0;
        platform.lost_focus_token = 0;
        platform.selection_changed_token = 0;
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
    }

    void editor_handler::map_text(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const std::string new_text(view.text());
        // UpdateText's ping-pong guard: bail if this control already shows the value (the MauiPasswordTextBox
        // branch in TextBoxExtensions.UpdateText has no analog on this control — no IsPassword here).
        if (platform->text == new_text)
        {
            return;
        }
        platform->text = new_text;
        platform->last_known_text = new_text;
        const text_box box = as_text_box(platform->native);
        box.Text(maui::platform::windows::to_hstring(new_text));
        if (!new_text.empty())
        {
            box.Select(static_cast<std::int32_t>(box.Text().size()), 0);
        }
    }

    void editor_handler::map_placeholder(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->placeholder = std::string(view.placeholder());
        as_text_box(platform->native).PlaceholderText(maui::platform::windows::to_hstring(platform->placeholder));
    }

    void editor_handler::map_placeholder_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->placeholder_color = view.placeholder_color();
        const text_box box = as_text_box(platform->native);
        // UpdatePlaceholderColor's Windows.Foundation.UniversalApiContract >= 5 branch only (the direct
        // PlaceholderForeground property); the < 5 resource-key fallback is legacy-OS back-compat this
        // MSVC-19.44/std:c++latest target doesn't need — see entry_handler.cpp's identical note.
        if (!is_set(view, "placeholder_color"))
        {
            box.ClearValue(text_box::PlaceholderForegroundProperty());
            return;
        }
        box.PlaceholderForeground(
            winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->placeholder_color)});
    }

    void editor_handler::map_text_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const text_box box = as_text_box(platform->native);
        // As in entry/label/button: an UNSET TextColor must leave the theme brush alone rather than paint
        // transparent black — see [[cpp-unset-color-sentinel-collision]]. This pushes the direct
        // Foreground only (identical AT REST to UpdateTextColor's resource-key override; diverges only in
        // the hover/focused/disabled visual states — the same documented simplification entry_handler.cpp
        // takes for the same property).
        if (!is_set(view, "text_color"))
        {
            box.ClearValue(winui::Controls::Control::ForegroundProperty());
            return;
        }
        box.Foreground(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->text_color)});
    }

    void editor_handler::map_font(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const text_box box = as_text_box(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip (matching entry/label_handler's map_font, NOT button_handler's skip-if-
        // unset): FontExtensions.UpdateFont resolves fontManager.GetFontSize/GetFontFamily unconditionally,
        // and those resolve the FRAMEWORK default when the font is unset.
        box.FontSize(f.size() > 0 ? f.size() : maui::platform::windows::default_font_size());
        box.FontFamily(f.family().empty() ? maui::platform::windows::default_font_family()
                                          : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        box.FontStyle(to_font_style(f.slant()));
        box.FontWeight(to_font_weight(f.weight()));
        // No IsTextScaleFactorEnabled push: that property lives on TextBlock (see button/entry_handler.cpp's
        // identical note for its content TextBlock) — a bare TextBox has no content TextBlock to target.
    }

    void editor_handler::map_character_spacing(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units.
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_text_box(platform->native).CharacterSpacing(em);
        // ApplyCharacterSpacingToPlaceholder (the "PlaceholderTextContentPresenter" descendant) needs the
        // same VisualTreeHelper name-lookup this backend doesn't have yet (see the header note and
        // entry_handler.cpp's identical shortcut) — the placeholder keeps its own default spacing;
        // documented, not silently dropped.
    }

    void editor_handler::map_horizontal_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        as_text_box(platform->native).TextAlignment(to_platform(platform->horizontal_alignment));
    }

    void editor_handler::map_vertical_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
        // Control.VerticalContentAlignment — see the header note on why this substitutes for
        // MauiTextBox's ScrollViewer/placeholder-descendant reach.
        as_text_box(platform->native).VerticalContentAlignment(to_platform_vertical(platform->vertical_alignment));
    }

    void editor_handler::map_is_read_only(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_read_only = view.is_read_only();
        as_text_box(platform->native).IsReadOnly(platform->is_read_only);
    }

    void editor_handler::map_is_text_prediction_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void editor_handler::map_is_spell_check_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void editor_handler::map_max_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->max_length = view.max_length();
        const text_box box = as_text_box(platform->native);
        int max_length = platform->max_length;
        // MaxLength == 0 forces read-only (Editor's own "no characters allowed" semantic, distinct from
        // native TextBox.MaxLength == 0 meaning "unlimited"); otherwise defer to IsReadOnly as-is. Identical
        // to entry_handler.cpp's map_max_length — TextBoxExtensions.UpdateMaxLength is shared verbatim.
        box.IsReadOnly(max_length == 0 ? true : view.is_read_only());
        if (max_length == -1) // defensive: the port's own max_length default is already int::max
        {
            max_length = std::numeric_limits<int>::max();
        }
        box.MaxLength(max_length);
        const winrt::hstring current = box.Text();
        if (static_cast<int>(current.size()) > max_length)
        {
            box.Text(winrt::hstring{current.c_str(), static_cast<std::uint32_t>(max_length)});
        }
    }

    void editor_handler::map_keyboard(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void editor_handler::map_cursor_position(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const text_box box = as_text_box(platform->native);
        const auto text_length = static_cast<int>(box.Text().size());
        int position = view.cursor_position();
        // "It seems that the TextBox does not limit the CursorPosition to the Text.Length natively" —
        // UpdateCursorPosition clamps AND writes the clamped value back onto the virtual view.
        if (position > text_length)
        {
            position = text_length;
            view.set_cursor_position(position);
        }
        platform->cursor_position = position;
        if (box.SelectionStart() != position)
        {
            box.SelectionStart(position);
        }
    }

    void editor_handler::map_selection_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const text_box box = as_text_box(platform->native);
        const auto max_length = static_cast<int>(box.Text().size()) - box.SelectionStart();
        int length = view.selection_length();
        if (length > max_length)
        {
            length = max_length;
            view.set_selection_length(length);
        }
        platform->selection_length = length;
        if (box.SelectionLength() != length)
        {
            box.SelectionLength(length);
        }
    }

    maui::graphics::size editor_handler::get_desired_size(double width_constraint, double height_constraint) const
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
        const text_box box = as_text_box(platform->native);
        // ARRANGE/EXPLICIT-SIZE FIX (see entry_handler.cpp's identical get_desired_size and
        // image_button_handler.cpp's original writeup, commit a2444f94ba): pin Width/Height to the view's
        // own explicit request (this port's unset sentinel is NaN, same as C#'s NaN-is-unspecified
        // convention) instead of clearing to NaN unconditionally, then only WIDEN the incoming constraint
        // at measure time -- see the oracle at ViewHandlerExtensions.Windows.cs:56-74
        // GetDesiredSizeFromHandler + :91-105 AdjustForExplicitSize. platform_arrange's OWN stamp (below)
        // is UNTOUCHED.
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

    void editor_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite (see entry/label/button_handler.cpp's
        // identical note on the stowed-exception 0xC000027B otherwise).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const text_box box = as_text_box(platform->native);
        winui::Controls::Canvas::SetLeft(box, frame.x);
        winui::Controls::Canvas::SetTop(box, frame.y);
        box.Width(frame.width);
        box.Height(frame.height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the editor has a real size.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot.
    void editor_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void editor_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void editor_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void editor_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void editor_platform::update_background(const maui::graphics::paint* value)
    {
        // EditorHandler.Windows's dedicated MapBackground (-> TextBoxExtensions.UpdateBackground's
        // TextControlBackground resource-key dance, matching text_color's hover-state persistence) is NOT
        // replicated: this port's editor_handler.hpp routes Background through the generic-IView push
        // instead (matching entry/label/button, which have no dedicated map_background either). Identical
        // AT REST; diverges only in the hover/focused/disabled visual states.
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core

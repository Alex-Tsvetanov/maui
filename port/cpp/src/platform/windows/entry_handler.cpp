// entry_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.TextBox, the same base
// native type EntryHandler.Windows.cs creates (a MauiPasswordTextBox : TextBox). Ported from
// EntryHandler.Windows.cs + TextBoxExtensions.cs + the obfuscation half of MauiPasswordTextBox.cs.
//
// DOCUMENTED DEVIATION — IsPassword / MauiPasswordTextBox. C# ALWAYS creates a MauiPasswordTextBox (a
// custom TextBox SUBCLASS that obfuscates its own Text with '●' while leaving a separate `Password`
// property holding the real value); IsPassword only toggles its internal behavior. Authoring a real
// WinRT subclass (an .idl + MIDL/cppwinrt codegen step) is infrastructure this backend does not have —
// every control here is a plain projected WinUI type (see PROFILE.md / the button/label twins), so this
// slice uses a plain `TextBox` and reproduces MauiPasswordTextBox's OBSERVABLE behavior in free functions
// instead of a subclass:
//   - entry_platform::text stands in for MauiPasswordTextBox.Password (the real value); the native
//     TextBox.Text holds the OBFUSCATED string whenever is_password is set.
//   - `determine_text_from_password` ports MauiPasswordTextBox.DetermineTextFromPassword verbatim (the
//     algorithm that reconstructs the real text from the box's raw post-edit content).
//   - The DELAYED "leave the last typed character briefly visible" path (IsObfuscationDelayed /
//     DelayObfuscation, needing a DispatcherQueue timer) is NOT ported: C#'s own
//     `s_shouldBeDelayed = DeviceInfo.Idiom != DeviceIdiom.Desktop` is FALSE for every desktop MAUI-on-
//     Windows app (this port's only Windows target), so the oracle itself always takes the IMMEDIATE
//     obfuscation path (Obfuscate(text, leaveLastVisible: false)) here — the delayed path is dead code
//     for this target, not a feature this slice drops.
//
// Not ported (documented mirror-only, matching the headless partial + the macOS twin's identical
// shortcuts for the same properties):
//   - ClearButtonVisibility: MauiTextBox.SetIsDeleteButtonEnabled resizes a "DeleteButton" template-part
//     column found via a VisualTreeHelper descendant-by-name walk; this backend has no descendant-lookup
//     helper yet (see label_handler.cpp's NeedsContainer note for the same class of gap).
//   - Keyboard / ReturnType's InputScope construction: TextBoxExtensions.UpdateInputScope's InputScope
//     object (the soft/touch-keyboard layout hint + "Enter/Search/Go/Send" key label) has no visible
//     effect on a desktop hardware keyboard and is invisible to a parity screenshot; the two BOOL side
//     effects UpdateInputScope also carries (IsTextPredictionEnabled / IsSpellCheckEnabled) ARE pushed,
//     since those are directly observable. ReturnType.Next's Tab-to-next-field behavior IS implemented
//     (OnPlatformKeyUp), just not through InputScope.
//   - VerticalTextAlignment: MauiTextBox.SetVerticalTextAlignment reaches into the control template for
//     the ScrollViewer + placeholder TextBlock (again a descendant-by-name walk); this pushes
//     Control.VerticalContentAlignment instead, a plain public property most of the same template binds
//     its content presenter to — a real, direct API rather than a template-part reach.

#include "maui/core/entry_handler.hpp"
#include "maui/core/i_ios_entry_specifics.hpp" // --- platform configuration (W2-24) ---

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule (see winui_interop.hpp): you need the FULL header for every
// namespace whose MEMBERS you call. Without this one, IVector<T>::Append is only
// forward-declared and every call fails with "error C3779: a function that returns 'auto'
// cannot be used before it is defined" - which does not read as "add an include".
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Text.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/return_type.hpp"
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

    // "Was this property explicitly set?" — see the twin in button_handler.cpp / label_handler.cpp for
    // why this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // TextAlignmentExtensions.ToPlatform(isLtr: true) — same rationale as label_handler.cpp (no
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

    // ---- UTF-8 (the port's std::string) <-> UTF-16 (the WinRT string the obfuscation math needs to
    // index code-unit-for-code-unit against SelectionStart) — thin wrappers over the shared hstring
    // conversions in winui_interop.hpp (nothing new to get wrong here).
    std::wstring to_wide(std::string_view utf8)
    {
        const winrt::hstring hs = maui::platform::windows::to_hstring(utf8);
        return {hs.c_str(), hs.size()};
    }

    std::string wide_to_utf8(const std::wstring& wide)
    {
        return maui::platform::windows::to_utf8(winrt::hstring(wide));
    }

    // ---- password obfuscation (MauiPasswordTextBox.cs, immediate-only path — see the header note) ----
    constexpr wchar_t k_obfuscation_char = L'●'; // MauiPasswordTextBox.ObfuscationCharacter '●'

    // Obfuscate(text, leaveLastVisible: false) — the only branch reachable on desktop idiom.
    std::wstring obfuscate(const std::wstring& text)
    {
        return std::wstring(text.size(), k_obfuscation_char);
    }

    winrt::hstring obfuscated_hstring(const std::wstring& real)
    {
        return winrt::hstring(obfuscate(real));
    }

    // MauiPasswordTextBox.DetermineTextFromPassword, ported verbatim (real_text/start/password_text ==
    // C#'s realText/start/passwordText). password_text is the box's raw post-edit content — a mix of
    // untouched '●' runs and the newly typed/pasted literal characters; real_text is the PREVIOUS real
    // value. Bounds-clamped beyond the oracle (which trusts its inputs) since this can't be exercised on
    // real hardware before landing.
    std::wstring determine_text_from_password(const std::wstring& real_text, std::int32_t start,
                                              const std::wstring& password_text)
    {
        std::wstring adjusted = real_text;
        const auto length_difference =
            static_cast<std::ptrdiff_t>(password_text.size()) - static_cast<std::ptrdiff_t>(adjusted.size());
        if (length_difference > 0)
        {
            auto insert_at = std::max<std::ptrdiff_t>(0, static_cast<std::ptrdiff_t>(start) - length_difference);
            insert_at = std::min<std::ptrdiff_t>(insert_at, static_cast<std::ptrdiff_t>(adjusted.size()));
            adjusted.insert(static_cast<std::size_t>(insert_at),
                            std::wstring(static_cast<std::size_t>(length_difference), k_obfuscation_char));
        }
        else if (length_difference < 0)
        {
            const auto erase_at = std::clamp<std::ptrdiff_t>(start, 0, static_cast<std::ptrdiff_t>(adjusted.size()));
            const auto erase_len =
                std::min<std::ptrdiff_t>(-length_difference, static_cast<std::ptrdiff_t>(adjusted.size()) - erase_at);
            adjusted.erase(static_cast<std::size_t>(erase_at), static_cast<std::size_t>(erase_len));
        }
        std::wstring result;
        result.reserve(password_text.size());
        for (std::size_t i = 0; i < password_text.size(); ++i)
        {
            const bool keep_old = password_text[i] == k_obfuscation_char && i < adjusted.size();
            result.push_back(keep_old ? adjusted[i] : password_text[i]);
        }
        return result;
    }

    // TextBoxExtensions.UpdateInputScope's two BOOL side effects (IsTextPredictionEnabled /
    // IsSpellCheckEnabled) — the InputScope object itself (the touch-keyboard hint) is skipped, see the
    // header note. Shared by map_is_text_prediction_enabled / map_is_spell_check_enabled / map_keyboard,
    // exactly as C#'s three mappers all funnel into the one UpdateInputScope.
    void apply_input_scope_flags(const text_box& box, const maui::core::i_entry& view)
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
    entry_platform::~entry_platform()
    {
        // Revoke exactly what on_connect_handler registered, even if disconnect never ran (mirrors
        // button_platform's detach-in-destructor safety net).
        entry_handler::on_disconnect_handler(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<entry_platform> entry_handler::create_platform_view()
    {
        auto platform = std::make_unique<entry_platform>();
        text_box box;
        platform->native = maui::platform::windows::take<winui::UIElement>(box);
        return platform;
    }

    void entry_handler::on_connect_handler(entry_platform& platform)
    {
        // Cross-platform hooks: forward to the virtual view (headless tests invoke these directly; the
        // native events wired below call them instead of send_* directly, matching button_handler's
        // on_click/on_press/on_release indirection convention on this backend).
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

        // TextChanged: EntryHandler.OnPlatformTextChanged + (for the password case)
        // MauiPasswordTextBox.OnNativeTextChanged/UpdatePasswordIfNeeded collapsed into one pass — see the
        // header note on why a single handler can do both without the C# subclass's two-subscriber dance.
        platform.text_changed_token =
            box.TextChanged([this](const IInspectable&, const winui::Controls::TextChangedEventArgs&) {
                   auto* platform_view = typed_platform_view();
                   if (platform_view == nullptr || platform_view->native == nullptr)
                   {
                       return;
                   }
                   const text_box native_box = as_text_box(platform_view->native);
                   const std::wstring old_real = to_wide(platform_view->last_known_text);
                   const winrt::hstring raw_text = native_box.Text();
                   const std::wstring raw_wide(raw_text.c_str(), raw_text.size());
                   const std::wstring new_real =
                       platform_view->is_password
                           ? determine_text_from_password(old_real, native_box.SelectionStart(), raw_wide)
                           : raw_wide;
                   const std::string new_value = wide_to_utf8(new_real);
                   if (new_value == platform_view->last_known_text)
                   {
                       // Same real value (e.g. the recursive bounce from OUR OWN re-obfuscating Text
                       // assignment below, or a paste of identical text) — TextBoxExtensions'
                       // UpdatePasswordIfNeeded `else UpdateVisibleText()` branch: just make sure the
                       // obfuscated display still matches, and do not re-report a no-op edit.
                       if (platform_view->is_password)
                       {
                           const winrt::hstring desired = obfuscated_hstring(new_real);
                           if (native_box.Text() != desired)
                           {
                               native_box.Text(desired);
                           }
                       }
                       return;
                   }
                   if (platform_view->is_password)
                   {
                       native_box.Text(obfuscated_hstring(new_real));
                   }
                   if (platform_view->on_text_changed)
                   {
                       platform_view->on_text_changed(platform_view->last_known_text, new_value);
                   }
               })
                .value;

        // KeyUp: EntryHandler.OnPlatformKeyUp — Enter both raises Completed and, for ReturnType.Next,
        // moves focus to the next element.
        platform.key_up_token =
            box.KeyUp([this](const IInspectable&, const winui::Input::KeyRoutedEventArgs& args) {
                   if (args.Key() != winrt::Windows::System::VirtualKey::Enter)
                   {
                       return;
                   }
                   if (auto* view = virtual_view();
                       view != nullptr && view->return_type() == maui::core::return_type::next)
                   {
                       // The simpler GLOBAL FocusManager::TryMoveFocus(direction) overload, not MAUI's
                       // XamlRoot-scoped ViewExtensions.TryMoveFocus(FindNextElementOptions{SearchRoot=...})
                       // — this slice doesn't thread XamlRoot through the handler. Equivalent whenever
                       // there is a single XamlRoot, true for every gallery page.
                       winui::Input::FocusManager::TryMoveFocus(winui::Input::FocusNavigationDirection::Next);
                   }
                   if (auto* platform_view = typed_platform_view();
                       platform_view != nullptr && platform_view->on_completed)
                   {
                       platform_view->on_completed();
                   }
               })
                .value;

        // SelectionChanged: EntryHandler.OnPlatformSelectionChanged — the user moving the caret writes
        // CursorPosition/SelectionLength back onto the virtual view. No cross-platform hook exists for
        // this (i_entry has no send_cursor_changed — cursor/selection are plain inbound setters), so this
        // calls the virtual view directly, matching the AppKit twin's mauiSelectionChanged.
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

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        if (platform.native != nullptr)
        {
            const text_box box = as_text_box(platform.native);
            box.TextChanged(winrt::event_token{platform.text_changed_token});
            box.KeyUp(winrt::event_token{platform.key_up_token});
            box.SelectionChanged(winrt::event_token{platform.selection_changed_token});
        }
        platform.text_changed_token = 0;
        platform.key_up_token = 0;
        platform.selection_changed_token = 0;
        platform.on_text_changed = nullptr;
        platform.on_completed = nullptr;
    }

    void entry_handler::map_text(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const std::string new_text(view.text());
        // UpdateText's ping-pong guard: bail if this control already shows the value — compared against
        // the REAL-value mirror (the password-mode equivalent of MauiPasswordTextBox.Password ==
        // newText), since the native Text itself is obfuscated when is_password is set.
        if (platform->text == new_text)
        {
            return;
        }
        platform->text = new_text;
        platform->last_known_text = new_text;
        const text_box box = as_text_box(platform->native);
        const winrt::hstring plain = maui::platform::windows::to_hstring(new_text);
        box.Text(platform->is_password ? obfuscated_hstring({plain.c_str(), plain.size()}) : plain);
        if (!new_text.empty())
        {
            box.Select(static_cast<std::int32_t>(box.Text().size()), 0);
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->placeholder = std::string(view.placeholder());
        as_text_box(platform->native).PlaceholderText(maui::platform::windows::to_hstring(platform->placeholder));
    }

    void entry_handler::map_placeholder_color(entry_handler& handler, i_entry& view)
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
        // MSVC-19.44/std:c++latest target doesn't need.
        if (!is_set(view, "placeholder_color"))
        {
            box.ClearValue(text_box::PlaceholderForegroundProperty());
            return;
        }
        box.PlaceholderForeground(
            winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->placeholder_color)});
    }

    void entry_handler::map_is_password(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_password = view.is_password();
        const text_box box = as_text_box(platform->native);
        // OnIsPasswordChanged -> UpdateVisibleText(): refresh Text to either the obfuscated or plain form
        // of the CURRENT real value.
        const winrt::hstring plain = maui::platform::windows::to_hstring(platform->text);
        box.Text(platform->is_password ? obfuscated_hstring({plain.c_str(), plain.size()}) : plain);
        if (!platform->text.empty())
        {
            box.Select(static_cast<std::int32_t>(box.Text().size()), 0);
        }
        // OnIsPasswordChanged -> UpdateInputScope(): force prediction/spellcheck off while obfuscating;
        // otherwise re-derive them from the (unrelated, independent) developer-set properties — there is
        // no "cached" value to restore because those properties don't change when IsPassword toggles.
        if (platform->is_password)
        {
            box.IsTextPredictionEnabled(false);
            box.IsSpellCheckEnabled(false);
        }
        else
        {
            apply_input_scope_flags(box, view);
        }
    }

    void entry_handler::map_is_read_only(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_read_only = view.is_read_only();
        as_text_box(platform->native).IsReadOnly(platform->is_read_only);
    }

    void entry_handler::map_max_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->max_length = view.max_length();
        const text_box box = as_text_box(platform->native);
        int max_length = platform->max_length;
        // MaxLength == 0 forces read-only (Entry's own "no characters allowed" semantic, distinct from
        // native TextBox.MaxLength == 0 meaning "unlimited"); otherwise defer to IsReadOnly as-is.
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

    void entry_handler::map_text_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const text_box box = as_text_box(platform->native);
        // As in label/button: an UNSET TextColor must leave the theme brush alone rather than paint
        // transparent black — see [[cpp-unset-color-sentinel-collision]]. This pushes the direct
        // Foreground only (identical AT REST to UpdateTextColor's resource-key override; diverges only
        // in the hover/focused/disabled visual states — the same documented simplification
        // button_handler.cpp takes for StrokeColor/CornerRadius).
        if (!is_set(view, "text_color"))
        {
            box.ClearValue(winui::Controls::Control::ForegroundProperty());
            return;
        }
        box.Foreground(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->text_color)});
    }

    void entry_handler::map_font(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const text_box box = as_text_box(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip (matching label_handler's map_font, NOT button_handler's skip-if-
        // unset): FontExtensions.UpdateFont resolves fontManager.GetFontSize/GetFontFamily
        // unconditionally, and those resolve the FRAMEWORK default when the font is unset.
        box.FontSize(f.size() > 0 ? f.size() : maui::platform::windows::default_font_size());
        box.FontFamily(f.family().empty() ? maui::platform::windows::default_font_family()
                                          : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        box.FontStyle(to_font_style(f.slant()));
        box.FontWeight(to_font_weight(f.weight()));
        // No IsTextScaleFactorEnabled push: that property lives on TextBlock (see button_handler.cpp's
        // identical note for its content TextBlock) — a bare TextBox has no content TextBlock to target.
    }

    void entry_handler::map_character_spacing(entry_handler& handler, i_entry& view)
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
        // same VisualTreeHelper name-lookup this backend doesn't have yet (see the header note) — the
        // placeholder keeps its own default spacing; documented, not silently dropped.
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        as_text_box(platform->native).TextAlignment(to_platform(platform->horizontal_alignment));
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& handler, i_entry& view)
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

    void entry_handler::map_is_text_prediction_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void entry_handler::map_is_spell_check_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void entry_handler::map_keyboard(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        apply_input_scope_flags(as_text_box(platform->native), view);
    }

    void entry_handler::map_return_type(entry_handler& handler, i_entry& view)
    {
        // Mirror only: UpdateReturnType's sole effect is UpdateInputScope's software-keyboard key LABEL
        // (skipped — see the header note). The actual ReturnType.Next behavior (Tab to the next field on
        // Enter) is implemented directly in the KeyUp handler wired in on_connect_handler, which reads
        // view.return_type() at the time of the keypress rather than this mirror.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->entry_return_type = view.return_type();
        }
    }

    void entry_handler::map_clear_button_visibility(entry_handler& handler, i_entry& view)
    {
        // Mirror only — see the header note (MauiTextBox's DeleteButton template-part lookup needs
        // descendant-by-name infrastructure this backend doesn't have yet; the macOS twin takes the same
        // documented shortcut for the same property).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->clear_button = view.clear_button_visibility();
        }
    }

    void entry_handler::map_cursor_position(entry_handler& handler, i_entry& view)
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

    void entry_handler::map_selection_length(entry_handler& handler, i_entry& view)
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

    // --- platform configuration (W2-24): the iOSSpecific Entry.CursorColor map — Windows has no insertion
    // -point tint analog either (C# only maps this knob on iOS), so this keeps the cross-platform mirror
    // only, with the same IsSet guard as TextExtensions.UpdateCursorColor.
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

    // --- platform configuration (W2-24): the iOSSpecific Entry.AdjustsFontSizeToFitWidth map — C# only
    // maps this knob on iOS (TextBox has no per-field "shrink font to fit width" equivalent), so this
    // keeps the cross-platform mirror only, UNCONDITIONAL like TextExtensions.UpdateAdjustsFontSizeToFitWidth.
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
        // ARRANGE/EXPLICIT-SIZE FIX (generalised from image_button_handler.cpp, commit a2444f94ba): pin
        // Width/Height to the view's own explicit request (this port's unset sentinel is NaN, same as
        // C#'s NaN-is-unspecified convention) instead of clearing to NaN unconditionally, then only WIDEN
        // the incoming constraint at measure time -- see the oracle at ViewHandlerExtensions.Windows.cs:
        // 56-74 GetDesiredSizeFromHandler + :91-105 AdjustForExplicitSize, and image_button_handler.cpp's
        // get_desired_size for the full writeup. platform_arrange's OWN stamp (below) is UNTOUCHED.
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

    void entry_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite (see label/button_handler.cpp's identical
        // note on the stowed-exception 0xC000027B otherwise).
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
        // this re-invoke is what actually installs the clip once the entry has a real size.
        if (const auto* view = virtual_view(); view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void entry_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void entry_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void entry_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void entry_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void entry_platform::update_background(const maui::graphics::paint* value)
    {
        // EntryHandler.Windows's dedicated MapBackground (-> TextBoxExtensions.UpdateBackground's
        // TextControlBackground resource-key dance, matching text_color/stroke_color's hover-state
        // persistence) is NOT replicated: this port's entry_handler.hpp routes Background through the
        // generic-IView push instead (matching label/button, which have no dedicated map_background
        // either). Identical AT REST; diverges only in the hover/focused/disabled visual states.
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core

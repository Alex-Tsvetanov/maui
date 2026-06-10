#pragma once
// maui::core::i_search_bar  <=  Microsoft.Maui.ISearchBar
//
// The virtual-view contract for a search input. Ported from src/Core/src/Core/ISearchBar.cs
// (ISearchBar : IView, ITextInput, ITextAlignment { CancelButtonColor; SearchIconColor; ReturnType;
// SearchButtonPressed(); }).
//
// Inbound channel (native → virtual): C#'s ISearchBar.SearchButtonPressed() becomes
// send_search_button_pressed() (the i_button send_* naming convention — the control raises its
// search_button_pressed event in response); the TextChanged notification is surfaced as
// send_text_changed(old, new), exactly as on i_entry / i_editor.

#include <string_view>

#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_text_input.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/return_type.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_search_bar : public i_view, public i_text_input, public i_text_alignment
    {
    public:
        [[nodiscard]] virtual maui::graphics::color cancel_button_color() const = 0;
        [[nodiscard]] virtual maui::graphics::color search_icon_color() const = 0;
        // Qualified-return to keep the method name from hiding the enum type (the i_entry convention).
        [[nodiscard]] virtual maui::core::return_type return_type() const = 0;

        // Inbound channel (called by the handler on native events).
        virtual void send_search_button_pressed() = 0;
        virtual void send_text_changed(std::string_view old_value, std::string_view new_value) = 0;
    };
} // namespace maui::core

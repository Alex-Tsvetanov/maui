#pragma once
// maui::core::i_entry  <=  Microsoft.Maui.IEntry
//
// The virtual-view contract for a single-line text entry — the first INBOUND-TEXT + first editable
// native control. Ported from src/Core/src/Core/IEntry.cs (IEntry : IView, ITextInput, ITextAlignment).
//
// Inbound channel (native → virtual, mirroring button's send_*): C#'s IEntry.Completed() becomes
// send_completed(); the TextChanged notification (raised by InputView when its Text changes — see
// InputView.OnTextChanged) is surfaced as send_text_changed(old, new) so the headless backend and the
// AppKit delegate can drive the control's text_changed event with the (old, new) pair.

#include <string_view>

#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_text_input.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/return_type.hpp"

namespace maui::core
{
    class i_entry : public i_view, public i_text_input, public i_text_alignment
    {
    public:
        [[nodiscard]] virtual bool is_password() const = 0;
        // return_type()/clear_button_visibility() are qualified-return- to keep the method name from
        // hiding the enum type (as i_text_style::font() does).
        [[nodiscard]] virtual maui::core::return_type return_type() const = 0;
        [[nodiscard]] virtual maui::core::clear_button_visibility clear_button_visibility() const = 0;

        // Inbound channel (called by the handler on native events).
        virtual void send_completed() = 0;
        virtual void send_text_changed(std::string_view old_value, std::string_view new_value) = 0;
    };
} // namespace maui::core

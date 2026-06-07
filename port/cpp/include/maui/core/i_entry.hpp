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
//
// Deferred (OUT OF SCOPE this cut): ReturnType and ClearButtonVisibility (enum subsystems not yet
// ported) — documented here, not stubbed.

#include <string_view>

#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_text_input.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_entry : public i_view, public i_text_input, public i_text_alignment
    {
    public:
        [[nodiscard]] virtual bool is_password() const = 0;

        // Inbound channel (called by the handler on native events).
        virtual void send_completed() = 0;
        virtual void send_text_changed(std::string_view old_value, std::string_view new_value) = 0;
    };
} // namespace maui::core

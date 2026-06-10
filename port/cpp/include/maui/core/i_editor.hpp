#pragma once
// maui::core::i_editor  <=  Microsoft.Maui.IEditor
//
// The virtual-view contract for a multi-line text editor. Ported from src/Core/src/Core/IEditor.cs
// (IEditor : IView, ITextInput, ITextStyle, ITextAlignment — ITextStyle arrives via ITextInput's IText
// here, exactly as i_entry collapses the same chain).
//
// Inbound channel (native → virtual, mirroring entry's send_*): C#'s IEditor.Completed() becomes
// send_completed(); the TextChanged notification (InputView.OnTextChanged) is surfaced as
// send_text_changed(old, new) so the headless backend and the native delegates can drive the control's
// text_changed event with the (old, new) pair.

#include <string_view>

#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_text_input.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_editor : public i_view, public i_text_input, public i_text_alignment
    {
    public:
        // Inbound channel (called by the handler on native events).
        virtual void send_completed() = 0;
        virtual void send_text_changed(std::string_view old_value, std::string_view new_value) = 0;
    };
} // namespace maui::core

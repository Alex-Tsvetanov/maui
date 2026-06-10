#pragma once
// maui::core::i_check_box  <=  Microsoft.Maui.ICheckBox
//
// The virtual-view contract for a binary-choice check box. Ported from src/Core/src/Core/ICheckBox.cs.
//
// IsChecked is MUTABLE in C# — the setter is the INBOUND channel: the native control's checked-changed
// handler writes the user's toggle back through it (the concrete control stores it at the from-handler
// specificity, exactly like C# CheckBox's `ICheckBox.IsChecked` explicit implementation).
//
// Naming note (the i_button convention): C# distinguishes the class property setter (manual) from the
// explicit ICheckBox.IsChecked setter (from-handler) — C++ cannot give one signature two meanings, so
// the inbound setter takes the send_ prefix (send_is_checked), leaving set_is_checked free for the
// control's developer-facing manual setter.
//
// Foreground is C#'s `Paint?` — a nullable brush: a raw borrow of a paint the control owns, null when
// the developer set no Color (the control computes it from its Color property — CheckBox.Foreground =>
// Color?.AsPaint()), matching the i_view::background() borrow convention.

#include "maui/core/i_view.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::core
{
    class i_check_box : public i_view
    {
    public:
        // Gets whether the check box is checked; send_is_checked is the inbound native channel (see the
        // naming note above).
        [[nodiscard]] virtual bool is_checked() const = 0;
        virtual void send_is_checked(bool value) = 0;

        // Gets the check box foreground paint (null = platform default).
        [[nodiscard]] virtual const maui::graphics::paint* foreground() const = 0;
    };
} // namespace maui::core

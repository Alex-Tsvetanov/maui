#pragma once
// maui::core::i_button  <=  Microsoft.Maui.IButton
//
// The virtual-view contract for a button: an IView that also customizes padding + stroke and reacts to
// touch. Ported from src/Core/src/Core/IButton.cs (IButton : IView, IPadding, IButtonStroke). The
// three methods are the INBOUND channel from the platform view: the native control's event handlers
// call them (handler.virtual_view()->send_clicked() etc.), and the concrete control raises its public
// events in response. (The actual Button control is an ITextButton — it also implements i_text.)
//
// Naming note: C#'s IButton.Pressed()/Released()/Clicked() are spelled send_pressed/send_released/
// send_clicked here. C# can give a control both an event `Clicked` and a method `IButton.Clicked()`
// (via explicit interface impl); C++ cannot have a member and a method of the same name, so the
// inbound methods take the `send_` prefix (matching C#'s IButtonController.SendClicked trigger
// semantics), leaving `clicked`/`pressed`/`released` free for the control's developer-facing events.

#include "maui/core/i_button_stroke.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_button : public i_view, public i_padding, public i_button_stroke
    {
    public:
        // Called by the platform view on touch-down.
        virtual void send_pressed() = 0;
        // Called by the platform view on touch-up (inside or outside) and on cancel.
        virtual void send_released() = 0;
        // Called by the platform view on a completed tap (touch-up inside).
        virtual void send_clicked() = 0;
    };
} // namespace maui::core

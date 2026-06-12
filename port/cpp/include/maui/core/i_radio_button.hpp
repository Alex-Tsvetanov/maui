#pragma once
// maui::core::i_radio_button  <=  Microsoft.Maui.IRadioButton
//
// The virtual-view contract for a mutually-exclusive selection control. Ported from
// src/Core/src/Core/IRadioButton.cs (IRadioButton : IView, ITextStyle, IContentView, IButtonStroke).
//
// C# composes the contract from IContentView, which extends IView. C++ cannot inherit both i_view and
// i_content_view without a diamond over i_view (the port deliberately avoids virtual bases — the
// i_image_button precedent), so this interface derives i_view + the non-view faces (i_text_style /
// i_button_stroke) and replaces the IContentView surface with the STRING content the native fallback
// renders (documented deviation: the port's radio_button is string-content only — C#'s object
// Content/PresentedContent + the ControlTemplate path are deferred to the templates layer; see
// maui::controls::radio_button).
//
// IsChecked is MUTABLE in C# — the setter is the INBOUND channel: a native tap SELECTS the radio
// button and writes back through it (the concrete control stores it at the from-handler specificity,
// like C# RadioButton's `IRadioButton.IsChecked` explicit implementation). Same naming note as
// i_check_box: the inbound setter takes the send_ prefix, leaving set_is_checked free for the
// developer-facing manual setter.

#include <string_view>

#include "maui/core/i_button_stroke.hpp"
#include "maui/core/i_text_style.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_radio_button : public i_view, public i_text_style, public i_button_stroke
    {
    public:
        // Gets whether this radio button is checked; send_is_checked is the inbound native channel
        // (see the naming note above).
        [[nodiscard]] virtual bool is_checked() const = 0;
        virtual void send_is_checked(bool value) = 0;

        // The string the native fallback displays (C# RadioButton.ContentAsString() — the port's
        // string-content cut of IContentView.PresentedContent).
        [[nodiscard]] virtual std::string_view content_as_string() const = 0;
    };
} // namespace maui::core

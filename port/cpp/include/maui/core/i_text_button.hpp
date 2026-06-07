#pragma once
// maui::core::i_text_button  <=  Microsoft.Maui.ITextButton
//
// A button that displays text — the union of i_button and i_text. Ported from
// src/Core/src/Core/ITextButton.cs (ITextButton : IView, IButton, IText). This is the virtual-view
// type the Button control implements and the type the handler's text mapper is keyed on (C#'s
// TextButtonMapper<ITextButton>): keying on i_text_button (rather than i_text) keeps the mapper's
// Virtual an i_element, as the property_mapper requires. No diamond — i_view arrives only via i_button,
// i_text_style only via i_text.

#include "maui/core/i_button.hpp"
#include "maui/core/i_text.hpp"

namespace maui::core
{
    class i_text_button : public i_button, public i_text
    {
    };
} // namespace maui::core

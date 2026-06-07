#pragma once
// maui::core::i_label  <=  Microsoft.Maui.ILabel
//
// The virtual-view contract for a text label. Ported from src/Core/src/Core/ILabel.cs
// (ILabel : IView, IText, ITextAlignment, IPadding). text_decorations()'s return type is qualified
// because the method name would otherwise hide the type (as with i_text_style::font()).

#include "maui/core/i_padding.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/text_decorations.hpp"

namespace maui::core
{
    class i_label : public i_view, public i_text, public i_text_alignment, public i_padding
    {
    public:
        [[nodiscard]] virtual maui::core::text_decorations text_decorations() const = 0;
        [[nodiscard]] virtual double line_height() const = 0;
    };
} // namespace maui::core

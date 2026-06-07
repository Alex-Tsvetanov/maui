#pragma once
// maui::core::i_text_style  <=  Microsoft.Maui.ITextStyle
// Text appearance contract. Ported from src/Core/src/Core/ITextStyle.cs. Returns are by value (the
// interface makes no storage assumption); font() is qualified because the method name would otherwise
// hide the `font` type.

#include "maui/core/font.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_text_style
    {
    public:
        virtual ~i_text_style() = default;

        [[nodiscard]] virtual maui::graphics::color text_color() const = 0;
        [[nodiscard]] virtual maui::core::font font() const = 0;
        [[nodiscard]] virtual double character_spacing() const = 0;

    protected:
        i_text_style() = default;
        i_text_style(const i_text_style&) = default;
        i_text_style(i_text_style&&) = default;
        i_text_style& operator=(const i_text_style&) = default;
        i_text_style& operator=(i_text_style&&) = default;
    };
} // namespace maui::core

#pragma once
// maui::core::i_button_stroke  <=  Microsoft.Maui.IButtonStroke
//
// The border (stroke + corner radius) customization surface for a button. Ported from
// src/Core/src/Core/IButtonStroke.cs.

#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_button_stroke
    {
    public:
        virtual ~i_button_stroke() = default;

        [[nodiscard]] virtual maui::graphics::color stroke_color() const = 0;
        [[nodiscard]] virtual double stroke_thickness() const = 0;
        [[nodiscard]] virtual int corner_radius() const = 0;

    protected:
        i_button_stroke() = default;
        i_button_stroke(const i_button_stroke&) = default;
        i_button_stroke(i_button_stroke&&) = default;
        i_button_stroke& operator=(const i_button_stroke&) = default;
        i_button_stroke& operator=(i_button_stroke&&) = default;
    };
} // namespace maui::core

#pragma once
// maui::graphics::solid_paint  <=  Microsoft.Maui.Graphics.SolidPaint
//
// A paint that fills shapes with a single solid color. Ported from src/Graphics/src/Graphics/SolidPaint.cs:
// it holds one Color; background_color() returns it and is_transparent() is (color.Alpha < 1). The default
// color is the value-type default (transparent black, alpha 0) — C#'s parameterless ctor leaves Color null
// (treated as default by consumers); our color value type defaults to black with alpha 1, so the explicit
// default below mirrors C#'s "no color set" by leaving the member's color() default.
//
// The out-of-line definitions live in solid_paint.cpp (one primary type per header + matching .cpp).

#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::graphics
{
    class solid_paint : public paint
    {
    public:
        solid_paint() = default;
        explicit solid_paint(maui::graphics::color color);

        // C# SolidPaint.Color — the fill color.
        [[nodiscard]] maui::graphics::color color() const;
        void set_color(maui::graphics::color value);

        // ---- paint ----
        // C# Paint.BackgroundColor is the same underlying color for a solid paint (BackgroundColor and the
        // SolidPaint.Color are kept in sync by consumers; here Color is the single source).
        [[nodiscard]] maui::graphics::color background_color() const override;
        // C# SolidPaint.IsTransparent => Color.Alpha < 1.
        [[nodiscard]] bool is_transparent() const override;

    private:
        maui::graphics::color color_;
    };
} // namespace maui::graphics

// maui::graphics::solid_paint — out-of-line definitions. See solid_paint.hpp. Ported from
// src/Graphics/src/Graphics/SolidPaint.cs: the color is the single fill source; IsTransparent is the
// alpha-below-one test.

#include "maui/graphics/solid_paint.hpp"

#include "maui/graphics/color.hpp"

namespace maui::graphics
{
    solid_paint::solid_paint(maui::graphics::color color) : color_(color)
    {
    }

    maui::graphics::color solid_paint::color() const
    {
        return color_;
    }

    void solid_paint::set_color(maui::graphics::color value)
    {
        color_ = value;
    }

    maui::graphics::color solid_paint::background_color() const
    {
        return color_;
    }

    bool solid_paint::is_transparent() const
    {
        // C# SolidPaint.IsTransparent: Color.Alpha < 1.
        return color_.alpha < 1.0F;
    }
} // namespace maui::graphics

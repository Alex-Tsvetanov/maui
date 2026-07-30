#pragma once
// maui::graphics::shapes::rectangle  <=  Microsoft.Maui.Controls.Shapes.Rectangle (as an IShape clip)
//
// A rectangular clip shape. Ported from src/Controls/src/Core/Shapes/Rectangle.cs's GetPath/PathForBounds:
// path_for_bounds builds a rectangle path over the bounds via path_f::append_rectangle.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): the full Shape machinery — the Stretch/Aspect
// fit (TransformPathForBounds, which needs path_f::Transform(Matrix3x2), an omitted M0 feature) and the
// RadiusX/RadiusY rounding — is out of scope here. But the default StrokeThickness IS 1.0, not 0
// (Shape.cs:80-81's StrokeThicknessProperty default), and Rectangle.GetPath insets by half of it
// (Rectangle.cs:64-67: x = StrokeThickness/2, w = width - StrokeThickness) — a fixed 0.5 DIP/side inset,
// independent of any Border/Shape.StrokeThickness the DEVELOPER sets (that's a different property on the
// owning Border/Shape, not this shape's own). With Aspect Fill (Rectangle's ctor) and GetPath's inset box
// exactly filling the deflated view bounds Shape.TransformPathForBounds computes (Shape.cs:320-323), the
// Fill-aspect scale nets to 1.0 and the translate nets to 0 — so the two deflations compose into a single
// net 0.5 DIP/side inset of the bounds rect, not the full 1.0 StrokeThickness. append_rectangle over that
// deflated rect reproduces the composed result directly. Header-only (the body is a single append call).

#include <algorithm>

#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics::shapes
{
    class rectangle : public maui::graphics::i_shape
    {
    public:
        rectangle() = default;

        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override
        {
            // Shape's default StrokeThickness is 1.0 (Shape.cs:80-81); the net effect of GetPath's own
            // inset plus TransformPathForBounds' Fill-aspect fit (see the header note) is a fixed 0.5
            // DIP/side deflate of the bounds rect, thickness-independent because this shape has no mutable
            // StrokeThickness of its own.
            constexpr double k_half_inset = 0.5;
            const double w = std::max(0.0, bounds.width - (2.0 * k_half_inset));
            const double h = std::max(0.0, bounds.height - (2.0 * k_half_inset));
            maui::graphics::path_f path;
            path.append_rectangle(static_cast<float>(bounds.x + k_half_inset),
                                  static_cast<float>(bounds.y + k_half_inset), static_cast<float>(w),
                                  static_cast<float>(h));
            return path;
        }
    };
} // namespace maui::graphics::shapes

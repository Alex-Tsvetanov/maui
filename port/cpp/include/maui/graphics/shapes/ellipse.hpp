#pragma once
// maui::graphics::shapes::ellipse  <=  Microsoft.Maui.Controls.Shapes.Ellipse (as an IShape clip)
//
// An elliptical clip shape. Ported from src/Controls/src/Core/Shapes/Ellipse.cs's GetPath/PathForBounds:
// path_for_bounds builds an ellipse path over the bounds via path_f::append_ellipse.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): the Stretch/Aspect fit (TransformPathForBounds,
// needing the omitted path_f::Transform(Matrix3x2)) is out of scope. But the default StrokeThickness IS
// 1.0, not 0 (Shape.cs:80-81), and Ellipse.GetPath insets by half of it (x = StrokeThickness/2, w = width
// - StrokeThickness — the same pattern as Rectangle.cs:64-67), independent of any Border/Shape.
// StrokeThickness the developer sets. With Aspect Fill (Ellipse's ctor) and GetPath's inset box exactly
// filling the deflated view bounds TransformPathForBounds computes (Shape.cs:320-323), the Fill-aspect
// scale nets to 1.0 and the translate nets to 0 — so the two deflations compose into a single net 0.5 DIP/
// side inset of the bounds rect (see rectangle.hpp's header note for the identical derivation).
// append_ellipse over that deflated rect reproduces the composed result directly. Header-only.

#include <algorithm>

#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics::shapes
{
    class ellipse : public maui::graphics::i_shape
    {
    public:
        ellipse() = default;

        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override
        {
            // Fixed 0.5 DIP/side deflate — see the header note (identical mechanism to rectangle.hpp).
            constexpr double k_half_inset = 0.5;
            const double w = std::max(0.0, bounds.width - (2.0 * k_half_inset));
            const double h = std::max(0.0, bounds.height - (2.0 * k_half_inset));
            maui::graphics::path_f path;
            path.append_ellipse(static_cast<float>(bounds.x + k_half_inset),
                                static_cast<float>(bounds.y + k_half_inset), static_cast<float>(w),
                                static_cast<float>(h));
            return path;
        }
    };
} // namespace maui::graphics::shapes

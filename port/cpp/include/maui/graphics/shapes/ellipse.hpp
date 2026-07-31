#pragma once
// maui::graphics::shapes::ellipse  <=  Microsoft.Maui.Controls.Shapes.Ellipse (as an IShape clip)
//
// An elliptical clip shape. Ported from src/Controls/src/Core/Shapes/Ellipse.cs's GetPath/PathForBounds:
// path_for_bounds builds an ellipse path over the bounds via path_f::append_ellipse.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): the StrokeThickness insets and the
// Stretch/Aspect fit (TransformPathForBounds, needing the omitted path_f::Transform(Matrix3x2)) are out
// of scope; under Aspect Fill the C# result is the ellipse filling the bounds, which we build directly:
// append_ellipse over the bounds rect. Header-only.
//
// MIND THE OMITTED SELF-INSET. Earlier revisions of this note claimed C#'s default StrokeThickness is 0.
// It is 1.0 (Shape.StrokeThicknessProperty, Shape.cs:80-81), and TransformPathForBounds turns that into a
// constant 0.5/side inward inset of the bounds — real and visible wherever MAUI derives a Border's
// geometry from a Shape (Border.StrokeShape is always one). It is deliberately still NOT reinstated here:
// doing so leaked into clip paths, which MAUI never deflates (Geometry.PathForBounds has no such step) —
// the revert is recorded in docs/comparison/PARITY_REVIEW.md. The compensation lives in the platform
// border handlers instead, via maui::core::shape_self_inset (core/border_handler.hpp), which carries the
// derivation and the measured evidence. See also rectangle.hpp / round_rectangle.hpp (same mistake).

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
            maui::graphics::path_f path;
            path.append_ellipse(static_cast<float>(bounds.x), static_cast<float>(bounds.y),
                                static_cast<float>(bounds.width), static_cast<float>(bounds.height));
            return path;
        }
    };
} // namespace maui::graphics::shapes

#pragma once
// maui::graphics::shapes::round_rectangle  <=  Microsoft.Maui.Controls.Shapes.RoundRectangle (IShape clip)
//
// A rounded-rectangle clip shape. Ported from src/Controls/src/Core/Shapes/RoundRectangle.cs's
// GetPath/PathForBounds: path_for_bounds builds a rounded-rectangle path over the bounds via
// path_f::append_rounded_rectangle, using the four per-corner radii (top-left, top-right, bottom-left,
// bottom-right — the order in RoundRectangle.GetPath).
//
// SIMPLIFIED PORT (recorded in port/STATUS.md): the Stretch/Aspect fit (TransformPathForBounds) and the
// StrokeThickness insets / inner-path are out of scope; under Aspect Fill the C# result is the rounded
// rectangle filling the bounds, which we build directly. A uniform single-radius ctor is kept for
// back-compat. The out-of-line body lives in round_rectangle.cpp.
//
// MIND THE OMITTED SELF-INSET. Earlier revisions of this note claimed C#'s default StrokeThickness is 0.
// It is 1.0 (Shape.StrokeThicknessProperty, Shape.cs:80-81), and TransformPathForBounds turns that into a
// constant 0.5/side inward inset of the bounds — real and visible wherever MAUI derives a Border's
// geometry from a Shape (Border.StrokeShape is always one). It is deliberately still NOT reinstated here:
// doing so leaked into clip paths, which MAUI never deflates (Geometry.PathForBounds has no such step) —
// the revert is recorded in docs/comparison/PARITY_REVIEW.md. The compensation lives in the platform
// border handlers instead, via maui::core::shape_self_inset (core/border_handler.hpp), which carries the
// derivation and the measured evidence. See also rectangle.hpp / ellipse.hpp, which shared the mistake.

#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics::shapes
{
    class round_rectangle : public maui::graphics::i_shape
    {
    public:
        round_rectangle() = default;
        // Back-compat: a single uniform radius applied to all four corners.
        explicit round_rectangle(double uniform_radius);
        // The four per-corner radii (C#'s CornerRadius).
        explicit round_rectangle(const maui::graphics::corner_radius& radius);

        [[nodiscard]] const maui::graphics::corner_radius& corner_radius() const;
        void set_corner_radius(const maui::graphics::corner_radius& value);

        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override;

    private:
        maui::graphics::corner_radius corner_radius_;
    };
} // namespace maui::graphics::shapes

#pragma once
// maui::graphics::shapes::rectangle  <=  Microsoft.Maui.Controls.Shapes.Rectangle (as an IShape clip)
//
// A rectangular clip shape. Ported from src/Controls/src/Core/Shapes/Rectangle.cs's GetPath/PathForBounds:
// path_for_bounds builds a rectangle path over the bounds via path_f::append_rectangle.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): the full Shape machinery — StrokeThickness
// insets, the Stretch/Aspect fit (TransformPathForBounds, which needs path_f::Transform(Matrix3x2), an
// omitted M0 feature), and the RadiusX/RadiusY rounding — is out of scope here. Under Aspect Fill C#'s
// result is the unit rectangle scaled to fill the bounds; we build that directly: append_rectangle over
// the bounds rect. Header-only (the body is a single append call).
//
// MIND THE OMITTED SELF-INSET. Earlier revisions of this note claimed C#'s default StrokeThickness is 0.
// It is 1.0 (Shape.StrokeThicknessProperty, Shape.cs:80-81), and TransformPathForBounds turns that into a
// constant 0.5/side inward inset of the bounds. Because Border.StrokeShape DEFAULTS to a
// Microsoft.Maui.Controls.Shapes.Rectangle, that inset is real and visible wherever MAUI derives a
// Border's geometry from a Shape. It is deliberately still NOT reinstated here: doing so leaked into clip
// paths, which MAUI never deflates (Geometry.PathForBounds has no such step) — the revert is recorded in
// docs/comparison/PARITY_REVIEW.md. The compensation lives in the platform border handlers instead, via
// maui::core::shape_self_inset (core/border_handler.hpp), which carries the derivation and the evidence.

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
            maui::graphics::path_f path;
            path.append_rectangle(static_cast<float>(bounds.x), static_cast<float>(bounds.y),
                                  static_cast<float>(bounds.width), static_cast<float>(bounds.height));
            return path;
        }
    };
} // namespace maui::graphics::shapes

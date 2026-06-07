#pragma once
// maui::graphics::shapes::rectangle  <=  Microsoft.Maui.Controls.Shapes.Rectangle (as an IShape clip)
//
// A rectangular clip shape. Ported from src/Controls/src/Core/Shapes/Rectangle.cs's GetPath/PathForBounds:
// path_for_bounds builds a rectangle path over the bounds via path_f::append_rectangle.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): the full Shape machinery — StrokeThickness
// insets, the Stretch/Aspect fit (TransformPathForBounds, which needs path_f::Transform(Matrix3x2), an
// omitted M0 feature), and the RadiusX/RadiusY rounding — is out of scope here. With the default
// StrokeThickness 0 and Aspect Fill, C#'s result is the unit rectangle scaled to fill the bounds; we build
// that directly: append_rectangle over the bounds rect. Header-only (the body is a single append call).

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

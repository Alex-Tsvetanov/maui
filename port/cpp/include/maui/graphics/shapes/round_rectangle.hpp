#pragma once
// maui::graphics::shapes::round_rectangle  <=  Microsoft.Maui.Controls.Shapes.RoundRectangle (IShape clip)
//
// A rounded-rectangle clip shape. Ported from src/Controls/src/Core/Shapes/RoundRectangle.cs's
// GetPath/PathForBounds: path_for_bounds builds a rounded-rectangle path over the bounds via
// path_f::append_rounded_rectangle.
//
// SIMPLIFIED PORT (this unit, recorded in port/STATUS.md): C#'s CornerRadius carries four independent
// corner radii; here a single uniform corner_radius is modeled (the common clip case). The Stretch/Aspect
// fit (TransformPathForBounds, needing the omitted path_f::Transform(Matrix3x2)) and StrokeThickness insets
// are out of scope; with default StrokeThickness 0 + Aspect Fill the C# result is the rounded rectangle
// filling the bounds, which we build directly. The out-of-line body lives in round_rectangle.cpp.

#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics::shapes
{
    class round_rectangle : public maui::graphics::i_shape
    {
    public:
        round_rectangle() = default;
        explicit round_rectangle(double corner_radius);

        // The uniform corner radius applied to all four corners.
        [[nodiscard]] double corner_radius() const;
        void set_corner_radius(double value);

        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override;

    private:
        double corner_radius_ = 0;
    };
} // namespace maui::graphics::shapes

#pragma once
// maui::graphics::shapes::round_rectangle  <=  Microsoft.Maui.Controls.Shapes.RoundRectangle (IShape clip)
//
// A rounded-rectangle clip shape. Ported from src/Controls/src/Core/Shapes/RoundRectangle.cs's
// GetPath/PathForBounds: path_for_bounds builds a rounded-rectangle path over the bounds via
// path_f::append_rounded_rectangle, using the four per-corner radii (top-left, top-right, bottom-left,
// bottom-right — the order in RoundRectangle.GetPath).
//
// SIMPLIFIED PORT (recorded in port/STATUS.md): the Stretch/Aspect fit (TransformPathForBounds) and the
// StrokeThickness insets / inner-path are out of scope; with default StrokeThickness 0 + Aspect Fill the
// C# result is the rounded rectangle filling the bounds, which we build directly. A uniform single-radius
// ctor is kept for back-compat. The out-of-line body lives in round_rectangle.cpp.

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

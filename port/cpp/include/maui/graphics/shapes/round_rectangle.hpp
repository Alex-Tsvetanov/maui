#pragma once
// maui::graphics::shapes::round_rectangle  <=  Microsoft.Maui.Controls.Shapes.RoundRectangle (IShape clip)
//
// A rounded-rectangle clip shape. Ported from src/Controls/src/Core/Shapes/RoundRectangle.cs's
// GetPath/PathForBounds: path_for_bounds builds a rounded-rectangle path over the bounds via
// path_f::append_rounded_rectangle, using the four per-corner radii (top-left, top-right, bottom-left,
// bottom-right — the order in RoundRectangle.GetPath).
//
// SIMPLIFIED PORT (recorded in port/STATUS.md): the full Stretch/Aspect fit (TransformPathForBounds) and
// the InnerPathForBounds inner-path are out of scope. But RoundRectangle.GetPath (RoundRectangle.cs)
// does NOT itself inset by StrokeThickness (unlike Rectangle/Ellipse) — it appends over the FULL (0,0,w,h)
// box. With Aspect Fill (RoundRectangle's ctor) and the default StrokeThickness 1.0 (Shape.cs:80-81),
// TransformPathForBounds' Fill-aspect fit scales that full-box path by (boundsW-1)/boundsW ×
// (boundsH-1)/boundsH (a per-axis factor that -> 1 as the box grows — negligible for any real border,
// affecting only the corner-radius curvature by a sub-pixel amount at typical sizes) and translates it by
// (+0.5,+0.5) — composing to the SAME net 0.5 DIP/side deflate of the outer rect that rectangle.hpp/
// ellipse.hpp derive by the more direct route (GetPath's own inset canceling TransformPathForBounds'
// scale). We build that composed rect directly with the ORIGINAL (unscaled) corner radii — the scale
// factor's effect on radius is below any observable pixel difference. A uniform single-radius ctor is
// kept for back-compat. The out-of-line body lives in round_rectangle.cpp.

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

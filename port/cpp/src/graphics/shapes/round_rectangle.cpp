// maui::graphics::shapes::round_rectangle — out-of-line definitions. See round_rectangle.hpp. Builds the
// rounded-rectangle clip path over the bounds via path_f::append_rounded_rectangle (the four-per-corner
// overload), the simplified port of RoundRectangle.GetPath/PathForBounds (radius order: top-left,
// top-right, bottom-left, bottom-right) composed with TransformPathForBounds' net 0.5 DIP/side deflate
// (see the header note).

#include "maui/graphics/shapes/round_rectangle.hpp"

#include <algorithm>

#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics::shapes
{
    round_rectangle::round_rectangle(double uniform_radius) : corner_radius_(uniform_radius)
    {
    }

    round_rectangle::round_rectangle(const maui::graphics::corner_radius& radius) : corner_radius_(radius)
    {
    }

    const maui::graphics::corner_radius& round_rectangle::corner_radius() const
    {
        return corner_radius_;
    }

    void round_rectangle::set_corner_radius(const maui::graphics::corner_radius& value)
    {
        corner_radius_ = value;
    }

    maui::graphics::path_f round_rectangle::path_for_bounds(const maui::graphics::rect& bounds) const
    {
        // Fixed 0.5 DIP/side deflate — the composed net effect of GetPath (full box, no inset) plus
        // TransformPathForBounds' Fill-aspect scale+translate (see the header note). The corner radii are
        // carried through unscaled (the true scale factor's effect on them is sub-pixel at any real size).
        constexpr double k_half_inset = 0.5;
        const double w = std::max(0.0, bounds.width - (2.0 * k_half_inset));
        const double h = std::max(0.0, bounds.height - (2.0 * k_half_inset));
        maui::graphics::path_f path;
        path.append_rounded_rectangle(
            static_cast<float>(bounds.x + k_half_inset), static_cast<float>(bounds.y + k_half_inset),
            static_cast<float>(w), static_cast<float>(h), static_cast<float>(corner_radius_.top_left),
            static_cast<float>(corner_radius_.top_right), static_cast<float>(corner_radius_.bottom_left),
            static_cast<float>(corner_radius_.bottom_right));
        return path;
    }
} // namespace maui::graphics::shapes

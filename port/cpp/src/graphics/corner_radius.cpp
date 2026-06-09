// maui::graphics::corner_radius  <=  Microsoft.Maui.CornerRadius. Out-of-line definitions; see
// corner_radius.hpp. Faithful port of src/Core/src/Primitives/CornerRadius.cs (ctors + Equals).
#include "maui/graphics/corner_radius.hpp"

namespace maui::graphics
{
    corner_radius::corner_radius(double uniform_radius)
        : corner_radius(uniform_radius, uniform_radius, uniform_radius, uniform_radius)
    {
    }

    corner_radius::corner_radius(double top_left, double top_right, double bottom_left, double bottom_right)
        : top_left(top_left), top_right(top_right), bottom_left(bottom_left), bottom_right(bottom_right),
          is_parameterized_(true)
    {
    }

    bool corner_radius::equals(const corner_radius& other) const
    {
        // CornerRadius.Equals: two default (non-parameterized) instances are equal without comparing
        // fields; otherwise compare all four corners.
        if (!is_parameterized_ && !other.is_parameterized_)
        {
            return true;
        }

        return top_left == other.top_left && top_right == other.top_right && bottom_left == other.bottom_left &&
               bottom_right == other.bottom_right;
    }

    bool operator==(const corner_radius& a, const corner_radius& b)
    {
        return a.equals(b);
    }
    bool operator!=(const corner_radius& a, const corner_radius& b)
    {
        return !a.equals(b);
    }
} // namespace maui::graphics

#pragma once
// maui::controls::shapes::fill_rule  <=  Microsoft.Maui.Controls.Shapes.FillRule
// How the interior of a shape is determined. Ported from FillRule.cs.

#include <cstdint>

namespace maui::controls::shapes
{
    enum class fill_rule : std::uint8_t
    {
        // A point is inside if a ray from it crosses an odd number of path segments.
        even_odd = 0,
        // A point is inside if the winding count (sum of crossing directions) is nonzero.
        nonzero,
    };
} // namespace maui::controls::shapes

#pragma once
// maui::graphics::line_join  <=  Microsoft.Maui.Graphics.LineJoin
// The style used to join two stroked line segments where they meet. Ported from
// src/Graphics/src/Graphics/LineJoin.cs (Miter / Round / Bevel, in that order).

#include <cstdint>

namespace maui::graphics
{
    enum class line_join : std::uint8_t
    {
        miter = 0, // sharp corner (bevel used when the angle is too sharp)
        round = 1, // rounded corner, circle diameter == line width
        bevel = 2  // beveled corner (triangle connecting the outer edges)
    };
} // namespace maui::graphics

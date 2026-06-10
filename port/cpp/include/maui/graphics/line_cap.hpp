#pragma once
// maui::graphics::line_cap  <=  Microsoft.Maui.Graphics.LineCap
// The shape drawn at the start and end of a stroked line. Ported from
// src/Graphics/src/Graphics/LineCap.cs (Butt / Round / Square, in that order).

#include <cstdint>

namespace maui::graphics
{
    enum class line_cap : std::uint8_t
    {
        butt = 0,  // the line ends at the endpoint with no extension
        round = 1, // semicircular cap, diameter == line thickness
        square = 2 // square cap extending half the line thickness past the endpoint
    };
} // namespace maui::graphics

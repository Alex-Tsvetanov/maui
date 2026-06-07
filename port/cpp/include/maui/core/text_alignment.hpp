#pragma once
// maui::core::text_alignment  <=  Microsoft.Maui.TextAlignment
// How text is aligned within its view. Ported from src/Core/src/Primitives/TextAlignment.cs.

#include <cstdint>

namespace maui::core
{
    enum class text_alignment : std::uint8_t
    {
        start = 0,  // left (horizontal) / top (vertical)
        center = 1, // centered
        end = 2,    // right (horizontal) / bottom (vertical)
        justify = 3 // stretched to fill the line
    };
} // namespace maui::core

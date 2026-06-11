#pragma once
// maui::core::scroll_orientation  <=  Microsoft.Maui.ScrollOrientation
// The scrolling directions a scroll view allows. Ported from src/Core/src/Primitives/
// ScrollOrientation.cs (Vertical = 0 / Horizontal / Both / Neither, in that order).

#include <cstdint>

namespace maui::core
{
    enum class scroll_orientation : std::uint8_t
    {
        vertical = 0, // the content scrolls vertically (the default)
        horizontal,   // the content scrolls horizontally
        both,         // the content scrolls both ways
        neither       // the content cannot scroll
    };
} // namespace maui::core

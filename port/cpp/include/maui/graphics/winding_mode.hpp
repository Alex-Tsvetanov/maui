#pragma once
// maui::graphics::winding_mode  <=  Microsoft.Maui.Graphics.WindingMode
// The fill rule deciding which regions are inside a path. Ported from
// src/Graphics/src/Graphics/WindingMode.cs (NonZero = 0, EvenOdd = 1).

#include <cstdint>

namespace maui::graphics
{
    enum class winding_mode : std::uint8_t
    {
        non_zero = 0, // non-zero winding rule
        even_odd = 1  // even-odd (alternating crossings) rule
    };
} // namespace maui::graphics

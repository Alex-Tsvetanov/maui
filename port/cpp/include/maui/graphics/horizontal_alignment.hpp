#pragma once
// maui::graphics::horizontal_alignment  <=  Microsoft.Maui.Graphics.HorizontalAlignment
// Horizontal alignment of text (and elements) on a canvas. Ported from
// src/Graphics/src/Graphics/HorizontalAlignment.cs (Left = 0, Center = 1, Right = 2, Justified = 3).

#include <cstdint>

namespace maui::graphics
{
    enum class horizontal_alignment : std::uint8_t
    {
        left = 0,
        center = 1,
        right = 2,
        justified = 3
    };
} // namespace maui::graphics

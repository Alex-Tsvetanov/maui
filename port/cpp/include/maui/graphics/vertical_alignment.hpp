#pragma once
// maui::graphics::vertical_alignment  <=  Microsoft.Maui.Graphics.VerticalAlignment
// Vertical alignment of text (and elements) on a canvas. Ported from
// src/Graphics/src/Graphics/VerticalAlignment.cs (Top = 0, Center = 1, Bottom = 2).

#include <cstdint>

namespace maui::graphics
{
    enum class vertical_alignment : std::uint8_t
    {
        top = 0,
        center = 1,
        bottom = 2
    };
} // namespace maui::graphics

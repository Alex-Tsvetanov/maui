#pragma once
// maui::graphics::text_flow  <=  Microsoft.Maui.Graphics.TextFlow
// How text behaves when it exceeds its bounding box. Ported from
// src/Graphics/src/Graphics/TextFlow.cs (ClipBounds = 0, OverflowBounds = 1).

#include <cstdint>

namespace maui::graphics
{
    enum class text_flow : std::uint8_t
    {
        clip_bounds = 0,    // overflowing text is clipped
        overflow_bounds = 1 // overflowing text is still rendered
    };
} // namespace maui::graphics

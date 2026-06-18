#pragma once
// maui::controls::stack_orientation  <=  Microsoft.Maui.Controls.StackOrientation
//
// The direction in which a stack_layout positions its children: vertical (top-to-bottom, the default)
// or horizontal (left-to-right). Ported from src/Controls/src/Core/StackOrientation.cs.

#include <cstdint>

namespace maui::controls
{
    enum class stack_orientation : std::uint8_t
    {
        vertical = 0,   // children are arranged vertically (C# StackOrientation.Vertical)
        horizontal = 1, // children are arranged horizontally (C# StackOrientation.Horizontal)
    };
} // namespace maui::controls

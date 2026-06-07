#pragma once
// maui::core::layout_alignment  <=  Microsoft.Maui.Primitives.LayoutAlignment
// A view's arrangement within its container along one axis. Ported from
// src/Core/src/Primitives/LayoutAlignment.cs.

#include <cstdint>

namespace maui::core
{
    enum class layout_alignment : std::uint8_t
    {
        fill = 0,
        start = 1,
        center = 2,
        end = 3,
    };
} // namespace maui::core

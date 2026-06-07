#pragma once
// maui::core::flow_direction  <=  Microsoft.Maui.FlowDirection
// UI scan direction. Ported from src/Core/src/Primitives/FlowDirection.cs.

#include <cstdint>

namespace maui::core
{
    enum class flow_direction : std::uint8_t
    {
        match_parent = 0,
        left_to_right = 1,
        right_to_left = 2,
    };
} // namespace maui::core

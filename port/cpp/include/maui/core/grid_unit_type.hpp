#pragma once
// maui::core::grid_unit_type  <=  Microsoft.Maui.GridUnitType
//
// How a grid_length's value is interpreted. Ported from src/Core/src/Primitives/GridUnitType.cs.
// (C#'s `Auto` is spelled `automatic` here — `auto` is a C++ keyword.)

#include <cstdint>

namespace maui::core
{
    enum class grid_unit_type : std::uint8_t
    {
        absolute = 0, // an exact device-independent size
        star = 1,     // a proportional share of the leftover space
        automatic = 2 // sized to fit the row/column's content
    };
} // namespace maui::core

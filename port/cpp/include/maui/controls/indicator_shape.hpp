#pragma once
// maui::controls::indicator_shape  <=  Microsoft.Maui.Controls.IndicatorShape
// The shape of the dots an indicator_view draws.

#include <cstdint>

namespace maui::controls
{
    enum class indicator_shape : std::uint8_t
    {
        circle = 0,
        square,
    };
} // namespace maui::controls

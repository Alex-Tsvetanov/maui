#pragma once
// maui::controls::shapes::sweep_direction  <=  Microsoft.Maui.Controls.SweepDirection
// The direction an elliptical arc is drawn in. Ported from src/Controls/src/Core/SweepDirection.cs
// (the enum lives in the Controls root namespace in C#; the port keeps it beside its only consumer,
// the arc_segment).

#include <cstdint>

namespace maui::controls::shapes
{
    enum class sweep_direction : std::uint8_t
    {
        counter_clockwise = 0,
        clockwise,
    };
} // namespace maui::controls::shapes

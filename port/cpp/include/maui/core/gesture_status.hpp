#pragma once
// maui::core::gesture_status  <=  Microsoft.Maui.GestureStatus
//
// The lifecycle phase of a continuous gesture (pan / pinch), carried by the recognizers' update event
// args. Ported from src/Core/src/Primitives/GestureStatus.cs (values and order preserved).

#include <cstdint>

namespace maui::core
{
    enum class gesture_status : std::uint8_t
    {
        // The gesture started.
        started = 0,
        // The gesture is still being recognized.
        running = 1,
        // The gesture completed.
        completed = 2,
        // The gesture was canceled.
        canceled = 3,
    };
} // namespace maui::core

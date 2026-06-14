#pragma once
// maui::core::swipe_transition_mode  <=  Microsoft.Maui.SwipeTransitionMode
// The visual transition the SwipeView uses while swiping. Ported from
// src/Core/src/Primitives/SwipeTransitionMode.cs (Reveal = 0 / Drag = 1).

#include <cstdint>

namespace maui::core
{
    enum class swipe_transition_mode : std::uint8_t
    {
        reveal = 0, // the swipe items are revealed as the content slides (the default)
        drag = 1    // the swipe items drag in alongside the content
    };
} // namespace maui::core

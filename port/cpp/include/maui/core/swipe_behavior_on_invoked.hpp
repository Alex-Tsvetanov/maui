#pragma once
// maui::core::swipe_behavior_on_invoked  <=  Microsoft.Maui.SwipeBehaviorOnInvoked
// How a SwipeView behaves after a command is invoked. Ported from
// src/Core/src/Primitives/SwipeBehaviorOnInvoked.cs (Auto = 0 / Close / RemainOpen, in that order).

#include <cstdint>

namespace maui::core
{
    enum class swipe_behavior_on_invoked : std::uint8_t
    {
        // In Reveal mode the SwipeView closes after invocation; in Execute mode it remains open (default).
        automatic = 0,
        close,      // the SwipeView closes after an item is invoked
        remain_open // the SwipeView remains open after an item is invoked
    };
} // namespace maui::core

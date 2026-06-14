#pragma once
// maui::core::swipe_mode  <=  Microsoft.Maui.SwipeMode
// The effect of a swipe interaction. Ported from src/Core/src/Primitives/SwipeMode.cs
// (Reveal = 0 / Execute, in that order).

#include <cstdint>

namespace maui::core
{
    enum class swipe_mode : std::uint8_t
    {
        reveal = 0, // display additional context items which may be selected (the default)
        execute     // immediately execute the associated command upon invocation
    };
} // namespace maui::core

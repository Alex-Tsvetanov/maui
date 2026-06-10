#pragma once
// maui::core::swipe_direction  <=  Microsoft.Maui.SwipeDirection ([Flags])
//
// The direction(s) of a swipe gesture. A [Flags] enum — combinations like left|right are valid.
// Ported from src/Core/src/Primitives/SwipeDirection.cs; the is_* queries port the
// SwipeDirectionExtensions helpers SwipeGestureRecognizer.cs keys its detection on. C# spells the
// "no direction" value `default(SwipeDirection)` (0, no named member); the port names it `none` so
// the default is spellable (the text_decorations precedent). The flag operators combine through
// std::bit_cast: a combination like left|right (3) is a valid value of the fixed-underlying-type
// enum but matches no single enumerator, which an integral cast would trip the analyzer's
// enum-range check on.

#include <bit>
#include <cstdint>

namespace maui::core
{
    enum class swipe_direction : std::uint8_t
    {
        // No direction (C#'s default(SwipeDirection)).
        none = 0,
        // Indicates a rightward swipe.
        right = 1,
        // Indicates a leftward swipe.
        left = 2,
        // Indicates an upward swipe.
        up = 4,
        // Indicates a downward swipe.
        down = 8,
    };

    [[nodiscard]] constexpr swipe_direction operator|(swipe_direction lhs, swipe_direction rhs)
    {
        return std::bit_cast<swipe_direction>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs)));
    }

    [[nodiscard]] constexpr swipe_direction operator&(swipe_direction lhs, swipe_direction rhs)
    {
        return std::bit_cast<swipe_direction>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs)));
    }

    constexpr swipe_direction& operator|=(swipe_direction& lhs, swipe_direction rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    // SwipeDirectionExtensions.IsLeft / IsRight / IsUp / IsDown (SwipeGestureRecognizer.cs): whether
    // the flag combination includes the given direction.
    [[nodiscard]] constexpr bool is_left(swipe_direction self)
    {
        return (self & swipe_direction::left) == swipe_direction::left;
    }
    [[nodiscard]] constexpr bool is_right(swipe_direction self)
    {
        return (self & swipe_direction::right) == swipe_direction::right;
    }
    [[nodiscard]] constexpr bool is_up(swipe_direction self)
    {
        return (self & swipe_direction::up) == swipe_direction::up;
    }
    [[nodiscard]] constexpr bool is_down(swipe_direction self)
    {
        return (self & swipe_direction::down) == swipe_direction::down;
    }
} // namespace maui::core

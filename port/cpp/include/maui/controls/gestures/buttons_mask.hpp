#pragma once
// maui::controls::buttons_mask  <=  Microsoft.Maui.Controls.ButtonsMask ([Flags])
//
// Flag values that represent mouse buttons, used by the tap and pointer gesture recognizers to select
// which button(s) trigger the gesture. Ported from src/Controls/src/Core/Button/ButtonsMask.cs.
// The flag operators combine through std::bit_cast: primary|secondary (3) is a valid value of the
// fixed-underlying-type enum but matches no single enumerator, which an integral cast would trip the
// analyzer's enum-range check on (same rationale as maui::core::swipe_direction).

#include <bit>
#include <cstdint>

namespace maui::controls
{
    enum class buttons_mask : std::uint8_t
    {
        // The primary (left) mouse button.
        primary = 1U << 0U,
        // The secondary (right) mouse button.
        secondary = 1U << 1U,
    };

    [[nodiscard]] constexpr buttons_mask operator|(buttons_mask lhs, buttons_mask rhs)
    {
        return std::bit_cast<buttons_mask>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs)));
    }

    [[nodiscard]] constexpr buttons_mask operator&(buttons_mask lhs, buttons_mask rhs)
    {
        return std::bit_cast<buttons_mask>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs)));
    }

    constexpr buttons_mask& operator|=(buttons_mask& lhs, buttons_mask rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    // Whether `mask` includes every button in `button` — the C# `(mask & button) == button` idiom the
    // gesture bridges filter native events with.
    [[nodiscard]] constexpr bool contains(buttons_mask mask, buttons_mask button)
    {
        return (mask & button) == button;
    }
} // namespace maui::controls

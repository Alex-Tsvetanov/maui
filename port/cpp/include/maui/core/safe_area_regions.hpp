#pragma once
// maui::core::safe_area_regions  <=  Microsoft.Maui.SafeAreaRegions
//
// Which platform safe-area edges a layout / visual element obeys. A [Flags] enum in C# (see
// src/Core/src/Primitives/SafeAreaRegions.cs); the bitwise operators below give the same `&`/`|`
// surface. Recognized on iOS / Mac Catalyst only — other backends ignore it (ISafeAreaView's contract).
//
// The bit values are part of the public contract (PublicAPI.Shipped.txt) and the SafeAreaEdges
// IsSoftInput / IsContainer probes read them, so they must match the oracle EXACTLY: None=0,
// SoftInput=1<<0, Container=1<<1, Default=-1, All=1<<15.
//
// The underlying type is the (signed) int C# uses, because `Default = -1`. The flag operators combine
// through std::bit_cast (the port-wide [Flags] idiom, cf. keyboard_flags / font_attributes): a combined
// value like (soft_input | container) is a valid value of the fixed-underlying-type enum but matches no
// single enumerator, which a plain integral cast would trip the analyzer's enum-range check on.

#include <bit>
#include <cstdint>

namespace maui::core
{
    enum class safe_area_regions : std::int32_t
    {
        // Content goes edge to edge with no safe-area padding.
        none = 0,
        // Always pad so content doesn't go under the soft input / keyboard.
        soft_input = 1 << 0,
        // Content flows under the keyboard but stays out of top/bottom bars and notch.
        container = 1 << 1,
        // Default behavior — apply platform defaults.
        default_value = -1,
        // Obey all safe-area insets — content only in the safe area (bars, notch, keyboard).
        all = 1 << 15,
    };

    [[nodiscard]] constexpr safe_area_regions operator|(safe_area_regions a, safe_area_regions b)
    {
        return std::bit_cast<safe_area_regions>(
            static_cast<std::int32_t>(static_cast<std::int32_t>(a) | static_cast<std::int32_t>(b)));
    }
    [[nodiscard]] constexpr safe_area_regions operator&(safe_area_regions a, safe_area_regions b)
    {
        return std::bit_cast<safe_area_regions>(
            static_cast<std::int32_t>(static_cast<std::int32_t>(a) & static_cast<std::int32_t>(b)));
    }
    constexpr safe_area_regions& operator|=(safe_area_regions& a, safe_area_regions b)
    {
        a = a | b;
        return a;
    }

    // Whether `value` carries every bit of `flag` (the C# `(region & X) == X` test the SafeAreaEdges
    // probes use).
    [[nodiscard]] constexpr bool has_flag(safe_area_regions value, safe_area_regions flag)
    {
        return (value & flag) == flag;
    }
} // namespace maui::core

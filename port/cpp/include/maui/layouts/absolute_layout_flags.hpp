#pragma once
// maui::layouts::absolute_layout_flags  <=  Microsoft.Maui.Layouts.AbsoluteLayoutFlags
//
// A [Flags] enum describing how an AbsoluteLayout interprets a child's LayoutBounds: each of X / Y /
// Width / Height can be a proportional fraction (0..1 of the available space) instead of an absolute
// device value. Ported from src/Core/src/Layouts/AbsoluteLayoutFlags.cs. The composite values mirror
// the C# definitions exactly (position_proportional = x | y, size_proportional = width | height).

#include <cstdint>

namespace maui::layouts
{
    enum class absolute_layout_flags : std::uint32_t
    {
        none = 0,
        x_proportional = 1U << 0U,
        y_proportional = 1U << 1U,
        width_proportional = 1U << 2U,
        height_proportional = 1U << 3U,
        position_proportional = (1U << 0U) | (1U << 1U),
        size_proportional = (1U << 2U) | (1U << 3U),
        all = ~0U,
    };

    [[nodiscard]] constexpr absolute_layout_flags operator|(absolute_layout_flags a, absolute_layout_flags b)
    {
        return static_cast<absolute_layout_flags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    [[nodiscard]] constexpr absolute_layout_flags operator&(absolute_layout_flags a, absolute_layout_flags b)
    {
        return static_cast<absolute_layout_flags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    // C# AbsoluteLayoutManager.HasFlag(a, b): (a & b) == b — does `a` contain every bit of `b`.
    [[nodiscard]] constexpr bool has_flag(absolute_layout_flags value, absolute_layout_flags flag)
    {
        return (value & flag) == flag;
    }
} // namespace maui::layouts

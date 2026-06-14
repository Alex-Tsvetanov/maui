#pragma once
// maui::layouts flex enums  <=  Microsoft.Maui.Layouts.{FlexDirection,FlexJustify,FlexAlignContent,
//   FlexAlignItems,FlexAlignSelf,FlexWrap,FlexPosition}
//
// The public-facing FlexLayout enums. Ported from src/Core/src/Layouts/FlexEnums.cs. Each value's
// numeric is the matching internal Flex algorithm enum value (so a cast to the internal flex::detail
// enums is value-preserving, exactly as the C# `(Flex.X)(FlexX)value` casts rely on). This is a tightly
// coupled family of small enums that share a single conceptual contract (the FlexLayout knob set), so
// they share one header (PROFILE §3 cluster rule) rather than seven near-empty files.

#include <cstdint>

namespace maui::layouts
{
    // FlexJustify — main-axis distribution. Numerics match Flex.Justify.
    enum class flex_justify : std::uint8_t
    {
        start = 3,
        center = 2,
        end = 4,
        space_between = 5,
        space_around = 6,
        space_evenly = 7,
    };

    // FlexPosition — relative (flex rules) vs absolute (fixed) child positioning. Numerics match Flex.Position.
    enum class flex_position : std::uint8_t
    {
        relative = 0,
        absolute = 1,
    };

    // FlexDirection — main axis + stacking order. Numerics match Flex.Direction.
    enum class flex_direction : std::uint8_t
    {
        column = 2,
        column_reverse = 3,
        row = 0,
        row_reverse = 1,
    };

    // FlexAlignContent — cross-axis line distribution (wrap only). Numerics match Flex.AlignContent.
    enum class flex_align_content : std::uint8_t
    {
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
        space_between = 5,
        space_around = 6,
        space_evenly = 7,
    };

    // FlexAlignItems — cross-axis item alignment. Numerics match Flex.AlignItems.
    enum class flex_align_items : std::uint8_t
    {
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
    };

    // FlexAlignSelf — per-child cross-axis alignment override (auto defers to the parent's AlignItems).
    // Numerics match Flex.AlignSelf.
    enum class flex_align_self : std::uint8_t
    {
        auto_ = 0,
        stretch = 1,
        center = 2,
        start = 3,
        end = 4,
    };

    // FlexWrap — single line vs multi-line (with optional reverse). Numerics match Flex.Wrap.
    // C# names the reverse value `Reverse` (the underlying Flex.Wrap is `WrapReverse`).
    enum class flex_wrap : std::uint8_t
    {
        no_wrap = 0,
        wrap = 1,
        reverse = 2,
    };
} // namespace maui::layouts

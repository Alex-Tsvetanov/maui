#pragma once
// maui::core::open_swipe_item  <=  Microsoft.Maui.OpenSwipeItem
// Which side of a SwipeView a programmatic Open targets. Ported from
// src/Core/src/Primitives/OpenSwipeItem.cs (LeftItems = 0 / TopItems / RightItems / BottomItems —
// the C# declaration order is preserved so the underlying values round-trip).

#include <cstdint>

namespace maui::core
{
    enum class open_swipe_item : std::uint8_t
    {
        left_items = 0, // items displayed when the user swipes from the left side
        top_items,      // items displayed when the user swipes from the top side
        right_items,    // items displayed when the user swipes from the right side
        bottom_items    // items displayed when the user swipes from the bottom side
    };
} // namespace maui::core

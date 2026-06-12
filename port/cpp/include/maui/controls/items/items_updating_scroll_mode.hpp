#pragma once
// maui::controls::items_updating_scroll_mode  <=  Microsoft.Maui.Controls.ItemsUpdatingScrollMode
// The scroll behavior when the items collection changes: keep the visible items in view (default),
// keep the absolute offset, or keep the last item in view (chat-style).

#include <cstdint>

namespace maui::controls
{
    enum class items_updating_scroll_mode : std::uint8_t
    {
        keep_items_in_view = 0,
        keep_scroll_offset,
        keep_last_item_in_view,
    };
} // namespace maui::controls

#pragma once
// maui::controls::items_layout_orientation  <=  Microsoft.Maui.Controls.ItemsLayoutOrientation
// The scroll direction of an items_layout.

#include <cstdint>

namespace maui::controls
{
    enum class items_layout_orientation : std::uint8_t
    {
        vertical = 0,
        horizontal,
    };
} // namespace maui::controls

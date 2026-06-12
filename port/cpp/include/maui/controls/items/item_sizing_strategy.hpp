#pragma once
// maui::controls::item_sizing_strategy  <=  Microsoft.Maui.Controls.ItemSizingStrategy
// How items are measured: each individually, or the first item's size reused for all.

#include <cstdint>

namespace maui::controls
{
    enum class item_sizing_strategy : std::uint8_t
    {
        measure_all_items = 0,
        measure_first_item,
    };
} // namespace maui::controls

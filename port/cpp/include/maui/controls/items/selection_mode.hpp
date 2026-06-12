#pragma once
// maui::controls::selection_mode  <=  Microsoft.Maui.Controls.SelectionMode
// The selection mode of a selectable_items_view: disabled, one item at a time, or many.

#include <cstdint>

namespace maui::controls
{
    enum class selection_mode : std::uint8_t
    {
        none = 0,
        single,
        multiple,
    };
} // namespace maui::controls

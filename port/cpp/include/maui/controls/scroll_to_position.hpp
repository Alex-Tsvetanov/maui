#pragma once
// maui::controls::scroll_to_position  <=  Microsoft.Maui.Controls.ScrollToPosition
// Where a scrolled-to item lands in the viewport (the items layer's ScrollTo position argument).

#include <cstdint>

namespace maui::controls
{
    enum class scroll_to_position : std::uint8_t
    {
        make_visible = 0,
        start,
        center,
        end,
    };
} // namespace maui::controls

#pragma once
// maui::controls::snap_points_type  <=  Microsoft.Maui.Controls.SnapPointsType
// The snap behavior when scrolling stops: free scrolling, snap to the nearest item, or advance one
// snap point per gesture (the carousel paging mode).

#include <cstdint>

namespace maui::controls
{
    enum class snap_points_type : std::uint8_t
    {
        none = 0,
        mandatory,
        mandatory_single,
    };
} // namespace maui::controls

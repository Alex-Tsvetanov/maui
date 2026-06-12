#pragma once
// maui::controls::snap_points_alignment  <=  Microsoft.Maui.Controls.SnapPointsAlignment
// Where a snapped item aligns within the viewport (only meaningful with a non-none snap_points_type).

#include <cstdint>

namespace maui::controls
{
    enum class snap_points_alignment : std::uint8_t
    {
        start = 0,
        center,
        end,
    };
} // namespace maui::controls

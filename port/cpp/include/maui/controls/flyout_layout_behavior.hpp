#pragma once
// maui::controls::flyout_layout_behavior  <=  Microsoft.Maui.Controls.FlyoutLayoutBehavior
//
// How a flyout_page lays out its flyout pane relative to the detail (FlyoutLayoutBehavior.cs). The
// `default_` spelling avoids the C++ keyword (the C# member is `Default`).

#include <cstdint>

namespace maui::controls
{
    enum class flyout_layout_behavior : std::uint8_t
    {
        default_ = 0, // platform decides (split on landscape for non-phone idioms)
        split_on_landscape = 1,
        split = 2,
        popover = 3,
        split_on_portrait = 4,
    };
} // namespace maui::controls

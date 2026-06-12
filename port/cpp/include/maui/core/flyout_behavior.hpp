#pragma once
// maui::core::flyout_behavior  <=  Microsoft.Maui.FlyoutBehavior
//
// How a flyout view presents its flyout pane (src/Core/src/Primitives/FlyoutBehavior.cs): as a
// dismissable flyout, disabled entirely, or locked open beside the detail (split mode).

#include <cstdint>

namespace maui::core
{
    enum class flyout_behavior : std::uint8_t
    {
        flyout = 0,   // shown/hidden on demand (the overlay/popover presentation)
        disabled = 1, // the flyout cannot be opened
        locked = 2,   // always visible beside the detail (split presentation)
    };
} // namespace maui::core

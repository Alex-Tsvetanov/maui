#pragma once
// maui::controls::platform_configuration::macos_specific::navigation_transition_style
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.macOSSpecific.NavigationTransitionStyle
// Ported from src/Controls/src/Core/PlatformConfiguration/macOSSpecific/NavigationTransitionStyle.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::macos_specific
{
    enum class navigation_transition_style : std::uint8_t
    {
        none = 0,
        crossfade = 1,
        slide_up = 2,
        slide_down = 3,
        slide_left = 4,
        slide_right = 5,
        slide_forward = 6,
        slide_backward = 7,
    };
} // namespace maui::controls::platform_configuration::macos_specific

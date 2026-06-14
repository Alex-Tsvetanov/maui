#pragma once
// maui::controls::platform_configuration::ios_specific::status_bar_text_color_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.StatusBarTextColorMode
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/StatusBarTextColorMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class status_bar_text_color_mode : std::uint8_t
    {
        match_navigation_bar_text_luminosity = 0,
        do_not_adjust = 1,
    };
} // namespace maui::controls::platform_configuration::ios_specific

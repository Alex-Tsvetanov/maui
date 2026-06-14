#pragma once
// maui::controls::platform_configuration::ios_specific::large_title_display_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.LargeTitleDisplayMode
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/LargeTitleDisplayMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class large_title_display_mode : std::uint8_t
    {
        automatic = 0,
        always = 1,
        never = 2,
    };
} // namespace maui::controls::platform_configuration::ios_specific

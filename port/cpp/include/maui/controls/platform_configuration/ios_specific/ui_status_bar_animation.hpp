#pragma once
// maui::controls::platform_configuration::ios_specific::ui_status_bar_animation
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.UIStatusBarAnimation
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/UIStatusBarAnimation.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class ui_status_bar_animation : std::uint8_t
    {
        none = 0,
        slide = 1,
        fade = 2,
    };
} // namespace maui::controls::platform_configuration::ios_specific

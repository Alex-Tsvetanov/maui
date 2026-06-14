#pragma once
// maui::controls::platform_configuration::ios_specific::blur_effect_style
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.BlurEffectStyle
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/BlurEffectStyle.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class blur_effect_style : std::uint8_t
    {
        none = 0,
        extra_light = 1,
        light = 2,
        dark = 3,
    };
} // namespace maui::controls::platform_configuration::ios_specific

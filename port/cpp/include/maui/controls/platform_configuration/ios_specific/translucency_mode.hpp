#pragma once
// maui::controls::platform_configuration::ios_specific::translucency_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.TranslucencyMode
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/TranslucencyMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class translucency_mode : std::uint8_t
    {
        default_mode = 0, // C# Default (reserved-word stand-in)
        translucent = 1,
        opaque = 2,
    };
} // namespace maui::controls::platform_configuration::ios_specific

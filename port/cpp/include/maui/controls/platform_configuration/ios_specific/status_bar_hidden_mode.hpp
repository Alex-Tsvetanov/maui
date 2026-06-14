#pragma once
// maui::controls::platform_configuration::ios_specific::status_bar_hidden_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.StatusBarHiddenMode
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/StatusBarHiddenMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class status_bar_hidden_mode : std::uint8_t
    {
        default_mode = 0, // C# Default (a reserved word stand-in, like binding_mode::default_mode)
        true_mode = 1,    // C# True
        false_mode = 2,   // C# False
    };
} // namespace maui::controls::platform_configuration::ios_specific

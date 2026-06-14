#pragma once
// maui::controls::platform_configuration::macos_specific::tabs_style
//   <=  Microsoft.Maui.Controls.TabsStyle (declared in macOSSpecific/TabsStyle.cs)
// Ported from src/Controls/src/Core/PlatformConfiguration/macOSSpecific/TabsStyle.cs. (The C# enum
// sits in the Microsoft.Maui.Controls namespace; the port keeps it with its only consumer.)

#include <cstdint>

namespace maui::controls::platform_configuration::macos_specific
{
    enum class tabs_style : std::uint8_t
    {
        default_style = 0, // C# Default (reserved-word stand-in)
        hidden = 1,
        icons = 2,
        on_navigation = 3,
        on_bottom = 4,
    };
} // namespace maui::controls::platform_configuration::macos_specific

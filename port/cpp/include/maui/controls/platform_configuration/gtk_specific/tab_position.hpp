#pragma once
// maui::controls::platform_configuration::gtk_specific::tab_position
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.GTKSpecific.TabPosition
// Ported from src/Controls/src/Core/PlatformConfiguration/GTKSpecific/TabPosition.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::gtk_specific
{
    enum class tab_position : std::uint8_t
    {
        default_position = 0, // C# Default (reserved-word stand-in)
        top = 1,
        bottom = 2,
    };
} // namespace maui::controls::platform_configuration::gtk_specific

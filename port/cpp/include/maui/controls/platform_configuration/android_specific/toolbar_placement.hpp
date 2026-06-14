#pragma once
// maui::controls::platform_configuration::android_specific::toolbar_placement
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.ToolbarPlacement
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/ToolbarPlacement.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::android_specific
{
    enum class toolbar_placement : std::uint8_t
    {
        default_placement = 0, // C# Default (reserved-word stand-in)
        top = 1,
        bottom = 2,
    };
} // namespace maui::controls::platform_configuration::android_specific

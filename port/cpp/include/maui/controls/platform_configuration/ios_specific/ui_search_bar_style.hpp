#pragma once
// maui::controls::platform_configuration::ios_specific::ui_search_bar_style
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.UISearchBarStyle
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/UISearchBarStyle.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class ui_search_bar_style : std::uint8_t
    {
        default_style = 0, // C# Default (reserved-word stand-in)
        prominent = 1,
        minimal = 2,
    };
} // namespace maui::controls::platform_configuration::ios_specific

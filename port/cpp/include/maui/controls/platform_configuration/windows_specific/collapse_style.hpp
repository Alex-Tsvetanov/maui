#pragma once
// maui::controls::platform_configuration::windows_specific::collapse_style
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.CollapseStyle
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/CollapseStyle.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::windows_specific
{
    enum class collapse_style : std::uint8_t
    {
        full = 0,
        partial = 1,
    };
} // namespace maui::controls::platform_configuration::windows_specific

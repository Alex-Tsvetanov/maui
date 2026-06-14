#pragma once
// maui::controls::platform_configuration::tizen_specific::focus_direction
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.FocusDirection
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/FocusDirection.cs — the
// focus-move direction name constants (string-typed in C#, kept string-typed here).

#include <string_view>

namespace maui::controls::platform_configuration::tizen_specific::focus_direction
{
    inline constexpr std::string_view none = "None";
    inline constexpr std::string_view back = "Back";
    inline constexpr std::string_view forward = "Forward";
    inline constexpr std::string_view up = "Up";
    inline constexpr std::string_view down = "Down";
    inline constexpr std::string_view right = "Right";
    inline constexpr std::string_view left = "Left";
} // namespace maui::controls::platform_configuration::tizen_specific::focus_direction

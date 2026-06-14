#pragma once
// maui::controls::platform_configuration::windows_specific::access_key_placement
//   <=  Microsoft.Maui.Controls.AccessKeyPlacement (declared in WindowsSpecific/AccessKeyPlacement.cs)
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/AccessKeyPlacement.cs. (The
// C# enum sits in the Microsoft.Maui.Controls namespace; the port keeps it with its only consumer.)

#include <cstdint>

namespace maui::controls::platform_configuration::windows_specific
{
    enum class access_key_placement : std::uint8_t
    {
        automatic = 0, // C# Auto (reserved-ish stand-in for consistency with the other enums)
        top = 1,
        bottom = 2,
        right = 3,
        left = 4,
        center = 5,
    };
} // namespace maui::controls::platform_configuration::windows_specific

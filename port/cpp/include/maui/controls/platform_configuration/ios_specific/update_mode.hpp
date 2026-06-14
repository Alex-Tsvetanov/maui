#pragma once
// maui::controls::platform_configuration::ios_specific::update_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.UpdateMode
// When a picker-style control's selected value is committed during user interaction. Ported from
// src/Controls/src/Core/PlatformConfiguration/iOSSpecific/UpdateMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class update_mode : std::uint8_t
    {
        immediately = 0,
        when_finished = 1,
    };
} // namespace maui::controls::platform_configuration::ios_specific

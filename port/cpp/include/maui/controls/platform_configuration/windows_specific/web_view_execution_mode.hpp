#pragma once
// maui::controls::platform_configuration::windows_specific::web_view_execution_mode
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.WebViewExecutionMode
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/WebViewExecutionMode.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::windows_specific
{
    enum class web_view_execution_mode : std::uint8_t
    {
        same_thread = 0,
        separate_thread = 1,
        separate_process = 2,
    };
} // namespace maui::controls::platform_configuration::windows_specific

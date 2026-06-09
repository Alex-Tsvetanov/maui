#pragma once
// maui::core::app_theme  <=  Microsoft.Maui.ApplicationModel.AppTheme
//
// The OS / application color theme. Ported verbatim from AppTheme.shared.cs (an Essentials enum the
// Controls Application surfaces via UserAppTheme / PlatformAppTheme / RequestedTheme). Lives in maui::core
// (the lowest layer that names it — Application is in controls, but the enum is a plain value type with no
// dependencies, so it sits in core alongside the other primitives).

#include <cstdint>

namespace maui::core
{
    enum class app_theme : std::uint8_t
    {
        unspecified = 0, // Default / unknown / unspecified theme.
        light,           // Light theme.
        dark,            // Dark theme.
    };
} // namespace maui::core

#pragma once
// maui::controls::platform_configuration::tizen_specific::font_weight
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.FontWeight
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/FontWeight.cs — the Tizen
// font-weight name constants (string-typed in C#, kept string-typed here).

#include <string_view>

namespace maui::controls::platform_configuration::tizen_specific::font_weight
{
    inline constexpr std::string_view none = "None";
    inline constexpr std::string_view normal = "Normal";
    inline constexpr std::string_view thin = "Thin";
    inline constexpr std::string_view ultra_light = "UltraLight";
    inline constexpr std::string_view light = "Light";
    inline constexpr std::string_view book = "Book";
    inline constexpr std::string_view medium = "Medium";
    inline constexpr std::string_view semi_bold = "SemiBold";
    inline constexpr std::string_view bold = "Bold";
    inline constexpr std::string_view ultra_bold = "UltraBold";
    inline constexpr std::string_view black = "Black";
    inline constexpr std::string_view extra_black = "ExtraBlack";
} // namespace maui::controls::platform_configuration::tizen_specific::font_weight

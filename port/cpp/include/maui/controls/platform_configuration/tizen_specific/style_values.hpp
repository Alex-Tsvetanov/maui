#pragma once
// maui::controls::platform_configuration::tizen_specific (style value constants)
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.{ButtonStyle, SwitchStyle,
//       ProgressBarStyle, TabbedPageStyle}
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/StyleValues.cs — the Tizen
// theme-style name constants consumed by visual_element::set_style (string-typed in C#).

#include <string_view>

namespace maui::controls::platform_configuration::tizen_specific
{
    namespace button_style
    {
        inline constexpr std::string_view default_style = "default"; // C# Default
        inline constexpr std::string_view circle = "circle";
        inline constexpr std::string_view bottom = "bottom";
        inline constexpr std::string_view text = "textbutton"; // C# Text
        inline constexpr std::string_view select_mode = "select_mode";
    } // namespace button_style

    namespace switch_style
    {
        inline constexpr std::string_view check_box = "default"; // C# CheckBox
        inline constexpr std::string_view toggle = "toggle";
        inline constexpr std::string_view favorite = "favorite";
        inline constexpr std::string_view on_off = "on&off";     // C# OnOff
        inline constexpr std::string_view small_style = "small"; // C# Small
    } // namespace switch_style

    namespace progress_bar_style
    {
        inline constexpr std::string_view default_style = "default"; // C# Default
        inline constexpr std::string_view pending = "pending";
    } // namespace progress_bar_style

    namespace tabbed_page_style
    {
        inline constexpr std::string_view default_style = "default"; // C# Default
        inline constexpr std::string_view tabbar = "tabbar";
        inline constexpr std::string_view tabbar_with_title = "tabbar_with_title";
    } // namespace tabbed_page_style
} // namespace maui::controls::platform_configuration::tizen_specific

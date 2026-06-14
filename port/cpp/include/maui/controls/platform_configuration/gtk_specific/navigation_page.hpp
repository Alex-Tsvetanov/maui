#pragma once
// maui::controls::platform_configuration::gtk_specific::navigation_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.GTKSpecific.NavigationPage
// Ported from src/Controls/src/Core/PlatformConfiguration/GTKSpecific/NavigationPage.cs.
// STORED-INERT (no GTK backend).

#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/navigation_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::gtk_specific::navigation_page
{
    using forms_element = maui::controls::navigation_page; // C# `using FormsElement = ...NavigationPage`

    inline constexpr std::string_view back_button_icon_key = "gtk.NavigationPage.BackButtonIcon";

    // ---- BackButtonIcon (string, default null → ""; attached) ----
    [[nodiscard]] inline std::string get_back_button_icon(const element& target)
    {
        return target.platform_spec<std::string>(back_button_icon_key, std::string{});
    }
    inline void set_back_button_icon(element& target, std::string back_button_icon)
    {
        target.set_platform_spec(back_button_icon_key, std::move(back_button_icon));
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::string get_back_button_icon(config<gtk, TElement> cfg)
    {
        return get_back_button_icon(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<gtk, TElement> set_back_button_icon(config<gtk, TElement> cfg, std::string back_button_icon)
    {
        set_back_button_icon(cfg.element(), std::move(back_button_icon));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::gtk_specific::navigation_page

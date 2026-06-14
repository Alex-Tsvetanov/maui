#pragma once
// maui::controls::platform_configuration::macos_specific::tabbed_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.macOSSpecific.TabbedPage
// Ported from src/Controls/src/Core/PlatformConfiguration/macOSSpecific/TabbedPage.cs. STORED-INERT
// (see the macos_specific::navigation_page header note).

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/macos_specific/tabs_style.hpp"
#include "maui/controls/tabbed_page.hpp"

namespace maui::controls::platform_configuration::macos_specific::tabbed_page
{
    using forms_element = maui::controls::tabbed_page; // C# `using FormsElement = ...TabbedPage`

    inline constexpr std::string_view tabs_style_key = "macos.TabbedPage.TabsStyle";

    // ---- TabsStyle (TabsStyle, default Default) ----
    [[nodiscard]] inline macos_specific::tabs_style get_tabs_style(const element& target)
    {
        return target.platform_spec<macos_specific::tabs_style>(tabs_style_key,
                                                                macos_specific::tabs_style::default_style);
    }
    inline void set_tabs_style(element& target, macos_specific::tabs_style value)
    {
        target.set_platform_spec(tabs_style_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] macos_specific::tabs_style get_tabs_style(config<macos, TElement> cfg)
    {
        return get_tabs_style(cfg.element());
    }
    // C#'s chaining setter is named SetShowTabs (SetTabsStyle is element-level only).
    template <std::derived_from<forms_element> TElement>
    config<macos, TElement> set_show_tabs(config<macos, TElement> cfg, macos_specific::tabs_style value)
    {
        set_tabs_style(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<macos, TElement> show_tabs_on_navigation(config<macos, TElement> cfg)
    {
        set_tabs_style(cfg.element(), macos_specific::tabs_style::on_navigation);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement> config<macos, TElement> show_tabs(config<macos, TElement> cfg)
    {
        set_tabs_style(cfg.element(), macos_specific::tabs_style::default_style);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement> config<macos, TElement> hide_tabs(config<macos, TElement> cfg)
    {
        set_tabs_style(cfg.element(), macos_specific::tabs_style::hidden);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::macos_specific::tabbed_page

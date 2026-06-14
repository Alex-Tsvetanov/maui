#pragma once
// maui::controls::platform_configuration::gtk_specific::tabbed_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.GTKSpecific.TabbedPage
// Ported from src/Controls/src/Core/PlatformConfiguration/GTKSpecific/TabbedPage.cs.
// STORED-INERT (no GTK backend).

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/gtk_specific/tab_position.hpp"
#include "maui/controls/tabbed_page.hpp"

namespace maui::controls::platform_configuration::gtk_specific::tabbed_page
{
    using forms_element = maui::controls::tabbed_page; // C# `using FormsElement = ...TabbedPage`

    inline constexpr std::string_view tab_position_key = "gtk.TabbedPage.TabPosition";

    // ---- TabPosition (TabPosition, default Default) ----
    [[nodiscard]] inline gtk_specific::tab_position get_tab_position(const element& target)
    {
        return target.platform_spec<gtk_specific::tab_position>(tab_position_key,
                                                                gtk_specific::tab_position::default_position);
    }
    inline void set_tab_position(element& target, gtk_specific::tab_position tab_position)
    {
        target.set_platform_spec(tab_position_key, tab_position);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] gtk_specific::tab_position get_tab_position(config<gtk, TElement> cfg)
    {
        return get_tab_position(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<gtk, TElement> set_tab_position(config<gtk, TElement> cfg, gtk_specific::tab_position tab_position)
    {
        set_tab_position(cfg.element(), tab_position);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::gtk_specific::tabbed_page

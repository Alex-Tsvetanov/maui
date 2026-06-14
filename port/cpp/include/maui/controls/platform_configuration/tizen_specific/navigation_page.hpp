#pragma once
// maui::controls::platform_configuration::tizen_specific::navigation_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.NavigationPage
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/NavigationPage.cs.
// STORED-INERT (no Tizen backend).

#include <concepts>
#include <string_view>

#include "maui/controls/navigation_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::tizen_specific::navigation_page
{
    using forms_element = maui::controls::navigation_page; // C# `using FormsElement = ...NavigationPage`

    inline constexpr std::string_view has_bread_crumbs_bar_key = "tizen.NavigationPage.HasBreadCrumbsBar";

    // ---- HasBreadCrumbsBar (bool, default false; attached) ----
    [[nodiscard]] inline bool get_has_bread_crumbs_bar(const element& target)
    {
        return target.platform_spec<bool>(has_bread_crumbs_bar_key, false);
    }
    inline void set_has_bread_crumbs_bar(element& target, bool value)
    {
        target.set_platform_spec(has_bread_crumbs_bar_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool has_bread_crumbs_bar(config<tizen, TElement> cfg)
    {
        return get_has_bread_crumbs_bar(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_has_bread_crumbs_bar(config<tizen, TElement> cfg, bool value)
    {
        set_has_bread_crumbs_bar(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::navigation_page

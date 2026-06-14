#pragma once
// maui::controls::platform_configuration::tizen_specific::page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.Page
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/Page.cs. STORED-INERT (no
// Tizen backend).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"

namespace maui::controls::platform_configuration::tizen_specific::page
{
    inline constexpr std::string_view bread_crumb_key = "tizen.Page.BreadCrumb";

    // ---- BreadCrumb (string, default null → ""; attached) ----
    [[nodiscard]] inline std::string get_bread_crumb(const element& target)
    {
        return target.platform_spec<std::string>(bread_crumb_key, std::string{});
    }
    inline void set_bread_crumb(element& target, std::string value)
    {
        target.set_platform_spec(bread_crumb_key, std::move(value));
    }
    template <page_element TElement> [[nodiscard]] std::string get_bread_crumb(config<tizen, TElement> cfg)
    {
        return get_bread_crumb(cfg.element());
    }
    template <page_element TElement>
    config<tizen, TElement> set_bread_crumb(config<tizen, TElement> cfg, std::string value)
    {
        set_bread_crumb(cfg.element(), std::move(value));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::page

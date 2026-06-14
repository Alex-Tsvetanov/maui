#pragma once
// maui::controls::platform_configuration::macos_specific::page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.macOSSpecific.Page
// Ported from src/Controls/src/Core/PlatformConfiguration/macOSSpecific/Page.cs. STORED-INERT (see
// the macos_specific::navigation_page header note). The C# VisualElement[] TabOrder becomes a
// std::vector of NON-owning element pointers (the caller owns the referenced views).

#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"

namespace maui::controls::platform_configuration::macos_specific::page
{
    inline constexpr std::string_view tab_order_key = "macos.Page.TabOrder";

    // ---- TabOrder (VisualElement[], default null → empty) ----
    [[nodiscard]] inline std::vector<element*> get_tab_order(const element& target)
    {
        return target.platform_spec<std::vector<element*>>(tab_order_key, {});
    }
    inline void set_tab_order(element& target, std::vector<element*> value)
    {
        target.set_platform_spec(tab_order_key, std::move(value));
    }
    template <page_element TElement> [[nodiscard]] std::vector<element*> get_tab_order(config<macos, TElement> cfg)
    {
        return get_tab_order(cfg.element());
    }
    template <page_element TElement>
    config<macos, TElement> set_tab_order(config<macos, TElement> cfg, std::vector<element*> value)
    {
        set_tab_order(cfg.element(), std::move(value));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::macos_specific::page

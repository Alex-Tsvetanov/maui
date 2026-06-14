#pragma once
// maui::controls::platform_configuration::ios_specific::search_bar
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.SearchBar
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/SearchBar.cs. STORED knob.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_search_bar_style.hpp"
#include "maui/controls/search_bar.hpp"

namespace maui::controls::platform_configuration::ios_specific::search_bar
{
    using forms_element = maui::controls::search_bar; // C# `using FormsElement = ...SearchBar`

    inline constexpr std::string_view search_bar_style_key = "ios.SearchBar.SearchBarStyle";

    // ---- SearchBarStyle (UISearchBarStyle, default Default) ----
    [[nodiscard]] inline ui_search_bar_style get_search_bar_style(const element& target)
    {
        return target.platform_spec<ui_search_bar_style>(search_bar_style_key, ui_search_bar_style::default_style);
    }
    inline void set_search_bar_style(element& target, ui_search_bar_style style)
    {
        target.set_platform_spec(search_bar_style_key, style);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] ui_search_bar_style get_search_bar_style(config<ios, TElement> cfg)
    {
        return get_search_bar_style(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_search_bar_style(config<ios, TElement> cfg, ui_search_bar_style style)
    {
        set_search_bar_style(cfg.element(), style);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::search_bar

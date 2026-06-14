#pragma once
// maui::controls::platform_configuration::ios_specific::tabbed_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.TabbedPage
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/TabbedPage.cs. STORED knob.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/translucency_mode.hpp"
#include "maui/controls/tabbed_page.hpp"

namespace maui::controls::platform_configuration::ios_specific::tabbed_page
{
    using forms_element = maui::controls::tabbed_page; // C# `using FormsElement = ...TabbedPage`

    inline constexpr std::string_view translucency_mode_key = "ios.TabbedPage.TranslucencyMode";

    // ---- TranslucencyMode (TranslucencyMode, default Default) ----
    [[nodiscard]] inline ios_specific::translucency_mode get_translucency_mode(const element& target)
    {
        return target.platform_spec<ios_specific::translucency_mode>(translucency_mode_key,
                                                                     ios_specific::translucency_mode::default_mode);
    }
    inline void set_translucency_mode(element& target, ios_specific::translucency_mode value)
    {
        target.set_platform_spec(translucency_mode_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] ios_specific::translucency_mode get_translucency_mode(config<ios, TElement> cfg)
    {
        return get_translucency_mode(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_translucency_mode(config<ios, TElement> cfg, ios_specific::translucency_mode value)
    {
        set_translucency_mode(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::tabbed_page

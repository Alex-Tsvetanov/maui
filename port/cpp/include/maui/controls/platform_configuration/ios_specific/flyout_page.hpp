#pragma once
// maui::controls::platform_configuration::ios_specific::flyout_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.FlyoutPage
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/FlyoutPage.cs. STORED knob.

#include <concepts>
#include <string_view>

#include "maui/controls/flyout_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::ios_specific::flyout_page
{
    using forms_element = maui::controls::flyout_page; // C# `using FormsElement = ...FlyoutPage`

    inline constexpr std::string_view apply_shadow_key = "ios.FlyoutPage.ApplyShadow";

    // ---- ApplyShadow (bool, default false) ----
    [[nodiscard]] inline bool get_apply_shadow(const element& target)
    {
        return target.platform_spec<bool>(apply_shadow_key, false);
    }
    inline void set_apply_shadow(element& target, bool value)
    {
        target.set_platform_spec(apply_shadow_key, value);
    }
    template <std::derived_from<forms_element> TElement> [[nodiscard]] bool get_apply_shadow(config<ios, TElement> cfg)
    {
        return get_apply_shadow(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_apply_shadow(config<ios, TElement> cfg, bool value)
    {
        set_apply_shadow(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::flyout_page

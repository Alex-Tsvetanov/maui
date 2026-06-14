#pragma once
// maui::controls::platform_configuration::ios_specific::slider
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Slider
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/Slider.cs. STORED knob.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/slider.hpp"

namespace maui::controls::platform_configuration::ios_specific::slider
{
    using forms_element = maui::controls::slider; // C# `using FormsElement = ...Slider`

    inline constexpr std::string_view update_on_tap_key = "ios.Slider.UpdateOnTap";

    // ---- UpdateOnTap (bool, default false) ----
    [[nodiscard]] inline bool get_update_on_tap(const element& target)
    {
        return target.platform_spec<bool>(update_on_tap_key, false);
    }
    inline void set_update_on_tap(element& target, bool value)
    {
        target.set_platform_spec(update_on_tap_key, value);
    }
    template <std::derived_from<forms_element> TElement> [[nodiscard]] bool get_update_on_tap(config<ios, TElement> cfg)
    {
        return get_update_on_tap(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_update_on_tap(config<ios, TElement> cfg, bool value)
    {
        set_update_on_tap(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::slider

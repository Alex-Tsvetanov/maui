#pragma once
// maui::controls::platform_configuration::ios_specific::picker
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Picker
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/Picker.cs. STORED knob (the C#
// consumer is the UIKit picker renderer's done-button commit policy); default Immediately.

#include <concepts>
#include <string_view>

#include "maui/controls/picker.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/update_mode.hpp"

namespace maui::controls::platform_configuration::ios_specific::picker
{
    using forms_element = maui::controls::picker; // C# `using FormsElement = ...Picker`

    inline constexpr std::string_view update_mode_key = "ios.Picker.UpdateMode";

    // ---- UpdateMode (UpdateMode, default Immediately = default(UpdateMode)) ----
    [[nodiscard]] inline ios_specific::update_mode get_update_mode(const element& target)
    {
        return target.platform_spec<ios_specific::update_mode>(update_mode_key, ios_specific::update_mode::immediately);
    }
    inline void set_update_mode(element& target, ios_specific::update_mode value)
    {
        target.set_platform_spec(update_mode_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] ios_specific::update_mode update_mode(config<ios, TElement> cfg)
    {
        return get_update_mode(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_update_mode(config<ios, TElement> cfg, ios_specific::update_mode value)
    {
        set_update_mode(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::picker

#pragma once
// maui::controls::platform_configuration::ios_specific::scroll_view
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.ScrollView
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/ScrollView.cs. STORED knob.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/scroll_view.hpp"

namespace maui::controls::platform_configuration::ios_specific::scroll_view
{
    using forms_element = maui::controls::scroll_view; // C# `using FormsElement = ...ScrollView`

    inline constexpr std::string_view should_delay_content_touches_key = "ios.ScrollView.ShouldDelayContentTouches";

    // ---- ShouldDelayContentTouches (bool, default true) ----
    [[nodiscard]] inline bool get_should_delay_content_touches(const element& target)
    {
        return target.platform_spec<bool>(should_delay_content_touches_key, true);
    }
    inline void set_should_delay_content_touches(element& target, bool value)
    {
        target.set_platform_spec(should_delay_content_touches_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool should_delay_content_touches(config<ios, TElement> cfg)
    {
        return get_should_delay_content_touches(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_should_delay_content_touches(config<ios, TElement> cfg, bool value)
    {
        set_should_delay_content_touches(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::scroll_view

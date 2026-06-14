#pragma once
// maui::controls::platform_configuration::tizen_specific::switch_control
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.Switch
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/Switch.cs. STORED-INERT (no
// Tizen backend). The namespace is switch_control because `switch` is a C++ keyword; the C# Switch
// control is the port's toggle_switch.

#include <concepts>
#include <optional>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls::platform_configuration::tizen_specific::switch_control
{
    using forms_element = maui::controls::toggle_switch; // C# `using FormsElement = ...Switch`

    inline constexpr std::string_view color_key = "tizen.Switch.Color";

    // ---- Color (Color, default null → nullopt) ----
    [[nodiscard]] inline std::optional<maui::graphics::color> get_color(const element& target)
    {
        return target.platform_spec<std::optional<maui::graphics::color>>(color_key, std::nullopt);
    }
    inline void set_color(element& target, std::optional<maui::graphics::color> color)
    {
        target.set_platform_spec(color_key, color);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_color(config<tizen, TElement> cfg)
    {
        return get_color(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_color(config<tizen, TElement> cfg, std::optional<maui::graphics::color> color)
    {
        set_color(cfg.element(), color);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::switch_control

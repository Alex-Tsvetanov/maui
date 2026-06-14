#pragma once
// maui::controls::platform_configuration::tizen_specific::label
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.Label
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/Label.cs. STORED-INERT (no
// Tizen backend). FontWeight is string-typed in C# (the font_weight constants).

#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/tizen_specific/font_weight.hpp"

namespace maui::controls::platform_configuration::tizen_specific::label
{
    using forms_element = maui::controls::label; // C# `using FormsElement = ...Label`

    inline constexpr std::string_view font_weight_key = "tizen.Label.FontWeight";

    // ---- FontWeight (string, default FontWeight.None) ----
    [[nodiscard]] inline std::string get_font_weight(const element& target)
    {
        return target.platform_spec<std::string>(font_weight_key, std::string{font_weight::none});
    }
    inline void set_font_weight(element& target, std::string weight)
    {
        target.set_platform_spec(font_weight_key, std::move(weight));
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::string get_font_weight(config<tizen, TElement> cfg)
    {
        return get_font_weight(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_font_weight(config<tizen, TElement> cfg, std::string weight)
    {
        set_font_weight(cfg.element(), std::move(weight));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::label

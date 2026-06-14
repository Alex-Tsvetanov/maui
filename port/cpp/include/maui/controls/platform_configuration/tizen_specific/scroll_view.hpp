#pragma once
// maui::controls::platform_configuration::tizen_specific::scroll_view
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.ScrollView
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/ScrollView.cs. STORED-INERT
// (no Tizen backend). C# coerces negative scroll steps to -1 (coerceValue) — ported in the setters.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/scroll_view.hpp"

namespace maui::controls::platform_configuration::tizen_specific::scroll_view
{
    using forms_element = maui::controls::scroll_view; // C# `using FormsElement = ...ScrollView`

    inline constexpr std::string_view vertical_scroll_step_key = "tizen.ScrollView.VerticalScrollStep";
    inline constexpr std::string_view horizontal_scroll_step_key = "tizen.ScrollView.HorizontalScrollStep";

    // ---- VerticalScrollStep (int, default -1; negatives coerce to -1) ----
    [[nodiscard]] inline int get_vertical_scroll_step(const element& target)
    {
        return target.platform_spec<int>(vertical_scroll_step_key, -1);
    }
    inline void set_vertical_scroll_step(element& target, int scroll_step)
    {
        target.set_platform_spec(vertical_scroll_step_key, scroll_step < 0 ? -1 : scroll_step);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] int get_vertical_scroll_step(config<tizen, TElement> cfg)
    {
        return get_vertical_scroll_step(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_vertical_scroll_step(config<tizen, TElement> cfg, int scroll_step)
    {
        set_vertical_scroll_step(cfg.element(), scroll_step);
        return cfg;
    }

    // ---- HorizontalScrollStep (int, default -1; negatives coerce to -1) ----
    [[nodiscard]] inline int get_horizontal_scroll_step(const element& target)
    {
        return target.platform_spec<int>(horizontal_scroll_step_key, -1);
    }
    inline void set_horizontal_scroll_step(element& target, int scroll_step)
    {
        target.set_platform_spec(horizontal_scroll_step_key, scroll_step < 0 ? -1 : scroll_step);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] int get_horizontal_scroll_step(config<tizen, TElement> cfg)
    {
        return get_horizontal_scroll_step(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_horizontal_scroll_step(config<tizen, TElement> cfg, int scroll_step)
    {
        set_horizontal_scroll_step(cfg.element(), scroll_step);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::scroll_view

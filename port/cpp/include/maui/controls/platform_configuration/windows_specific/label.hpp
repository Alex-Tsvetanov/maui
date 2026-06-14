#pragma once
// maui::controls::platform_configuration::windows_specific::label
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.Label
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/Label.cs. STORED-INERT
// (no Windows backend). The key differs from the InputView twin (distinct C# declaring types).

#include <concepts>
#include <string_view>

#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::windows_specific::label
{
    using forms_element = maui::controls::label; // C# `using FormsElement = ...Label`

    inline constexpr std::string_view detect_reading_order_from_content_key =
        "windows.Label.DetectReadingOrderFromContent";

    // ---- DetectReadingOrderFromContent (bool, default false) ----
    [[nodiscard]] inline bool get_detect_reading_order_from_content(const element& target)
    {
        return target.platform_spec<bool>(detect_reading_order_from_content_key, false);
    }
    inline void set_detect_reading_order_from_content(element& target, bool value)
    {
        target.set_platform_spec(detect_reading_order_from_content_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_detect_reading_order_from_content(config<windows, TElement> cfg)
    {
        return get_detect_reading_order_from_content(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_detect_reading_order_from_content(config<windows, TElement> cfg, bool value)
    {
        set_detect_reading_order_from_content(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::label

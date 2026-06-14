#pragma once
// maui::controls::platform_configuration::windows_specific::input_view
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.InputView
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/InputView.cs. STORED-INERT
// (no Windows backend). The C# FormsElement is the abstract InputView; the port constrains on the
// declared trio (element_concepts.hpp).

#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"

namespace maui::controls::platform_configuration::windows_specific::input_view
{
    inline constexpr std::string_view detect_reading_order_from_content_key =
        "windows.InputView.DetectReadingOrderFromContent";

    // ---- DetectReadingOrderFromContent (bool, default false) ----
    [[nodiscard]] inline bool get_detect_reading_order_from_content(const element& target)
    {
        return target.platform_spec<bool>(detect_reading_order_from_content_key, false);
    }
    inline void set_detect_reading_order_from_content(element& target, bool value)
    {
        target.set_platform_spec(detect_reading_order_from_content_key, value);
    }
    template <input_view_element TElement>
    [[nodiscard]] bool get_detect_reading_order_from_content(config<windows, TElement> cfg)
    {
        return get_detect_reading_order_from_content(cfg.element());
    }
    template <input_view_element TElement>
    config<windows, TElement> set_detect_reading_order_from_content(config<windows, TElement> cfg, bool value)
    {
        set_detect_reading_order_from_content(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::input_view

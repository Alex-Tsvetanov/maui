#pragma once
// maui::controls::platform_configuration::windows_specific::page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.Page
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/Page.cs. STORED-INERT
// (no Windows backend).

#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"
#include "maui/controls/platform_configuration/windows_specific/toolbar_placement.hpp"

namespace maui::controls::platform_configuration::windows_specific::page
{
    inline constexpr std::string_view toolbar_placement_key = "windows.Page.ToolbarPlacement";
    inline constexpr std::string_view toolbar_dynamic_overflow_enabled_key =
        "windows.Page.ToolbarDynamicOverflowEnabled";

    // ---- ToolbarPlacement (ToolbarPlacement, default Default; attached) ----
    [[nodiscard]] inline windows_specific::toolbar_placement get_toolbar_placement(const element& target)
    {
        return target.platform_spec<windows_specific::toolbar_placement>(
            toolbar_placement_key, windows_specific::toolbar_placement::default_placement);
    }
    inline void set_toolbar_placement(element& target, windows_specific::toolbar_placement toolbar_placement)
    {
        target.set_platform_spec(toolbar_placement_key, toolbar_placement);
    }
    template <page_element TElement>
    [[nodiscard]] windows_specific::toolbar_placement get_toolbar_placement(config<windows, TElement> cfg)
    {
        return get_toolbar_placement(cfg.element());
    }
    template <page_element TElement>
    config<windows, TElement> set_toolbar_placement(config<windows, TElement> cfg,
                                                    windows_specific::toolbar_placement value)
    {
        set_toolbar_placement(cfg.element(), value);
        return cfg;
    }

    // ---- ToolbarDynamicOverflowEnabled (bool, default true; attached) ----
    [[nodiscard]] inline bool get_toolbar_dynamic_overflow_enabled(const element& target)
    {
        return target.platform_spec<bool>(toolbar_dynamic_overflow_enabled_key, true);
    }
    inline void set_toolbar_dynamic_overflow_enabled(element& target, bool value)
    {
        target.set_platform_spec(toolbar_dynamic_overflow_enabled_key, value);
    }
    template <page_element TElement>
    [[nodiscard]] bool get_toolbar_dynamic_overflow_enabled(config<windows, TElement> cfg)
    {
        return get_toolbar_dynamic_overflow_enabled(cfg.element());
    }
    template <page_element TElement>
    config<windows, TElement> set_toolbar_dynamic_overflow_enabled(config<windows, TElement> cfg, bool value)
    {
        set_toolbar_dynamic_overflow_enabled(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::page

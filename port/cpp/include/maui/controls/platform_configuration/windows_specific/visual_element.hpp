#pragma once
// maui::controls::platform_configuration::windows_specific::visual_element
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.VisualElement
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/VisualElement.cs.
// STORED-INERT (no Windows backend). The C# AccessKey string default is null — collapsed to "" (the
// port keeps strings by value; has_platform_spec distinguishes never-set).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"
#include "maui/controls/platform_configuration/windows_specific/access_key_placement.hpp"

namespace maui::controls::platform_configuration::windows_specific::visual_element
{
    inline constexpr std::string_view access_key_key = "windows.VisualElement.AccessKey";
    inline constexpr std::string_view access_key_placement_key = "windows.VisualElement.AccessKeyPlacement";
    inline constexpr std::string_view access_key_horizontal_offset_key =
        "windows.VisualElement.AccessKeyHorizontalOffset";
    inline constexpr std::string_view access_key_vertical_offset_key = "windows.VisualElement.AccessKeyVerticalOffset";
    inline constexpr std::string_view is_legacy_color_mode_enabled_key =
        "windows.VisualElement.IsLegacyColorModeEnabled";

    // ---- AccessKey (string, default null → "") ----
    [[nodiscard]] inline std::string get_access_key(const element& target)
    {
        return target.platform_spec<std::string>(access_key_key, std::string{});
    }
    inline void set_access_key(element& target, std::string value)
    {
        target.set_platform_spec(access_key_key, std::move(value));
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::string get_access_key(config<windows, TElement> cfg)
    {
        return get_access_key(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<windows, TElement> set_access_key(config<windows, TElement> cfg, std::string value)
    {
        set_access_key(cfg.element(), std::move(value));
        return cfg;
    }

    // ---- AccessKeyPlacement (AccessKeyPlacement, default Auto) ----
    [[nodiscard]] inline windows_specific::access_key_placement get_access_key_placement(const element& target)
    {
        return target.platform_spec<windows_specific::access_key_placement>(
            access_key_placement_key, windows_specific::access_key_placement::automatic);
    }
    inline void set_access_key_placement(element& target, windows_specific::access_key_placement value)
    {
        target.set_platform_spec(access_key_placement_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] windows_specific::access_key_placement get_access_key_placement(config<windows, TElement> cfg)
    {
        return get_access_key_placement(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<windows, TElement> set_access_key_placement(config<windows, TElement> cfg,
                                                       windows_specific::access_key_placement value)
    {
        set_access_key_placement(cfg.element(), value);
        return cfg;
    }

    // ---- AccessKeyHorizontalOffset (double, default 0.0) ----
    [[nodiscard]] inline double get_access_key_horizontal_offset(const element& target)
    {
        return target.platform_spec<double>(access_key_horizontal_offset_key, 0.0);
    }
    inline void set_access_key_horizontal_offset(element& target, double value)
    {
        target.set_platform_spec(access_key_horizontal_offset_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] double get_access_key_horizontal_offset(config<windows, TElement> cfg)
    {
        return get_access_key_horizontal_offset(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<windows, TElement> set_access_key_horizontal_offset(config<windows, TElement> cfg, double value)
    {
        set_access_key_horizontal_offset(cfg.element(), value);
        return cfg;
    }

    // ---- AccessKeyVerticalOffset (double, default 0.0) ----
    [[nodiscard]] inline double get_access_key_vertical_offset(const element& target)
    {
        return target.platform_spec<double>(access_key_vertical_offset_key, 0.0);
    }
    inline void set_access_key_vertical_offset(element& target, double value)
    {
        target.set_platform_spec(access_key_vertical_offset_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] double get_access_key_vertical_offset(config<windows, TElement> cfg)
    {
        return get_access_key_vertical_offset(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<windows, TElement> set_access_key_vertical_offset(config<windows, TElement> cfg, double value)
    {
        set_access_key_vertical_offset(cfg.element(), value);
        return cfg;
    }

    // ---- IsLegacyColorModeEnabled (bool, default true; attached) ----
    [[nodiscard]] inline bool get_is_legacy_color_mode_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_legacy_color_mode_enabled_key, true);
    }
    inline void set_is_legacy_color_mode_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_legacy_color_mode_enabled_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] bool get_is_legacy_color_mode_enabled(config<windows, TElement> cfg)
    {
        return get_is_legacy_color_mode_enabled(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<windows, TElement> set_is_legacy_color_mode_enabled(config<windows, TElement> cfg, bool value)
    {
        set_is_legacy_color_mode_enabled(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::visual_element

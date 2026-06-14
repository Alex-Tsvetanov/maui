#pragma once
// maui::controls::platform_configuration::android_specific::visual_element
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.VisualElement
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/VisualElement.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <optional>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"

namespace maui::controls::platform_configuration::android_specific::visual_element
{
    inline constexpr std::string_view elevation_key = "android.VisualElement.Elevation";
    inline constexpr std::string_view is_legacy_color_mode_enabled_key =
        "android.VisualElement.IsLegacyColorModeEnabled";

    // ---- Elevation (float?, no default → nullopt) ----
    [[nodiscard]] inline std::optional<float> get_elevation(const element& target)
    {
        return target.platform_spec<std::optional<float>>(elevation_key, std::nullopt);
    }
    inline void set_elevation(element& target, std::optional<float> value)
    {
        target.set_platform_spec(elevation_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::optional<float> get_elevation(config<android, TElement> cfg)
    {
        return get_elevation(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<android, TElement> set_elevation(config<android, TElement> cfg, std::optional<float> value)
    {
        set_elevation(cfg.element(), value);
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
    [[nodiscard]] bool get_is_legacy_color_mode_enabled(config<android, TElement> cfg)
    {
        return get_is_legacy_color_mode_enabled(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<android, TElement> set_is_legacy_color_mode_enabled(config<android, TElement> cfg, bool value)
    {
        set_is_legacy_color_mode_enabled(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::android_specific::visual_element

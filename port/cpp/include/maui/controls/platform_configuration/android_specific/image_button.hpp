#pragma once
// maui::controls::platform_configuration::android_specific::image_button
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.ImageButton
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/ImageButton.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).
// NOTE: C# declares ShadowOffsetProperty with typeof(VisualElement) as the declaring type (an oracle
// quirk); the key follows the knob class like the rest of the set.

#include <concepts>
#include <optional>
#include <string_view>

#include "maui/controls/image_button.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::platform_configuration::android_specific::image_button
{
    using forms_element = maui::controls::image_button; // C# `using FormsImageButton = ...ImageButton`

    inline constexpr std::string_view is_shadow_enabled_key = "android.ImageButton.IsShadowEnabled";
    inline constexpr std::string_view shadow_color_key = "android.ImageButton.ShadowColor";
    inline constexpr std::string_view shadow_radius_key = "android.ImageButton.ShadowRadius";
    inline constexpr std::string_view shadow_offset_key = "android.ImageButton.ShadowOffset";
    inline constexpr std::string_view ripple_color_key = "android.ImageButton.RippleColor";

    // ---- IsShadowEnabled (bool, default false) ----
    [[nodiscard]] inline bool get_is_shadow_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_shadow_enabled_key, false);
    }
    inline void set_is_shadow_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_shadow_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_is_shadow_enabled(config<android, TElement> cfg)
    {
        return get_is_shadow_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_is_shadow_enabled(config<android, TElement> cfg, bool value)
    {
        set_is_shadow_enabled(cfg.element(), value);
        return cfg;
    }

    // ---- ShadowColor (Color, default null → nullopt) ----
    [[nodiscard]] inline std::optional<maui::graphics::color> get_shadow_color(const element& target)
    {
        return target.platform_spec<std::optional<maui::graphics::color>>(shadow_color_key, std::nullopt);
    }
    inline void set_shadow_color(element& target, std::optional<maui::graphics::color> value)
    {
        target.set_platform_spec(shadow_color_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_shadow_color(config<android, TElement> cfg)
    {
        return get_shadow_color(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_shadow_color(config<android, TElement> cfg,
                                               std::optional<maui::graphics::color> value)
    {
        set_shadow_color(cfg.element(), value);
        return cfg;
    }

    // ---- ShadowRadius (double, default 10.0) ----
    [[nodiscard]] inline double get_shadow_radius(const element& target)
    {
        return target.platform_spec<double>(shadow_radius_key, 10.0);
    }
    inline void set_shadow_radius(element& target, double value)
    {
        target.set_platform_spec(shadow_radius_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] double get_shadow_radius(config<android, TElement> cfg)
    {
        return get_shadow_radius(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_shadow_radius(config<android, TElement> cfg, double value)
    {
        set_shadow_radius(cfg.element(), value);
        return cfg;
    }

    // ---- ShadowOffset (Size, default Size.Zero) ----
    [[nodiscard]] inline maui::graphics::size get_shadow_offset(const element& target)
    {
        return target.platform_spec<maui::graphics::size>(shadow_offset_key, maui::graphics::size{});
    }
    inline void set_shadow_offset(element& target, maui::graphics::size value)
    {
        target.set_platform_spec(shadow_offset_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] maui::graphics::size get_shadow_offset(config<android, TElement> cfg)
    {
        return get_shadow_offset(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_shadow_offset(config<android, TElement> cfg, maui::graphics::size value)
    {
        set_shadow_offset(cfg.element(), value);
        return cfg;
    }

    // ---- RippleColor (Color, default null → nullopt) ----
    [[nodiscard]] inline std::optional<maui::graphics::color> get_ripple_color(const element& target)
    {
        return target.platform_spec<std::optional<maui::graphics::color>>(ripple_color_key, std::nullopt);
    }
    inline void set_ripple_color(element& target, std::optional<maui::graphics::color> value)
    {
        target.set_platform_spec(ripple_color_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_ripple_color(config<android, TElement> cfg)
    {
        return get_ripple_color(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_ripple_color(config<android, TElement> cfg,
                                               std::optional<maui::graphics::color> value)
    {
        set_ripple_color(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::android_specific::image_button

#pragma once
// maui::controls::platform_configuration::ios_specific::visual_element
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.VisualElement
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/VisualElement.cs. STORED knobs.
// DEVIATION (documented): C#'s IsShadowEnabled propertyChanged attaches/detaches a ShadowEffect
// (RoutingEffect) — the Effects subsystem is not ported (STATUS deferred backlog), so the knob stores
// without the effect side-channel.

#include <concepts>
#include <optional>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"
#include "maui/controls/platform_configuration/ios_specific/blur_effect_style.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::platform_configuration::ios_specific::visual_element
{
    inline constexpr std::string_view blur_effect_key = "ios.VisualElement.BlurEffect";
    inline constexpr std::string_view is_shadow_enabled_key = "ios.VisualElement.IsShadowEnabled";
    inline constexpr std::string_view shadow_color_key = "ios.VisualElement.ShadowColor";
    inline constexpr std::string_view shadow_radius_key = "ios.VisualElement.ShadowRadius";
    inline constexpr std::string_view shadow_offset_key = "ios.VisualElement.ShadowOffset";
    inline constexpr std::string_view shadow_opacity_key = "ios.VisualElement.ShadowOpacity";
    inline constexpr std::string_view is_legacy_color_mode_enabled_key = "ios.VisualElement.IsLegacyColorModeEnabled";
    inline constexpr std::string_view can_become_first_responder_key = "ios.VisualElement.CanBecomeFirstResponder";

    // ---- BlurEffect (BlurEffectStyle, default None) ----
    [[nodiscard]] inline blur_effect_style get_blur_effect(const element& target)
    {
        return target.platform_spec<blur_effect_style>(blur_effect_key, blur_effect_style::none);
    }
    inline void set_blur_effect(element& target, blur_effect_style value)
    {
        target.set_platform_spec(blur_effect_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] blur_effect_style get_blur_effect(config<ios, TElement> cfg)
    {
        return get_blur_effect(cfg.element());
    }
    // C#'s chaining setter is named UseBlurEffect.
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> use_blur_effect(config<ios, TElement> cfg, blur_effect_style value)
    {
        set_blur_effect(cfg.element(), value);
        return cfg;
    }

    // ---- IsShadowEnabled (bool, default false; see the header DEVIATION note) ----
    [[nodiscard]] inline bool get_is_shadow_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_shadow_enabled_key, false);
    }
    inline void set_is_shadow_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_shadow_enabled_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] bool get_is_shadow_enabled(config<ios, TElement> cfg)
    {
        return get_is_shadow_enabled(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_is_shadow_enabled(config<ios, TElement> cfg, bool value)
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
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_shadow_color(config<ios, TElement> cfg)
    {
        return get_shadow_color(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_shadow_color(config<ios, TElement> cfg, std::optional<maui::graphics::color> value)
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
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] double get_shadow_radius(config<ios, TElement> cfg)
    {
        return get_shadow_radius(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_shadow_radius(config<ios, TElement> cfg, double value)
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
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] maui::graphics::size get_shadow_offset(config<ios, TElement> cfg)
    {
        return get_shadow_offset(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_shadow_offset(config<ios, TElement> cfg, maui::graphics::size value)
    {
        set_shadow_offset(cfg.element(), value);
        return cfg;
    }

    // ---- ShadowOpacity (double, default 0.5) ----
    [[nodiscard]] inline double get_shadow_opacity(const element& target)
    {
        return target.platform_spec<double>(shadow_opacity_key, 0.5);
    }
    inline void set_shadow_opacity(element& target, double value)
    {
        target.set_platform_spec(shadow_opacity_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] double get_shadow_opacity(config<ios, TElement> cfg)
    {
        return get_shadow_opacity(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_shadow_opacity(config<ios, TElement> cfg, double value)
    {
        set_shadow_opacity(cfg.element(), value);
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
    [[nodiscard]] bool get_is_legacy_color_mode_enabled(config<ios, TElement> cfg)
    {
        return get_is_legacy_color_mode_enabled(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_is_legacy_color_mode_enabled(config<ios, TElement> cfg, bool value)
    {
        set_is_legacy_color_mode_enabled(cfg.element(), value);
        return cfg;
    }

    // ---- CanBecomeFirstResponder (bool, default false) ----
    [[nodiscard]] inline bool get_can_become_first_responder(const element& target)
    {
        return target.platform_spec<bool>(can_become_first_responder_key, false);
    }
    inline void set_can_become_first_responder(element& target, bool value)
    {
        target.set_platform_spec(can_become_first_responder_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] bool can_become_first_responder(config<ios, TElement> cfg)
    {
        return get_can_become_first_responder(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<ios, TElement> set_can_become_first_responder(config<ios, TElement> cfg, bool value)
    {
        set_can_become_first_responder(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::visual_element

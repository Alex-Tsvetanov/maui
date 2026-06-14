#pragma once
// maui::controls::platform_configuration::android_specific::button
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.Button
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/Button.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <optional>
#include <string_view>

#include "maui/controls/button.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls::platform_configuration::android_specific::button
{
    using forms_element = maui::controls::button; // C# `using FormsElement = ...Button` (implicit)

    inline constexpr std::string_view use_default_padding_key = "android.Button.UseDefaultPadding";
    inline constexpr std::string_view use_default_shadow_key = "android.Button.UseDefaultShadow";
    inline constexpr std::string_view ripple_color_key = "android.Button.RippleColor";

    // ---- UseDefaultPadding (bool, default false) ----
    [[nodiscard]] inline bool get_use_default_padding(const element& target)
    {
        return target.platform_spec<bool>(use_default_padding_key, false);
    }
    inline void set_use_default_padding(element& target, bool value)
    {
        target.set_platform_spec(use_default_padding_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool use_default_padding(config<android, TElement> cfg)
    {
        return get_use_default_padding(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_use_default_padding(config<android, TElement> cfg, bool value)
    {
        set_use_default_padding(cfg.element(), value);
        return cfg;
    }

    // ---- UseDefaultShadow (bool, default false) ----
    [[nodiscard]] inline bool get_use_default_shadow(const element& target)
    {
        return target.platform_spec<bool>(use_default_shadow_key, false);
    }
    inline void set_use_default_shadow(element& target, bool value)
    {
        target.set_platform_spec(use_default_shadow_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool use_default_shadow(config<android, TElement> cfg)
    {
        return get_use_default_shadow(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_use_default_shadow(config<android, TElement> cfg, bool value)
    {
        set_use_default_shadow(cfg.element(), value);
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
} // namespace maui::controls::platform_configuration::android_specific::button

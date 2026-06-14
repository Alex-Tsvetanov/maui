#pragma once
// maui::controls::platform_configuration::ios_specific::entry
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Entry
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/Entry.cs.
// WIRED-REAL (W2-24, iOS backend): cursor_color drives the real UITextField.tintColor — the controls
// layer appends the "ios.Entry.CursorColor" mapping to entry_handler::mapper() (entry.cpp), the
// TextExtensions.UpdateCursorColor port (only acts when the knob IsSet and the color is non-null).
// adjusts_font_size_to_fit_width is also pushed (UITextField.adjustsFontSizeToFitWidth).

#include <concepts>
#include <optional>
#include <string_view>

#include "maui/controls/entry.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls::platform_configuration::ios_specific::entry
{
    using forms_element = maui::controls::entry; // C# `using FormsElement = ...Entry`

    inline constexpr std::string_view adjusts_font_size_to_fit_width_key = "ios.Entry.AdjustsFontSizeToFitWidth";
    inline constexpr std::string_view cursor_color_key = "ios.Entry.CursorColor";

    // ---- AdjustsFontSizeToFitWidth (bool, default false) ----
    [[nodiscard]] inline bool get_adjusts_font_size_to_fit_width(const element& target)
    {
        return target.platform_spec<bool>(adjusts_font_size_to_fit_width_key, false);
    }
    inline void set_adjusts_font_size_to_fit_width(element& target, bool value)
    {
        target.set_platform_spec(adjusts_font_size_to_fit_width_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool adjusts_font_size_to_fit_width(config<ios, TElement> cfg)
    {
        return get_adjusts_font_size_to_fit_width(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_adjusts_font_size_to_fit_width(config<ios, TElement> cfg, bool value)
    {
        set_adjusts_font_size_to_fit_width(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> enable_adjusts_font_size_to_fit_width(config<ios, TElement> cfg)
    {
        set_adjusts_font_size_to_fit_width(cfg.element(), true);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> disable_adjusts_font_size_to_fit_width(config<ios, TElement> cfg)
    {
        set_adjusts_font_size_to_fit_width(cfg.element(), false);
        return cfg;
    }

    // ---- CursorColor (Color, default null → nullopt) ----
    [[nodiscard]] inline std::optional<maui::graphics::color> get_cursor_color(const element& target)
    {
        return target.platform_spec<std::optional<maui::graphics::color>>(cursor_color_key, std::nullopt);
    }
    inline void set_cursor_color(element& target, std::optional<maui::graphics::color> value)
    {
        target.set_platform_spec(cursor_color_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_cursor_color(config<ios, TElement> cfg)
    {
        return get_cursor_color(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_cursor_color(config<ios, TElement> cfg, std::optional<maui::graphics::color> value)
    {
        set_cursor_color(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::entry

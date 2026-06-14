#pragma once
// maui::controls::platform_configuration::ios_specific::navigation_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.NavigationPage
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/NavigationPage.cs.
// WIRED-REAL (W2-24, iOS backend): is_navigation_bar_translucent drives the port's custom navigation
// bar — the controls layer appends the "ios.NavigationPage.IsNavigationBarTranslucent" mapping to
// navigation_page_handler::mapper() (navigation_page.cpp); the iOS partial gives the bar a
// UIVisualEffectView backdrop and lets the content extend under it (the NavigationRenderer
// UpdateTranslucent analog over a custom bar). The other three knobs are STORED.
// NOTE: StatusBarTextColorMode's C# property-name string is "StatusBarColorTextMode" (a C# typo kept
// on the descriptor); the port key keeps the API name — nothing keys off the C# string.

#include <concepts>
#include <string_view>

#include "maui/controls/navigation_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/status_bar_text_color_mode.hpp"

namespace maui::controls::platform_configuration::ios_specific::navigation_page
{
    using forms_element = maui::controls::navigation_page; // C# `using FormsElement = ...NavigationPage`

    inline constexpr std::string_view is_navigation_bar_translucent_key =
        "ios.NavigationPage.IsNavigationBarTranslucent";
    inline constexpr std::string_view status_bar_text_color_mode_key = "ios.NavigationPage.StatusBarTextColorMode";
    // The next two C# properties are declared with typeof(Page) as the declaring type but live in (and
    // are consumed through) the NavigationPage knob class — the keys follow the knob class.
    inline constexpr std::string_view prefers_large_titles_key = "ios.NavigationPage.PrefersLargeTitles";
    inline constexpr std::string_view hide_navigation_bar_separator_key =
        "ios.NavigationPage.HideNavigationBarSeparator";

    // ---- IsNavigationBarTranslucent (bool, default false) ----
    [[nodiscard]] inline bool get_is_navigation_bar_translucent(const element& target)
    {
        return target.platform_spec<bool>(is_navigation_bar_translucent_key, false);
    }
    inline void set_is_navigation_bar_translucent(element& target, bool value)
    {
        target.set_platform_spec(is_navigation_bar_translucent_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_navigation_bar_translucent(config<ios, TElement> cfg)
    {
        return get_is_navigation_bar_translucent(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_is_navigation_bar_translucent(config<ios, TElement> cfg, bool value)
    {
        set_is_navigation_bar_translucent(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> enable_translucent_navigation_bar(config<ios, TElement> cfg)
    {
        set_is_navigation_bar_translucent(cfg.element(), true);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> disable_translucent_navigation_bar(config<ios, TElement> cfg)
    {
        set_is_navigation_bar_translucent(cfg.element(), false);
        return cfg;
    }

    // ---- StatusBarTextColorMode (StatusBarTextColorMode, default MatchNavigationBarTextLuminosity) ----
    [[nodiscard]] inline ios_specific::status_bar_text_color_mode get_status_bar_text_color_mode(const element& target)
    {
        return target.platform_spec<ios_specific::status_bar_text_color_mode>(
            status_bar_text_color_mode_key,
            ios_specific::status_bar_text_color_mode::match_navigation_bar_text_luminosity);
    }
    inline void set_status_bar_text_color_mode(element& target, ios_specific::status_bar_text_color_mode value)
    {
        target.set_platform_spec(status_bar_text_color_mode_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] ios_specific::status_bar_text_color_mode get_status_bar_text_color_mode(config<ios, TElement> cfg)
    {
        return get_status_bar_text_color_mode(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_status_bar_text_color_mode(config<ios, TElement> cfg,
                                                         ios_specific::status_bar_text_color_mode value)
    {
        set_status_bar_text_color_mode(cfg.element(), value);
        return cfg;
    }

    // ---- PrefersLargeTitles (bool, default false) ----
    [[nodiscard]] inline bool get_prefers_large_titles(const element& target)
    {
        return target.platform_spec<bool>(prefers_large_titles_key, false);
    }
    inline void set_prefers_large_titles(element& target, bool value)
    {
        target.set_platform_spec(prefers_large_titles_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool prefers_large_titles(config<ios, TElement> cfg)
    {
        return get_prefers_large_titles(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_prefers_large_titles(config<ios, TElement> cfg, bool value)
    {
        set_prefers_large_titles(cfg.element(), value);
        return cfg;
    }

    // ---- HideNavigationBarSeparator (bool, default false) ----
    [[nodiscard]] inline bool get_hide_navigation_bar_separator(const element& target)
    {
        return target.platform_spec<bool>(hide_navigation_bar_separator_key, false);
    }
    inline void set_hide_navigation_bar_separator(element& target, bool value)
    {
        target.set_platform_spec(hide_navigation_bar_separator_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool hide_navigation_bar_separator(config<ios, TElement> cfg)
    {
        return get_hide_navigation_bar_separator(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_hide_navigation_bar_separator(config<ios, TElement> cfg, bool value)
    {
        set_hide_navigation_bar_separator(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::navigation_page

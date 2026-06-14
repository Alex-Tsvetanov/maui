#pragma once
// maui::controls::platform_configuration::macos_specific::navigation_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.macOSSpecific.NavigationPage
// Ported from src/Controls/src/Core/PlatformConfiguration/macOSSpecific/NavigationPage.cs.
// STORED-INERT: MAUI itself ships no macOS/AppKit consumer for the macOSSpecific knob sets (they are
// Xamarin.Forms.macOS legacy surface), so the port stores them faithfully and wires nothing — the
// AppKit window_handler hosts windows, not navigation transitions (STATUS.md W2-24).

#include <concepts>
#include <string_view>

#include "maui/controls/navigation_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/macos_specific/navigation_transition_style.hpp"

namespace maui::controls::platform_configuration::macos_specific::navigation_page
{
    using forms_element = maui::controls::navigation_page; // C# `using FormsElement = ...NavigationPage`

    inline constexpr std::string_view navigation_transition_push_style_key =
        "macos.NavigationPage.NavigationTransitionPushStyle";
    inline constexpr std::string_view navigation_transition_pop_style_key =
        "macos.NavigationPage.NavigationTransitionPopStyle";

    // ---- NavigationTransitionPushStyle (NavigationTransitionStyle, default SlideForward) ----
    [[nodiscard]] inline navigation_transition_style get_navigation_transition_push_style(const element& target)
    {
        return target.platform_spec<navigation_transition_style>(navigation_transition_push_style_key,
                                                                 navigation_transition_style::slide_forward);
    }
    inline void set_navigation_transition_push_style(element& target, navigation_transition_style value)
    {
        target.set_platform_spec(navigation_transition_push_style_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] navigation_transition_style get_navigation_transition_push_style(config<macos, TElement> cfg)
    {
        return get_navigation_transition_push_style(cfg.element());
    }

    // ---- NavigationTransitionPopStyle (NavigationTransitionStyle, default SlideBackward) ----
    [[nodiscard]] inline navigation_transition_style get_navigation_transition_pop_style(const element& target)
    {
        return target.platform_spec<navigation_transition_style>(navigation_transition_pop_style_key,
                                                                 navigation_transition_style::slide_backward);
    }
    inline void set_navigation_transition_pop_style(element& target, navigation_transition_style value)
    {
        target.set_platform_spec(navigation_transition_pop_style_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] navigation_transition_style get_navigation_transition_pop_style(config<macos, TElement> cfg)
    {
        return get_navigation_transition_pop_style(cfg.element());
    }

    // ---- SetNavigationTransitionStyle(element, push, pop) + the config chaining form ----
    inline void set_navigation_transition_style(element& target, navigation_transition_style push_style,
                                                navigation_transition_style pop_style)
    {
        set_navigation_transition_push_style(target, push_style);
        set_navigation_transition_pop_style(target, pop_style);
    }
    template <std::derived_from<forms_element> TElement>
    config<macos, TElement> set_navigation_transition_style(config<macos, TElement> cfg,
                                                            navigation_transition_style push_style,
                                                            navigation_transition_style pop_style)
    {
        set_navigation_transition_style(cfg.element(), push_style, pop_style);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::macos_specific::navigation_page

#pragma once
// maui::controls::platform_configuration::ios_specific::application
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Application
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/Application.cs. STORED knobs
// (the C# consumers live in gesture/font services the port has not reached); see STATUS.md W2-24.

#include <concepts>
#include <string_view>

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::ios_specific::application
{
    using forms_element = maui::controls::application; // C# `using FormsElement = ...Application` (implicit)

    inline constexpr std::string_view pan_gesture_recognizer_should_recognize_simultaneously_key =
        "ios.Application.PanGestureRecognizerShouldRecognizeSimultaneously";
    inline constexpr std::string_view handle_control_updates_on_main_thread_key =
        "ios.Application.HandleControlUpdatesOnMainThread";
    inline constexpr std::string_view enable_accessibility_scaling_for_named_font_sizes_key =
        "ios.Application.EnableAccessibilityScalingForNamedFontSizes";

    // ---- PanGestureRecognizerShouldRecognizeSimultaneously (bool, default false) ----
    [[nodiscard]] inline bool get_pan_gesture_recognizer_should_recognize_simultaneously(const element& target)
    {
        return target.platform_spec<bool>(pan_gesture_recognizer_should_recognize_simultaneously_key, false);
    }
    inline void set_pan_gesture_recognizer_should_recognize_simultaneously(element& target, bool value)
    {
        target.set_platform_spec(pan_gesture_recognizer_should_recognize_simultaneously_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_pan_gesture_recognizer_should_recognize_simultaneously(config<ios, TElement> cfg)
    {
        return get_pan_gesture_recognizer_should_recognize_simultaneously(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_pan_gesture_recognizer_should_recognize_simultaneously(config<ios, TElement> cfg,
                                                                                     bool value)
    {
        set_pan_gesture_recognizer_should_recognize_simultaneously(cfg.element(), value);
        return cfg;
    }

    // ---- HandleControlUpdatesOnMainThread (bool, default false) ----
    [[nodiscard]] inline bool get_handle_control_updates_on_main_thread(const element& target)
    {
        return target.platform_spec<bool>(handle_control_updates_on_main_thread_key, false);
    }
    inline void set_handle_control_updates_on_main_thread(element& target, bool value)
    {
        target.set_platform_spec(handle_control_updates_on_main_thread_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_handle_control_updates_on_main_thread(config<ios, TElement> cfg)
    {
        return get_handle_control_updates_on_main_thread(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_handle_control_updates_on_main_thread(config<ios, TElement> cfg, bool value)
    {
        set_handle_control_updates_on_main_thread(cfg.element(), value);
        return cfg;
    }

    // ---- EnableAccessibilityScalingForNamedFontSizes (bool, default true) ----
    [[nodiscard]] inline bool get_enable_accessibility_scaling_for_named_font_sizes(const element& target)
    {
        return target.platform_spec<bool>(enable_accessibility_scaling_for_named_font_sizes_key, true);
    }
    inline void set_enable_accessibility_scaling_for_named_font_sizes(element& target, bool value)
    {
        target.set_platform_spec(enable_accessibility_scaling_for_named_font_sizes_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_enable_accessibility_scaling_for_named_font_sizes(config<ios, TElement> cfg)
    {
        return get_enable_accessibility_scaling_for_named_font_sizes(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<ios, TElement> set_enable_accessibility_scaling_for_named_font_sizes(config<ios, TElement> cfg, bool value)
    {
        set_enable_accessibility_scaling_for_named_font_sizes(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::application

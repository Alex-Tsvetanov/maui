#pragma once
// maui::controls::platform_configuration::android_specific::application
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.Application
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/Application.cs.
// STORED-INERT until the Android JNI per-control fan-out wires real consumers (STATUS.md W2-24).

#include <concepts>
#include <cstdint>
#include <string_view>

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::android_specific
{
    // C# AndroidSpecific.WindowSoftInputModeAdjust (declared alongside Application).
    enum class window_soft_input_mode_adjust : std::uint8_t
    {
        pan = 0,
        resize = 1,
        unspecified = 2,
    };

    namespace application
    {
        using forms_element = maui::controls::application; // C# `using FormsElement = ...Application` (implicit)

        inline constexpr std::string_view window_soft_input_mode_adjust_key =
            "android.Application.WindowSoftInputModeAdjust";

        // ---- WindowSoftInputModeAdjust (WindowSoftInputModeAdjust, default Pan) ----
        [[nodiscard]] inline android_specific::window_soft_input_mode_adjust get_window_soft_input_mode_adjust(
            const element& target)
        {
            return target.platform_spec<android_specific::window_soft_input_mode_adjust>(
                window_soft_input_mode_adjust_key, android_specific::window_soft_input_mode_adjust::pan);
        }
        inline void set_window_soft_input_mode_adjust(element& target,
                                                      android_specific::window_soft_input_mode_adjust value)
        {
            target.set_platform_spec(window_soft_input_mode_adjust_key, value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] android_specific::window_soft_input_mode_adjust get_window_soft_input_mode_adjust(
            config<android, TElement> cfg)
        {
            return get_window_soft_input_mode_adjust(cfg.element());
        }
        // C#'s chaining setter is named UseWindowSoftInputModeAdjust.
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> use_window_soft_input_mode_adjust(
            config<android, TElement> cfg, android_specific::window_soft_input_mode_adjust value)
        {
            set_window_soft_input_mode_adjust(cfg.element(), value);
            return cfg;
        }
    } // namespace application
} // namespace maui::controls::platform_configuration::android_specific

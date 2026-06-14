#pragma once
// maui::controls::platform_configuration::android_specific::app_compat::application
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat.Application
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/AppCompat/Application.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <string_view>

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::android_specific::app_compat::application
{
    using forms_element = maui::controls::application; // C# `using FormsElement = ...Application` (implicit)

    inline constexpr std::string_view send_disappearing_event_on_pause_key =
        "android.AppCompat.Application.SendDisappearingEventOnPause";
    inline constexpr std::string_view send_appearing_event_on_resume_key =
        "android.AppCompat.Application.SendAppearingEventOnResume";
    inline constexpr std::string_view should_preserve_keyboard_on_resume_key =
        "android.AppCompat.Application.ShouldPreserveKeyboardOnResume";

    // ---- SendDisappearingEventOnPause (bool, default true) ----
    [[nodiscard]] inline bool get_send_disappearing_event_on_pause(const element& target)
    {
        return target.platform_spec<bool>(send_disappearing_event_on_pause_key, true);
    }
    inline void set_send_disappearing_event_on_pause(element& target, bool value)
    {
        target.set_platform_spec(send_disappearing_event_on_pause_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_send_disappearing_event_on_pause(config<android, TElement> cfg)
    {
        return get_send_disappearing_event_on_pause(cfg.element());
    }
    // C#'s chaining setter is named SendDisappearingEventOnPause.
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> send_disappearing_event_on_pause(config<android, TElement> cfg, bool value)
    {
        set_send_disappearing_event_on_pause(cfg.element(), value);
        return cfg;
    }

    // ---- SendAppearingEventOnResume (bool, default true) ----
    [[nodiscard]] inline bool get_send_appearing_event_on_resume(const element& target)
    {
        return target.platform_spec<bool>(send_appearing_event_on_resume_key, true);
    }
    inline void set_send_appearing_event_on_resume(element& target, bool value)
    {
        target.set_platform_spec(send_appearing_event_on_resume_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_send_appearing_event_on_resume(config<android, TElement> cfg)
    {
        return get_send_appearing_event_on_resume(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> send_appearing_event_on_resume(config<android, TElement> cfg, bool value)
    {
        set_send_appearing_event_on_resume(cfg.element(), value);
        return cfg;
    }

    // ---- ShouldPreserveKeyboardOnResume (bool, default false) ----
    [[nodiscard]] inline bool get_should_preserve_keyboard_on_resume(const element& target)
    {
        return target.platform_spec<bool>(should_preserve_keyboard_on_resume_key, false);
    }
    inline void set_should_preserve_keyboard_on_resume(element& target, bool value)
    {
        target.set_platform_spec(should_preserve_keyboard_on_resume_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_should_preserve_keyboard_on_resume(config<android, TElement> cfg)
    {
        return get_should_preserve_keyboard_on_resume(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> should_preserve_keyboard_on_resume(config<android, TElement> cfg, bool value)
    {
        set_should_preserve_keyboard_on_resume(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::android_specific::app_compat::application

#pragma once
// maui::controls::platform_configuration::windows_specific::web_view
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.WebView
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/WebView.cs. STORED-INERT
// (no Windows backend).

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/windows_specific/web_view_execution_mode.hpp"
#include "maui/controls/web_view.hpp"

namespace maui::controls::platform_configuration::windows_specific::web_view
{
    using forms_element = maui::controls::web_view; // C# `using FormsElement = ...WebView`

    inline constexpr std::string_view is_java_script_alert_enabled_key = "windows.WebView.IsJavaScriptAlertEnabled";
    inline constexpr std::string_view execution_mode_key = "windows.WebView.ExecutionMode";

    // ---- IsJavaScriptAlertEnabled (bool, default false) ----
    [[nodiscard]] inline bool get_is_java_script_alert_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_java_script_alert_enabled_key, false);
    }
    inline void set_is_java_script_alert_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_java_script_alert_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_java_script_alert_enabled(config<windows, TElement> cfg)
    {
        return get_is_java_script_alert_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_is_java_script_alert_enabled(config<windows, TElement> cfg, bool value)
    {
        set_is_java_script_alert_enabled(cfg.element(), value);
        return cfg;
    }

    // ---- ExecutionMode (WebViewExecutionMode, default SameThread) ----
    [[nodiscard]] inline web_view_execution_mode get_execution_mode(const element& target)
    {
        return target.platform_spec<web_view_execution_mode>(execution_mode_key, web_view_execution_mode::same_thread);
    }
    inline void set_execution_mode(element& target, web_view_execution_mode value)
    {
        target.set_platform_spec(execution_mode_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] web_view_execution_mode get_execution_mode(config<windows, TElement> cfg)
    {
        return get_execution_mode(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_execution_mode(config<windows, TElement> cfg, web_view_execution_mode value)
    {
        set_execution_mode(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::web_view

#pragma once
// maui::controls::platform_configuration::windows_specific::application
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.Application
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/Application.cs.
// STORED-INERT (no Windows backend; C#'s propertyChanged pushes the directory to the WinUI image
// loader — nothing to push here).

#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::windows_specific::application
{
    using forms_element = maui::controls::application; // C# `using FormsElement = ...Application`

    inline constexpr std::string_view image_directory_key = "windows.Application.ImageDirectory";

    // ---- ImageDirectory (string, default "") ----
    [[nodiscard]] inline std::string get_image_directory(const element& target)
    {
        return target.platform_spec<std::string>(image_directory_key, std::string{});
    }
    inline void set_image_directory(element& target, std::string value)
    {
        target.set_platform_spec(image_directory_key, std::move(value));
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::string get_image_directory(config<windows, TElement> cfg)
    {
        return get_image_directory(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_image_directory(config<windows, TElement> cfg, std::string value)
    {
        set_image_directory(cfg.element(), std::move(value));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::application

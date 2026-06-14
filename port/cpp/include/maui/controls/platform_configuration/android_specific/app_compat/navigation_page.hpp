#pragma once
// maui::controls::platform_configuration::android_specific::app_compat::navigation_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat.NavigationPage
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/AppCompat/NavigationPage.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <string_view>

#include "maui/controls/navigation_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::android_specific::app_compat::navigation_page
{
    using forms_element = maui::controls::navigation_page; // C# `using FormsElement = ...NavigationPage` (implicit)

    inline constexpr std::string_view bar_height_key = "android.AppCompat.NavigationPage.BarHeight";

    // ---- BarHeight (int, default 0 = default(int)) ----
    [[nodiscard]] inline int get_bar_height(const element& target)
    {
        return target.platform_spec<int>(bar_height_key, 0);
    }
    inline void set_bar_height(element& target, int value)
    {
        target.set_platform_spec(bar_height_key, value);
    }
    template <std::derived_from<forms_element> TElement> [[nodiscard]] int get_bar_height(config<android, TElement> cfg)
    {
        return get_bar_height(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_bar_height(config<android, TElement> cfg, int value)
    {
        set_bar_height(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::android_specific::app_compat::navigation_page

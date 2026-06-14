#pragma once
// maui::controls::platform_configuration::windows_specific::flyout_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.FlyoutPage
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/FlyoutPage.cs.
// STORED-INERT (no Windows backend; C#'s CollapseStyle propertyChanged pokes the WinUI renderer).

#include <concepts>
#include <stdexcept>
#include <string_view>

#include "maui/controls/flyout_page.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/windows_specific/collapse_style.hpp"

namespace maui::controls::platform_configuration::windows_specific::flyout_page
{
    using forms_element = maui::controls::flyout_page; // C# `using FormsElement = ...FlyoutPage`

    inline constexpr std::string_view collapse_style_key = "windows.FlyoutPage.CollapseStyle";
    inline constexpr std::string_view collapsed_pane_width_key = "windows.FlyoutPage.CollapsedPaneWidth";

    // ---- CollapseStyle (CollapseStyle, default Full; attached) ----
    [[nodiscard]] inline windows_specific::collapse_style get_collapse_style(const element& target)
    {
        return target.platform_spec<windows_specific::collapse_style>(collapse_style_key,
                                                                      windows_specific::collapse_style::full);
    }
    inline void set_collapse_style(element& target, windows_specific::collapse_style value)
    {
        target.set_platform_spec(collapse_style_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] windows_specific::collapse_style get_collapse_style(config<windows, TElement> cfg)
    {
        return get_collapse_style(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_collapse_style(config<windows, TElement> cfg, windows_specific::collapse_style value)
    {
        set_collapse_style(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> use_partial_collapse(config<windows, TElement> cfg)
    {
        set_collapse_style(cfg.element(), windows_specific::collapse_style::partial);
        return cfg;
    }

    // ---- CollapsedPaneWidth (double, default 48; C# validateValue rejects negatives — ported as
    // invalid_argument, like the Android OffscreenPageLimit guard) ----
    [[nodiscard]] inline double get_collapsed_pane_width(const element& target)
    {
        return target.platform_spec<double>(collapsed_pane_width_key, 48.0);
    }
    inline void set_collapsed_pane_width(element& target, double collapsed_pane_width)
    {
        if (collapsed_pane_width < 0)
        {
            throw std::invalid_argument("CollapsedPaneWidth must be >= 0 (BindableProperty validateValue)");
        }
        target.set_platform_spec(collapsed_pane_width_key, collapsed_pane_width);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] double collapsed_pane_width(config<windows, TElement> cfg)
    {
        return get_collapsed_pane_width(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> collapsed_pane_width(config<windows, TElement> cfg, double value)
    {
        set_collapsed_pane_width(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::flyout_page

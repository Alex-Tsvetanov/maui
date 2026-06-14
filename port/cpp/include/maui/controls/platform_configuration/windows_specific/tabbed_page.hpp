#pragma once
// maui::controls::platform_configuration::windows_specific::tabbed_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.TabbedPage
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/TabbedPage.cs. STORED-INERT
// (no Windows backend). NOTE: the C# property-name strings are nameof(HeaderIconsEnabledProperty) /
// nameof(HeaderIconsSizeProperty) — i.e. they carry the "...Property" suffix (an oracle quirk); the
// port keys keep the clean names (nothing keys off the C# strings).

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::platform_configuration::windows_specific::tabbed_page
{
    using forms_element = maui::controls::tabbed_page; // C# `using FormsElement = ...TabbedPage` (implicit)

    inline constexpr std::string_view header_icons_enabled_key = "windows.TabbedPage.HeaderIconsEnabled";
    inline constexpr std::string_view header_icons_size_key = "windows.TabbedPage.HeaderIconsSize";

    // ---- HeaderIconsEnabled (bool, default true) ----
    [[nodiscard]] inline bool get_header_icons_enabled(const element& target)
    {
        return target.platform_spec<bool>(header_icons_enabled_key, true);
    }
    inline void set_header_icons_enabled(element& target, bool value)
    {
        target.set_platform_spec(header_icons_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_header_icons_enabled(config<windows, TElement> cfg)
    {
        return get_header_icons_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_header_icons_enabled(config<windows, TElement> cfg, bool value)
    {
        set_header_icons_enabled(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_header_icons_enabled(config<windows, TElement> cfg)
    {
        return get_header_icons_enabled(cfg.element());
    }
    // C#'s EnableHeaderIcons/DisableHeaderIcons conveniences return void.
    template <std::derived_from<forms_element> TElement> void enable_header_icons(config<windows, TElement> cfg)
    {
        set_header_icons_enabled(cfg.element(), true);
    }
    template <std::derived_from<forms_element> TElement> void disable_header_icons(config<windows, TElement> cfg)
    {
        set_header_icons_enabled(cfg.element(), false);
    }

    // ---- HeaderIconsSize (Size, default 16x16) ----
    [[nodiscard]] inline maui::graphics::size get_header_icons_size(const element& target)
    {
        return target.platform_spec<maui::graphics::size>(header_icons_size_key, maui::graphics::size{16, 16});
    }
    inline void set_header_icons_size(element& target, maui::graphics::size value)
    {
        target.set_platform_spec(header_icons_size_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] maui::graphics::size get_header_icons_size(config<windows, TElement> cfg)
    {
        return get_header_icons_size(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_header_icons_size(config<windows, TElement> cfg, maui::graphics::size value)
    {
        set_header_icons_size(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::windows_specific::tabbed_page

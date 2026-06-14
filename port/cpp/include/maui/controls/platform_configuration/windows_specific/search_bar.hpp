#pragma once
// maui::controls::platform_configuration::windows_specific::search_bar
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific.SearchBar
// Ported from src/Controls/src/Core/PlatformConfiguration/WindowsSpecific/SearchBar.cs. STORED-INERT
// (no Windows backend). NOTE: the C# property-name string is "IsSpellCheckEnabled " — with a trailing
// space (an oracle typo on the descriptor); the port key drops it (nothing keys off the C# string).

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/search_bar.hpp"

namespace maui::controls::platform_configuration::windows_specific::search_bar
{
    using forms_element = maui::controls::search_bar; // C# `using FormsElement = ...SearchBar`

    inline constexpr std::string_view is_spell_check_enabled_key = "windows.SearchBar.IsSpellCheckEnabled";

    // ---- IsSpellCheckEnabled (bool, default false) ----
    [[nodiscard]] inline bool get_is_spell_check_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_spell_check_enabled_key, false);
    }
    inline void set_is_spell_check_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_spell_check_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_is_spell_check_enabled(config<windows, TElement> cfg)
    {
        return get_is_spell_check_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<windows, TElement> set_is_spell_check_enabled(config<windows, TElement> cfg, bool value)
    {
        set_is_spell_check_enabled(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_spell_check_enabled(config<windows, TElement> cfg)
    {
        return get_is_spell_check_enabled(cfg.element());
    }
    // C#'s EnableSpellCheck/DisableSpellCheck conveniences return void.
    template <std::derived_from<forms_element> TElement> void enable_spell_check(config<windows, TElement> cfg)
    {
        set_is_spell_check_enabled(cfg.element(), true);
    }
    template <std::derived_from<forms_element> TElement> void disable_spell_check(config<windows, TElement> cfg)
    {
        set_is_spell_check_enabled(cfg.element(), false);
    }
} // namespace maui::controls::platform_configuration::windows_specific::search_bar

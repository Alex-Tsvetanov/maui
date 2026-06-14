#pragma once
// maui::controls::platform_configuration::tizen_specific::application
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.Application
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/Application.cs. STORED-INERT
// (no Tizen backend). ActiveBezelInteractionElement is a read-only BindablePropertyKey in C# whose
// public Set goes through the key — both surface as plain get/set here.

#include <concepts>
#include <string_view>

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::tizen_specific::application
{
    using forms_element = maui::controls::application; // C# `using FormsElement = ...Application`

    inline constexpr std::string_view use_bezel_interaction_key = "tizen.Application.UseBezelInteraction";
    inline constexpr std::string_view overlay_content_key = "tizen.Application.OverlayContent";
    inline constexpr std::string_view active_bezel_interaction_element_key =
        "tizen.Application.ActiveBezelInteractionElement";

    // ---- UseBezelInteraction (bool, default true) ----
    [[nodiscard]] inline bool get_use_bezel_interaction(const element& target)
    {
        return target.platform_spec<bool>(use_bezel_interaction_key, true);
    }
    inline void set_use_bezel_interaction(element& target, bool value)
    {
        target.set_platform_spec(use_bezel_interaction_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_use_bezel_interaction(config<tizen, TElement> cfg)
    {
        return get_use_bezel_interaction(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_use_bezel_interaction(config<tizen, TElement> cfg, bool value)
    {
        set_use_bezel_interaction(cfg.element(), value);
        return cfg;
    }

    // ---- OverlayContent (View, default null — NON-owning) ----
    [[nodiscard]] inline element* get_overlay_content(const element& target)
    {
        return target.platform_spec<element*>(overlay_content_key, nullptr);
    }
    inline void set_overlay_content(element& target, element* value)
    {
        target.set_platform_spec(overlay_content_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] element* get_overlay_content(config<tizen, TElement> cfg)
    {
        return get_overlay_content(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_overlay_content(config<tizen, TElement> cfg, element* value)
    {
        set_overlay_content(cfg.element(), value);
        return cfg;
    }

    // ---- ActiveBezelInteractionElement (Element, default null — NON-owning) ----
    [[nodiscard]] inline element* get_active_bezel_interaction_element(const element& target)
    {
        return target.platform_spec<element*>(active_bezel_interaction_element_key, nullptr);
    }
    inline void set_active_bezel_interaction_element(element& target, element* value)
    {
        target.set_platform_spec(active_bezel_interaction_element_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] element* get_active_bezel_interaction_element(config<tizen, TElement> cfg)
    {
        return get_active_bezel_interaction_element(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_active_bezel_interaction_element(config<tizen, TElement> cfg, element* value)
    {
        set_active_bezel_interaction_element(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::application

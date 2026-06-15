#pragma once
// maui::controls::platform_configuration::ios_specific::page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Page
//
// The iOS page knob set, ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/Page.cs.
// Each C# attached BindableProperty becomes a namespaced key in the element's platform-spec store
// (element.hpp W2-24); the static Get/Set pairs become get_/set_ free functions over element& (C# takes
// BindableObject) and the extension methods become config-chaining overloads constrained on the
// declared page set (element_concepts.hpp).
//
// WIRED-REAL (W2-24, iOS backend): prefers_status_bar_hidden / preferred_status_bar_update_animation /
// prefers_home_indicator_auto_hidden surface through maui::core::i_ios_page_specifics (implemented by
// content_page) to the window's root UIViewController; use_safe_area + safe_area_insets flow through
// content_page's measure/arrange (the MauiView.AdjustForSafeArea analog). Everything else is stored
// (inert until a backend consumes it), exactly like C# on platforms without the consumer.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"
#include "maui/controls/platform_configuration/ios_specific/large_title_display_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/status_bar_hidden_mode.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_modal_presentation_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/ui_status_bar_animation.hpp"
#include "maui/core/safe_area_edges.hpp" // --- U20: the per-edge SafeAreaEdges config value
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls::platform_configuration::ios_specific::page
{
    // The attached-store keys ("<platform>.<DeclaringClass>.<PropertyName>"). The first five are ALSO
    // referenced (as literals, C#'s cross-assembly nameof analog) by maui::core::i_ios_page_specifics
    // and the iOS content-page wiring — keep them in sync.
    inline constexpr std::string_view prefers_status_bar_hidden_key = "ios.Page.PrefersStatusBarHidden";
    inline constexpr std::string_view preferred_status_bar_update_animation_key =
        "ios.Page.PreferredStatusBarUpdateAnimation";
    inline constexpr std::string_view use_safe_area_key = "ios.Page.UseSafeArea";
    inline constexpr std::string_view safe_area_insets_key = "ios.Page.SafeAreaInsets";
    inline constexpr std::string_view prefers_home_indicator_auto_hidden_key =
        "ios.Page.PrefersHomeIndicatorAutoHidden";
    inline constexpr std::string_view large_title_display_key = "ios.Page.LargeTitleDisplay";
    inline constexpr std::string_view modal_presentation_style_key = "ios.Page.ModalPresentationStyle";
    inline constexpr std::string_view modal_popover_source_view_key = "ios.Page.ModalPopoverSourceView";
    inline constexpr std::string_view modal_popover_rect_key = "ios.Page.ModalPopoverRect";

    // ---- PrefersStatusBarHidden (StatusBarHiddenMode, default Default) ----
    [[nodiscard]] inline status_bar_hidden_mode get_prefers_status_bar_hidden(const element& target)
    {
        return target.platform_spec<status_bar_hidden_mode>(prefers_status_bar_hidden_key,
                                                            status_bar_hidden_mode::default_mode);
    }
    inline void set_prefers_status_bar_hidden(element& target, status_bar_hidden_mode value)
    {
        target.set_platform_spec(prefers_status_bar_hidden_key, value);
    }
    template <page_element TElement>
    [[nodiscard]] status_bar_hidden_mode prefers_status_bar_hidden(config<ios, TElement> cfg)
    {
        return get_prefers_status_bar_hidden(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_prefers_status_bar_hidden(config<ios, TElement> cfg, status_bar_hidden_mode value)
    {
        set_prefers_status_bar_hidden(cfg.element(), value);
        return cfg;
    }

    // ---- PreferredStatusBarUpdateAnimation (UIStatusBarAnimation, default None) ----
    // (C#'s Set branches on Fade/Slide/other but every branch stores the value — collapsed here.)
    [[nodiscard]] inline ui_status_bar_animation get_preferred_status_bar_update_animation(const element& target)
    {
        return target.platform_spec<ui_status_bar_animation>(preferred_status_bar_update_animation_key,
                                                             ui_status_bar_animation::none);
    }
    inline void set_preferred_status_bar_update_animation(element& target, ui_status_bar_animation value)
    {
        target.set_platform_spec(preferred_status_bar_update_animation_key, value);
    }
    template <page_element TElement>
    [[nodiscard]] ui_status_bar_animation preferred_status_bar_update_animation(config<ios, TElement> cfg)
    {
        return get_preferred_status_bar_update_animation(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_preferred_status_bar_update_animation(config<ios, TElement> cfg,
                                                                    ui_status_bar_animation value)
    {
        set_preferred_status_bar_update_animation(cfg.element(), value);
        return cfg;
    }

    // ---- UseSafeArea (bool, default false; the C# MACCATALYST build defaults true — not a port target).
    // C# marks the whole knob obsolete in favor of per-edge SafeAreaEdges; the port keeps the legacy
    // surface (SafeAreaEdges is not ported). ----
    [[nodiscard]] inline bool get_use_safe_area(const element& target)
    {
        return target.platform_spec<bool>(use_safe_area_key, false);
    }
    inline void set_use_safe_area(element& target, bool value)
    {
        target.set_platform_spec(use_safe_area_key, value);
    }
    template <page_element TElement> [[nodiscard]] bool using_safe_area(config<ios, TElement> cfg)
    {
        return get_use_safe_area(cfg.element());
    }
    template <page_element TElement> config<ios, TElement> set_use_safe_area(config<ios, TElement> cfg, bool value)
    {
        set_use_safe_area(cfg.element(), value);
        return cfg;
    }

    // ---- SafeAreaEdges (SafeAreaEdges, per-element default None) — the per-edge replacement for the
    // obsolete UseSafeArea (C# SafeAreaElement.SafeAreaEdges on ContentPage). Unlike the other knobs, the
    // storage is the control's bindable property<safe_area_edges> (so IsSet works for
    // GetSafeAreaRegionsForEdge), not the attached platform-spec store; these wrappers delegate to the
    // content_page accessors. A non-content_page element reads the None default and ignores a set. The
    // config-chaining overloads constrain on content_page (where the property lives). ----
    [[nodiscard]] inline maui::core::safe_area_edges get_safe_area_edges(const element& target)
    {
        if (const auto* page = dynamic_cast<const content_page*>(&target))
        {
            return page->safe_area_edges();
        }
        return maui::core::safe_area_edges::none();
    }
    inline void set_safe_area_edges(element& target, maui::core::safe_area_edges value)
    {
        if (auto* page = dynamic_cast<content_page*>(&target))
        {
            page->set_safe_area_edges(value);
        }
    }
    template <std::derived_from<content_page> TElement>
    [[nodiscard]] maui::core::safe_area_edges safe_area_edges(config<ios, TElement> cfg)
    {
        return get_safe_area_edges(cfg.element());
    }
    template <std::derived_from<content_page> TElement>
    config<ios, TElement> set_safe_area_edges(config<ios, TElement> cfg, maui::core::safe_area_edges value)
    {
        set_safe_area_edges(cfg.element(), value);
        return cfg;
    }

    // ---- LargeTitleDisplay (LargeTitleDisplayMode, default Automatic) ----
    [[nodiscard]] inline large_title_display_mode get_large_title_display(const element& target)
    {
        return target.platform_spec<large_title_display_mode>(large_title_display_key,
                                                              large_title_display_mode::automatic);
    }
    inline void set_large_title_display(element& target, large_title_display_mode value)
    {
        target.set_platform_spec(large_title_display_key, value);
    }
    template <page_element TElement>
    [[nodiscard]] large_title_display_mode large_title_display(config<ios, TElement> cfg)
    {
        return get_large_title_display(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_large_title_display(config<ios, TElement> cfg, large_title_display_mode value)
    {
        set_large_title_display(cfg.element(), value);
        return cfg;
    }

    // ---- SafeAreaInsets (Thickness, default zero; C# read-only BindablePropertyKey — the setter is the
    // PLATFORM write-back channel (ISafeAreaView2.SafeAreaInsets → SetSafeAreaInsets), not a user knob;
    // the iOS content-page host pushes the real insets through it) ----
    [[nodiscard]] inline maui::core::thickness get_safe_area_insets(const element& target)
    {
        return target.platform_spec<maui::core::thickness>(safe_area_insets_key, maui::core::thickness{});
    }
    inline void set_safe_area_insets(element& target, maui::core::thickness value)
    {
        target.set_platform_spec(safe_area_insets_key, value);
    }
    template <page_element TElement> [[nodiscard]] maui::core::thickness safe_area_insets(config<ios, TElement> cfg)
    {
        return get_safe_area_insets(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_safe_area_insets(config<ios, TElement> cfg, maui::core::thickness value)
    {
        set_safe_area_insets(cfg.element(), value);
        return cfg;
    }

    // ---- ModalPresentationStyle (UIModalPresentationStyle, default FullScreen) ----
    [[nodiscard]] inline ui_modal_presentation_style get_modal_presentation_style(const element& target)
    {
        return target.platform_spec<ui_modal_presentation_style>(modal_presentation_style_key,
                                                                 ui_modal_presentation_style::full_screen);
    }
    inline void set_modal_presentation_style(element& target, ui_modal_presentation_style value)
    {
        target.set_platform_spec(modal_presentation_style_key, value);
    }
    template <page_element TElement>
    [[nodiscard]] ui_modal_presentation_style modal_presentation_style(config<ios, TElement> cfg)
    {
        return get_modal_presentation_style(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_modal_presentation_style(config<ios, TElement> cfg, ui_modal_presentation_style value)
    {
        set_modal_presentation_style(cfg.element(), value);
        return cfg;
    }

    // ---- ModalPopoverSourceView (View, default null — NON-owning, like every cross-element ref) ----
    [[nodiscard]] inline element* get_popover_source_view(const element& target)
    {
        return target.platform_spec<element*>(modal_popover_source_view_key, nullptr);
    }
    inline void set_modal_popover_view(element& target, element* value)
    {
        target.set_platform_spec(modal_popover_source_view_key, value);
    }
    template <page_element TElement> [[nodiscard]] element* modal_popover_source_view(config<ios, TElement> cfg)
    {
        return get_popover_source_view(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_modal_popover_view(config<ios, TElement> cfg, element* value)
    {
        set_modal_popover_view(cfg.element(), value);
        return cfg;
    }

    // ---- ModalPopoverRect (System.Drawing.Rectangle → maui::graphics::rect, default empty) ----
    [[nodiscard]] inline maui::graphics::rect get_popover_rect(const element& target)
    {
        return target.platform_spec<maui::graphics::rect>(modal_popover_rect_key, maui::graphics::rect{});
    }
    inline void set_modal_popover_rect(element& target, maui::graphics::rect value)
    {
        target.set_platform_spec(modal_popover_rect_key, value);
    }
    template <page_element TElement> [[nodiscard]] maui::graphics::rect modal_popover_rect(config<ios, TElement> cfg)
    {
        return get_popover_rect(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_modal_popover_rect(config<ios, TElement> cfg, maui::graphics::rect value)
    {
        set_modal_popover_rect(cfg.element(), value);
        return cfg;
    }

    // ---- PrefersHomeIndicatorAutoHidden (bool, default false) ----
    [[nodiscard]] inline bool get_prefers_home_indicator_auto_hidden(const element& target)
    {
        return target.platform_spec<bool>(prefers_home_indicator_auto_hidden_key, false);
    }
    inline void set_prefers_home_indicator_auto_hidden(element& target, bool value)
    {
        target.set_platform_spec(prefers_home_indicator_auto_hidden_key, value);
    }
    template <page_element TElement> [[nodiscard]] bool prefers_home_indicator_auto_hidden(config<ios, TElement> cfg)
    {
        return get_prefers_home_indicator_auto_hidden(cfg.element());
    }
    template <page_element TElement>
    config<ios, TElement> set_prefers_home_indicator_auto_hidden(config<ios, TElement> cfg, bool value)
    {
        set_prefers_home_indicator_auto_hidden(cfg.element(), value);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::ios_specific::page

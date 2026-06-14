#pragma once
// maui::controls::platform_configuration::android_specific::tabbed_page
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.TabbedPage
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/TabbedPage.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "maui/controls/platform_configuration/android_specific/toolbar_placement.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/tabbed_page.hpp"

namespace maui::controls::platform_configuration::android_specific::tabbed_page
{
    using forms_element = maui::controls::tabbed_page; // C# `using FormsElement = ...TabbedPage` (implicit)

    inline constexpr std::string_view is_swipe_paging_enabled_key = "android.TabbedPage.IsSwipePagingEnabled";
    inline constexpr std::string_view is_smooth_scroll_enabled_key = "android.TabbedPage.IsSmoothScrollEnabled";
    inline constexpr std::string_view offscreen_page_limit_key = "android.TabbedPage.OffscreenPageLimit";
    inline constexpr std::string_view toolbar_placement_key = "android.TabbedPage.ToolbarPlacement";

    // ---- IsSwipePagingEnabled (bool, default true) ----
    [[nodiscard]] inline bool get_is_swipe_paging_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_swipe_paging_enabled_key, true);
    }
    inline void set_is_swipe_paging_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_swipe_paging_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_swipe_paging_enabled(config<android, TElement> cfg)
    {
        return get_is_swipe_paging_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_is_swipe_paging_enabled(config<android, TElement> cfg, bool value)
    {
        set_is_swipe_paging_enabled(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> enable_swipe_paging(config<android, TElement> cfg)
    {
        set_is_swipe_paging_enabled(cfg.element(), true);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> disable_swipe_paging(config<android, TElement> cfg)
    {
        set_is_swipe_paging_enabled(cfg.element(), false);
        return cfg;
    }

    // ---- IsSmoothScrollEnabled (bool, default true) ----
    [[nodiscard]] inline bool get_is_smooth_scroll_enabled(const element& target)
    {
        return target.platform_spec<bool>(is_smooth_scroll_enabled_key, true);
    }
    inline void set_is_smooth_scroll_enabled(element& target, bool value)
    {
        target.set_platform_spec(is_smooth_scroll_enabled_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool is_smooth_scroll_enabled(config<android, TElement> cfg)
    {
        return get_is_smooth_scroll_enabled(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_is_smooth_scroll_enabled(config<android, TElement> cfg, bool value)
    {
        set_is_smooth_scroll_enabled(cfg.element(), value);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> enable_smooth_scroll(config<android, TElement> cfg)
    {
        set_is_smooth_scroll_enabled(cfg.element(), true);
        return cfg;
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> disable_smooth_scroll(config<android, TElement> cfg)
    {
        set_is_smooth_scroll_enabled(cfg.element(), false);
        return cfg;
    }

    // ---- OffscreenPageLimit (int, default 3; C# validateValue rejects negatives — SetValue throws
    // ArgumentException, ported as invalid_argument) ----
    [[nodiscard]] inline int get_offscreen_page_limit(const element& target)
    {
        return target.platform_spec<int>(offscreen_page_limit_key, 3);
    }
    inline void set_offscreen_page_limit(element& target, int value)
    {
        if (value < 0)
        {
            throw std::invalid_argument("OffscreenPageLimit must be >= 0 (BindableProperty validateValue)");
        }
        target.set_platform_spec(offscreen_page_limit_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] int offscreen_page_limit(config<android, TElement> cfg)
    {
        return get_offscreen_page_limit(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_offscreen_page_limit(config<android, TElement> cfg, int value)
    {
        set_offscreen_page_limit(cfg.element(), value);
        return cfg;
    }

    // ---- ToolbarPlacement (ToolbarPlacement, default Top per the C# descriptor) ----
    [[nodiscard]] inline android_specific::toolbar_placement get_toolbar_placement(const element& target)
    {
        return target.platform_spec<android_specific::toolbar_placement>(toolbar_placement_key,
                                                                         android_specific::toolbar_placement::top);
    }
    inline void set_toolbar_placement(element& target, android_specific::toolbar_placement value)
    {
        target.set_platform_spec(toolbar_placement_key, value);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] android_specific::toolbar_placement get_toolbar_placement(config<android, TElement> cfg)
    {
        return get_toolbar_placement(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<android, TElement> set_toolbar_placement(config<android, TElement> cfg,
                                                    android_specific::toolbar_placement value)
    {
        set_toolbar_placement(cfg.element(), value);
        return cfg;
    }

    // ---- GetMaxItemCount: 5 for bottom placement, unlimited otherwise ----
    [[nodiscard]] inline int get_max_item_count(const element& target)
    {
        if (get_toolbar_placement(target) == android_specific::toolbar_placement::bottom)
        {
            return 5;
        }
        return std::numeric_limits<int>::max();
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] int get_max_item_count(config<android, TElement> cfg)
    {
        return get_max_item_count(cfg.element());
    }
} // namespace maui::controls::platform_configuration::android_specific::tabbed_page

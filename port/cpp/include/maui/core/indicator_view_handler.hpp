#pragma once
// maui::core::indicator_view_handler  <=  Microsoft.Maui.Handlers.IndicatorViewHandler
//
// The handler for the position-indicator dots (maui::controls::indicator_view): Count / Position /
// HideSingle / MaximumVisible / IndicatorSize / IndicatorColor / SelectedIndicatorColor /
// IndicatorsShape flow virtual→native through the mapper, and a native dot tap writes Position back
// (the inbound channel). Ported from IndicatorViewHandler.cs (cross-platform) + IndicatorViewHandler
// .iOS.cs / IndicatorViewExtensions.cs / MauiPageControl.cs (the platform recipe). The Apple backend
// translates the UIPageControl recipe to an NSStackView-of-dots (AppKit has no UIPageControl); the
// headless mirror records count / position / shape (+ the maximum-visible clamp) for the oracle.
//
// IndicatorViewExtensions.GetMaximumVisible is reproduced as max_visible_indicators (free function):
// min(MaximumVisible, Count), floored at 0, with the HideSingle collapse (a single dot → 0 when
// HideSingle). IsCircleShape collapses to `shape == circle` (the port carries the enum directly).

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/indicator_shape.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // IndicatorViewExtensions.GetMaximumVisible: min(MaximumVisible, Count) floored at 0, HideSingle
    // collapses a lone dot to 0. The number of dots actually shown.
    [[nodiscard]] int max_visible_indicators(const i_indicator_view& view);

    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native dots).
    struct indicator_view_platform : view_platform_base
    {
        indicator_view_platform();
        ~indicator_view_platform() override; // backend-defined: releases the retained native control
        indicator_view_platform(const indicator_view_platform&) = delete;
        indicator_view_platform(indicator_view_platform&&) = delete;
        indicator_view_platform& operator=(const indicator_view_platform&) = delete;
        indicator_view_platform& operator=(indicator_view_platform&&) = delete;

        // The native view the handler composes into the tree:
        //   - headless: null (no native tree);
        //   - apple: a real NSStackView hosting `dot_count` NSView dots;
        //   - ios: the real MauiPageControl (UIPageControl).
        void* native = nullptr;

        // ---- headless mirror of the mapped surface (the oracle record) ----
        int dot_count = 0; // == max_visible_indicators (UpdatePages numberOfPages)
        int current_page = 0;
        double indicator_size = 6.0;
        maui::controls::indicator_shape shape = maui::controls::indicator_shape::circle;
        maui::graphics::color indicator_color;
        maui::graphics::color selected_indicator_color;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSStackView (defined in
        // src/platform/apple/indicator_view_handler.mm). NSStackView is not an NSControl, so is_enabled
        // keeps the base mirror; same ODR note as the other platform structs.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the fundamental IView properties to the UIPageControl (defined in
        // src/platform/ios/indicator_view_handler.mm).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class indicator_view_handler
        : public view_handler<indicator_view_handler, i_indicator_view, indicator_view_platform>
    {
    public:
        indicator_view_handler();

        // Shared mapper tables (cross-platform — defined in src/core/indicator_view_handler.cpp).
        static property_mapper<i_indicator_view, indicator_view_handler>& mapper();
        static command_mapper<i_indicator_view, indicator_view_handler>& command_mapper();

        // Per-backend (headless .cpp / apple+ios .mm): mint the platform struct (+ native control).
        static std::unique_ptr<indicator_view_platform> create_platform_view();
        // C# ConnectHandler: SetIndicatorView(VirtualView) + UpdateIndicator; DisconnectHandler clears it.
        void on_connect_handler(indicator_view_platform& platform);
        static void on_disconnect_handler(indicator_view_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- mapper entries (the C# MapXxx table) ----
        static void map_count(indicator_view_handler& handler, i_indicator_view& view);
        static void map_position(indicator_view_handler& handler, i_indicator_view& view);
        static void map_hide_single(indicator_view_handler& handler, i_indicator_view& view);
        static void map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view);
        static void map_indicator_size(indicator_view_handler& handler, i_indicator_view& view);
        static void map_indicator_color(indicator_view_handler& handler, i_indicator_view& view);
        static void map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view);
        static void map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view);

#ifdef MAUI_PLATFORM_IOS
        // ---- the iOS native bridge (UIPageControl) ----
        // Mirror the C# MauiPageControl methods so the on-simulator suite asserts the real control:
        // UpdateIndicatorCount (numberOfPages), UpdatePosition (currentPage), UpdateIndicatorSize,
        // and the two tint colors + the square-shape image swap.
        void native_update_count();
        void native_update_position();
        void native_update_size();
        void native_update_colors();
        void native_update_shape();
        // Mount the native control in a host window + force a layout pass (test seam; the run_loop_pump
        // helper turns the loop). Returns the live UIPageControl.numberOfPages.
        int native_force_layout(double width, double height);
        [[nodiscard]] int native_number_of_pages() const; // UIPageControl.numberOfPages
        [[nodiscard]] int native_current_page() const;    // UIPageControl.currentPage
        // Simulate a user tap that advances the page (the ValueChanged path → write Position back).
        void native_set_current_page(int page);
#endif
    };
} // namespace maui::core

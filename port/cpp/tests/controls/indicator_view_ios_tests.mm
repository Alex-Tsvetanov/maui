// iOS (UIKit, on-simulator) backend tests for indicator_view — the REAL MauiPageControl (a
// UIPageControl subclass) driven through the indicator_view_handler's ios bridge. These assert the
// genuine native control, not the cross-platform dot mirror (indicator_view_tests.cpp covers that):
//   - the native view is a UIPageControl;
//   - Count → numberOfPages, honoring HideSingle (a lone dot → 0 pages);
//   - Position → currentPage, clamped into the visible range;
//   - a native page tap (ValueChanged) writes Position back to the virtual view (the inbound channel);
//   - MaximumVisible caps numberOfPages.
//
// The run loop is pumped via tests/support/run_loop_pump.hpp so the control lays out. Compiled as
// Objective-C++ with ARC for the `ios` backend; run ON the booted simulator via tools/ios-sim-run.sh.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/indicator_view.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "tests/support/run_loop_pump.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::indicator_view;
    using maui::core::indicator_view_handler;
    using maui::tests::pump_run_loop;
    using maui::tests::pump_until;

    UIPageControl* native_page_control(const std::shared_ptr<indicator_view_handler>& handler)
    {
        return (__bridge UIPageControl*)handler->native_view();
    }

    UIWindow* make_host_window()
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init]; // SDK-deprecated; see window_handler.mm precedent
#pragma clang diagnostic pop
        return window;
    }

    struct rig
    {
        indicator_view view;
        std::shared_ptr<indicator_view_handler> handler = std::make_shared<indicator_view_handler>();

        rig()
        {
            view.set_handler(handler);
        }

        UIWindow* mount(double width = 200, double height = 30) const
        {
            UIWindow* const window = make_host_window();
            UIView* const control = (__bridge UIView*)handler->native_view();
            [window addSubview:control];
            [window makeKeyAndVisible];
            handler->native_force_layout(width, height);
            return window;
        }
    };

    TEST(indicator_view_ios, native_view_is_a_uipagecontrol)
    {
        const rig r;
        EXPECT_TRUE([native_page_control(r.handler) isKindOfClass:[UIPageControl class]]);
    }

    TEST(indicator_view_ios, count_drives_number_of_pages)
    {
        rig r;
        UIWindow* const window = r.mount();
        r.view.set_count(5);
        pump_until([&] { return r.handler->native_number_of_pages() == 5; });
        EXPECT_EQ(r.handler->native_number_of_pages(), 5);
        (void)window;
    }

    // HideSingle (default true): a single dot collapses to 0 pages.
    TEST(indicator_view_ios, hide_single_collapses_a_lone_page)
    {
        rig r;
        UIWindow* const window = r.mount();
        r.view.set_count(1);
        pump_until([&] { return r.handler->native_number_of_pages() == 0; });
        EXPECT_EQ(r.handler->native_number_of_pages(), 0);
        r.view.set_hide_single(false);
        pump_until([&] { return r.handler->native_number_of_pages() == 1; });
        EXPECT_EQ(r.handler->native_number_of_pages(), 1);
        (void)window;
    }

    TEST(indicator_view_ios, position_drives_current_page_clamped)
    {
        rig r;
        UIWindow* const window = r.mount();
        r.view.set_count(4);
        r.view.set_position_manual(2);
        pump_until([&] { return r.handler->native_current_page() == 2; });
        EXPECT_EQ(r.handler->native_current_page(), 2);
        // Past the last page clamps to the last index.
        r.view.set_position_manual(9);
        pump_until([&] { return r.handler->native_current_page() == 3; });
        EXPECT_EQ(r.handler->native_current_page(), 3);
        (void)window;
    }

    // MaximumVisible caps numberOfPages below Count.
    TEST(indicator_view_ios, maximum_visible_caps_pages)
    {
        rig r;
        UIWindow* const window = r.mount();
        r.view.set_count(10);
        r.view.set_maximum_visible(3);
        pump_until([&] { return r.handler->native_number_of_pages() == 3; });
        EXPECT_EQ(r.handler->native_number_of_pages(), 3);
        (void)window;
    }

    // A native page tap (ValueChanged) writes Position back to the virtual view (the inbound channel).
    TEST(indicator_view_ios, native_tap_writes_position_back)
    {
        rig r;
        UIWindow* const window = r.mount();
        r.view.set_count(5);
        pump_until([&] { return r.handler->native_number_of_pages() == 5; });
        r.handler->native_force_layout(200, 30); // lay the 5 dots out so currentPage sticks
        pump_run_loop(0.1);

        r.handler->native_set_current_page(3); // simulate the user tapping to page 3
        pump_until([&] { return r.view.position() == 3; });
        EXPECT_EQ(r.view.position(), 3);
        (void)window;
    }
} // namespace

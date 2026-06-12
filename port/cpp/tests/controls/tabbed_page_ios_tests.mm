// iOS (UIKit) backend tests for the tabbed_page seam, run ON the simulator. The host is a real
// UITabBarController: one child UIViewController per page (vc.view IS the page's native UIView — the
// child-VC composition the W1-10 task asserts), tabBarItem titles from each page's Title,
// selectedIndex tracking CurrentPage both ways (programmatic + the UITabBarControllerDelegate's
// didSelectViewController, invoked directly here to simulate the user tap — UIKit only calls it for
// user selections), and the bar styling painted on the real UITabBar. Compiled as Objective-C++ with
// ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::tabbed_page;
    using maui::core::content_page_handler;
    using maui::core::tabbed_page_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITabBarController* native_controller(const std::shared_ptr<tabbed_page_handler>& handler)
    {
        return (__bridge UITabBarController*)handler->typed_platform_view()->controller;
    }

    // A content_page with its handler attached, so it owns a real native UIView host the child view
    // controller can adopt. Returns the page's native UIView for hierarchy assertions.
    UIView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge UIView*)page_handler->native_view();
    }

    TEST(ios_tabbed_page_seam, controller_hosts_one_child_vc_per_page_with_titles)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        UIView* const first_native = attach_page(first);
        UIView* const second_native = attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);

        UITabBarController* const controller = native_controller(handler);
        ASSERT_NE(controller, nil);
        ASSERT_EQ(controller.viewControllers.count, 2U);
        // Child-VC composition: each page's native UIView is its wrapper view controller's view.
        EXPECT_EQ(controller.viewControllers[0].view, first_native);
        EXPECT_EQ(controller.viewControllers[1].view, second_native);
        EXPECT_EQ(to_std_string(controller.viewControllers[0].tabBarItem.title), "First");
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Second");
        // The handler's native root is the controller's view.
        EXPECT_EQ((__bridge UIView*)handler->typed_platform_view()->native, controller.view);
    }

    TEST(ios_tabbed_page_seam, selected_index_tracks_current_page_both_ways)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        attach_page(first);
        attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);

        // Virtual -> native: the first child became current on add.
        EXPECT_EQ(controller.selectedIndex, 0U);
        tabs.set_current_page(&second);
        EXPECT_EQ(controller.selectedIndex, 1U);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 1);

        // Native -> virtual: a user tab tap lands in the delegate's didSelectViewController (invoked
        // directly — UIKit only fires it for user selections).
        controller.selectedIndex = 0;
        [controller.delegate tabBarController:controller didSelectViewController:controller.viewControllers[0]];
        EXPECT_EQ(tabs.current_page(), &first);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 0);
    }

    TEST(ios_tabbed_page_seam, switching_current_page_keeps_the_controller_hierarchy)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        UIView* const first_native = attach_page(first);
        UIView* const second_native = attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);

        tabs.set_current_page(&second);

        // Both children remain in the controller hierarchy after the switch; the selected child VC is
        // the second page's wrapper.
        ASSERT_EQ(controller.viewControllers.count, 2U);
        EXPECT_EQ(controller.viewControllers[0].view, first_native);
        EXPECT_EQ(controller.viewControllers[1].view, second_native);
        EXPECT_EQ(controller.selectedViewController, controller.viewControllers[1]);
        EXPECT_EQ(handler->typed_platform_view()->hosted_current, &second);
    }

    TEST(ios_tabbed_page_seam, pages_and_title_changes_rebuild_the_children)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBarController* const controller = native_controller(handler);
        ASSERT_EQ(controller.viewControllers.count, 1U);

        content_page second;
        second.set_title("Second");
        attach_page(second);
        tabs.add(second); // PagesChanged -> the children rebuild
        ASSERT_EQ(controller.viewControllers.count, 2U);
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Second");

        second.set_title("Renamed"); // the Title wiring -> the children rebuild again
        EXPECT_EQ(to_std_string(controller.viewControllers[1].tabBarItem.title), "Renamed");
    }

    TEST(ios_tabbed_page_seam, bar_styling_paints_the_real_tab_bar)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        UITabBar* const bar = native_controller(handler).tabBar;

        tabs.set_bar_background_color(maui::graphics::colors::red);
        tabs.set_selected_tab_color(maui::graphics::colors::blue);
        tabs.set_unselected_tab_color(maui::graphics::colors::gray);

        // BarBackgroundColor → the appearance background; Selected/Unselected → the tint pair.
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        [bar.standardAppearance.backgroundColor getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_DOUBLE_EQ(red, 1.0);
        EXPECT_DOUBLE_EQ(green, 0.0);
        EXPECT_DOUBLE_EQ(blue, 0.0);
        [bar.tintColor getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_DOUBLE_EQ(blue, 1.0);
        ASSERT_NE(bar.unselectedItemTintColor, nil);

        // The mirrors stay observable too (the shared seam contract).
        ASSERT_TRUE(handler->typed_platform_view()->bar_background_color.has_value());
        EXPECT_EQ(*handler->typed_platform_view()->bar_background_color, maui::graphics::colors::red);
    }
} // namespace

// Apple (AppKit) backend tests for the tabbed_page seam. The host is a real NSTabViewController: one
// NSTabViewItem per child page (the item's wrapper NSViewController's view IS the page's native
// NSView), labelled with the page's Title, selectedTabViewItemIndex tracking CurrentPage both ways
// (programmatic + the didSelectTabViewItem forwarding). The four bar colors are MIRROR-only on AppKit
// (no tab-chrome color API — the documented deviation). Compiled as Objective-C++ with ARC for the
// `apple` backend.
#import <AppKit/AppKit.h>

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

    NSTabViewController* native_controller(const std::shared_ptr<tabbed_page_handler>& handler)
    {
        return (__bridge NSTabViewController*)handler->typed_platform_view()->controller;
    }

    // A content_page with its handler attached, so it owns a real native NSView host the tab item can
    // host. Returns the page's native NSView for hierarchy assertions.
    NSView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge NSView*)page_handler->native_view();
    }

    class apple_tabbed_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_tabbed_page_seam, controller_hosts_one_tab_item_per_page_with_titles)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        NSView* const first_native = attach_page(first);
        NSView* const second_native = attach_page(second);
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);

        NSTabViewController* const controller = native_controller(handler);
        ASSERT_NE(controller, nil);
        ASSERT_EQ(controller.tabViewItems.count, 2U);
        EXPECT_EQ(to_std_string(controller.tabViewItems[0].label), "First");
        EXPECT_EQ(to_std_string(controller.tabViewItems[1].label), "Second");
        // Child-VC composition: each page's native NSView is the tab item's view controller's view.
        EXPECT_EQ(controller.tabViewItems[0].viewController.view, first_native);
        EXPECT_EQ(controller.tabViewItems[1].viewController.view, second_native);
        // The handler's native root is the controller's view.
        EXPECT_EQ((__bridge NSView*)handler->typed_platform_view()->native, controller.view);
    }

    TEST_F(apple_tabbed_page_seam, selection_tracks_current_page_both_ways)
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
        NSTabViewController* const controller = native_controller(handler);

        // Virtual -> native: the first child became current on add.
        EXPECT_EQ(controller.selectedTabViewItemIndex, 0);
        tabs.set_current_page(&second);
        EXPECT_EQ(controller.selectedTabViewItemIndex, 1);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 1);

        // Native -> virtual: a user tab selection lands in tabView:didSelectTabViewItem:, which the
        // MauiTabViewController forwards to i_tabbed_view::on_tab_selected.
        controller.selectedTabViewItemIndex = 0;
        [controller tabView:controller.tabView didSelectTabViewItem:controller.tabViewItems[0]];
        EXPECT_EQ(tabs.current_page(), &first);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 0);
    }

    TEST_F(apple_tabbed_page_seam, pages_and_title_changes_rebuild_the_tab_items)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        NSTabViewController* const controller = native_controller(handler);
        ASSERT_EQ(controller.tabViewItems.count, 1U);

        content_page second;
        second.set_title("Second");
        attach_page(second);
        tabs.add(second); // PagesChanged -> the items rebuild
        ASSERT_EQ(controller.tabViewItems.count, 2U);
        EXPECT_EQ(to_std_string(controller.tabViewItems[1].label), "Second");

        second.set_title("Renamed"); // the Title wiring -> the items rebuild again
        EXPECT_EQ(to_std_string(controller.tabViewItems[1].label), "Renamed");

        tabs.remove(first);
        ASSERT_EQ(controller.tabViewItems.count, 1U);
        EXPECT_EQ(to_std_string(controller.tabViewItems[0].label), "Renamed");
    }

    TEST_F(apple_tabbed_page_seam, bar_colors_stay_mirror_only_on_appkit)
    {
        tabbed_page tabs;
        content_page first;
        attach_page(first);
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        EXPECT_FALSE(platform->bar_background_color.has_value()); // unset -> system default
        tabs.set_bar_background_color(maui::graphics::colors::red);
        tabs.set_selected_tab_color(maui::graphics::colors::blue);

        ASSERT_TRUE(platform->bar_background_color.has_value());
        EXPECT_EQ(*platform->bar_background_color, maui::graphics::colors::red);
        ASSERT_TRUE(platform->selected_tab_color.has_value());
        EXPECT_EQ(*platform->selected_tab_color, maui::graphics::colors::blue);
    }
} // namespace

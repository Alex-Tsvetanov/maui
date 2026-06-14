// Apple (AppKit) backend tests for the shell_handler seam (W3-32). The host is a real
// NSSplitViewController: a SIDEBAR NSSplitViewItem (the flyout list) + a content item hosting an NSTabView
// of the current shell_item's sections. Route navigation reconfigures the NSTabView's items; the selected
// tab tracks the current section; FlyoutIsPresented collapses/expands the sidebar (the documented AppKit
// deviation — no pan drawer; per-section navigation shows the top page only, no NSNavigationController).
// Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell_handler.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::shell;
    using maui::controls::shell_item;
    using maui::controls::shell_navigation_state;
    using maui::core::shell_handler;

    NSSplitViewController* native_controller(const std::shared_ptr<shell_handler>& handler)
    {
        return (__bridge NSSplitViewController*)handler->typed_platform_view()->controller;
    }

    NSTabView* native_tab_host(const std::shared_ptr<shell_handler>& handler)
    {
        return (__bridge NSTabView*)handler->typed_platform_view()->tab_host;
    }

    // A shell-handler suite that reuses the shell_test_base model helpers but runs the apple natives.
    class apple_shell_seam : public maui::controls::shell_tests::shell_test_base
    {
    protected:
        void SetUp() override
        {
            shell_test_base::SetUp();
            [NSApplication sharedApplication];
        }

        void build_two_item_shell(shell& sh)
        {
            auto one = std::make_shared<shell_item>();
            one->set_route("one");
            auto two = std::make_shared<shell_item>();
            two->set_route("two");
            one->add(make_simple_shell_section("tabone", "content"));
            one->add(make_simple_shell_section("tabtwo", "content"));
            two->add(make_simple_shell_section("tabthree", "content"));
            two->add(make_simple_shell_section("tabfour", "content"));
            sh.add_item(one);
            sh.add_item(two);
        }
    };

    TEST_F(apple_shell_seam, controller_is_split_with_sidebar_and_tab_host)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        NSSplitViewController* const split = native_controller(handler);
        ASSERT_NE(split, nil);
        ASSERT_EQ(split.splitViewItems.count, 2U);
        // The first item is the sidebar (the flyout — the AppKit deviation: a persistent sidebar, no drawer).
        EXPECT_EQ(split.splitViewItems[0].behavior, NSSplitViewItemBehaviorSidebar);
        // The content item hosts the NSTabView tab host.
        EXPECT_EQ(split.splitViewItems[1].viewController.view, native_tab_host(handler));
        EXPECT_EQ((__bridge NSView*)handler->typed_platform_view()->native, split.view);
    }

    TEST_F(apple_shell_seam, tab_host_has_one_tab_per_section_with_current_selected)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        NSTabView* const tabs = native_tab_host(handler);
        ASSERT_NE(tabs, nil);
        // Item one has two sections (tabone + tabtwo); tabone is current → tab 0 selected.
        EXPECT_EQ(tabs.numberOfTabViewItems, 2);
        ASSERT_NE(tabs.selectedTabViewItem, nil);
        EXPECT_EQ([tabs indexOfTabViewItem:tabs.selectedTabViewItem], 0);
    }

    TEST_F(apple_shell_seam, route_navigation_reconfigures_tab_host)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSTabView* const tabs = native_tab_host(handler);

        sh.go_to_async(shell_navigation_state{"//two/tabfour/"});
        ASSERT_EQ("//two/tabfour/content", maui::controls::shell_tests::shell_test_base::location_of(sh));

        // The NSTabView now hosts item two's sections (tabthree + tabfour), with tabfour (index 1) selected.
        EXPECT_EQ(tabs.numberOfTabViewItems, 2);
        EXPECT_EQ([tabs indexOfTabViewItem:tabs.selectedTabViewItem], 1);
        // The mirror tracks the same model state.
        EXPECT_EQ(handler->typed_platform_view()->tree.current_item_renderer.item, sh.current_item());
    }

    TEST_F(apple_shell_seam, flyout_is_presented_drives_sidebar_collapse)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSSplitViewController* const split = native_controller(handler);

        sh.set_flyout_is_presented(true);
        EXPECT_FALSE(split.splitViewItems[0].collapsed);
        EXPECT_TRUE(handler->typed_platform_view()->tree.flyout_presented);

        sh.set_flyout_is_presented(false);
        EXPECT_TRUE(split.splitViewItems[0].collapsed);
        EXPECT_FALSE(handler->typed_platform_view()->tree.flyout_presented);
    }
} // namespace

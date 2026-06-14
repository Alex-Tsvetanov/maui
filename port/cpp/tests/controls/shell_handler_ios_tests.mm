// iOS (UIKit) backend tests for the shell_handler seam (W3-32) — the on-simulator route→VC-stack e2e
// gate. The host is a real container UISplitViewController: the secondary column is the current item's
// UITabBarController (the tab host); each tab is a UINavigationController (the per-section renderer) whose
// viewControllers ARE the section's vc_stack (root content + pushed pages). The primary column is the
// pan-presented flyout drawer; FlyoutIsPresented drives the split's preferredDisplayMode.
//
// THE E2E: drive go_to("//route?...") through the shell_navigation_manager and assert the resulting real
// VC stack / active section matches the navigated model — i.e. route navigation actually reconfigures the
// native container. Compiled as Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell_handler.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::routing;
    using maui::controls::shell;
    using maui::controls::shell_item;
    using maui::controls::shell_navigation_state;
    using maui::core::shell_handler;

    UISplitViewController* native_controller(const std::shared_ptr<shell_handler>& handler)
    {
        return (__bridge UISplitViewController*)handler->typed_platform_view()->controller;
    }

    UITabBarController* native_tab_host(const std::shared_ptr<shell_handler>& handler)
    {
        return (__bridge UITabBarController*)handler->typed_platform_view()->tab_host;
    }

    class ios_shell_seam : public maui::controls::shell_tests::shell_test_base
    {
    protected:
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

    // The container is a double-column UISplitViewController: secondary = the tab host (the flyout drawer is
    // the primary column).
    TEST_F(ios_shell_seam, container_is_split_with_tab_host_as_secondary)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        UISplitViewController* const split = native_controller(handler);
        ASSERT_NE(split, nil);
        EXPECT_EQ([split viewControllerForColumn:UISplitViewControllerColumnSecondary], native_tab_host(handler));
        EXPECT_EQ((__bridge UIView*)handler->typed_platform_view()->native, split.view);
    }

    // The tab host (UITabBarController) hosts one UINavigationController per visible section; each nav's
    // viewControllers ARE the section's vc_stack (root content). The current section's tab is selected.
    TEST_F(ios_shell_seam, tab_host_has_one_nav_controller_per_section)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        UITabBarController* const tabs = native_tab_host(handler);
        ASSERT_NE(tabs, nil);
        ASSERT_EQ(tabs.viewControllers.count, 2U); // tabone + tabtwo
        EXPECT_EQ(tabs.selectedIndex, 0U);         // tabone is current
        // Each tab is a UINavigationController (the per-section renderer).
        ASSERT_TRUE([tabs.viewControllers[0] isKindOfClass:[UINavigationController class]]);
        UINavigationController* const nav0 = (UINavigationController*)tabs.viewControllers[0];
        // The root section has a single root VC (no pushed pages yet).
        EXPECT_EQ(nav0.viewControllers.count, 1U);
    }

    // THE E2E (route → VC-stack reconfiguration): go_to("//two/tabfour/") switches the model's current
    // item/section; the property mapper rebuilds, and the real UITabBarController now hosts item two's
    // sections with tabfour selected.
    TEST_F(ios_shell_seam, go_to_route_reconfigures_vc_stack)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UITabBarController* const tabs = native_tab_host(handler);
        ASSERT_EQ(tabs.selectedIndex, 0U);

        sh.go_to_async(shell_navigation_state{"//two/tabfour/"});
        ASSERT_EQ("//two/tabfour/content", maui::controls::shell_tests::shell_test_base::location_of(sh));

        // The real container reconfigured to item two (tabthree + tabfour), tabfour (index 1) selected.
        ASSERT_EQ(tabs.viewControllers.count, 2U);
        EXPECT_EQ(tabs.selectedIndex, 1U);
        EXPECT_EQ(handler->typed_platform_view()->tree.current_item_renderer.item, sh.current_item());
    }

    // THE E2E (a push route grows the per-section UINavigationController stack): go_to("Details?id=3")
    // pushes a page onto the current section; the active tab's UINavigationController now has 2 VCs (root +
    // Details), and the query parameter reached the model. This is the route-with-query e2e the task names.
    TEST_F(ios_shell_seam, go_to_route_with_query_pushes_and_grows_nav_stack)
    {
        routing::register_route<maui::controls::shell_tests::shell_test_page>("Details");

        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UITabBarController* const tabs = native_tab_host(handler);

        // Push "Details" with a query (id=3) onto the current section //one/tabone.
        sh.go_to_async(shell_navigation_state{"Details?id=3"}, false);
        ASSERT_EQ("//one/tabone/content/Details", maui::controls::shell_tests::shell_test_base::location_of(sh));

        // The selected tab's UINavigationController stack grew to 2 (root content + the pushed Details VC).
        UINavigationController* const active = (UINavigationController*)tabs.selectedViewController;
        ASSERT_TRUE([active isKindOfClass:[UINavigationController class]]);
        EXPECT_EQ(active.viewControllers.count, 2U);
        // The active section's model stack matches (slot 0 root marker + the pushed page).
        EXPECT_EQ(sh.current_section()->stack().size(), 2U);
    }

    TEST_F(ios_shell_seam, flyout_is_presented_drives_display_mode)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);

        sh.set_flyout_is_presented(true);
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeOneBesideSecondary);
        EXPECT_TRUE(handler->typed_platform_view()->tree.flyout_presented);

        sh.set_flyout_is_presented(false);
        EXPECT_EQ(split.preferredDisplayMode, UISplitViewControllerDisplayModeSecondaryOnly);
        EXPECT_FALSE(handler->typed_platform_view()->tree.flyout_presented);
    }
} // namespace

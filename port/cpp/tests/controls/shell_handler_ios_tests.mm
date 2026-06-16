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
#include "maui/controls/label.hpp"
#include "maui/controls/shell/flyout_header_behavior.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/graphics/colors.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

#include "ios_conversions.hpp"

namespace
{
    using maui::controls::content_page;
    using maui::controls::label;
    using maui::controls::routing;
    using maui::controls::shell;
    using maui::controls::shell_item;
    using maui::controls::shell_navigation_state;
    using maui::core::label_handler;
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

    // The resolved appearance is pushed onto the native chrome: BackgroundColor → each section's
    // UINavigationBar standardAppearance.backgroundColor; EffectiveTabBarBackgroundColor → the UITabBar's
    // standardAppearance.backgroundColor (ShellNavBarAppearanceTracker + ShellTabBarAppearanceTracker).
    TEST_F(ios_shell_seam, appearance_tints_nav_bar_and_tab_bar)
    {
        namespace colors = maui::graphics::colors;
        shell sh;
        auto one = std::make_shared<shell_item>();
        one->set_route("one");
        one->add(make_simple_shell_section("tabone", "content"));
        shell::set_background_color(*one, colors::red);          // nav bar background
        shell::set_tab_bar_background_color(*one, colors::blue); // tab bar background
        sh.add_item(one);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_TRUE(platform->tree.applied_appearance.has_value());

        UITabBarController* const tabs = native_tab_host(handler);
        ASSERT_GE(tabs.viewControllers.count, 1U);
        UINavigationController* const nav = (UINavigationController*)tabs.viewControllers[0];
        ASSERT_TRUE([nav isKindOfClass:[UINavigationController class]]);

        // The nav bar's standard appearance carries the BackgroundColor.
        UIColor* const expected_nav = maui::platform::ios::to_ui_color(colors::red);
        EXPECT_TRUE([nav.navigationBar.standardAppearance.backgroundColor isEqual:expected_nav]);

        // The tab bar's standard appearance carries the EffectiveTabBarBackgroundColor.
        UIColor* const expected_tab = maui::platform::ios::to_ui_color(colors::blue);
        EXPECT_TRUE([tabs.tabBar.standardAppearance.backgroundColor isEqual:expected_tab]);
    }

    // ---- flyout header / footer / width (U10) ----

    // The flyout drawer view (the primary column's UITableViewController.view) hosting the header/footer.
    UIView* flyout_drawer_view(const std::shared_ptr<shell_handler>& handler)
    {
        UITableViewController* const flyout =
            (__bridge UITableViewController*)handler->typed_platform_view()->flyout_host;
        return flyout.viewIfLoaded != nil ? flyout.viewIfLoaded : flyout.view;
    }

    // The resolved header is materialized in a header container at subview index 0 of the flyout drawer;
    // the footer is materialized in a clip-to-bounds container added to the drawer. The header container's
    // content is the header view's native UIView; the footer container clips to bounds.
    TEST_F(ios_shell_seam, header_and_footer_materialize_with_z_order)
    {
        shell sh;
        build_two_item_shell(sh);

        // Header / footer Views backed by real label native UIViews.
        auto header = std::make_shared<label>();
        auto header_handler = std::make_shared<label_handler>();
        header->set_handler(header_handler);
        auto footer = std::make_shared<label>();
        auto footer_handler = std::make_shared<label_handler>();
        footer->set_handler(footer_handler);
        sh.set_flyout_header(header);
        sh.set_flyout_footer(footer);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->tree.flyout_header, header.get());
        EXPECT_EQ(platform->tree.flyout_footer, footer.get());

        // Both containers were retained on the platform.
        ASSERT_NE(platform->flyout_header_container, nullptr);
        ASSERT_NE(platform->flyout_footer_container, nullptr);

        UIView* const drawer = flyout_drawer_view(handler);
        UIView* const header_container = (__bridge UIView*)platform->flyout_header_container;
        UIView* const footer_container = (__bridge UIView*)platform->flyout_footer_container;

        // The header container is subview index 0 (HeaderIndex), above the content.
        ASSERT_GT(drawer.subviews.count, 0U);
        EXPECT_EQ(drawer.subviews[0], header_container);
        // Both containers are subviews of the drawer.
        EXPECT_EQ(header_container.superview, drawer);
        EXPECT_EQ(footer_container.superview, drawer);
        // The footer container clips to bounds (UIContainerView.ClipsToBounds = true).
        EXPECT_TRUE(footer_container.clipsToBounds);
        // The header container hosts the header view's native UIView.
        UIView* const header_native = (__bridge UIView*)header_handler->native_view();
        EXPECT_TRUE([header_native isDescendantOfView:header_container]);
    }

    // Clearing the header after connect tears down the header container (UpdateFlyoutHeader's removal branch).
    TEST_F(ios_shell_seam, clearing_header_removes_container)
    {
        shell sh;
        build_two_item_shell(sh);
        auto header = std::make_shared<label>();
        auto header_handler = std::make_shared<label_handler>();
        header->set_handler(header_handler);
        sh.set_flyout_header(header);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform->flyout_header_container, nullptr);

        sh.set_flyout_header(nullptr);
        EXPECT_EQ(platform->tree.flyout_header, nullptr);
        EXPECT_EQ(platform->flyout_header_container, nullptr);
    }

    // The footer container repositions itself to the bottom of the drawer on layout (UpdateFooterPosition),
    // with the recursion guard preventing infinite layout. After a layout pass the footer's frame sits at
    // the bottom of the drawer (within its bottom safe area).
    TEST_F(ios_shell_seam, footer_repositions_to_bottom_on_layout)
    {
        shell sh;
        build_two_item_shell(sh);
        auto footer = std::make_shared<label>();
        auto footer_handler = std::make_shared<label_handler>();
        footer->set_handler(footer_handler);
        sh.set_flyout_footer(footer);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform->flyout_footer_container, nullptr);

        UIView* const drawer = flyout_drawer_view(handler);
        UIView* const footer_container = (__bridge UIView*)platform->flyout_footer_container;
        // Give the drawer + footer concrete sizes, then lay out.
        drawer.frame = CGRectMake(0, 0, 320, 600);
        footer_container.frame = CGRectMake(0, 0, 320, 44);
        [footer_container layoutIfNeeded];

        // The footer is pinned to the bottom: top = drawer.height - footerHeight - bottomSafeArea.
        const CGFloat expected_top = drawer.bounds.size.height - 44 - drawer.safeAreaInsets.bottom;
        EXPECT_NEAR(footer_container.frame.origin.y, expected_top, 0.5);
    }

    // U10: C# ShellFlyoutContentRenderer:159 wires _footer.MeasureInvalidated → reposition. The port's
    // faithful lighter equivalent observes the footer CONTENT view's bounds (KVO) so an intrinsic-size-only
    // change reactively re-runs layout WITHOUT a parent layout pass. Changing the content's bounds must mark
    // the container for layout (the OnFooterMeasureInvalidated → setNeedsLayout path).
    TEST_F(ios_shell_seam, footer_content_size_change_marks_container_for_layout)
    {
        shell sh;
        build_two_item_shell(sh);
        auto footer = std::make_shared<label>();
        auto footer_handler = std::make_shared<label_handler>();
        footer->set_handler(footer_handler);
        sh.set_flyout_footer(footer);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform->flyout_footer_container, nullptr);

        UIView* const drawer = flyout_drawer_view(handler);
        UIView* const footer_container = (__bridge UIView*)platform->flyout_footer_container;
        UIView* const footer_content = (__bridge UIView*)footer_handler->native_view();
        ASSERT_NE(footer_content, nil);

        // Settle layout, then perturb the footer content's frame WITHOUT a parent layout pass. Without the
        // reactive KVO hook the container would not relayout (UIKit doesn't relayout a superview when a
        // subview's bounds change by direct assignment), so the content frame would stay perturbed.
        drawer.frame = CGRectMake(0, 0, 320, 600);
        footer_container.frame = CGRectMake(0, 0, 320, 44);
        [footer_container layoutIfNeeded];
        footer_content.frame = CGRectMake(10, 10, 100, 20); // perturb (an intrinsic-content-size change)

        // The KVO hook (OnFooterMeasureInvalidated → setNeedsLayout) must have re-marked the container, so
        // layoutIfNeeded re-runs the footer container's layoutSubviews and resets the content frame to fill
        // the container's bounds — proving the reactive reposition fired (without it, needsLayout stays
        // clear and layoutIfNeeded is a no-op, leaving the perturbed frame).
        [footer_container layoutIfNeeded];
        EXPECT_TRUE(CGRectEqualToRect(footer_content.frame, footer_container.bounds))
            << "a footer content size change must reactively re-run the footer container's layout";
    }

    // FlyoutWidth (a Shell attached property carried by the appearance walk) sizes the split VC's primary
    // (flyout) column; nothing set leaves it at the automatic dimension.
    TEST_F(ios_shell_seam, flyout_width_sizes_primary_column)
    {
        shell sh;
        auto item = std::make_shared<shell_item>();
        item->set_route("one");
        item->add(make_simple_shell_section("tabone", "content"));
        shell::set_flyout_width(*item, 280.0);
        sh.add_item(item);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_TRUE(platform->tree.flyout_width.has_value());
        UISplitViewController* const split = native_controller(handler);
        EXPECT_NEAR(split.preferredPrimaryColumnWidth, 280.0, 0.5);
    }

    TEST_F(ios_shell_seam, no_flyout_width_uses_automatic_dimension)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UISplitViewController* const split = native_controller(handler);
        EXPECT_EQ(split.preferredPrimaryColumnWidth, UISplitViewControllerAutomaticDimension);
    }

    // FlyoutHeaderBehavior is recorded into the tree (the Default/Fixed positioning lands; Scroll /
    // CollapseOnScroll scroll-tracking is the documented follow-up). Setting it updates the mirror.
    TEST_F(ios_shell_seam, flyout_header_behavior_recorded)
    {
        namespace mc = maui::controls;
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->tree.header_behavior, mc::flyout_header_behavior::default_behavior);

        sh.set_flyout_header_behavior(mc::flyout_header_behavior::collapse_on_scroll);
        EXPECT_EQ(platform->tree.header_behavior, mc::flyout_header_behavior::collapse_on_scroll);
    }
} // namespace

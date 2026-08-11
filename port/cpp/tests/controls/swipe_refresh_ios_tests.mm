// iOS (UIKit) backend seam tests for swipe_view + refresh_view (run ON the iOS simulator via
// tools/ios-sim-run.sh): the content's real native view is hosted as a subview of the host UIView, the
// swipe state machine drives through the ios handler (the shared cross-platform machine; a documented
// pan-driven-reveal cut on a plain UIView host), the refresh host owns a REAL UIRefreshControl whose
// ValueChanged → IsRefreshing write-back fires the Refreshing event + command, and MapIsRefreshing
// drives beginRefreshing/endRefreshing. The content child is a button (its handler owns a real UIButton).
// Compiled as Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::refresh_view;
    using maui::controls::swipe_item;
    using maui::controls::swipe_items;
    using maui::controls::swipe_view;
    using maui::core::button_handler;
    using maui::core::open_swipe_item;
    using maui::core::refresh_view_handler;
    using maui::core::swipe_direction;
    using maui::core::swipe_machine_state;
    using maui::core::swipe_mode;
    using maui::core::swipe_view_handler;

    UIView* swipe_host(const std::shared_ptr<swipe_view_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }
    UIView* refresh_host(const std::shared_ptr<refresh_view_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    TEST(ios_swipe_refresh_seam, swipe_host_hosts_content_and_arranges)
    {
        swipe_view view;
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);
        EXPECT_TRUE([swipe_host(handler) isKindOfClass:[UIView class]]);

        button child;
        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge UIView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        view.set_content(child);
        EXPECT_EQ(swipe_host(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, swipe_host(handler));

        view.arrange(maui::graphics::rect(0, 0, 200, 120));
        EXPECT_EQ(swipe_host(handler).frame.size.width, 200.0);
    }

    // A SwipeView in a vertical stack must arrange to its CONTENT height (not an inflated value), and its
    // content's native view (a subview of the swipe host) must land HOST-RELATIVE — at the host's own
    // 0-origin, NOT at the row's absolute Y. Regression for the basic_swipe over-spacing bug: the swipe
    // arranged its content in ABSOLUTE page coordinates, so a row at y=72 in the stack drew its content at
    // y=144 (host origin + the same origin re-added), garbling the rows. Two stacked 60px-content swipes
    // are arranged through the cross-platform stack manager; assert both host heights, the inter-row gap
    // (= content 60 + spacing 12 = 72), and that the deeper row's content sits at the host's local origin.
    TEST(ios_swipe_refresh_seam, swipe_in_a_stack_arranges_to_content_height_host_relative)
    {
        using maui::controls::grid;
        using maui::controls::vertical_stack_layout;
        using maui::core::layout_handler;

        vertical_stack_layout stack;
        stack.set_spacing(12);
        auto stack_handler = std::make_shared<layout_handler>();

        grid g1;
        g1.set_height_request(60);
        g1.set_width_request(300);
        auto g1_handler = std::make_shared<layout_handler>();
        swipe_view s1;
        s1.set_height_request(60);
        auto s1_handler = std::make_shared<swipe_view_handler>();

        grid g2;
        g2.set_height_request(60);
        g2.set_width_request(300);
        auto g2_handler = std::make_shared<layout_handler>();
        swipe_view s2;
        s2.set_height_request(60);
        auto s2_handler = std::make_shared<swipe_view_handler>();

        // Attach handlers FIRST (bottom-up), then wire the tree through the control API so each add hosts
        // the native subview AND records the logical child the layout manager measures.
        g1.set_handler(g1_handler);
        g2.set_handler(g2_handler);
        s1.set_handler(s1_handler);
        s2.set_handler(s2_handler);
        stack.set_handler(stack_handler);
        s1.set_content(g1);
        s2.set_content(g2);
        stack.add(s1);
        stack.add(s2);

        stack.measure(402, 778);
        stack.arrange(maui::graphics::rect(0, 0, 402, 778));

        UIView* const h1 = swipe_host(s1_handler);
        UIView* const h2 = swipe_host(s2_handler);

        // Each swipe host arranges to its 60px content (NOT an inflated value).
        EXPECT_EQ(h1.frame.size.height, 60.0);
        EXPECT_EQ(h2.frame.size.height, 60.0);
        // Stacked compactly: row 2 begins 72px below row 1 (content 60 + spacing 12), not ~150px.
        EXPECT_EQ(h2.frame.origin.y - h1.frame.origin.y, 72.0);

        // The deeper row's content (a subview of the swipe host at y=72) must sit at the host's LOCAL
        // origin, not re-offset by the row's absolute Y. The double-offset bug put it at y=72 within its
        // own host. Padding is 0 here, so host-relative y is 0.
        ASSERT_EQ(h2.subviews.count, 1U);
        UIView* const content2 = h2.subviews.firstObject;
        EXPECT_EQ(content2.frame.origin.y, 0.0);
        EXPECT_EQ(content2.frame.size.height, 60.0);
    }

    // Guard: a plain box (a grid) in the SAME vertical stack alongside a SwipeView still arranges normally —
    // the host-relative content fix must not perturb non-swipe children.
    TEST(ios_swipe_refresh_seam, plain_box_beside_a_swipe_in_a_stack_is_unaffected)
    {
        using maui::controls::grid;
        using maui::controls::vertical_stack_layout;
        using maui::core::layout_handler;

        vertical_stack_layout stack;
        stack.set_spacing(12);
        auto stack_handler = std::make_shared<layout_handler>();

        grid content;
        content.set_height_request(60);
        content.set_width_request(300);
        auto content_handler = std::make_shared<layout_handler>();
        swipe_view s1;
        s1.set_height_request(60);
        auto s1_handler = std::make_shared<swipe_view_handler>();

        grid box; // a plain box, no swipe wrapper
        box.set_height_request(60);
        box.set_width_request(300);
        auto box_handler = std::make_shared<layout_handler>();

        content.set_handler(content_handler);
        s1.set_handler(s1_handler);
        box.set_handler(box_handler);
        stack.set_handler(stack_handler);
        s1.set_content(content);
        stack.add(s1);
        stack.add(box);

        stack.measure(402, 778);
        stack.arrange(maui::graphics::rect(0, 0, 402, 778));

        UIView* const swipe_native = swipe_host(s1_handler);
        auto* const box_native = (__bridge UIView*)box_handler->typed_platform_view()->native;
        EXPECT_EQ(swipe_native.frame.size.height, 60.0);
        EXPECT_EQ(box_native.frame.size.height, 60.0);
        // The plain box sits 72px below the swipe (content 60 + spacing 12) — unchanged by the swipe fix.
        EXPECT_EQ(box_native.frame.origin.y - swipe_native.frame.origin.y, 72.0);
    }

    TEST(ios_swipe_refresh_seam, programmatic_open_drives_the_state_machine)
    {
        swipe_item item;
        swipe_view view;
        view.left_items_collection().add(item);
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);

        view.open(open_swipe_item::left_items);
        EXPECT_EQ(handler->typed_platform_view()->state.state, swipe_machine_state::open);
        EXPECT_TRUE(view.is_open());
        view.close();
        EXPECT_FALSE(view.is_open());
    }

    TEST(ios_swipe_refresh_seam, synthetic_swipe_executes_an_item)
    {
        swipe_item item;
        int invoked = 0;
        item.invoked.connect([&invoked] { ++invoked; });
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items));
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80);
        handler->end_swipe();
        EXPECT_EQ(invoked, 1);
    }

    TEST(ios_swipe_refresh_seam, refresh_host_drives_refreshing_and_mirrors_color)
    {
        button child;
        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);

        refresh_view view;
        view.set_content(child);
        view.set_refresh_color(maui::graphics::colors::green);
        auto handler = std::make_shared<refresh_view_handler>();
        view.set_handler(handler);

        // The host owns a real UIRefreshControl (private to the handler); assert the observable effects.
        EXPECT_TRUE([refresh_host(handler) isKindOfClass:[UIView class]]);
        EXPECT_TRUE(handler->typed_platform_view()->has_refresh_color);
        EXPECT_EQ(handler->typed_platform_view()->refresh_color_argb, maui::graphics::colors::green.to_uint());

        view.set_is_refreshing(true); // MapIsRefreshing -> beginRefreshing + the mirror
        EXPECT_TRUE(handler->typed_platform_view()->refreshing);
        view.set_is_refreshing(false); // -> endRefreshing
        EXPECT_FALSE(handler->typed_platform_view()->refreshing);
    }

    // MauiRefreshView.TryInsertRefresh:132-180 recurses Subviews (:169-177) — the scroller does NOT have to
    // be the RefreshView's direct child. And :155 turns on AlwaysBounceVertical, without which a scroller
    // whose content is shorter than its frame cannot overscroll, so the pull never reaches the control.
    TEST(ios_swipe_refresh_seam, refresh_finds_a_nested_scroller_and_bounces)
    {
        using maui::controls::scroll_view;
        using maui::controls::vertical_stack_layout;
        using maui::core::layout_handler;
        using maui::core::scroll_view_handler;

        button child;
        auto child_handler = std::make_shared<button_handler>();
        scroll_view scroller;
        auto scroller_handler = std::make_shared<scroll_view_handler>();
        vertical_stack_layout wrapper; // the INTERMEDIATE view a direct-child-only check would miss
        auto wrapper_handler = std::make_shared<layout_handler>();
        refresh_view view;
        auto handler = std::make_shared<refresh_view_handler>();

        // Handlers first (bottom-up), then wire the tree so each set/add hosts a real native view.
        child.set_handler(child_handler);
        scroller.set_handler(scroller_handler);
        wrapper.set_handler(wrapper_handler);
        view.set_handler(handler);
        scroller.set_content(child);
        wrapper.add(scroller);
        view.set_content(wrapper);

        auto* const native_scroller = (__bridge UIScrollView*)scroller_handler->typed_platform_view()->native;
        ASSERT_TRUE([native_scroller isKindOfClass:[UIScrollView class]]);
        EXPECT_NE(native_scroller.refreshControl, nil); // found through the wrapper
        EXPECT_TRUE(native_scroller.alwaysBounceVertical);
    }

    TEST(ios_swipe_refresh_seam, request_refresh_writes_back_and_runs_command)
    {
        refresh_view view;
        int refreshed = 0;
        view.refreshing.connect([&refreshed] { ++refreshed; });
        bool executed = false;
        view.set_command([&executed] { executed = true; });
        auto handler = std::make_shared<refresh_view_handler>();
        view.set_handler(handler);

        handler->request_refresh();
        EXPECT_TRUE(view.is_refreshing());
        EXPECT_EQ(refreshed, 1);
        EXPECT_TRUE(executed);
    }
} // namespace

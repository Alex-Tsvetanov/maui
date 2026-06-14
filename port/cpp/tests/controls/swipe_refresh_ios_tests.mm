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
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
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

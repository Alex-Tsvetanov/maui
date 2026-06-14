// Apple (AppKit) backend seam tests for swipe_view + refresh_view: the content's real native view is
// hosted as a subview of the host NSView (set_content), the host frames on arrange, the swipe state
// machine drives through the apple handler (a documented AppKit pan-driven-reveal deviation — the machine
// is the shared cross-platform port, the host is a plain NSView), and the refresh host mirrors
// IsRefreshing / the spinner color while request_refresh writes IsRefreshing back. Compiled as
// Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/label.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/label_handler.hpp"
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
    using maui::controls::label;
    using maui::controls::refresh_view;
    using maui::controls::swipe_item;
    using maui::controls::swipe_view;
    using maui::core::label_handler;
    using maui::core::open_swipe_item;
    using maui::core::refresh_view_handler;
    using maui::core::swipe_direction;
    using maui::core::swipe_machine_state;
    using maui::core::swipe_mode;
    using maui::core::swipe_view_handler;

    NSView* swipe_host(const std::shared_ptr<swipe_view_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }
    NSView* refresh_host(const std::shared_ptr<refresh_view_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    class apple_swipe_refresh_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_swipe_refresh_seam, swipe_host_is_an_nsview_and_hosts_content)
    {
        swipe_view view;
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([swipe_host(handler) isKindOfClass:[NSView class]]);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge NSView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        view.set_content(child);
        EXPECT_EQ(swipe_host(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, swipe_host(handler));
    }

    TEST_F(apple_swipe_refresh_seam, programmatic_open_drives_the_state_machine)
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
        EXPECT_EQ(handler->typed_platform_view()->state.state, swipe_machine_state::idle);
        EXPECT_FALSE(view.is_open());
    }

    TEST_F(apple_swipe_refresh_seam, synthetic_swipe_executes_an_item_through_the_apple_handler)
    {
        swipe_item item;
        int invoked = 0;
        item.invoked.connect([&invoked] { ++invoked; });

        auto items = std::make_unique<maui::controls::swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items)); // a LEFT swipe reveals the RIGHT items
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80); // |80| >= 60% of 100
        handler->end_swipe();

        EXPECT_EQ(invoked, 1);        // the apple handler drives the SAME shared machine
        EXPECT_FALSE(view.is_open()); // Auto behavior in Execute mode closes after invoke
    }

    TEST_F(apple_swipe_refresh_seam, refresh_host_mirrors_and_request_refresh_writes_back)
    {
        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);

        refresh_view view;
        view.set_content(child);
        view.set_refresh_color(maui::graphics::colors::green);
        auto handler = std::make_shared<refresh_view_handler>();
        view.set_handler(handler);

        EXPECT_TRUE([refresh_host(handler) isKindOfClass:[NSView class]]);
        EXPECT_EQ(refresh_host(handler).subviews.count, 1U);
        EXPECT_TRUE(handler->typed_platform_view()->has_refresh_color);
        EXPECT_EQ(handler->typed_platform_view()->refresh_color_argb, maui::graphics::colors::green.to_uint());

        int refreshed = 0;
        view.refreshing.connect([&refreshed] { ++refreshed; });
        handler->request_refresh();
        EXPECT_TRUE(view.is_refreshing());
        EXPECT_EQ(refreshed, 1);
        EXPECT_TRUE(handler->typed_platform_view()->refreshing);
    }

    TEST_F(apple_swipe_refresh_seam, arrange_sizes_the_swipe_host)
    {
        swipe_view view;
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);
        view.arrange(maui::graphics::rect(5, 10, 200, 120));
        const NSRect frame = swipe_host(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }
} // namespace

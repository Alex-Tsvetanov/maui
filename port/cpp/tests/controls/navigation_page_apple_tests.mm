// Apple (AppKit) backend tests for the navigation_page seam — after a push, the current page's real
// native NSView is the navigation container's single visible subview (and the previously-current page's
// view is removed); pop/pop_to_root swap it back. The container is a plain NSView; each page here is a
// content_page whose handler owns a real NSView host (its native_view()), so the container hosts that.
// Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::navigation_page;
    using maui::core::content_page_handler;
    using maui::core::navigation_page_handler;

    NSView* native_container(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    // A content_page with its handler attached, so it owns a real native NSView host the container can
    // host. Returns the page's native NSView for superview assertions.
    NSView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge NSView*)page_handler->native_view();
    }

    class apple_navigation_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_navigation_page_seam, container_is_an_nsview)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_container(handler) isKindOfClass:[NSView class]]);
        EXPECT_EQ(native_container(handler).subviews.count, 0U);
    }

    TEST_F(apple_navigation_page_seam, push_hosts_the_current_pages_native_view)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        ASSERT_NE(root_native, nil);

        nav.push(root); // -> request_navigation -> the container hosts the root's NSView

        EXPECT_EQ(native_container(handler).subviews.count, 1U);
        EXPECT_EQ(root_native.superview, native_container(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &root);
    }

    TEST_F(apple_navigation_page_seam, push_swaps_the_visible_subview)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        content_page second;
        NSView* const second_native = attach_page(second);

        nav.push(root);
        EXPECT_EQ(root_native.superview, native_container(handler));

        nav.push(second); // the previous page's view leaves, the new one is hosted
        EXPECT_EQ(native_container(handler).subviews.count, 1U);
        EXPECT_EQ(second_native.superview, native_container(handler));
        EXPECT_EQ(root_native.superview, nil);
    }

    TEST_F(apple_navigation_page_seam, pop_restores_the_revealed_pages_view)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        content_page second;
        NSView* const second_native = attach_page(second);

        nav.push(root);
        nav.push(second);
        EXPECT_EQ(second_native.superview, native_container(handler));

        nav.pop(); // the popped page leaves; the revealed root is hosted again
        EXPECT_EQ(native_container(handler).subviews.count, 1U);
        EXPECT_EQ(root_native.superview, native_container(handler));
        EXPECT_EQ(second_native.superview, nil);
    }

    TEST_F(apple_navigation_page_seam, arrange_sizes_the_container_and_the_current_page)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        nav.push(root);

        nav.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> platform_arrange frames container + page

        const NSRect container_frame = native_container(handler).frame;
        EXPECT_EQ(container_frame.origin.x, 5.0);
        EXPECT_EQ(container_frame.origin.y, 10.0);
        EXPECT_EQ(container_frame.size.width, 200.0);
        EXPECT_EQ(container_frame.size.height, 120.0);

        // The current page fills the container (origin 0,0 in the container's coordinate space).
        const NSRect page_frame = root_native.frame;
        EXPECT_EQ(page_frame.origin.x, 0.0);
        EXPECT_EQ(page_frame.origin.y, 0.0);
        EXPECT_EQ(page_frame.size.width, 200.0);
        EXPECT_EQ(page_frame.size.height, 120.0);
    }
} // namespace

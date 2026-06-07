// Apple (AppKit) backend tests for the content_page seam — the content's real native view becomes a
// subview of the host NSView after set_content (and leaves when the content is replaced/cleared), and
// the host frames correctly on arrange. The host is a plain NSView container; the content here is a
// label (its handler owns a real NSTextField). Compiled as Objective-C++ with ARC for the `apple`
// backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::label;
    using maui::core::content_page_handler;
    using maui::core::label_handler;

    NSView* native_host(const std::shared_ptr<content_page_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    class apple_content_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_content_page_seam, host_is_an_nsview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_host(handler) isKindOfClass:[NSView class]]);
        EXPECT_EQ(native_host(handler).subviews.count, 0U);
    }

    TEST_F(apple_content_page_seam, content_becomes_a_subview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        // A content child with its own native view (a label-backed NSTextField). native_view() returns
        // the real NSTextField the handler's pimpl owns (platform_view() would return the pimpl pointer).
        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge NSView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        page.set_content(child); // -> handler->invoke("set_content") -> map_set_content -> addSubview:

        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_host(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);
    }

    TEST_F(apple_content_page_seam, replacing_content_swaps_the_subview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        label first;
        auto first_handler = std::make_shared<label_handler>();
        first.set_handler(first_handler);
        auto* const first_native = (__bridge NSView*)first_handler->native_view();

        label second;
        auto second_handler = std::make_shared<label_handler>();
        second.set_handler(second_handler);
        auto* const second_native = (__bridge NSView*)second_handler->native_view();

        page.set_content(first);
        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(first_native.superview, native_host(handler));

        page.set_content(second); // the old content leaves, the new one is hosted
        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(second_native.superview, native_host(handler));
        EXPECT_EQ(first_native.superview, nil);

        page.set_content(nullptr); // clearing empties the host
        EXPECT_EQ(native_host(handler).subviews.count, 0U);
    }

    TEST_F(apple_content_page_seam, arrange_sizes_the_host)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        page.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the host

        const NSRect frame = native_host(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }
} // namespace

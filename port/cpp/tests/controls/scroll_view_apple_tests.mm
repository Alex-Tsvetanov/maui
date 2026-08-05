// Apple (AppKit) backend tests for the scroll_view seam — the host is a real NSScrollView: the
// content's native view becomes the documentView, the orientation + scroll-bar visibilities drive the
// scroller knobs, and a scroll_to clamps + moves the clip origin, writes the offsets back through the
// bounds-change proxy (raising Scrolled), and acknowledges completion. Compiled as Objective-C++ with
// ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::controls::scroll_view;
    using maui::core::label_handler;
    using maui::core::scroll_bar_visibility;
    using maui::core::scroll_orientation;
    using maui::core::scroll_view_handler;

    NSScrollView* native_scroller(const std::shared_ptr<scroll_view_handler>& handler)
    {
        return (__bridge NSScrollView*)handler->typed_platform_view()->native;
    }

    class apple_scroll_view_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_scroll_view_seam, host_is_an_nsscrollview)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_scroller(handler) isKindOfClass:[NSScrollView class]]);
        // The Vertical default: a vertical scroller, no horizontal one.
        EXPECT_TRUE(native_scroller(handler).hasVerticalScroller);
        EXPECT_FALSE(native_scroller(handler).hasHorizontalScroller);
    }

    TEST_F(apple_scroll_view_seam, content_becomes_the_document_view)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);
        EXPECT_EQ(native_scroller(handler).documentView, nil);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge NSView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        scroller.set_content(child);
        EXPECT_EQ(native_scroller(handler).documentView, child_native);
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);

        scroller.set_content(nullptr);
        EXPECT_EQ(native_scroller(handler).documentView, nil);
    }

    TEST_F(apple_scroll_view_seam, orientation_and_bar_visibility_drive_the_scroller_knobs)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        scroller.set_orientation(scroll_orientation::horizontal);
        EXPECT_TRUE(native_scroller(handler).hasHorizontalScroller);
        EXPECT_FALSE(native_scroller(handler).hasVerticalScroller);

        scroller.set_vertical_scroll_bar_visibility(scroll_bar_visibility::always);
        EXPECT_TRUE(native_scroller(handler).hasVerticalScroller); // pinned despite the orientation
        EXPECT_FALSE(native_scroller(handler).autohidesScrollers);

        scroller.set_vertical_scroll_bar_visibility(scroll_bar_visibility::never);
        EXPECT_FALSE(native_scroller(handler).hasVerticalScroller);
        EXPECT_TRUE(native_scroller(handler).autohidesScrollers);
    }

    TEST_F(apple_scroll_view_seam, scroll_to_moves_the_clip_origin_and_writes_back)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        scroller.set_content(child);

        // Frame the scroller to a 100x100 viewport and the document to 100x1000 (the control's
        // unbounded arrange would do the same; framed directly here to keep the native sizes exact).
        [native_scroller(handler) setFrame:NSMakeRect(0, 0, 100, 100)];
        [native_scroller(handler).documentView setFrame:NSMakeRect(0, 0, 100, 1000)];

        int completed = 0;
        scroller.scroll_to_completed.connect([&completed] { ++completed; });

        scroller.scroll_to_async(0, 200, false);
        EXPECT_EQ(native_scroller(handler).contentView.bounds.origin.y, 200.0);
        EXPECT_EQ(scroller.scroll_y(), 200.0); // the bounds-change proxy wrote the offset back
        EXPECT_EQ(completed, 1);
        ASSERT_EQ(handler->typed_platform_view()->scroll_requests.size(), 1U);
        EXPECT_TRUE(handler->typed_platform_view()->scroll_requests[0].instant);

        // A target beyond the range clamps to documentSize - viewport (1000 - 100 = 900).
        scroller.scroll_to_async(0, 5000, false);
        EXPECT_EQ(native_scroller(handler).contentView.bounds.origin.y, 900.0);
        EXPECT_EQ(scroller.scroll_y(), 900.0);
        EXPECT_EQ(completed, 2);
    }

    // LIFETIME. The NSScrollView outlives the handler in any real app — a superview retains it — and
    // the MauiScrollViewProxy it keeps in its associated objects carries a RAW scroll_view_handler* and
    // stays subscribed to the clip view's bounds-change notifications. Nothing calls disconnect_handler()
    // when a handler is merely destroyed (there is no ~view_handler doing it), so the unhook has to
    // happen in ~scroll_view_platform. Without it the next scroll dereferences freed memory:
    // heap-use-after-free READ at scroll_view_handler.mm:55 in -[MauiScrollViewProxy boundsDidChange:].
    // The ARC local below is the superview stand-in; dropping the local shared_ptr is what lets the
    // handler die (a held one is why every other test in this file misses this).
    TEST_F(apple_scroll_view_seam, scrolling_a_scroller_that_outlived_its_handler_is_inert)
    {
        NSScrollView* native = nil;
        {
            scroll_view scroller;
            scroller.set_handler(std::shared_ptr<scroll_view_handler>(new scroll_view_handler()));
            auto* const handler = dynamic_cast<scroll_view_handler*>(scroller.handler().get());
            ASSERT_NE(handler, nullptr);
            native = (__bridge NSScrollView*)handler->typed_platform_view()->native; // ARC retains it here
        } // scroller + handler + platform all die; `native` survives

        [native.contentView setBoundsOrigin:NSMakePoint(0, 50)]; // the scroll a live superview still delivers
        SUCCEED();                                               // no ASan report IS the assertion
    }

    // ORDERING. The write-back raises `scrolled` TWICE (once per axis, as C# does at
    // ScrollViewHandler.iOS.cs:247-248), and the first raise is user code that may destroy the scroll
    // view — which frees the handler, the platform and the cached `view` pointer. The second axis must
    // not run against any of it. An x AND y move is required: a y-only move makes the horizontal write
    // a no-op, so the first raise never happens and the hazard is invisible.
    TEST_F(apple_scroll_view_seam, a_scrolled_handler_may_destroy_the_view_between_the_two_axes)
    {
        auto* scroller = new scroll_view();
        scroller->set_handler(std::shared_ptr<scroll_view_handler>(new scroll_view_handler()));
        auto* const handler = dynamic_cast<scroll_view_handler*>(scroller->handler().get());
        ASSERT_NE(handler, nullptr);
        NSScrollView* const native = (__bridge NSScrollView*)handler->typed_platform_view()->native;

        scroller->scrolled.connect([&scroller](double, double) {
            delete scroller;
            scroller = nullptr;
        });
        [native.contentView setBoundsOrigin:NSMakePoint(150, 200)];
        EXPECT_EQ(scroller, nullptr);
    }
} // namespace

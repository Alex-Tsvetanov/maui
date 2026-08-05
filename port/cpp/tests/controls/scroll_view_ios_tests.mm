// iOS (UIKit) backend tests for the scroll_view seam — the on-simulator twin of
// scroll_view_apple_tests.mm: the host is a real UIScrollView, the content's native view becomes the
// TAGGED scrollable subview (MauiScrollView.ContentTag), orientation drives scrollEnabled and the bar
// visibilities the indicators, platform_arrange pushes the ContentSize from the unbounded-arranged
// content, and an INSTANT scroll_to clamps + moves the contentOffset, writes the offsets back through
// the delegate (raising Scrolled), and acknowledges completion synchronously (the C# instant path; an
// animated scroll completes via scrollViewDidEndScrollingAnimation, which needs a pumped run loop).
// Compiled as Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::controls::scroll_view;
    using maui::core::label_handler;
    using maui::core::scroll_bar_visibility;
    using maui::core::scroll_orientation;
    using maui::core::scroll_view_handler;
    using maui::graphics::rect;

    UIScrollView* native_scroller(const std::shared_ptr<scroll_view_handler>& handler)
    {
        return (__bridge UIScrollView*)handler->typed_platform_view()->native;
    }

    TEST(ios_scroll_view_seam, host_is_a_uiscrollview)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_scroller(handler) isKindOfClass:[UIScrollView class]]);
        EXPECT_TRUE(native_scroller(handler).scrollEnabled); // Vertical default + enabled
    }

    TEST(ios_scroll_view_seam, content_becomes_the_tagged_subview)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge UIView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        scroller.set_content(child);
        EXPECT_EQ(child_native.superview, native_scroller(handler));
        EXPECT_EQ(child_native.tag, 0x845fed); // MauiScrollView.ContentTag
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);

        scroller.set_content(nullptr);
        EXPECT_EQ(child_native.superview, nil);
    }

    TEST(ios_scroll_view_seam, orientation_and_bar_visibility_drive_the_scroller)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        scroller.set_orientation(scroll_orientation::neither);
        EXPECT_FALSE(native_scroller(handler).scrollEnabled); // UpdateIsEnabled: Neither disables

        scroller.set_orientation(scroll_orientation::both);
        EXPECT_TRUE(native_scroller(handler).scrollEnabled);

        scroller.set_horizontal_scroll_bar_visibility(scroll_bar_visibility::never);
        EXPECT_FALSE(native_scroller(handler).showsHorizontalScrollIndicator);
        scroller.set_horizontal_scroll_bar_visibility(scroll_bar_visibility::always);
        EXPECT_TRUE(native_scroller(handler).showsHorizontalScrollIndicator);
    }

    TEST(ios_scroll_view_seam, arrange_pushes_the_content_size_and_scroll_to_writes_back)
    {
        scroll_view scroller;
        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        // Explicit size requests make the label's desired size 100x1000 (resolve_size_request), so
        // the control's ArrangeContentUnbounded overflows the viewport — the headless oracle's flow.
        child.set_width_request(100);
        child.set_height_request(1000);
        scroller.set_content(child);

        // The control's measure + unbounded arrange: viewport 100x100, content 100x1000.
        scroller.measure(100, 100);
        scroller.arrange(rect(0, 0, 100, 100));
        EXPECT_EQ(native_scroller(handler).contentSize.height, 1000.0);
        EXPECT_EQ(native_scroller(handler).contentSize.width, 100.0);

        int completed = 0;
        scroller.scroll_to_completed.connect([&completed] { ++completed; });

        scroller.scroll_to_async(0, 200, false); // instant
        EXPECT_EQ(native_scroller(handler).contentOffset.y, 200.0);
        EXPECT_EQ(scroller.scroll_y(), 200.0); // the delegate wrote the offset back
        EXPECT_EQ(completed, 1);
        ASSERT_EQ(handler->typed_platform_view()->scroll_requests.size(), 1U);
        EXPECT_TRUE(handler->typed_platform_view()->scroll_requests[0].instant);

        // A target beyond the range clamps to ContentSize - Frame (1000 - 100 = 900).
        scroller.scroll_to_async(0, 5000, false);
        EXPECT_EQ(native_scroller(handler).contentOffset.y, 900.0);
        EXPECT_EQ(scroller.scroll_y(), 900.0);
        EXPECT_EQ(completed, 2);
    }

    // LIFETIME (the on-simulator twin of apple_scroll_view_seam's two lifetime tests; both defects were
    // REPRODUCED under ASan on the apple lane, which shares this trampoline shape one file over).
    // The UIScrollView outlives the handler in any real app — a superview retains it — and the
    // MauiScrollViewDelegate it keeps in its associated objects carries a RAW scroll_view_handler*.
    // Nothing calls disconnect_handler() when a handler is merely destroyed (there is no ~view_handler
    // doing it), so the unhook has to happen in ~scroll_view_platform. The ARC local is the superview
    // stand-in; dropping the local shared_ptr is what lets the handler die.
    TEST(ios_scroll_view_seam, scrolling_a_scroller_that_outlived_its_handler_is_inert)
    {
        UIScrollView* native = nil;
        {
            scroll_view scroller;
            scroller.set_handler(std::shared_ptr<scroll_view_handler>(new scroll_view_handler()));
            auto* const handler = dynamic_cast<scroll_view_handler*>(scroller.handler().get());
            ASSERT_NE(handler, nullptr);
            native = (__bridge UIScrollView*)handler->typed_platform_view()->native; // ARC retains it here
            native.frame = CGRectMake(0, 0, 100, 100);
            native.contentSize = CGSizeMake(100, 1000);
        } // scroller + handler + platform all die; `native` survives

        native.contentOffset = CGPointMake(0, 50); // the scroll a live superview still delivers
        SUCCEED();                                 // no ASan report IS the assertion
    }

    // ORDERING. The write-back raises `scrolled` TWICE, one per axis (as ScrollViewHandler.iOS.cs:247-248
    // does), and the first raise is user code that may destroy the scroll view — freeing the handler, the
    // platform and the cached `view`. An x AND y move is required: a y-only move makes the horizontal
    // write a no-op, so the first raise never happens and the hazard is invisible.
    TEST(ios_scroll_view_seam, a_scrolled_handler_may_destroy_the_view_between_the_two_axes)
    {
        auto* scroller = new scroll_view();
        scroller->set_handler(std::shared_ptr<scroll_view_handler>(new scroll_view_handler()));
        auto* const handler = dynamic_cast<scroll_view_handler*>(scroller->handler().get());
        ASSERT_NE(handler, nullptr);
        UIScrollView* const native = (__bridge UIScrollView*)handler->typed_platform_view()->native;
        native.frame = CGRectMake(0, 0, 100, 100);
        native.contentSize = CGSizeMake(1000, 1000);

        scroller->scrolled.connect([&scroller](double, double) {
            delete scroller;
            scroller = nullptr;
        });
        native.contentOffset = CGPointMake(150, 200);
        EXPECT_EQ(scroller, nullptr);
    }
} // namespace

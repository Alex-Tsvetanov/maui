// Apple (AppKit) backend tests for the window_handler seam — the real-native half of the native NSWindow
// host, run only for MAUI_BACKEND=apple. Drives a genuine NSWindow: Title maps to NSWindow.title, Content
// maps to NSWindow.contentView (the root page's native view), the geometry maps to setFrame:, and the
// window's notifications (NSWindowDidBecomeMain / NSWindowWillClose) flow back through the
// NSWindowDelegate trampoline to the window lifecycle (send_activated / send_destroying). AppKit objects
// construct without being shown, so the tests post the notifications directly rather than ordering the
// window front + running a loop. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::window;
    using maui::core::content_page_handler;
    using maui::core::window_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSWindow* native_window(const std::shared_ptr<window_handler>& handler)
    {
        return (__bridge NSWindow*)handler->typed_platform_view()->native;
    }

    // NSWindow creation needs the shared application object (no run loop required).
    class apple_window_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_window_seam, attaching_handler_creates_nswindow_and_maps_title)
    {
        window win;
        win.set_title("Sample");
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_window(handler) isKindOfClass:[NSWindow class]]);
        EXPECT_EQ(to_std_string(native_window(handler).title), "Sample");
    }

    TEST_F(apple_window_seam, content_view_is_the_root_pages_native_view)
    {
        // The page hosts a label as its content; the page itself is the window's content. The window's
        // contentView must be the page's native host NSView (content_page_handler's host).
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        auto* const page_native = (__bridge NSView*)page_handler->native_view();
        ASSERT_NE(page_native, nil);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler); // MapContent: contentView = the page's native view

        EXPECT_EQ(native_window(handler).contentView, page_native);
        EXPECT_TRUE(handler->typed_platform_view()->content_hosted);
    }

    TEST_F(apple_window_seam, title_change_repushes_to_the_nswindow)
    {
        window win;
        win.set_title("First");
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        EXPECT_EQ(to_std_string(native_window(handler).title), "First");

        win.set_title("Second");
        EXPECT_EQ(to_std_string(native_window(handler).title), "Second");
    }

    TEST_F(apple_window_seam, did_become_main_notification_activates_the_window)
    {
        // The NSWindowDelegate trampoline maps NSWindowDidBecomeMainNotification → window::send_activated,
        // which Loads + Appears the hosted page. Posting the notification stands in for showing the window.
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        ASSERT_FALSE(win.is_activated());
        ASSERT_FALSE(page.has_appeared());

        [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowDidBecomeMainNotification
                                                            object:native_window(handler)];

        EXPECT_TRUE(win.is_activated());           // trampoline → send_activated
        EXPECT_TRUE(page.has_appeared());          // window-rooted Appearing fired
        EXPECT_EQ(page.containing_window(), &win); // the page knows its window
    }

    TEST_F(apple_window_seam, will_close_notification_destroys_the_window)
    {
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // Activate first (so destroying has something to deactivate).
        [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowDidBecomeMainNotification
                                                            object:native_window(handler)];
        ASSERT_TRUE(win.is_activated());

        int destroying = 0;
        win.destroying.connect([&destroying] { ++destroying; });

        [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowWillCloseNotification
                                                            object:native_window(handler)];

        EXPECT_EQ(destroying, 1);         // trampoline → send_destroying
        EXPECT_FALSE(win.is_activated()); // destroying deactivates first (page Disappears/Unloads)
        EXPECT_FALSE(win.is_created());
    }

    TEST_F(apple_window_seam, geometry_set_moves_and_sizes_the_nswindow)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        win.set_x(120);
        win.set_y(140);
        win.set_width(640);
        win.set_height(480); // the last set triggers apply_frame with a fully-set (non-NaN) frame

        const NSRect frame = native_window(handler).frame;
        EXPECT_EQ(frame.origin.x, 120.0);
        EXPECT_EQ(frame.origin.y, 140.0);
        EXPECT_EQ(frame.size.width, 640.0);
        EXPECT_EQ(frame.size.height, 480.0);
    }

    TEST_F(apple_window_seam, frame_changed_does_not_re_push_to_the_window)
    {
        // FrameChanged sets the geometry at the handler specificity within a batch that suppresses the
        // re-push (Window._batchFrameUpdate), so the native window is NOT moved by a platform frame report
        // (it already has that frame). Just assert the window survives the call and the values land.
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        win.frame_changed(maui::graphics::rect(5, 6, 7, 8));
        EXPECT_EQ(win.frame(), maui::graphics::rect(5, 6, 7, 8));
        EXPECT_TRUE([native_window(handler) isKindOfClass:[NSWindow class]]);
    }
} // namespace

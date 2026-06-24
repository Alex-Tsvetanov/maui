// iOS (UIKit) backend tests for the window_handler seam — the real-native twin of the headless
// window_handler_tests.cpp, run only for MAUI_BACKEND=ios (executed ON the iOS simulator via
// tools/ios-sim-run.sh). Drives a genuine UIWindow: connect makes it key and visible
// (ApplicationExtensions.CreatePlatformWindow) and reports its bounds back to the virtual window
// (WindowHandler.UpdateVirtualViewFrame), Title maps to the window scene's title
// (WindowExtensions.UpdateTitle), Content maps to a container root view controller hosting the content's
// native view (MapContent/ContainerViewController), a developer geometry set re-reports the platform
// bounds instead of moving the window (WindowExtensions.UpdateCoordinates), native frame changes flow
// back through the "frame" KVO (FrameObserverProxy), and the UIApplication lifecycle notifications drive
// the window lifecycle (AppHostBuilderExtensions.iOS's non-scene map).
//
// SIMULATOR-SPAWN NOTES (all verified empirically on the iOS 26 simulator): a UIWindow can be created,
// keyed (makeKeyAndVisible) and given a rootViewController in a spawned process with NO UIApplication;
// the spawned process's windows carry a placeholder UIWindowScene whose title is settable; and KVO on
// "frame" fires on setFrame:. The UIApplication lifecycle notifications, however, are only POSTED by a
// real application run loop (UIApplicationMain — see src/platform/ios/host_run.mm), so the tests post
// them to the default NSNotificationCenter directly — the same stand-in the apple twin uses for
// NSWindowDidBecomeMainNotification, exercising the very observers the handler registered.
//
// The window CONTENT here is a button: IWindow.Content is an IView (any view hosts through C#'s
// ToUIViewController/ContainerViewController), and the button is the control whose REAL UIKit partial
// exists in the M6 scaffold — the content_page's UIKit partial is a separate fan-out unit, so a
// content_page's native view is checked adaptively (the page_content test below) rather than assumed.
// Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::controls::window;
    using maui::core::button_handler;
    using maui::core::content_page_handler;
    using maui::core::window_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIWindow* native_window(const std::shared_ptr<window_handler>& handler)
    {
        return (__bridge UIWindow*)handler->typed_platform_view()->native;
    }

    // CGRect → the port's rect, for comparisons (CGRectEqualToRect lives in CoreGraphics, which the ios
    // build deliberately does not link — the handler code only uses UIKit-reachable CG inlines).
    maui::graphics::rect to_rect(CGRect value)
    {
        return {value.origin.x, value.origin.y, value.size.width, value.size.height};
    }

    void post_application_notification(NSNotificationName name)
    {
        // The real poster is the UIApplication singleton (absent in a spawned process); the handler
        // observes with object:nil, so a nil-object post reaches it exactly like the system's.
        [[NSNotificationCenter defaultCenter] postNotificationName:name object:nil];
    }

    TEST(ios_window_seam, attaching_handler_creates_a_key_visible_uiwindow_and_reports_its_bounds)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UIWindow* const native = native_window(handler);
        EXPECT_TRUE([native isKindOfClass:[UIWindow class]]);
        // ApplicationExtensions.CreatePlatformWindow: MakeKeyAndVisible right after the handler attach.
        EXPECT_TRUE(native.isKeyWindow);
        EXPECT_FALSE(native.hidden);
        // `new UIWindow()` adopts the main screen's bounds — a real, non-empty frame.
        EXPECT_GT(native.bounds.size.width, 0.0);
        EXPECT_GT(native.bounds.size.height, 0.0);
        // WindowHandler.UpdateVirtualViewFrame: the initial platform bounds were reported back, so the
        // virtual geometry converged on the platform truth (Window.X/Y/Width/Height at handler specificity).
        EXPECT_EQ(win.x(), native.bounds.origin.x);
        EXPECT_EQ(win.y(), native.bounds.origin.y);
        EXPECT_EQ(win.width(), native.bounds.size.width);
        EXPECT_EQ(win.height(), native.bounds.size.height);
    }

    TEST(ios_window_seam, title_maps_to_the_window_scene_and_repushes_on_change)
    {
        window win;
        win.set_title("Sample");
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // WindowExtensions.UpdateTitle: the title lives on the UIWindowScene (a UIWindow has no title of
        // its own). A spawned process's windows carry a placeholder scene (verified empirically), so the
        // push is observable here; the headless mirror tracks it either way.
        UIWindowScene* const scene = native_window(handler).windowScene;
        ASSERT_NE(scene, nil);
        EXPECT_EQ(to_std_string(scene.title), "Sample");
        EXPECT_EQ(handler->typed_platform_view()->title, "Sample");

        win.set_title("Second"); // OnPropertyChanged(Title) -> update_value("title") -> MapTitle
        EXPECT_EQ(to_std_string(scene.title), "Second");
        EXPECT_EQ(handler->typed_platform_view()->title, "Second");
    }

    TEST(ios_window_seam, content_with_a_native_view_is_hosted_under_the_root_view_controller)
    {
        // IWindow.Content is an IView — C#'s MapContent hosts ANY view through ToUIViewController's
        // ContainerViewController. A button brings the real UIKit native view available in this scaffold.
        button control;
        control.set_text("Tap");
        auto control_handler = std::make_shared<button_handler>();
        control.set_handler(control_handler);
        auto* const native_button = (__bridge UIButton*)control_handler->typed_platform_view()->native;
        ASSERT_NE(native_button, nil);

        window win;
        win.set_content(control);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler); // MapContent: RootViewController hosts the content's native view

        UIWindow* const native = native_window(handler);
        ASSERT_NE(native.rootViewController, nil);
        EXPECT_EQ(native_button.superview, native.rootViewController.view); // the container's subview
        EXPECT_TRUE(handler->typed_platform_view()->content_hosted);

        // ContainerViewController.ViewDidLayoutSubviews keeps the hosted view bounds-filling.
        [native.rootViewController.view layoutIfNeeded];
        EXPECT_EQ(to_rect(native_button.frame), to_rect(native.rootViewController.view.bounds));
    }

    TEST(ios_window_seam, a_window_without_content_still_installs_a_root_view_controller)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // The empty-host analog of the AppKit twin: a plain container controller, nothing hosted.
        EXPECT_NE(native_window(handler).rootViewController, nil);
        EXPECT_FALSE(handler->typed_platform_view()->content_hosted);
    }

    TEST(ios_window_seam, page_content_hosting_follows_the_pages_native_view)
    {
        // The content_page's UIKit partial is a separate M6 fan-out unit — on this scaffold the page
        // compiles its HEADLESS partial (no native UIView), so the host tracks whatever the page's
        // handler ACTUALLY provides: nothing today, the real page view once that unit lands. The window
        // lifecycle below is independent of the page's native view either way.
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        UIWindow* const native = native_window(handler);
        ASSERT_NE(native.rootViewController, nil);
        const bool page_has_native_view = page_handler->native_view() != nullptr;
        EXPECT_EQ(handler->typed_platform_view()->content_hosted, page_has_native_view);
        if (page_has_native_view)
        {
            auto* const page_view = (__bridge UIView*)page_handler->native_view();
            EXPECT_EQ(page_view.superview, native.rootViewController.view);
        }
    }

    TEST(ios_window_seam, did_become_active_notification_activates_the_window)
    {
        // AppHostBuilderExtensions.iOS (non-scene): OnActivated → Window.Activated. The lifecycle proxy
        // maps UIApplicationDidBecomeActiveNotification → window::send_activated, which Loads + Appears
        // the hosted page. Posting the notification stands in for the real application run loop.
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // Keying the window on connect does NOT activate (activation is application-level on iOS; the
        // port deliberately maps no UIWindow did-become-key signal).
        ASSERT_FALSE(win.is_activated());
        ASSERT_FALSE(page.has_appeared());

        post_application_notification(UIApplicationDidBecomeActiveNotification);

        EXPECT_TRUE(win.is_activated());           // proxy → send_activated
        EXPECT_TRUE(page.has_appeared());          // window-rooted Appearing fired
        EXPECT_EQ(page.containing_window(), &win); // the page knows its window
    }

    TEST(ios_window_seam, will_terminate_notification_destroys_the_window)
    {
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);

        window win;
        win.set_content(page);
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // Activate first (so destroying has something to deactivate).
        post_application_notification(UIApplicationDidBecomeActiveNotification);
        ASSERT_TRUE(win.is_activated());

        int destroying = 0;
        win.destroying.connect([&destroying] { ++destroying; });

        // AppHostBuilderExtensions.iOS (non-scene): WillTerminate → Window.Destroying.
        post_application_notification(UIApplicationWillTerminateNotification);

        EXPECT_EQ(destroying, 1);         // proxy → send_destroying
        EXPECT_FALSE(win.is_activated()); // destroying deactivates first (page Disappears/Unloads)
        EXPECT_FALSE(win.is_created());
    }

    TEST(ios_window_seam, foreground_and_background_notifications_resume_and_stop_the_window)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        int resumed = 0;
        int stopped = 0;
        win.resumed.connect([&resumed] { ++resumed; });
        win.stopped.connect([&stopped] { ++stopped; });

        // AppHostBuilderExtensions.iOS (non-scene): WillEnterForeground → Resumed, DidEnterBackground →
        // Stopped. (OnResignActivation → Deactivated is deliberately unmapped — see window_handler.mm.)
        post_application_notification(UIApplicationWillEnterForegroundNotification);
        EXPECT_EQ(resumed, 1);
        post_application_notification(UIApplicationDidEnterBackgroundNotification);
        EXPECT_EQ(stopped, 1);
    }

    TEST(ios_window_seam, geometry_set_reports_the_platform_bounds_back_instead_of_moving_the_window)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        UIWindow* const native = native_window(handler);
        const CGRect original = native.frame;
        ASSERT_EQ(win.width(), original.size.width); // converged on connect

        // WindowExtensions.UpdateCoordinates (plain iOS): a developer set cannot move/resize a UIWindow —
        // the update re-reports the platform bounds through FrameChanged, whose FromHandler values sit at
        // the TOP specificity (C# SetterSpecificity.FromHandler), so the manual 640 is immediately
        // overridden and the virtual geometry re-converges on the platform truth. (Same as C#: Width=640
        // → MapWidth → UpdateCoordinates → FrameChanged(Bounds) → SetValue(Width, bounds, FromHandler).)
        win.set_width(640);

        EXPECT_EQ(to_rect(native.frame), to_rect(original)); // the native window did not move
        EXPECT_EQ(win.width(), original.size.width);         // the bounds re-report overrode the set
        EXPECT_EQ(win.height(), original.size.height);       // still the platform truth
    }

    TEST(ios_window_seam, native_frame_change_flows_back_through_the_frame_observer)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);

        // FrameObserverProxy: KVO on "frame" → IWindow.FrameChanged with the window's new frame.
        [native_window(handler) setFrame:CGRectMake(10, 20, 320, 480)];

        EXPECT_EQ(win.frame(), maui::graphics::rect(10, 20, 320, 480));
    }

    TEST(ios_window_seam, frame_changed_does_not_re_push_to_the_window)
    {
        // FrameChanged sets the geometry at the handler specificity within a batch that suppresses the
        // re-push (Window._batchFrameUpdate), so the native window is NOT moved by a platform frame
        // report. Just assert the window survives the call and the values land.
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        const CGRect original = native_window(handler).frame;

        win.frame_changed(maui::graphics::rect(5, 6, 7, 8));

        EXPECT_EQ(win.frame(), maui::graphics::rect(5, 6, 7, 8));
        EXPECT_EQ(to_rect(native_window(handler).frame), to_rect(original));
    }

    TEST(ios_window_seam, disconnect_tears_down_the_lifecycle_observers)
    {
        window win;
        auto handler = std::make_shared<window_handler>();
        win.set_handler(handler);
        UIWindow* const native = native_window(handler); // keep the UIWindow alive past the disconnect

        win.set_handler(nullptr); // DisconnectHandler: un-observe + release the platform window
        EXPECT_EQ(handler->virtual_view(), nullptr);
        EXPECT_EQ(win.handler(), nullptr);

        // Neither the application notifications nor a native frame change reach the window any more.
        post_application_notification(UIApplicationDidBecomeActiveNotification);
        EXPECT_FALSE(win.is_activated());
        const maui::graphics::rect before = win.frame();
        [native setFrame:CGRectMake(1, 2, 3, 4)];
        EXPECT_EQ(win.frame(), before);
    }

    TEST(ios_window_seam, resolves_from_the_default_registry)
    {
        // window self-registers (MAUI_REGISTER_HANDLER in window.cpp), so hosting can create the handler.
        auto handler = maui::core::default_handler_registry().create_handler<maui::controls::window>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<window_handler*>(handler.get()), nullptr);
    }
} // namespace

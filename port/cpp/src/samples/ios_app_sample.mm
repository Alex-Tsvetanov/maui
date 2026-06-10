// ios_app_sample — the full M6 vertical slice on iOS: a REAL UIApplicationMain app whose delegate builds
// an application hosting a window hosting a content_page hosting a button, every layer wired through its
// handler, opened with app.open_window(win). The window_handler creates the UIWindow, hosts the content
// under its root view controller, makes it key and visible, and its lifecycle proxy receives the REAL
// UIApplication notifications (foreground/background round trips log through the cross-platform
// resumed/stopped events). Tapping the native UIButton flows back to the control's `clicked` event, which
// updates the button text (forward again through the mapper) — the whole virtual-view ⇄ handler ⇄ native
// seam end-to-end, this time under a genuine application run loop (the on-simulator unit tests run
// without one and post the notifications by hand).
//
// This is the UIKit twin of src/samples/maui_app_sample.mm; the AppDelegate stands in for its main()
// because iOS hands control to UIApplicationMain (the C# MauiUIApplicationDelegate shape), and it OWNS
// the element tree in C++ ivars (heap-owned shared_ptrs per PROFILE §8 — the delegate lives for the whole
// run loop). Build + run (ios preset; the target builds as a minimal .app bundle, installable on the
// booted simulator):
//   cmake --build --preset ios --target maui_ios_app_sample
//   xcrun simctl install booted build/ios/maui_ios_app_sample.app
//   xcrun simctl launch booted dev.maui-cpp.ios-app-sample
// The lifecycle traces go to the unified log (os_log):
//   xcrun simctl spawn booted log show --last 2m --predicate 'process == "maui_ios_app_sample"'
// Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>
#import <os/log.h>

#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/window_handler.hpp"

@interface MauiSampleAppDelegate : UIResponder <UIApplicationDelegate>
// The conventional non-scene UIApplicationDelegate window slot (the handler's pimpl owns the retain;
// this mirrors C#'s MauiUIApplicationDelegate.Window so UIKit sees a windowed classic-lifecycle app).
@property(nonatomic, strong) UIWindow* window;
@end

@implementation MauiSampleAppDelegate
{
    // The cross-platform element tree, owned by the delegate (alive for the whole run loop).
    std::shared_ptr<maui::controls::application> _app;
    std::shared_ptr<maui::controls::window> _win;
    std::shared_ptr<maui::controls::content_page> _page;
    std::shared_ptr<maui::controls::button> _control;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    (void)application;
    (void)launchOptions;

    _app = std::make_shared<maui::controls::application>();
    _win = std::make_shared<maui::controls::window>();
    _page = std::make_shared<maui::controls::content_page>();
    _control = std::make_shared<maui::controls::button>();

    _control->set_text("Click me");

    // Attach handlers bottom-up so each parent can host its child's native view: button → page → window.
    auto button_handler = std::make_shared<maui::core::button_handler>();
    _control->set_handler(button_handler);
    _page->set_content(*_control);

    auto page_handler = std::make_shared<maui::core::content_page_handler>();
    _page->set_handler(page_handler);

    _win->set_title("MAUI C++ — iOS sample");
    _win->set_content(*_page);
    auto window_handler = std::make_shared<maui::core::window_handler>();
    _win->set_handler(window_handler); // creates the UIWindow, hosts the content, makeKeyAndVisible

    // React to the native tap purely through the cross-platform API (a raw back-pointer, NOT the owning
    // shared_ptr — capturing the owner inside its own event would cycle, PROFILE §8).
    maui::controls::button* const control = _control.get();
    _control->clicked.connect([control] {
        static int count = 0;
        ++count;
        control->set_text("Clicked " + std::to_string(count));
        os_log(OS_LOG_DEFAULT, "[sample] button clicked: %d", count);
    });
    // The window lifecycle, driven by the REAL UIApplication notifications through the handler's proxy
    // (background the app and foreground it again to see the stopped/resumed round trip).
    _win->activated.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window activated"); });
    _win->resumed.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window resumed"); });
    _win->stopped.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window stopped"); });
    _win->destroying.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window destroying"); });

    // Open the window through the Application lifecycle: SendStart → Created → Activated (Loaded +
    // Appearing flow down the page subtree; the upcoming did-become-active notification then no-ops
    // idempotently, like C#'s OnActivated after CreateWindow).
    _app->open_window(*_win);

    auto* const native_window = (__bridge UIWindow*)window_handler->typed_platform_view()->native;
    auto* const native_button = (__bridge UIButton*)button_handler->typed_platform_view()->native;
    // The content_page's UIKit partial is a separate M6 fan-out unit — on this scaffold the page compiles
    // its HEADLESS partial (no native UIView), so the window's container hosts nothing yet and the
    // button's native view is parented manually. Once that unit lands, the page's native view carries the
    // button and this branch self-disables.
    if (native_button.superview == nil)
    {
        [native_window.rootViewController.view addSubview:native_button];
    }
    // Size + place the button within the host (the page does no auto-layout in this cut — the same
    // manual frame the macOS sample sets).
    [native_button setFrame:CGRectMake(60, 120, 280, 44)];

    self.window = native_window;
    return YES;
}
@end

int main(int argc, char* argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([MauiSampleAppDelegate class]));
    }
}

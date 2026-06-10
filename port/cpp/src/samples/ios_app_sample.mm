// ios_app_sample — the full M6 vertical slice on iOS, booted through the M-L HOSTING layer: a REAL
// UIApplicationMain app whose delegate builds a maui_app (maui_app_builder: use_maui_app<sample_app> —
// the subclass owns the window → content_page → button tree — + ConfigureLifecycleEvents), resolves
// every handler from the built handler table (attach_handler, bottom-up), and opens the window through
// maui_app::open_window. The window_handler creates the UIWindow, hosts the content under its root view
// controller, makes it key and visible, and its lifecycle proxy receives the REAL UIApplication
// notifications (foreground/background round trips log through the cross-platform resumed/stopped
// events AND the bridged lifecycle-service delegates). The delegate also drives the iOS lifecycle SHELL
// (ios_lifecycle.hpp): didFinishLaunching invokes the ios_finished_launching delegates registered via
// add_ios — the invoke side the C# platform delegate owns. Tapping the native UIButton flows back to
// the control's `clicked` event, which updates the button text — the whole virtual-view ⇄ handler ⇄
// native seam end-to-end under a genuine application run loop.
//
// This is the UIKit twin of src/samples/maui_app_sample.mm; the AppDelegate stands in for its main()
// because iOS hands control to UIApplicationMain (the C# MauiUIApplicationDelegate shape), and it OWNS
// the maui_app in a C++ ivar (which owns the application + tree — the delegate lives for the whole run
// loop). Build + run (ios preset; the target builds as a minimal .app bundle, installable on the booted
// simulator):
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
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/i_lifecycle_event_service.hpp"
#include "maui/hosting/ios_lifecycle.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/hosting/window_lifecycle_events.hpp"

namespace
{
    // The application subclass use_maui_app<TApp> mints: it owns the element tree (the C# App.xaml.cs
    // shape — Application.CreateWindow returns the window it manages).
    class sample_app final : public maui::controls::application
    {
    public:
        sample_app()
        {
            button_.set_text("Click me");
            page_.set_content(button_);
            window_.set_title("MAUI C++ — iOS sample");
            window_.set_content(page_);
        }

        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return &window_;
        }

        [[nodiscard]] maui::controls::window& sample_window()
        {
            return window_;
        }
        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }
        [[nodiscard]] maui::controls::button& button()
        {
            return button_;
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::button button_;
    };
} // namespace

@interface MauiSampleAppDelegate : UIResponder <UIApplicationDelegate>
// The conventional non-scene UIApplicationDelegate window slot (the handler's pimpl owns the retain;
// this mirrors C#'s MauiUIApplicationDelegate.Window so UIKit sees a windowed classic-lifecycle app).
@property(nonatomic, strong) UIWindow* window;
@end

@implementation MauiSampleAppDelegate
{
    // The built maui_app, owned by the delegate (alive for the whole run loop). It owns the
    // application, which owns the element tree (PROFILE §8 — destroyed before the tree, never after).
    std::unique_ptr<maui::hosting::maui_app> _mauiApp;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    (void)application;
    (void)launchOptions;

    // Builder boot: mint the app subclass, log the bridged window lifecycle, and register iOS-shell
    // delegates via add_ios (this delegate drives their invoke side below — the C# platform-delegate
    // role).
    _mauiApp = maui::hosting::maui_app::create_builder()
                   .use_maui_app<sample_app>()
                   .configure_lifecycle_events([](maui::hosting::i_lifecycle_builder& lifecycle) {
                       lifecycle.add_event(maui::hosting::window_lifecycle_events::created,
                                           [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window created"); });
                       lifecycle.add_event(maui::hosting::window_lifecycle_events::resumed,
                                           [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window resumed"); });
                       lifecycle.add_event(maui::hosting::window_lifecycle_events::stopped,
                                           [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window stopped"); });
                       maui::hosting::add_ios(lifecycle, [](maui::hosting::ios_lifecycle_builder& ios) {
                           ios.finished_launching(
                               [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: ios finished launching"); });
                       });
                   })
                   .build();
    sample_app* const app = _mauiApp->application_as<sample_app>().get();

    // Attach handlers from the built handler table, bottom-up: button → page → window (ToHandler).
    const auto button_handler =
        std::dynamic_pointer_cast<maui::core::button_handler>(_mauiApp->attach_handler(app->button()));
    _mauiApp->attach_handler(app->page());
    const auto window_handler =
        std::dynamic_pointer_cast<maui::core::window_handler>(_mauiApp->attach_handler(app->sample_window()));

    // React to the native tap purely through the cross-platform API (a raw back-pointer, NOT an owning
    // reference — the maui_app's application owns the tree; capturing an owner would cycle, PROFILE §8).
    maui::controls::button* const button = &app->button();
    button->clicked.connect([button] {
        static int count = 0;
        ++count;
        button->set_text("Clicked " + std::to_string(count));
        os_log(OS_LOG_DEFAULT, "[sample] button clicked: %d", count);
    });
    // The window lifecycle, driven by the REAL UIApplication notifications through the handler's proxy
    // (background the app and foreground it again to see the stopped/resumed round trip — both the raw
    // window events here and the bridged lifecycle delegates above fire).
    maui::controls::window& win = app->sample_window();
    win.activated.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window activated"); });
    win.destroying.connect([] { os_log(OS_LOG_DEFAULT, "[sample] window destroying"); });

    // Open the window through the hosting door: the lifecycle bridge connects FIRST, then the
    // application drives SendStart → Created → Activated (Loaded + Appearing flow down the page; the
    // upcoming did-become-active notification then no-ops idempotently, like C#'s OnActivated after
    // CreateWindow).
    _mauiApp->open_window(win);

    // Size + place the button within the page's native view (no auto-layout in this cut — the same
    // manual frame the macOS sample sets; the page's UIKit partial hosts the button's UIButton).
    auto* const native_window = (__bridge UIWindow*)window_handler->typed_platform_view()->native;
    auto* const native_button = (__bridge UIButton*)button_handler->typed_platform_view()->native;
    [native_button setFrame:CGRectMake(60, 120, 280, 44)];

    self.window = native_window;

    // Drive the iOS lifecycle SHELL's invoke side (the role the C# platform delegate plays).
    maui::hosting::invoke_events(_mauiApp->lifecycle_events(), maui::hosting::ios_lifecycle_events::finished_launching);
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

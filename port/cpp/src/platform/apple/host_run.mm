// run_app — the APPLE (macOS / AppKit) backend body of maui::hosting::run_app (host_run.hpp).
//
// The native-run-loop twin of src/platform/headless/host_run.cpp: instead of booting + one settle pass +
// return, it stands up a real NSApplication, builds the user's app through the SAME generic mount
// (app_host.hpp's mount_window + drive_layout — NO per-control knowledge), shows a real NSWindow, and
// enters [NSApp run]. The user writes ZERO Objective-C: all of the NSApplication / NSWindow / delegate glue
// that src/samples/maui_app_sample.mm + macos_gallery.mm hand-rolled now lives here, behind the pure-C++
// run_app seam — so the same pure-C++ samples/hello_world/main.cpp opens a native window on this backend.
//
// No C# class maps 1:1: this is the port's analog of what MAUI's MacCatalyst startup wraps (the
// MauiUIApplicationDelegate boot, the NSApplication run loop). The recipe (verified against the two sample
// mains): create NSApplication → an app delegate whose applicationDidFinishLaunching builds the app from
// the configurator, asks it for its window (IApplication.CreateWindow), generically mounts the window's
// element tree + opens it (mount_window), drives one layout over the window's content-view bounds
// (drive_layout), shows the NSWindow key + front, then [NSApp run].
//
// Lifetime (PROFILE §8): the built maui_app must outlive the whole run loop (it owns the application, which
// owns the window → page → control tree, and the handlers keep raw context pointers into it). It is held in
// a file-scope std::unique_ptr the delegate populates and never releases — the process exit tears it down.
//
// Build (apple preset): keyed into maui_hosting by CMake (the apple lane of the host_run generator
// expression); exactly one run_app links per backend. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>
#import <os/log.h>

#include <cstdio>
#include <exception>
#include <memory>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/host_run.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

namespace
{
    // The process-wide host state the AppKit-instantiated delegate must reach but we cannot pass to it
    // (AppKit creates the delegate, we never call its initializer). A single function-local-static struct
    // (a Meyers singleton) holds it — the idiomatic stand-in for a mutable global that keeps a single,
    // deterministically-constructed instance without a file-scope mutable variable. The same shape the iOS
    // lane uses for the UIApplicationMain hand-off.
    struct host_state
    {
        // The configurator from run_app(argc, argv, configure) — set once by run_app before [NSApp run],
        // read by the delegate's applicationDidFinishLaunching.
        maui::hosting::app_configurator configure = nullptr;
        // The built app, alive for the whole run loop (owns the application → window → tree; PROFILE §8).
        // Filled by the delegate and never freed (process exit tears it down, after the run loop, so the
        // maui_app outlives the windows it bridged — the maui_app.hpp teardown doctrine).
        std::unique_ptr<maui::hosting::maui_app> app;
    };

    host_state& state()
    {
        static host_state s; // constructed on first use, destroyed at process exit
        return s;
    }

    // The default macOS content size every page is measured/arranged into when the window has no explicit
    // geometry (the macos_gallery default). A native backend would derive this from the window frame; the
    // first layout pass uses this, and a future resize hook re-drives drive_layout at the new size.
    constexpr double k_window_width = 480.0;
    constexpr double k_window_height = 720.0;

    // Boot the app + show its window through the generic mount. Separated so a build()/mount throw surfaces
    // as a logged failure rather than escaping into AppKit. Returns true on a shown window, false otherwise.
    bool boot_window()
    {
        // (1) Build from a FRESH builder the user's configurator populates (use_maui_app<App>()).
        state().app = state().configure(maui::hosting::maui_app::create_builder()).build();

        // (2) Ask the application for its window (IApplication.CreateWindow — the user's create_window
        //     override). No application / no window ⇒ nothing to host.
        const std::shared_ptr<maui::controls::application>& application = state().app->application();
        if (application == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] no application configured (use_maui_app not called)");
            return false;
        }
        auto* const window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] application produced no window");
            return false;
        }

        // (3) Generic mount: attach handlers across the tree (children before parents), re-host each
        //     container, attach the window handler, open the window — the SAME driver the headless lane
        //     uses (app_host.hpp). The window_handler creates the real NSWindow + hosts the page in it.
        maui::hosting::mount_window(*state().app, *window);

        // (4) One layout pass over the window's content bounds (the window host does no auto-layout). On
        //     macOS the content view is unflipped, so a future resize hook re-drives this; the first pass
        //     settles the tree at the default size.
        maui::hosting::drive_layout(*window, k_window_width, k_window_height);

        // (4b) Install the relayout hook (window::request_relayout) AFTER the first pass — mirrors the
        //      Android-only jni/relayout.hpp precedent, generalized to every backend (see window.hpp's
        //      header comment). There is no resize hook on this lane yet (see the note above), so this is
        //      the only re-entry point today: a leaf's invalidate_measure() (e.g. a margin/orientation/
        //      SafeAreaEdges change post-boot) replays this SAME pass at the SAME fixed size.
        window->set_relayout_hook([window] { maui::hosting::drive_layout(*window, k_window_width, k_window_height); });

        // (5) Reach the native NSWindow through the window's now-attached handler and show it key + front.
        const auto window_handler = std::dynamic_pointer_cast<maui::core::window_handler>(window->handler());
        if (window_handler == nullptr || window_handler->typed_platform_view() == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] window handler did not produce a native NSWindow");
            return false;
        }
        auto* const ns_window = (__bridge NSWindow*)window_handler->typed_platform_view()->native;
        if (ns_window == nil)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] native NSWindow is nil");
            return false;
        }

        // Native appearance from the app's requested theme (the parity-capture dark/light path, the macOS twin
        // of the iOS lane's overrideUserInterfaceStyle). The app set its cross-platform theme in pure C++
        // (application::set_platform_app_theme); read it back and force the NSWindow's NSAppearance so native
        // AppKit controls render in the requested theme. Set on the window so it propagates to the content view
        // + every native child. `unspecified` leaves nil (the system default).
        switch (application->requested_theme())
        {
            case maui::core::app_theme::dark:
                ns_window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
                break;
            case maui::core::app_theme::light:
                ns_window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
                break;
            case maui::core::app_theme::unspecified:
                ns_window.appearance = nil;
                break;
        }
        // Trace to BOTH the unified log (os_log) and stderr: the unified-log store may drop default-level
        // messages on some hosts, so the stderr line is the deterministic "the mount ran" proof the brief
        // asks for when running this GUI app headless in CI.
        os_log(OS_LOG_DEFAULT, "[host_run] mounted app window — showing NSWindow");
        std::fprintf(stderr, "[host_run] mounted app window '%s' — showing NSWindow\n",
                     ns_window.title.UTF8String != nullptr ? ns_window.title.UTF8String : "");
        [ns_window setContentSize:NSMakeSize(k_window_width, k_window_height)];
        [ns_window center];
        [ns_window makeKeyAndOrderFront:nil];
        return true;
    }
} // namespace

// The AppKit application delegate run_app installs. AppKit instantiates the NSApplication delegate role via
// the NSApplicationDelegate protocol; our delegate's applicationDidFinishLaunching drives the boot (the
// macOS analog of the iOS lane's didFinishLaunchingWithOptions). It owns nothing (the maui_app is the
// file-scope owner) — it is a thin lifecycle hook.
@interface MauiAppKitHostDelegate : NSObject <NSApplicationDelegate>
@end

@implementation MauiAppKitHostDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    (void)notification;
    @try
    {
        if (!boot_window())
        {
            [NSApp terminate:nil];
        }
    }
    @catch (NSException* exception)
    {
        os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed (NSException %{public}s): %{public}s",
                     exception.name.UTF8String, exception.reason.UTF8String);
        [NSApp terminate:nil];
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    (void)sender;
    return YES; // a single-window pure-C++ app: closing the window ends the program.
}
@end

namespace maui::hosting
{
    int run_app(int /*argc*/, char** /*argv*/, app_configurator configure)
    {
        @autoreleasepool
        {
            try
            {
                state().configure = configure;

                NSApplication* const ns_app = [NSApplication sharedApplication];
                [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];

                // NSApplication.delegate is WEAK — a freshly-alloc'd delegate would be released right after
                // the assignment. Hold a strong reference in a function-local static (lives for the process,
                // no mutable global) so the delegate survives the run loop, then install it.
                static MauiAppKitHostDelegate* const app_delegate = [[MauiAppKitHostDelegate alloc] init];
                ns_app.delegate = app_delegate;

                [ns_app activateIgnoringOtherApps:YES];
                [ns_app run];
                return 0;
            }
            catch (const std::exception& error)
            {
                os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed: %{public}s", error.what());
                return 1;
            }
            catch (...)
            {
                os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed: unknown exception");
                return 1;
            }
        }
    }
} // namespace maui::hosting

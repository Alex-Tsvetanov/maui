// window_handler — iOS (UIKit) platform recipe: a real UIWindow host. The real-native twin of the
// headless partial, and the UIKit sibling of the AppKit recipe in src/platform/apple/window_handler.mm.
// The managed platform view is a UIWindow (held, retained, in window_platform::native); on connect the
// handler installs the lifecycle proxy, reports the window's initial bounds back to the virtual view, and
// makes the window key and visible. The mapper that follows hosts the root page's native view under a
// container root view controller and pushes the title to the window's scene.
//
// Ported DIRECTLY from the UIKit window pieces the AppKit twin was adapted from:
//   - WindowHandler.iOS.cs — ConnectHandler (FrameObserverProxy KVO-observing the UIWindow's "frame" →
//     IWindow.FrameChanged, + UpdateVirtualViewFrame pushing the initial Bounds; the Mac-Catalyst-16
//     WindowProxy/effectiveGeometry branch is out of scope with the rest of Mac Catalyst) and MapContent
//     (RootViewController = window.Content.ToUIViewController(...)).
//   - Platform/iOS/WindowExtensions.cs — UpdateTitle (the title lives on the UIWindowScene; a UIWindow
//     itself has NO title — nil scene → skip, like the C# WindowScene null guard) and UpdateCoordinates
//     (plain-iOS branch: X/Y/Width/Height cannot move a UIWindow, which spans its screen — the update
//     re-reports the platform Bounds back through FrameChanged instead, so the cross-platform geometry
//     re-converges on the platform truth).
//   - Platform/iOS/ApplicationExtensions.cs — CreatePlatformWindow's non-scene branch (`new UIWindow()`,
//     which adopts the main screen's bounds) + MakeKeyAndVisible right after the handler attach. The port
//     has no UIApplicationDelegate infrastructure, so both fold into the handler (create_platform_view /
//     connect) — the same window-creation collapse the AppKit twin documents.
//   - Platform/iOS/ContainerViewController.cs — the root view controller hosts the page's native view as
//     a bounds-filling subview of its own view (LoadPlatformView + ViewDidLayoutSubviews), over the
//     system background color.
//   - Hosting/LifecycleEvents/AppHostBuilderExtensions.iOS.cs — the NON-SCENE window lifecycle map (the
//     port's window, like C#'s `!HasSceneManifest()` path, is application-notification driven):
//       UIApplicationDidBecomeActiveNotification    → i_window::send_activated()   (OnActivated)
//       UIApplicationWillEnterForegroundNotification → i_window::send_resumed()    (WillEnterForeground)
//       UIApplicationDidEnterBackgroundNotification  → i_window::send_stopped()    (DidEnterBackground)
//       UIApplicationWillTerminateNotification       → i_window::send_destroying() (WillTerminate)
//     The UIScene lifecycle (SceneOnActivated etc.) is out of scope with the rest of scene support.
//
// NOTE: UIApplicationWillResignActiveNotification (C# OnResignActivation → Window.Deactivated) is
// intentionally NOT mapped to send_deactivated — the same rationale as the AppKit twin's unmapped
// windowDidResignMain: the port's send_deactivated drives the page Disappearing/Unloaded (the M5c
// windowed-appearing model), but C#'s Window.Deactivated only raises the event; mapping resign-active
// would Disappear the page on every app switch / control-center pull. Deactivation therefore flows only
// through send_destroying (will-terminate), keeping the page lifecycle faithful.
//
// Verified empirically on the iOS 26 simulator (simctl-spawned, no UIApplication): UIWindow init/
// makeKeyAndVisible/rootViewController all work without a UIApplication, the spawned process's windows
// carry a (placeholder) UIWindowScene whose title is settable, KVO on "frame" fires on setFrame:, and
// NSNotificationCenter delivers manually-posted UIApplication* notifications — so the whole recipe is
// testable on-simulator; only the SYSTEM posting of the application notifications needs a real app
// lifecycle (the ios_app_sample). Compiled as Objective-C++ with ARC for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/core/dimension.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    // KVO context disambiguating the frame observation (the canonical addObserver:…context: pattern):
    // a process-unique stable address, non-const because the KVO API traffics in plain void*.
    void* frame_kvo_context()
    {
        static char context = 0;
        return &context;
    }

    // CGRect → the port's rect (CGRectExtensions.ToRectangle).
    maui::graphics::rect to_rect(CGRect value)
    {
        return {value.origin.x, value.origin.y, value.size.width, value.size.height};
    }
} // namespace

// Obj-C trampoline: the container root view controller — ports ContainerViewController. Its (default,
// lazily-loaded) view hosts the page's native view as a subview; viewDidLayoutSubviews keeps the hosted
// view bounds-filling (ContainerViewController.ViewDidLayoutSubviews: currentPlatformView.Frame =
// View.Bounds).
@interface MauiWindowRootViewController : UIViewController
@property(nonatomic, strong) UIView* hostedView; // ContainerViewController.currentPlatformView
@end

@implementation MauiWindowRootViewController
- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
    if (self.hostedView != nil)
    {
        self.hostedView.frame = self.view.bounds;
    }
}
@end

// Obj-C trampoline: the lifecycle proxy. One object plays both C# proxies' roles: the application
// notification observers (the AppHostBuilderExtensions.iOS non-scene lifecycle map, header comment) and
// WindowHandler.iOS's FrameObserverProxy (KVO on the UIWindow's "frame" → IWindow.FrameChanged). The
// window_platform retains it in its notification_trampoline slot for the window's lifetime (the
// notification center and KVO both hold their observers weakly).
@interface MauiWindowLifecycleProxy : NSObject
@property(nonatomic) maui::core::i_window* window;
@property(nonatomic, weak) UIWindow* platformWindow; // FrameObserverProxy's weak platform-view ref
@end

@implementation MauiWindowLifecycleProxy
- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_activated(); // OnActivated → IWindow.Activated
    }
}

- (void)applicationWillEnterForeground:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_resumed(); // WillEnterForeground → IWindow.Resumed
    }
}

- (void)applicationDidEnterBackground:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_stopped(); // DidEnterBackground → IWindow.Stopped
    }
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_destroying(); // WillTerminate → IWindow.Destroying
    }
}

// FrameObserverProxy.FrameAction → Update: report the platform window's new frame to the virtual view.
- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id>*)change
                       context:(void*)context
{
    if (context == frame_kvo_context())
    {
        UIWindow* const platform_window = self.platformWindow;
        if (self.window != nullptr && platform_window != nil)
        {
            self.window->frame_changed(to_rect(platform_window.frame));
        }
        return;
    }
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}
@end

namespace
{
    UIWindow* as_window(void* native)
    {
        return (__bridge UIWindow*)native;
    }

    // The root page's native UIView, via its view-handler's native_view() (nil if the page is unattached
    // or its handler has no native view). Mirrors the AppKit twin's page_native_view helper.
    UIView* page_native_view(maui::core::i_element& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // Un-observe + release the lifecycle proxy (shared by disconnect() and the destructor for the
    // never-disconnected path — KVO observers MUST be removed before the observed UIWindow deallocates).
    void release_trampoline(maui::core::window_platform& platform)
    {
        if (platform.notification_trampoline == nullptr)
        {
            return;
        }
        auto* const proxy = (__bridge MauiWindowLifecycleProxy*)platform.notification_trampoline;
        [[NSNotificationCenter defaultCenter] removeObserver:proxy];
        if (platform.native != nullptr)
        {
            [as_window(platform.native) removeObserver:proxy forKeyPath:@"frame" context:frame_kvo_context()];
        }
        CFRelease(platform.notification_trampoline); // balances the __bridge_retained in connect()
        platform.notification_trampoline = nullptr;
    }
} // namespace

namespace maui::core
{
    window_platform::~window_platform()
    {
        release_trampoline(*this); // no-op when disconnect() already ran
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        auto platform = std::make_unique<window_platform>();
        // ApplicationExtensions.CreatePlatformWindow's non-scene branch: `new UIWindow()` — a window
        // adopting the main screen's bounds (verified on-simulator: init yields the full screen frame).
        // iOS 26 deprecates the scene-less initializer in favor of initWithWindowScene:, but UIWindowScene
        // adoption is out of scope with the rest of the scene lifecycle, and C#'s non-scene path calls the
        // very same deprecated initializer — the port mirrors it, suppression included (the button's
        // contentEdgeInsets precedent).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init];
#pragma clang diagnostic pop
        platform->native = (__bridge_retained void*)window; // the void* slot owns one reference
        return platform;
    }

    // C# ConnectHandler + ApplicationExtensions.CreatePlatformWindow: install the lifecycle proxy (the
    // application-notification observers + the "frame" KVO), report the window's initial bounds to the
    // virtual view (UpdateVirtualViewFrame), and make the window key and visible. The mapper that follows
    // (set_virtual_view) hosts the content + pushes the title/geometry — C# orders MakeKeyAndVisible after
    // the full handler attach, but nothing observes the gap (no run-loop turn in between), so the port
    // keys the window here, inside the one connect step.
    void window_handler::connect()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIWindow* const window = as_window(platform->native);
        MauiWindowLifecycleProxy* const proxy = [[MauiWindowLifecycleProxy alloc] init];
        proxy.window = window_view_;
        proxy.platformWindow = window;
        // FrameObserverProxy.Connect: KVO-observe the UIWindow's frame (KVO holds the observer weakly...).
        [window addObserver:proxy forKeyPath:@"frame" options:NSKeyValueObservingOptionNew context:frame_kvo_context()];
        // The non-scene lifecycle map (header comment). object:nil matches any poster — the poster is the
        // UIApplication singleton, which a simctl-spawned test process does not have.
        NSNotificationCenter* const center = [NSNotificationCenter defaultCenter];
        [center addObserver:proxy
                   selector:@selector(applicationDidBecomeActive:)
                       name:UIApplicationDidBecomeActiveNotification
                     object:nil];
        [center addObserver:proxy
                   selector:@selector(applicationWillEnterForeground:)
                       name:UIApplicationWillEnterForegroundNotification
                     object:nil];
        [center addObserver:proxy
                   selector:@selector(applicationDidEnterBackground:)
                       name:UIApplicationDidEnterBackgroundNotification
                     object:nil];
        [center addObserver:proxy
                   selector:@selector(applicationWillTerminate:)
                       name:UIApplicationWillTerminateNotification
                     object:nil];
        platform->notification_trampoline = (__bridge_retained void*)proxy; // (...so retain it here.)

        // WindowHandler.UpdateVirtualViewFrame: VirtualView.FrameChanged(window.Bounds) — the initial
        // platform geometry report (the plain-iOS ConnectHandler branch).
        if (window_view_ != nullptr)
        {
            window_view_->frame_changed(to_rect(window.bounds));
        }
        // ApplicationExtensions.CreatePlatformWindow: GetWindow()?.MakeKeyAndVisible().
        [window makeKeyAndVisible];
    }

    // C# DisconnectHandler — with one deliberate deviation: C# only disposes the frame observer on the
    // Mac-Catalyst-16 branch (plain iOS leaves it to the GC/finalizer); C++ has no finalizer safety net,
    // so the port un-observes deterministically on every disconnect (PROFILE §5: DisconnectHandler → RAII).
    void window_handler::disconnect() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        release_trampoline(*platform);
    }

    void window_handler::host_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIWindow* const window = as_window(platform->native);
        platform->content_hosted = false;
        if (window_view_ == nullptr)
        {
            return;
        }
        // C# MapContent: RootViewController = window.Content.ToUIViewController(MauiContext) — a fresh
        // container controller per run, whose view hosts the page's native view as a bounds-filling
        // subview (ContainerViewController.LoadPlatformView). A page without a native view (none set, or
        // its handler has no native UIView) still installs a plain root controller — the AppKit twin's
        // empty-host analog.
        MauiWindowRootViewController* const controller = [[MauiWindowRootViewController alloc] init];
        // ContainerViewController.LoadPlatformView paints the system background behind the hosted view
        // (C# skips it when the content brings its own Background; the port's container always paints —
        // the page's own background, when set, covers it).
        controller.view.backgroundColor = UIColor.systemBackgroundColor;
        auto* page = window_view_->content();
        if (UIView* const view = page != nullptr ? page_native_view(*page) : nil)
        {
            controller.hostedView = view;
            [controller.view addSubview:view];
            view.frame = controller.view.bounds;
            platform->content_hosted = true;
        }
        window.rootViewController = controller;
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || window_view_ == nullptr)
        {
            return;
        }
        const std::string title(window_view_->title());
        // WindowExtensions.UpdateTitle: the title lives on the UIWindowScene (a UIWindow has no title of
        // its own); nil scene → skip, like the C# `platformWindow.WindowScene is not null` guard. The C#
        // `window.Title ?? String.Empty` null-coalesce maps to the nil-UTF8 fallback.
        if (UIWindowScene* const scene = as_window(platform->native).windowScene)
        {
            NSString* const value = [NSString stringWithUTF8String:title.c_str()];
            scene.title = value != nil ? value : @"";
        }
        platform->title = title; // keep the mirror in sync (tests read it)
    }

    void window_handler::apply_frame()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || window_view_ == nullptr)
        {
            return;
        }
        // WindowExtensions.UpdateCoordinates (the plain-iOS branch): a developer X/Y/Width/Height set
        // cannot move a UIWindow (it spans its screen) — re-report the platform Bounds back through
        // FrameChanged instead. FrameChanged's from_handler values sit at the TOP specificity (C#
        // SetterSpecificity.FromHandler), so the re-report OVERRIDES the developer set and the virtual
        // geometry re-converges on the platform truth — exactly C#'s SetValueActual semantics, pinned by
        // the geometry seam test. The Catalyst-16 RequestGeometryUpdate branch is out of scope with the
        // rest of Mac Catalyst.
        window_view_->frame_changed(to_rect(as_window(platform->native).bounds));
    }

    // --- chrome (W1-11): the iOS window chrome recipes are MIRROR-ONLY by design (C# parity) ----------
    // Toolbar: C#'s iOS toolbar items surface through the NAVIGATION chrome (UINavigationBar
    // rightBarButtonItems), not the window — the port's navigation_page_handler.mm builds the bar
    // buttons; the window only records the chrome aggregate here.
    void window_handler::apply_toolbar(i_toolbar* toolbar)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_toolbar = toolbar;
        }
    }

    // MenuBar: stored-inert — C# materializes menu bars on desktop only (WindowHandler maps MenuBar on
    // WINDOWS, and on iOS only Mac Catalyst's UIMenuBuilder realizes it; a plain-iOS UIApplication has
    // no menu bar surface). The aggregate stays observable through the mirror.
    void window_handler::apply_menu_bar(i_menu_bar* menu_bar)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_menu_bar = menu_bar;
        }
    }

    // TitleBar: documented no-op — C# maps Window.TitleBar on Windows + Mac Catalyst only (a plain-iOS
    // UIWindow has no title bar). Mirror-only, like the menu bar.
    void window_handler::apply_title_bar(i_title_bar* title_bar)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_title_bar = title_bar;
        }
    }
} // namespace maui::core

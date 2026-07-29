// run_app — the iOS (UIKit) backend body of maui::hosting::run_app (host_run.hpp).
//
// The UIApplicationMain twin of src/platform/apple/host_run.mm: a classic (non-scene) UIApplicationDelegate
// builds the user's app through the SAME generic mount (app_host.hpp's mount_window + drive_layout — NO
// per-control knowledge), lays it out over the root view-controller's SAFE-AREA bounds (the recipe factored
// from src/samples/ios_gallery.mm boot_page), makes the UIWindow key + visible, and returns YES. main()
// hands control to UIApplicationMain. The user writes ZERO Objective-C: all of the UIApplicationMain /
// UIApplicationDelegate / safe-area glue the gallery + sample mains hand-rolled now lives here, behind the
// pure-C++ run_app seam — so the same pure-C++ samples/hello_world/main.cpp renders on the simulator.
//
// No C# class maps 1:1: this is the port's analog of MAUI's MauiUIApplicationDelegate boot under
// UIApplicationMain.
//
// THE CONFIGURATOR HAND-OFF (the iOS-specific wrinkle): UIApplicationMain instantiates the delegate itself
// — we never call its initializer, so we cannot pass the run_app(argc, argv, configure) function pointer to
// it directly. Instead run_app stores `configure` in a file-scope static BEFORE calling UIApplicationMain,
// and the delegate reads that static from didFinishLaunchingWithOptions. (The apple lane needs the same
// trick for its AppKit-instantiated delegate; both store g_configure at file scope.)
//
// Lifetime (PROFILE §8): the built maui_app must outlive the whole run loop (it owns the application → tree;
// handlers keep raw context pointers into it). It is held in a file-scope std::unique_ptr the delegate fills
// and never frees — process exit tears it down after the run loop, so it outlives the windows it bridged.
//
// Build (ios preset): keyed into maui_hosting by CMake (the ios lane of the host_run generator expression);
// exactly one run_app links per backend. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>
#import <os/log.h>

#include <cstdlib>
#include <exception>
#include <memory>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/host_run.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

// A layout-tracking view: its layoutSubviews fires whenever its superview re-lays-out (including a Mac
// Catalyst window resize), invoking `onLayout`. Used by boot_window to re-drive the maui layout so the tree
// reflows to fill the resized window (MAUI reflows on a size transition; the port's one-shot boot layout did
// not). It never draws (hidden) and takes no touches — purely a resize sensor. Declared before boot_window
// (which uses it); defined out-of-line below.
//
// It senses the SAFE AREA too (onSafeAreaChange). UIKit does not have the insets ready when the port drives
// its first layout — even after layoutIfNeeded, the first pass reads all-zero and the real values (e.g. a Mac
// Catalyst title bar's top=41) only arrive on a later pass. MAUI never notices because its layout is
// UIKit-driven (LayoutSubviews runs after UIKit populates them) and it re-lays-out from
// MauiView.SafeAreaInsetsDidChange; the port drives layout ITSELF, so without this hook it would bake in the
// zero-inset first pass. Pinned to fill root_view, so its insets ARE root_view's.
@interface MauiRelayoutView : UIView
@property(nonatomic, copy) void (^onLayout)(void);
@property(nonatomic, copy) void (^onSafeAreaChange)(void);
@end

namespace
{
    // The process-wide host state the UIApplicationMain-instantiated delegate must reach but we cannot pass
    // to it (UIApplicationMain creates the delegate, we never call its initializer — the configurator
    // hand-off the header documents). A single function-local-static struct (a Meyers singleton) holds it —
    // the idiomatic stand-in for a mutable global that keeps one deterministically-constructed instance.
    struct host_state
    {
        // The configurator from run_app(argc, argv, configure) — set once by run_app BEFORE
        // UIApplicationMain, read by the delegate's didFinishLaunchingWithOptions.
        maui::hosting::app_configurator configure = nullptr;
        // The built app, alive for the whole run loop (owns the application → window → tree; PROFILE §8).
        // Filled by the delegate and never freed (process exit tears it down after the run loop, so the
        // maui_app outlives the windows it bridged — the maui_app.hpp teardown doctrine).
        std::unique_ptr<maui::hosting::maui_app> app;
    };

    host_state& state()
    {
        static host_state s; // constructed on first use, destroyed at process exit
        return s;
    }

    // Build + mount + lay out the app's window, returning the native UIWindow to make key + visible (or nil
    // on failure, logged). The safe-area layout recipe is factored verbatim from ios_gallery.mm boot_page.
    UIWindow* boot_window()
    {
        // (1) Build from a FRESH builder the user's configurator populates (use_maui_app<App>()).
        state().app = state().configure(maui::hosting::maui_app::create_builder()).build();

        // (2) Ask the application for its window (IApplication.CreateWindow — the user's create_window
        //     override). No application / no window ⇒ nothing to host.
        const std::shared_ptr<maui::controls::application>& application = state().app->application();
        if (application == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] no application configured (use_maui_app not called)");
            return nil;
        }
        auto* const window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] application produced no window");
            return nil;
        }

        // (3) Generic mount: attach handlers across the tree (children before parents), re-host each
        //     container, attach the window handler, open the window — the SAME driver the headless +
        //     apple lanes use (app_host.hpp). The window_handler creates the real UIWindow + root VC and
        //     hosts the page's native view under it.
        maui::hosting::mount_window(*state().app, *window);

        // (4) Reach the native UIWindow through the window's now-attached handler.
        const auto window_handler = std::dynamic_pointer_cast<maui::core::window_handler>(window->handler());
        if (window_handler == nullptr || window_handler->typed_platform_view() == nullptr)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] window handler did not produce a native UIWindow");
            return nil;
        }
        auto* const native_window = (__bridge UIWindow*)window_handler->typed_platform_view()->native;
        if (native_window == nil)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] native UIWindow is nil");
            return nil;
        }

        // (4b) Native interface style from the app's requested theme (the parity-capture dark/light path). The
        //      app set its cross-platform theme in pure C++ (application::set_platform_app_theme from
        //      MAUI_APPEARANCE — there is no AppInfo singleton on iOS, so nothing else seeds it); read it back
        //      here and force the native UIWindow's overrideUserInterfaceStyle so native UIKit controls render
        //      in the requested theme. Set on the window so it propagates to the root VC + every native child.
        //      `unspecified` (no theme requested) leaves the system default (UIUserInterfaceStyleUnspecified).
        //      This keeps the gallery example pure C++ while preserving the parity dark captures the former
        //      ios_gallery.mm produced by forcing overrideUserInterfaceStyle directly.
        switch (application->requested_theme())
        {
            case maui::core::app_theme::dark:
                native_window.overrideUserInterfaceStyle = UIUserInterfaceStyleDark;
                break;
            case maui::core::app_theme::light:
                native_window.overrideUserInterfaceStyle = UIUserInterfaceStyleLight;
                break;
            case maui::core::app_theme::unspecified:
                native_window.overrideUserInterfaceStyle = UIUserInterfaceStyleUnspecified;
                break;
        }

        // (5) Lay out the tree over the root view-controller's bounds (the window host does no auto-layout).
        //     Compute BOTH the full controller bounds and the safe-area-inset rect, then hand both to the
        //     generic drive_layout: it picks the full bounds for a VC-backed root page (flyout/tabbed — the
        //     controller owns the chrome and each inner page tracks its own safe area) and the safe-area rect
        //     for every other page, via the shared root_view_controller() contract (app_host.hpp). Force a
        //     layout pass first so safeAreaInsets is populated; fall back to a status-bar/Dynamic-Island top
        //     inset if it is not yet (no run-loop spin has happened at boot).
        UIView* const root_view = native_window.rootViewController.view;
        [root_view layoutIfNeeded];
        if (window->content_element() == nullptr)
        {
            os_log(OS_LOG_DEFAULT, "[host_run] window has no content page — showing empty window");
            return native_window;
        }

        // The re-layout closure: recompute the controller's full + safe-area bounds and drive the generic
        // layout over them. Reused for the initial pass AND on every window resize (Mac Catalyst windows are
        // freely resizable), so the tree always fills the CURRENT window — the port's analog of MAUI re-running
        // its layout on a size transition. Without it the tree keeps its first-layout size and fill-height
        // pages (Grid "*" rows, FlexLayout) leave a gap when the window grows. `window` outlives the run loop
        // (state().app owns it), so the raw capture is safe.
        auto relayout = [window](UIView* view) {
            const CGRect full = view.bounds;
            if (full.size.width < 1.0 || full.size.height < 1.0)
            {
                return;
            }
            // The REAL insets, never a guess. A `if (insets.top < 1.0) insets.top = 59.0;` fallback used to
            // stand here for "status bar + Dynamic Island — no run-loop spin has happened yet": UIKit has
            // not populated the insets when the port drives its first layout, so the first pass read zero.
            // That guess was wrong twice over — it invented a 59pt inset on Mac Catalyst, whose title bar is
            // chrome OUTSIDE the safe area (real top=41, and a hardcoded 59 is not 41 on any device) — and
            // it only ever papered over the first pass. The tracker's onSafeAreaChange hook below now
            // re-drives layout the moment UIKit reports the true insets, which is what MAUI gets for free
            // from its UIKit-driven layout, so there is nothing left to guess at.
            const UIEdgeInsets insets = view.safeAreaInsets;
            const maui::graphics::rect full_bounds{0, 0, static_cast<double>(full.size.width),
                                                   static_cast<double>(full.size.height)};
            const maui::graphics::rect safe_area_bounds{
                static_cast<double>(insets.left), static_cast<double>(insets.top),
                static_cast<double>(full.size.width - insets.left - insets.right),
                static_cast<double>(full.size.height - insets.top - insets.bottom)};
            maui::hosting::drive_layout(*window, full_bounds, safe_area_bounds);
        };
        relayout(root_view);
        os_log(OS_LOG_DEFAULT, "[host_run] mounted app window — laid out (bounds %gx%g)", root_view.bounds.size.width,
               root_view.bounds.size.height);

        // Install the relayout hook (window::request_relayout) AFTER the first pass — mirrors the
        // Android-only jni/relayout.hpp precedent, generalized to every backend (see window.hpp's header
        // comment). Reuses the SAME `relayout` closure the resize/safe-area tracker below drives, so a
        // leaf's invalidate_measure() re-reads the CURRENT root_view bounds + safeAreaInsets exactly like a
        // real size transition would, rather than replaying a stale captured size.
        window->set_relayout_hook([relayout, root_view] { relayout(root_view); });

        // Install the resize hook: a hidden, non-interactive tracking view pinned to fill root_view. Its
        // layoutSubviews fires whenever root_view re-lays-out (incl. a window-size change), re-running the
        // relayout closure. Guarded on the size actually changing, so drive_layout's frame mutations don't
        // re-trigger it into a loop. Captured weakly to avoid a root_view ⇄ block retain cycle.
        MauiRelayoutView* const tracker = [[MauiRelayoutView alloc] initWithFrame:root_view.bounds];
        tracker.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        tracker.userInteractionEnabled = NO;
        tracker.hidden = YES;
        [root_view insertSubview:tracker atIndex:0];
        __weak UIView* const weak_root = root_view;
        __block CGSize last_size = root_view.bounds.size;
        tracker.onLayout = ^{
          UIView* const r = weak_root;
          if (r == nil || CGSizeEqualToSize(r.bounds.size, last_size))
          {
              return; // gone, or size unchanged — nothing to re-layout
          }
          last_size = r.bounds.size;
          relayout(r);
        };
        // The insets arrive AFTER the boot layout (see the MauiRelayoutView comment) and a pure inset change
        // moves no frames, so onLayout's size guard would swallow it — hence its own unguarded hook. This
        // cannot spin: the safe area is folded into the CHILDREN's bounds, never into a layout's own frame,
        // so re-driving leaves every native frame (and therefore every view's insets) exactly where it was.
        tracker.onSafeAreaChange = ^{
          UIView* const r = weak_root;
          if (r != nil)
          {
              relayout(r);
          }
        };

        return native_window;
    }
} // namespace

@implementation MauiRelayoutView
- (void)layoutSubviews
{
    [super layoutSubviews];
    if (self.onLayout != nil)
    {
        self.onLayout();
    }
}
- (void)safeAreaInsetsDidChange
{
    [super safeAreaInsetsDidChange];
    if (self.onSafeAreaChange != nil)
    {
        self.onSafeAreaChange();
    }
}
@end

// The UIKit application delegate UIApplicationMain instantiates (its class name is passed below). The
// classic (non-scene) UIApplicationDelegate path the port's window_handler mirrors (C#'s !HasSceneManifest
// branch); the conventional `window` slot keeps the UIWindow retained for the run loop.
@interface MauiUIKitHostDelegate : UIResponder <UIApplicationDelegate>
@property(nonatomic, strong) UIWindow* window;
@end

@implementation MauiUIKitHostDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    (void)application;
    (void)launchOptions;
    // The SAME try/@catch the gallery uses: a C++ throw OR an Obj-C NSException during boot returns NO
    // (a clean launch failure) instead of crashing the process.
    try
    {
        @try
        {
            self.window = boot_window();
        }
        @catch (NSException* exception)
        {
            os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed (NSException %{public}s): %{public}s",
                         exception.name.UTF8String, exception.reason.UTF8String);
            return NO;
        }
    }
    catch (const std::exception& error)
    {
        os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed: %{public}s", error.what());
        return NO;
    }
    catch (...)
    {
        os_log_error(OS_LOG_DEFAULT, "[host_run] boot failed: unknown exception");
        return NO;
    }
    if (self.window == nil)
    {
        return NO;
    }
    // Capture-determinism opt-in (MAUI_CAPTURE_TINT_NORMAL=1, set by port/tools/e2e/e2e.py): when the
    // parity tool launches this app in the BACKGROUND (`open -g`, so the operator's focus is never
    // stolen), the window is never key/active, and Catalyst then renders every default-tinted control
    // dimmed (UIButton's system-blue text renders gray) via the UITraitActiveAppearance trait (plus
    // the legacy tintAdjustmentMode path). Forcing BOTH to their active values keeps the CONTENT
    // pixels identical to the active-window render; the window CHROME (title text, traffic lights)
    // still draws inactive, which the parity review already exempts. Default (no env) keeps faithful
    // UIKit behavior.
    if (std::getenv("MAUI_CAPTURE_TINT_NORMAL") != nullptr)
    {
        self.window.tintAdjustmentMode = UIViewTintAdjustmentModeNormal;
        if (@available(macCatalyst 17.0, *))
        {
            self.window.traitOverrides.activeAppearance = UIUserInterfaceActiveAppearanceActive;
        }
    }
    [self.window makeKeyAndVisible];
    return YES;
}
@end

namespace maui::hosting
{
    int run_app(int argc, char** argv, app_configurator configure)
    {
        @autoreleasepool
        {
            // The configurator hand-off: stash it where the UIApplicationMain-instantiated delegate reads
            // it (file scope), THEN hand control to UIApplicationMain with our delegate class. The third
            // argument (principalClassName) stays nil — the default UIApplication; the fourth names the
            // delegate UIApplicationMain instantiates.
            state().configure = configure;
            return UIApplicationMain(argc, argv, nil, NSStringFromClass([MauiUIKitHostDelegate class]));
        }
    }
} // namespace maui::hosting

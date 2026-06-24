// ios_gallery — the UIKit host for the runnable demo gallery (the maui_ios_gallery .app target).
//
// The UIApplicationMain twin of macos_gallery.mm: a classic (non-scene) UIApplicationDelegate builds a
// maui_app for whichever of the 11 curated demo pages (src/samples/pages/*.hpp) the MAUI_SAMPLE_PAGE env
// var selects (default "value_controls"), attaches handlers bottom-up, opens the window, then lays out
// and shows it. The page list is single-sourced in gallery_host.hpp's MAUI_GALLERY_PAGES X-macro; this
// main expands it to dispatch the runtime env string to a compile-time gallery_app<PageType>.
//
// The mount recipe (now the GENERIC driver — app_host.hpp, Stage 5a):
//   1. use_maui_app<gallery_app<Page>> mints the app (it OWNS the window + page; create_window() returns it).
//   2. mount_window walks the page's element tree, attaches a handler to every element (children before
//      parents) + re-hosts each container, attaches the window handler, and opens the window — NO per-page
//      attach_handlers / gallery_rehost plumbing. The window host hosts the page's native view on open.
//   3. drive_layout (the two-rect form) measures + arranges the tree over the rootVC bounds, choosing the
//      safe-area inset for a normal page and the full controller bounds for a VC-backed root page
//      (flyout/tabbed), via the shared root_view_controller() contract.
//
// Build + run on the booted simulator:
//   cmake --build --preset ios --target maui_ios_gallery
//   xcrun simctl install booted build/ios/maui_ios_gallery.app
//   SIMCTL_CHILD_MAUI_SAMPLE_PAGE=pickers xcrun simctl launch booted dev.maui-cpp.ios-gallery
// Traces go to the unified log (os_log). Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>
#import <os/log.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include "gallery_host.hpp"

@interface MauiGalleryAppDelegate : UIResponder <UIApplicationDelegate>
// The conventional non-scene UIApplicationDelegate window slot (mirrors C#'s MauiUIApplicationDelegate.Window).
@property(nonatomic, strong) UIWindow* window;
@end

namespace
{
    // Boot + show one demo page through the full hosting recipe. Templated on the gallery_app<Page>
    // instantiation the dispatch below selects from MAUI_SAMPLE_PAGE. Returns the native UIWindow (for the
    // delegate's window slot), or nil on failure (logged).
    template <class Page> UIWindow* boot_page(std::unique_ptr<maui::hosting::maui_app>& maui_app)
    {
        using app_type = maui::samples::gallery_app<Page>;

        // (1) Builder boot — the delegate owns the maui_app (alive for the whole run loop).
        maui_app = maui::hosting::maui_app::create_builder().use_maui_app<app_type>().build();
        auto* const app = maui_app->application_as<app_type>().get();

        // Appearance: read MAUI_APPEARANCE BEFORE attaching handlers and seed the cross-platform application
        // theme. The iOS host has no AppInfo singleton, so nothing else sets it — application::requested_theme()
        // would stay `unspecified`, and theme-reactive pages (e.g. shape_app_theme) would render their LIGHT
        // slot even in a dark capture. Seeding here means the page reads the correct slot at attach;
        // set_platform_app_theme also fires requested_theme_changed so a subscribed page re-applies. The native
        // UIWindow interface style is forced from the same flag below.
        const char* const appearance = std::getenv("MAUI_APPEARANCE");
        const bool dark_appearance = appearance != nullptr && std::strcmp(appearance, "dark") == 0;
        maui_app->application()->set_platform_app_theme(dark_appearance ? maui::core::app_theme::dark
                                                                        : maui::core::app_theme::light);

        // (2) Generic mount: attach handlers across the page tree (children before parents), re-host each
        //     container, attach the window handler, open the window — NO per-page attach_handlers plumbing.
        //     gallery_pre_mount lets a page register a user-control handler first (custom_layout_page);
        //     gallery_post_mount runs a page's post-mount demo seeding after the tree is live (both no-ops
        //     unless the page opts in — gallery_host.hpp).
        maui::samples::gallery_pre_mount(*maui_app, app->page_member());
        maui::hosting::mount_window(*maui_app, app->win());
        maui::samples::gallery_post_mount(*maui_app, app->page_member());
        const auto window_handler = std::dynamic_pointer_cast<maui::core::window_handler>(app->win().handler());

        auto* const native_window = (__bridge UIWindow*)window_handler->typed_platform_view()->native;

        // Appearance toggle for the parity-comparison capture loop: MAUI_APPEARANCE=dark|light forces the
        // window's interface style so native UIKit controls render in the requested theme (default: light).
        // Set on the window so it propagates to the root view controller + every native child. Uses the same
        // dark_appearance flag that seeded the cross-platform theme above.
        native_window.overrideUserInterfaceStyle =
            dark_appearance ? UIUserInterfaceStyleDark : UIUserInterfaceStyleLight;

        // (3) Lay out the tree over the root view-controller's bounds (the window host does no auto-layout).
        // Compute BOTH the full controller bounds and the safe-area-inset rect, then hand both to the generic
        // drive_layout: it picks the full bounds for a VC-backed root page (flyout/tabbed — the controller
        // owns the chrome and each inner page tracks its own safe area) and the safe-area rect for every
        // other page, via the shared root_view_controller() contract (app_host.hpp). Force a layout pass
        // first so safeAreaInsets is populated; fall back to a status-bar-height top inset if it isn't yet.
        UIView* const root_view = native_window.rootViewController.view;
        [root_view layoutIfNeeded];

        const CGRect full = root_view.bounds;
        UIEdgeInsets insets = root_view.safeAreaInsets;
        if (insets.top < 1.0)
        {
            insets.top = 59.0; // status bar + Dynamic Island fallback (no run-loop spin has happened yet)
        }
        const maui::graphics::rect full_bounds{0, 0, static_cast<double>(full.size.width),
                                               static_cast<double>(full.size.height)};
        const maui::graphics::rect safe_area_bounds{static_cast<double>(insets.left), static_cast<double>(insets.top),
                                                    static_cast<double>(full.size.width - insets.left - insets.right),
                                                    static_cast<double>(full.size.height - insets.top - insets.bottom)};
        maui::hosting::drive_layout(app->win(), full_bounds, safe_area_bounds);
        os_log(OS_LOG_DEFAULT, "[gallery] laid out (safe-area inset top=%g)", insets.top);

        return native_window;
    }

    UIWindow* boot_selected(std::unique_ptr<maui::hosting::maui_app>& maui_app)
    {
        const char* const env = std::getenv("MAUI_SAMPLE_PAGE");
        const std::string selected = (env != nullptr && std::strlen(env) > 0) ? env : "value_controls";
        os_log(OS_LOG_DEFAULT, "[gallery] selected page: %{public}s", selected.c_str());

        // Dispatch the runtime env string to the compile-time gallery_app<PageType> (single-sourced).
#define MAUI_GALLERY_DISPATCH(name, page_type)                                                                         \
    if (selected == (name))                                                                                            \
    {                                                                                                                  \
        return boot_page<maui::samples::page_type>(maui_app);                                                          \
    }
        MAUI_GALLERY_PAGES(MAUI_GALLERY_DISPATCH)
#undef MAUI_GALLERY_DISPATCH

        os_log_error(OS_LOG_DEFAULT, "[gallery] unknown MAUI_SAMPLE_PAGE '%{public}s' — falling back to value_controls",
                     selected.c_str());
        return boot_page<maui::samples::value_controls_page>(maui_app);
    }
} // namespace

@implementation MauiGalleryAppDelegate
{
    // The built maui_app, owned by the delegate (it owns the application → page + window, PROFILE §8).
    std::unique_ptr<maui::hosting::maui_app> _mauiApp;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    (void)application;
    (void)launchOptions;
    try
    {
        @try
        {
            self.window = boot_selected(_mauiApp);
        }
        @catch (NSException* exception)
        {
            os_log_error(OS_LOG_DEFAULT, "[gallery] boot failed (NSException %{public}s): %{public}s",
                         exception.name.UTF8String, exception.reason.UTF8String);
            return NO;
        }
    }
    catch (const std::exception& error)
    {
        os_log_error(OS_LOG_DEFAULT, "[gallery] boot failed: %{public}s", error.what());
        return NO;
    }
    catch (...)
    {
        os_log_error(OS_LOG_DEFAULT, "[gallery] boot failed: unknown exception");
        return NO;
    }
    return YES;
}
@end

int main(int argc, char* argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([MauiGalleryAppDelegate class]));
    }
}

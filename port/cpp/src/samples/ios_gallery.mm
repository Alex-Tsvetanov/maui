// ios_gallery — the UIKit host for the runnable demo gallery (the maui_ios_gallery .app target).
//
// The UIApplicationMain twin of macos_gallery.mm: a classic (non-scene) UIApplicationDelegate builds a
// maui_app for whichever of the 11 curated demo pages (src/samples/pages/*.hpp) the MAUI_SAMPLE_PAGE env
// var selects (default "value_controls"), attaches handlers bottom-up, opens the window, then lays out
// and shows it. The page list is single-sourced in gallery_host.hpp's MAUI_GALLERY_PAGES X-macro; this
// main expands it to dispatch the runtime env string to a compile-time gallery_app<PageType>.
//
// The mount recipe (verified):
//   1. use_maui_app<gallery_app<Page>> mints the app (it OWNS the window + page; create_window() returns it).
//   2. Attach handlers BOTTOM-UP: the page's owned controls first (page.attach_handlers), the window LAST.
//   3. open_window drives the lifecycle + the window_handler host_content (the UIWindow's
//      rootViewController.view hosts the page's native view), then make it key + visible.
//   4. The window host does NOT auto-layout: measure(W,H) + arrange({0,0,W,H}) over the rootVC view bounds.
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

#include "maui/controls/window.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/window_handler.hpp"
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

        // (2) Attach handlers bottom-up: the page's owned controls first, the window LAST.
        app->page_member().attach_handlers(*maui_app);
        const auto window_handler =
            std::dynamic_pointer_cast<maui::core::window_handler>(maui_app->attach_handler(app->win()));

        // (3) Open the window: drives the lifecycle + hosts the page's native view, key + visible.
        maui_app->open_window(app->win());

        auto* const native_window = (__bridge UIWindow*)window_handler->typed_platform_view()->native;

        // Appearance toggle for the parity-comparison capture loop: MAUI_APPEARANCE=dark|light forces the
        // window's interface style so native UIKit controls render in the requested theme (default: light).
        // Set on the window so it propagates to the root view controller + every native child.
        const char* const appearance = std::getenv("MAUI_APPEARANCE");
        native_window.overrideUserInterfaceStyle = (appearance != nullptr && std::strcmp(appearance, "dark") == 0)
                                                       ? UIUserInterfaceStyleDark
                                                       : UIUserInterfaceStyleLight;

        // (4) Lay out the tree over the root view-controller's SAFE-AREA rect (the window host does no
        // auto-layout). A real app would inset via the page's SafeAreaEdges; the gallery host insets here
        // so the demo content clears the status bar / Dynamic Island. Force a layout pass first so
        // safeAreaInsets is populated; fall back to a status-bar-height top inset if it isn't yet.
        UIView* const root_view = native_window.rootViewController.view;
        [root_view layoutIfNeeded];
        UIEdgeInsets insets = root_view.safeAreaInsets;
        if (insets.top < 1.0)
        {
            insets.top = 59.0; // status bar + Dynamic Island fallback (no run-loop spin has happened yet)
        }
        const CGRect full = root_view.bounds;
        const auto width = static_cast<double>(full.size.width - insets.left - insets.right);
        const auto height = static_cast<double>(full.size.height - insets.top - insets.bottom);
        auto& root = static_cast<maui::core::i_view&>(app->page_member().page());
        root.measure(width, height);
        root.arrange(maui::graphics::rect{insets.left, insets.top, width, height});
        os_log(OS_LOG_DEFAULT, "[gallery] laid out %g x %g at inset top=%g", width, height, insets.top);

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
        self.window = boot_selected(_mauiApp);
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

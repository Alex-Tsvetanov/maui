// macos_gallery — the AppKit host for the runnable demo gallery (the maui_macos_gallery target).
//
// One native NSWindow hosting whichever of the 11 curated demo pages (src/samples/pages/*.hpp) the
// MAUI_SAMPLE_PAGE env var selects (default "value_controls"). The page list is single-sourced in
// gallery_host.hpp's MAUI_GALLERY_PAGES X-macro; this main expands it to dispatch the runtime env string
// to a compile-time gallery_app<PageType>.
//
// The mount recipe (verified):
//   1. Build the app through the hosting builder: use_maui_app<gallery_app<Page>> mints it (it OWNS the
//      window + the page, and create_window() returns the window).
//   2. Attach handlers BOTTOM-UP: the page's owned controls first (page.attach_handlers, leaves→page),
//      then the window LAST.
//   3. open_window drives the lifecycle + the window_handler host_content (the NSWindow's contentView
//      becomes the page's native view).
//   4. The window host does NOT auto-layout: measure(W,H) then arrange({0,0,W,H}) frames every native
//      child view.
//   5. Show the NSWindow (setContentSize / center / makeKeyAndOrderFront + activate).
//
// Build (apple preset) + run:
//   cmake --build --preset apple --target maui_macos_gallery
//   MAUI_SAMPLE_PAGE=pickers ./build/apple/maui_macos_gallery
// Traces go to the unified log (os_log). Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>
#import <os/log.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

#include "maui/controls/window.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include "gallery_host.hpp"

namespace
{
    // The macOS client size every page is measured/arranged into (the brief's sensible default).
    constexpr double k_window_width = 480.0;
    constexpr double k_window_height = 720.0;

    // Boot + show one demo page through the full hosting recipe. Templated on the gallery_app<Page>
    // instantiation the dispatch below selects from MAUI_SAMPLE_PAGE.
    template <class Page> int run_page()
    {
        using app_type = maui::samples::gallery_app<Page>;

        // (1) Builder boot. Static: alive for the whole run loop (it owns the application → page + window).
        static std::unique_ptr<maui::hosting::maui_app> maui_app =
            maui::hosting::maui_app::create_builder().use_maui_app<app_type>().build();
        auto* const app = maui_app->application_as<app_type>().get();

        // (2) Attach handlers bottom-up: the page's owned controls first, then the window LAST.
        app->page_member().attach_handlers(*maui_app);
        const auto window_handler =
            std::dynamic_pointer_cast<maui::core::window_handler>(maui_app->attach_handler(app->win()));

        // (3) Open the window: drives the lifecycle + hosts the page's native view in the NSWindow.
        maui_app->open_window(app->win());

        // (4) Lay out the tree: the window host does no auto-layout, so measure + arrange explicitly.
        // NOTE (AppKit): the page/layout host panels are plain (UNFLIPPED) NSViews, so a vertical stack
        // arranges its first child at MAUI-y≈0 — which AppKit renders at the BOTTOM. To keep the content
        // fully on-screen we size the window's content height to the MEASURED content height (clamped to
        // k_window_height), so the bottom-anchored block fills the visible window top-to-bottom rather
        // than sinking off the bottom edge of a taller window. A purely presentational gallery choice.
        auto& root = static_cast<maui::core::i_view&>(app->page_member().page());
        const maui::graphics::size measured = root.measure(k_window_width, k_window_height);
        const double content_height =
            std::min(k_window_height, measured.height > 0 ? measured.height : k_window_height);
        root.arrange(maui::graphics::rect{0, 0, k_window_width, content_height});

        // (5) Show the NSWindow centered + frontmost.
        auto* const ns_window = (__bridge NSWindow*)window_handler->typed_platform_view()->native;
        [ns_window setContentSize:NSMakeSize(k_window_width, content_height)];
        [ns_window center];
        [ns_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
        return 0;
    }

    int run_gallery()
    {
        NSApplication* const ns_app = [NSApplication sharedApplication];
        [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];

        const char* const env = std::getenv("MAUI_SAMPLE_PAGE");
        const std::string selected = (env != nullptr && std::strlen(env) > 0) ? env : "value_controls";
        os_log(OS_LOG_DEFAULT, "[gallery] selected page: %{public}s", selected.c_str());

        // Dispatch the runtime env string to the compile-time gallery_app<PageType> (single-sourced).
#define MAUI_GALLERY_DISPATCH(name, page_type)                                                                         \
    if (selected == (name))                                                                                            \
    {                                                                                                                  \
        return run_page<maui::samples::page_type>();                                                                   \
    }
        MAUI_GALLERY_PAGES(MAUI_GALLERY_DISPATCH)
#undef MAUI_GALLERY_DISPATCH

        os_log_error(OS_LOG_DEFAULT, "[gallery] unknown MAUI_SAMPLE_PAGE '%{public}s' — falling back to value_controls",
                     selected.c_str());
        return run_page<maui::samples::value_controls_page>();
    }
} // namespace

int main()
{
    @autoreleasepool
    {
        try
        {
            return run_gallery();
        }
        catch (const std::exception& error)
        {
            os_log_error(OS_LOG_DEFAULT, "[gallery] boot failed: %{public}s", error.what());
            return 1;
        }
        catch (...)
        {
            os_log_error(OS_LOG_DEFAULT, "[gallery] boot failed: unknown exception");
            return 1;
        }
    }
}

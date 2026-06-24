// macos_gallery — the AppKit host for the runnable demo gallery (the maui_macos_gallery target).
//
// One native NSWindow hosting whichever of the 11 curated demo pages (src/samples/pages/*.hpp) the
// MAUI_SAMPLE_PAGE env var selects (default "value_controls"). The page list is single-sourced in
// gallery_host.hpp's MAUI_GALLERY_PAGES X-macro; this main expands it to dispatch the runtime env string
// to a compile-time gallery_app<PageType>.
//
// The mount recipe (now the GENERIC driver — app_host.hpp, Stage 5a):
//   1. Build the app through the hosting builder: use_maui_app<gallery_app<Page>> mints it (it OWNS the
//      window + the page, and create_window() returns the window).
//   2. mount_window walks the page's element tree, attaches a handler to every element (children before
//      parents) + re-hosts each container, attaches the window handler, and opens the window — NO per-page
//      attach_handlers / gallery_rehost plumbing. The NSWindow's contentView becomes the page's native view.
//   3. The window host does NOT auto-layout: measure to get the content height, then drive_layout frames
//      every native child over the clamped content rect (the AppKit unflipped-bottom-anchor gallery choice
//      below).
//   4. Show the NSWindow (setContentSize / center / makeKeyAndOrderFront + activate).
//
// Build (apple preset) + run:
//   cmake --build --preset apple --target maui_macos_gallery
//   MAUI_SAMPLE_PAGE=pickers ./build/apple/maui_macos_gallery
// Traces go to the unified log (os_log). Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>
#import <CoreText/CoreText.h>
#import <os/log.h>

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

#include "maui/controls/window.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/hosting/app_host.hpp"
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

        // (2) Generic mount: attach handlers across the page tree (children before parents), re-host each
        //     container, attach the window handler, open the window — NO per-page attach_handlers plumbing.
        //     gallery_pre_mount lets a page register a user-control handler first (custom_layout_page);
        //     gallery_post_mount runs a page's post-mount demo seeding after the tree is live (both no-ops
        //     unless the page opts in — gallery_host.hpp).
        maui::samples::gallery_pre_mount(*maui_app, app->page_member());
        maui::hosting::mount_window(*maui_app, app->win());
        maui::samples::gallery_post_mount(*maui_app, app->page_member());
        const auto window_handler = std::dynamic_pointer_cast<maui::core::window_handler>(app->win().handler());

        // (3) Lay out the tree: the window host does no auto-layout, so measure + drive_layout explicitly.
        // NOTE (AppKit): the page/layout host panels are plain (UNFLIPPED) NSViews, so a vertical stack
        // arranges its first child at MAUI-y≈0 — which AppKit renders at the BOTTOM. To keep the content
        // fully on-screen we size the window's content height to the MEASURED content height (clamped to
        // k_window_height), so the bottom-anchored block fills the visible window top-to-bottom rather
        // than sinking off the bottom edge of a taller window. A purely presentational gallery choice.
        // The measure here computes that clamped height; drive_layout then measures + arranges the tree over
        // {0,0,k_window_width,content_height} through the generic path (the gallery has no VC-backed roots
        // on macOS — root_view_controller() is always null — so the safe-area branch == full bounds).
        auto& root = static_cast<maui::core::i_view&>(app->page_member().page());
        const maui::graphics::size measured = root.measure(k_window_width, k_window_height);
        const double content_height =
            std::min(k_window_height, measured.height > 0 ? measured.height : k_window_height);
        maui::hosting::drive_layout(app->win(), k_window_width, content_height);

        // (4) Show the NSWindow centered + frontmost.
        auto* const ns_window = (__bridge NSWindow*)window_handler->typed_platform_view()->native;

        // Appearance toggle for the parity-comparison capture loop: MAUI_APPEARANCE=dark|light forces the
        // window's NSAppearance so native AppKit controls render in the requested theme (default: light/Aqua).
        // Set on the window so it propagates to the content view + every native child before the snapshot.
        const char* const appearance = std::getenv("MAUI_APPEARANCE");
        const bool dark = appearance != nullptr && std::strcmp(appearance, "dark") == 0;
        ns_window.appearance = [NSAppearance appearanceNamed:(dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua)];

        [ns_window setContentSize:NSMakeSize(k_window_width, content_height)];
        [ns_window center];
        [ns_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];

        // Self-screenshot mode (for the doc-capture loop): if MAUI_SHOT_PATH is set, spin the run loop
        // briefly so the native controls render, snapshot the content view to a PNG, and exit — a clean,
        // deterministic, window-only capture with no external screen-capture tool or TCC permission.
        const char* const shot = std::getenv("MAUI_SHOT_PATH");
        if (shot != nullptr && std::strlen(shot) > 0)
        {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.7]];
            NSView* const view = ns_window.contentView;
            NSBitmapImageRep* const rep = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
            [view cacheDisplayInRect:view.bounds toBitmapImageRep:rep];
            NSData* const png = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
            [png writeToFile:@(shot) atomically:YES];
            os_log(OS_LOG_DEFAULT, "[gallery] wrote shot %{public}s", shot);
            return 0;
        }
        [NSApp run];
        return 0;
    }

    // Make the bundled demo assets resolvable. The gallery binary is a PLAIN executable (not a .app), so
    // the macOS image loader resolves from_file() paths against the process CWD and a non-bundle binary
    // cannot use Info.plist ATSApplicationFontsPath. The CMake POST_BUILD step copies the assets next to
    // the binary: chdir there so "dotnet_bot.png" etc. resolve, and register the bundled ionicons.ttf
    // with CoreText so the Image page's "Font Image Source" rows find the "Ionicons" family (the port has
    // no runtime font-registration seam — the iOS twin gets the same via UIAppFonts). Best-effort: a
    // missing dir / already-registered font just leaves the corresponding sources unresolved, as before.
    void prepare_gallery_resources()
    {
        NSString* const exe = [[NSBundle mainBundle] executablePath];
        NSString* const dir = exe.stringByDeletingLastPathComponent;
        if (dir.length == 0)
        {
            return;
        }
        if (chdir(dir.fileSystemRepresentation) != 0)
        {
            os_log_error(OS_LOG_DEFAULT, "[gallery] could not chdir to %{public}s (assets may not resolve)",
                         dir.fileSystemRepresentation);
        }

        NSString* const font_path = [dir stringByAppendingPathComponent:@"ionicons.ttf"];
        if (![NSFileManager.defaultManager fileExistsAtPath:font_path])
        {
            return;
        }
        CFErrorRef error = nullptr;
        if (!CTFontManagerRegisterFontsForURL((__bridge CFURLRef)[NSURL fileURLWithPath:font_path],
                                              kCTFontManagerScopeProcess, &error))
        {
            os_log(OS_LOG_DEFAULT, "[gallery] ionicons.ttf not newly registered (already present or in use)");
        }
        if (error != nullptr)
        {
            CFRelease(error);
        }
    }

    int run_gallery()
    {
        prepare_gallery_resources();

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

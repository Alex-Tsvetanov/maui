// maui_app_sample — the full M5c vertical slice on macOS: an application hosting a window hosting a
// content_page hosting a button, every layer wired through its handler, opened with app.open_window(win).
// Clicking the native NSButton flows back to the control's `clicked` event, which updates the button text
// (forward again through the mapper). So the app demonstrates the whole virtual-view ⇄ handler ⇄ native
// seam end-to-end through the Application/Window/Page lifecycle — the native NSWindow host (window_handler)
// in particular. Build (apple preset):
//   cmake --build --preset apple --target maui_app_sample
// then run ./build/apple/maui_app_sample. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"

int main()
{
    @autoreleasepool
    {
        NSApplication* const ns_app = [NSApplication sharedApplication];
        [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // The cross-platform element tree + a handler per element (static: lives for the whole run loop).
        static maui::controls::application app;
        static maui::controls::window win;
        static maui::controls::content_page page;
        static maui::controls::button control;

        control.set_text("Click me");

        // Attach handlers bottom-up so each parent can host its child's native view: button → page → window.
        auto button_handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(button_handler);
        page.set_content(control);

        auto page_handler = std::make_shared<maui::core::content_page_handler>();
        page.set_handler(page_handler);

        win.set_title("MAUI C++ — app sample");
        win.set_content(page);
        auto window_handler = std::make_shared<maui::core::window_handler>();
        win.set_handler(window_handler); // window_handler sets the NSWindow's contentView to the page's view

        // React to the native tap purely through the cross-platform API.
        control.clicked.connect([] {
            static int count = 0;
            ++count;
            control.set_text("Clicked " + std::to_string(count));
            NSLog(@"[sample] button clicked: %d", count);
        });

        // Size + place the button within the page's host (the page does no auto-layout in this cut).
        NSButton* const button = (__bridge NSButton*)button_handler->typed_platform_view()->native;
        [button setFrame:NSMakeRect(60, 80, 180, 44)];

        // Open the window through the Application lifecycle: SendStart → Created → Activated (Loaded +
        // Appearing flow down the page subtree). Then size + show the real NSWindow.
        app.open_window(win);

        NSWindow* const ns_window = (__bridge NSWindow*)window_handler->typed_platform_view()->native;
        [ns_window setContentSize:NSMakeSize(300, 200)];
        [ns_window center];
        [ns_window makeKeyAndOrderFront:nil];
        [ns_app activateIgnoringOtherApps:YES];
        [ns_app run];
        return 0;
    }
}

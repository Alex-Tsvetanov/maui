// Minimal macOS sample for the M2 Rosetta Stone: a real window hosting the cross-platform
// maui::controls::button via its AppKit handler. Clicking the native NSButton flows back to the
// control's `clicked` event, whose handler calls set_text(...) — which flows forward again through the
// mapper to update the NSButton's title. So a tap visibly round-trips the whole virtual-view ⇄ handler
// ⇄ native seam. Build (apple preset): `cmake --build --preset apple --target maui_button_sample`,
// then run `./build/apple/maui_button_sample`. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"

int main()
{
    @autoreleasepool
    {
        NSApplication* const app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // The cross-platform control + its AppKit handler (static: lives for the whole run loop).
        static maui::controls::button control;
        control.set_text("Click me");
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        // React to the native tap purely through the cross-platform API.
        control.clicked.connect([] {
            static int count = 0;
            ++count;
            control.set_text("Clicked " + std::to_string(count));
            NSLog(@"[sample] button clicked: %d", count);
        });

        NSButton* const button = (__bridge NSButton*)handler->typed_platform_view()->native;
        [button setFrame:NSMakeRect(60, 80, 180, 44)];

        NSWindow* const window =
            [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 300, 200)
                                        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
        [window setTitle:@"MAUI C++ — button sample"];
        [window.contentView addSubview:button];
        [window center];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];
        [app run];
        return 0;
    }
}

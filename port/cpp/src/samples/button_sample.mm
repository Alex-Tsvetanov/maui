// Minimal macOS sample for the M2 Rosetta Stone: a real window hosting the cross-platform
// maui::controls::button via its AppKit handler. Clicking the native NSButton flows back to the
// control's `clicked` event, whose handler calls set_text(...) — which flows forward again through the
// mapper to update the NSButton's title. So a tap visibly round-trips the whole virtual-view ⇄ handler
// ⇄ native seam. Since M-L the sample boots through the HOSTING layer: maui_app::create_builder()
// composes the registries (the default controls handler table included), and the button's handler is
// resolved from the built maui_app's handler table (attach_handler — the ToHandler analog) instead of
// being hand-constructed. Build (apple preset): `cmake --build --preset apple --target
// maui_button_sample`, then run `./build/apple/maui_button_sample`. Compiled as Objective-C++ with ARC.
// Traces go to the unified log (os_log, the non-vararg logging surface NSLog is not).
#import <AppKit/AppKit.h>
#import <os/log.h>

#include <exception>
#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

namespace
{
    // The sample body — separated from main so a boot failure (build()/attach_handler throw) surfaces
    // as a logged non-zero exit instead of an exception escaping main.
    int run_sample()
    {
        NSApplication* const app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Builder boot (statics: alive for the whole run loop; the maui_app outlives the handlers it
        // contexts, per the maui_app.hpp lifetime doctrine). The base controls::application suffices —
        // this minimal sample manages its own NSWindow rather than the Window/Page slice.
        static maui::controls::button control;
        static std::unique_ptr<maui::hosting::maui_app> maui_app =
            maui::hosting::maui_app::create_builder().use_maui_app<maui::controls::application>().build();

        control.set_text("Click me");
        // Resolve + attach the registered AppKit handler from the built handler table (ToHandler).
        const auto handler = std::dynamic_pointer_cast<maui::core::button_handler>(maui_app->attach_handler(control));

        // React to the native tap purely through the cross-platform API.
        control.clicked.connect([] {
            static int count = 0;
            ++count;
            control.set_text("Clicked " + std::to_string(count));
            os_log(OS_LOG_DEFAULT, "[sample] button clicked: %d", count);
        });

        auto* const button = (__bridge NSButton*)handler->typed_platform_view()->native;
        [button setFrame:NSMakeRect(60, 80, 180, 44)];

        NSWindow* const window =
            [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 300, 200)
                                        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
        [window setTitle:NSLocalizedString(@"MAUI C++ — button sample", nil)];
        [window.contentView addSubview:button];
        [window center];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];
        [app run];
        return 0;
    }
} // namespace

int main()
{
    @autoreleasepool
    {
        try
        {
            return run_sample();
        }
        catch (const std::exception& error)
        {
            os_log_error(OS_LOG_DEFAULT, "[sample] boot failed: %{public}s", error.what());
            return 1;
        }
        catch (...)
        {
            os_log_error(OS_LOG_DEFAULT, "[sample] boot failed: unknown exception");
            return 1;
        }
    }
}

// maui_app_sample — the full M5c vertical slice on macOS, booted through the M-L HOSTING layer: a
// maui_app_builder mints the application subclass (use_maui_app<sample_app>, which OWNS the window →
// content_page → button tree, the Application.CreateWindow shape), the handlers are resolved from the
// built handler table (attach_handler, bottom-up so each parent hosts its child's native view), the
// bridged window lifecycle logs through ConfigureLifecycleEvents, and the window opens through
// maui_app::open_window (SendStart → Created → Activated; the window_handler hosts the page in a real
// NSWindow). Clicking the native NSButton flows back to the control's `clicked` event, which updates
// the button text (forward again through the mapper) — the whole virtual-view ⇄ handler ⇄ native seam
// end-to-end through the builder-boot lifecycle. Build (apple preset):
//   cmake --build --preset apple --target maui_app_sample
// then run ./build/apple/maui_app_sample. Compiled as Objective-C++ with ARC. Traces go to the
// unified log (os_log).
#import <AppKit/AppKit.h>
#import <os/log.h>

#include <exception>
#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/i_lifecycle_builder.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/hosting/window_lifecycle_events.hpp"

namespace
{
    // The application subclass use_maui_app<TApp> mints: it owns the element tree (the C# App.xaml.cs
    // shape — Application.CreateWindow returns the window it manages).
    class sample_app final : public maui::controls::application
    {
    public:
        sample_app()
        {
            button_.set_text("Click me");
            page_.set_content(button_);
            window_.set_title("MAUI C++ — app sample");
            window_.set_content(page_);
        }

        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return &window_;
        }

        [[nodiscard]] maui::controls::window& sample_window()
        {
            return window_;
        }
        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }
        [[nodiscard]] maui::controls::button& button()
        {
            return button_;
        }

    private:
        maui::controls::window window_;
        maui::controls::content_page page_;
        maui::controls::button button_;
    };
} // namespace

namespace
{
    // The sample body — separated from main so a boot failure (build()/attach_handler throw) surfaces
    // as a logged non-zero exit instead of an exception escaping main. Traces go to the unified log.
    int run_sample()
    {
        NSApplication* const ns_app = [NSApplication sharedApplication];
        [ns_app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Builder boot (static: alive for the whole run loop; it owns the application, which owns the
        // tree). The bridged window lifecycle demonstrates ConfigureLifecycleEvents end-to-end.
        static std::unique_ptr<maui::hosting::maui_app> maui_app =
            maui::hosting::maui_app::create_builder()
                .use_maui_app<sample_app>()
                .configure_lifecycle_events([](maui::hosting::i_lifecycle_builder& lifecycle) {
                    lifecycle.add_event(maui::hosting::window_lifecycle_events::created,
                                        [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window created"); });
                    lifecycle.add_event(maui::hosting::window_lifecycle_events::activated,
                                        [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window activated"); });
                    lifecycle.add_event(maui::hosting::window_lifecycle_events::destroying,
                                        [] { os_log(OS_LOG_DEFAULT, "[sample] lifecycle: window destroying"); });
                })
                .build();
        sample_app* const app = maui_app->application_as<sample_app>().get();

        // Attach handlers from the built handler table, bottom-up so each parent can host its child's
        // native view: button → page → window (ToHandler at every level).
        const auto button_handler =
            std::dynamic_pointer_cast<maui::core::button_handler>(maui_app->attach_handler(app->button()));
        maui_app->attach_handler(app->page());
        const auto window_handler =
            std::dynamic_pointer_cast<maui::core::window_handler>(maui_app->attach_handler(app->sample_window()));

        // React to the native tap purely through the cross-platform API (a raw back-pointer, NOT an
        // owning reference — the maui_app's application owns the tree, PROFILE §8).
        maui::controls::button* const button = &app->button();
        button->clicked.connect([button] {
            static int count = 0;
            ++count;
            button->set_text("Clicked " + std::to_string(count));
            os_log(OS_LOG_DEFAULT, "[sample] button clicked: %d", count);
        });

        // Size + place the button within the page's host (the page does no auto-layout in this cut).
        auto* const native_button = (__bridge NSButton*)button_handler->typed_platform_view()->native;
        [native_button setFrame:NSMakeRect(60, 80, 180, 44)];

        // Open the window through the hosting door: the lifecycle bridge connects FIRST, then the
        // application drives SendStart → Created → Activated (Loaded + Appearing flow down the page).
        maui_app->open_window(app->sample_window());

        auto* const ns_window = (__bridge NSWindow*)window_handler->typed_platform_view()->native;
        [ns_window setContentSize:NSMakeSize(300, 200)];
        [ns_window center];
        [ns_window makeKeyAndOrderFront:nil];
        [ns_app activateIgnoringOtherApps:YES];
        [ns_app run];
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

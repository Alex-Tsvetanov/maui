// run_app — the HEADLESS backend body of maui::hosting::run_app (host_run.hpp).
//
// Headless has no native run loop and no device: booting the app + a single mount + one layout (settle)
// pass is enough to prove the cross-platform mount end-to-end (the M0-M5 "no device" doctrine, STATUS.md).
// It builds the app from the user's configurator, asks the application for its window (IApplication
// .CreateWindow — the user's create_window override), mounts the window's element tree generically
// (app_host.hpp), drives a layout pass at a default phone-ish size, and returns 0.
//
// Stage 2 (apple/ios/windows/android) supplies its OWN host_run translation unit: the SAME mount_window +
// drive_layout, wrapped in the platform run loop (UIApplicationMain, the WinUI lifecycle, …) and using the
// native safe-area-derived bounds instead of the fixed default below. The build links exactly one host_run
// .cpp per backend, keyed off the platform macros src/platform/* already use — so this file is the
// headless lane only (it is listed under the headless sources in CMakeLists.txt).

#include "maui/hosting/host_run.hpp"

#include <memory>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

namespace maui::hosting
{
    namespace
    {
        // The default headless content size — a portrait phone viewport (the size the ios gallery boots at).
        // A native backend derives this from the window/safe area instead; headless has no device, so it is
        // a fixed settle size purely to drive one deterministic layout pass.
        constexpr double k_default_width = 402.0;
        constexpr double k_default_height = 874.0;
    } // namespace

    int run_app(int /*argc*/, char** /*argv*/, app_configurator configure)
    {
        // (1) Build the app from a FRESH builder the user's configurator populates (use_maui_app<App>()).
        std::unique_ptr<maui_app> app = configure(maui_app::create_builder()).build();

        // (2) Ask the application for its window (IApplication.CreateWindow — the user's create_window
        //     override returns the app-owned window). No application / no window ⇒ nothing to host: a clean
        //     exit (headless has no UI to leave running).
        const std::shared_ptr<maui::controls::application>& application = app->application();
        if (application == nullptr)
        {
            return 0;
        }
        auto* window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            return 0;
        }

        // (3) Generic mount: attach handlers across the window's element tree (children before parents),
        //     re-host each container, attach the window handler, and open the window.
        mount_window(*app, *window);

        // (4) One layout (settle) pass at the default size — the headless analog of the native run loop's
        //     first layout. Proves the tree measured + arranged; there is no loop to enter afterward.
        drive_layout(*window, k_default_width, k_default_height);

        return 0;
    }
} // namespace maui::hosting

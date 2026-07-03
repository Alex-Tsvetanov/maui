// run_app — the WINDOWS (WinUI 3 / Windows App SDK) backend body of maui::hosting::run_app
// (host_run.hpp).
//
// The native-run-loop twin of src/platform/apple/host_run.mm: instead of NSApplication + [NSApp run],
// it bootstraps the Windows App Runtime for an UNPACKAGED process (MddBootstrapInitialize2), stands up
// a code-only Microsoft.UI.Xaml.Application (no markup, no XAML compiler — the Application subclass
// implements IXamlMetadataProvider by delegating to XamlControlsXamlMetaDataProvider and merges
// XamlControlsResources programmatically so the WinUI control default styles resolve), builds the
// user's app through the SAME generic mount (app_host.hpp's mount_window + drive_layout — NO
// per-control knowledge), shows the real Microsoft.UI.Xaml.Window the window_handler created, and lets
// Application::Start run the dispatcher until the last window closes.
//
// No C# class maps 1:1: this is the port's analog of what MAUI's WinUI startup wraps (the generated
// XamlApp bootstrap + MauiWinUIApplication). The unpackaged-deployment recipe (bootstrap constants,
// app-local Bootstrap.dll, the framework resources.pri beside the exe) is encoded in
// cmake/windows_appsdk.cmake and was verified live on this machine before this lane was written.
//
// Lifetime (PROFILE §8): the built maui_app must outlive the whole dispatcher loop (it owns the
// application → window → page → control tree; handlers keep raw context pointers into it). It is held
// in the function-local-static host_state the Application callback populates — the exact shape the
// apple/ios lanes use for the NSApplication/UIApplicationMain hand-off.

#define NOMINMAX      // windows.h's min/max macros break std::min/std::max in every header after it
#include <windows.h>
#undef GetCurrentTime // collides with the Microsoft.UI.Xaml.Media.Animation projection

#include <MddBootstrap.h>
#include <WindowsAppSDK-VersionInfo.h>

#include <chrono>
#include <cstdio>
#include <exception>
#include <memory>

#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/base.h>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/host_run.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Markup;

    // The process-wide host state the XAML-instantiated Application must reach but cannot be passed
    // (Application::Start creates it via a callback). A Meyers singleton, mirroring the apple lane.
    struct host_state
    {
        maui::hosting::app_configurator configure = nullptr;
        std::unique_ptr<maui::hosting::maui_app> app;
    };

    host_state& state()
    {
        static host_state s;
        return s;
    }

    // The default content size a page settles into when the window has no explicit geometry. 480×800
    // matches the Windows .NET MAUI reference app (~/maui-compare, fixed 480×800) so the parity
    // captures share a canvas; the apple lane's 480×720 stays apple-local.
    constexpr double k_window_width = 480.0;
    constexpr double k_window_height = 800.0;

    // Drive the generic layout from inside a XAML delegate (SizeChanged / Loaded), attributing any
    // escaping exception first: an exception leaving the delegate is stowed by XAML and fail-fasts
    // the process (0xC000027B) — the UnhandledException hook logs the WinRT-visible message, but
    // this line says WHICH layout pass raised it, and covers plain C++ exceptions the hook's
    // message would render opaquely. Log-then-rethrow: the fail-fast behavior itself is unchanged.
    void drive_layout_attributed(const char* site, maui::controls::window& window, double width, double height)
    {
        try
        {
            maui::hosting::drive_layout(window, width, height);
        }
        catch (winrt::hresult_error const& error)
        {
            std::fprintf(stderr, "[host_run] %s drive_layout threw (hresult 0x%08X): %s\n", site,
                         static_cast<unsigned>(error.code().value),
                         winrt::to_string(error.message()).c_str());
            throw;
        }
        catch (const std::exception& error)
        {
            std::fprintf(stderr, "[host_run] %s drive_layout threw: %s\n", site, error.what());
            throw;
        }
    }

    // Boot the app + show its window through the generic mount (the apple boot_window recipe, WinUI
    // vocabulary). Returns true on a shown window.
    bool boot_window()
    {
        // (1) Build from a FRESH builder the user's configurator populates (use_maui_app<App>()).
        state().app = state().configure(maui::hosting::maui_app::create_builder()).build();

        // (2) IApplication.CreateWindow — the user's create_window override returns the app-owned window.
        const std::shared_ptr<maui::controls::application>& application = state().app->application();
        if (application == nullptr)
        {
            std::fprintf(stderr, "[host_run] no application configured (use_maui_app not called)\n");
            return false;
        }
        auto* const window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            std::fprintf(stderr, "[host_run] application produced no window\n");
            return false;
        }

        // (3) Generic mount — the window_handler's windows partial creates the real
        //     Microsoft.UI.Xaml.Window and hosts the page's native view as its Content.
        maui::hosting::mount_window(*state().app, *window);

        // (4) One layout pass over the default content size (the host does no auto-layout);
        //     the SizeChanged hook below re-drives it on every native resize.
        maui::hosting::drive_layout(*window, k_window_width, k_window_height);

        // (5) Reach the native Window through the now-attached handler and show it.
        const auto window_handler = std::dynamic_pointer_cast<maui::core::window_handler>(window->handler());
        if (window_handler == nullptr || window_handler->typed_platform_view() == nullptr)
        {
            std::fprintf(stderr, "[host_run] window handler did not produce a native Window\n");
            return false;
        }
        auto native_window = maui::platform::win::borrow<mux::Window>(window_handler->typed_platform_view()->native);
        if (native_window == nullptr)
        {
            std::fprintf(stderr, "[host_run] native Window is null\n");
            return false;
        }

        // Native theme from the app's requested theme (the parity dark/light path — the windows twin of
        // forcing NSAppearance / overrideUserInterfaceStyle). WinUI: RequestedTheme on the root element
        // propagates to the whole native tree; `unspecified` leaves the system default.
        if (auto root = native_window.Content().try_as<mux::FrameworkElement>())
        {
            switch (application->requested_theme())
            {
                case maui::core::app_theme::dark:
                    root.RequestedTheme(mux::ElementTheme::Dark);
                    break;
                case maui::core::app_theme::light:
                    root.RequestedTheme(mux::ElementTheme::Light);
                    break;
                case maui::core::app_theme::unspecified:
                    break;
            }
        }

        // Default page background — what a .NET MAUI WinUI window shows behind an unstyled page.
        // A ThemeDictionaries walk is unreliable here (the dictionaries hand back brush instances
        // already evaluated under the OS theme, not the root's forced RequestedTheme), so the two
        // values are pinned from the REFERENCE APP's rendered pixels (~/maui-compare captures,
        // 2026-07-02: light #EDEDED, dark #181818 — the Mica-modulated base of
        // ApplicationPageBackgroundThemeBrush; the parity policy makes MAUI's render ground truth).
        // Only when the page left the panel background unset; `unspecified` follows the OS theme.
        if (auto root_panel = native_window.Content().try_as<muxc::Panel>())
        {
            if (root_panel.Background() == nullptr)
            {
                bool dark = root_panel.ActualTheme() == mux::ElementTheme::Dark;
                switch (application->requested_theme())
                {
                    case maui::core::app_theme::dark: dark = true; break;
                    case maui::core::app_theme::light: dark = false; break;
                    case maui::core::app_theme::unspecified: break;
                }
                const auto base = dark ? winrt::Windows::UI::Color{0xFF, 0x18, 0x18, 0x18}
                                       : winrt::Windows::UI::Color{0xFF, 0xED, 0xED, 0xED};
                root_panel.Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{base});
            }
        }

        // Size the CLIENT area to the settle size (the capture canvas), then re-drive the generic
        // layout on every native resize — Windows windows are user-resizable by default. The resize
        // subscription precedes ResizeClient so the resize it causes re-drives the layout too.
        native_window.SizeChanged([window](winrt::Windows::Foundation::IInspectable const&,
                                           mux::WindowSizeChangedEventArgs const& args) {
            drive_layout_attributed("SizeChanged", *window, args.Size().Width, args.Size().Height);
        });
        if (auto app_window = native_window.AppWindow())
        {
            app_window.ResizeClient(winrt::Windows::Graphics::SizeInt32{
                static_cast<int32_t>(k_window_width), static_cast<int32_t>(k_window_height)});
        }

        // The pre-activation drive_layout above measured TEMPLATE-LESS controls (XAML applies control
        // templates on load), so its desired sizes under-report (text-only button heights — the
        // clipped-title first render). Re-drive once the root has LOADED, when every control measures
        // through its real template — the windows analog of the first native layout pass. THEN a
        // bounded settle timer re-drives a handful of times over ~1.5s: a WinUI BitmapImage decodes
        // ASYNCHRONOUSLY, so an Image measures 0x0 at Loaded and only reports its real size once
        // ImageOpened fires — but the port drives its Canvas layout MANUALLY (unlike WinUI's own
        // reactive layout), so a late invalidate_measure has nothing to re-run. Re-driving over the
        // first ~1.5s (well inside the 2s capture settle) catches local image/gif decodes so image
        // rows no longer collapse to zero height. (A fully reactive layout — re-drive on any child
        // invalidate — is the future improvement; this bounded pass fixes the parity captures.)
        if (auto root = native_window.Content().try_as<mux::FrameworkElement>())
        {
            const auto dispatcher = native_window.DispatcherQueue();
            root.Loaded([window, dispatcher](winrt::Windows::Foundation::IInspectable const&,
                                             mux::RoutedEventArgs const&) {
                drive_layout_attributed("Loaded", *window, k_window_width, k_window_height);
                if (dispatcher == nullptr)
                {
                    return;
                }
                auto timer = dispatcher.CreateTimer();
                timer.Interval(std::chrono::milliseconds(250));
                auto ticks = std::make_shared<int>(0);
                timer.Tick([window, timer, ticks](winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer const&,
                                                  winrt::Windows::Foundation::IInspectable const&) {
                    drive_layout_attributed("settle", *window, k_window_width, k_window_height);
                    if (++*ticks >= 6)
                    {
                        timer.Stop();
                    }
                });
                timer.Start();
            });
        }

        std::fprintf(stderr, "[host_run] mounted app window '%s' — showing Window\n",
                     std::string(window->title()).c_str());
        native_window.Activate();
        return true;
    }
} // namespace

namespace
{
    // The code-only WinUI 3 Application. IXamlMetadataProvider is REQUIRED as soon as
    // XamlControlsResources is used (microsoft-ui-xaml#7606); delegating to
    // XamlControlsXamlMetaDataProvider covers every built-in WinUI type without a XAML compiler.
    struct maui_host_app : mux::ApplicationT<maui_host_app, muxm::IXamlMetadataProvider>
    {
        maui_host_app()
        {
            // A XAML-raised error (one thrown inside a framework callback — a layout pass, an event
            // delegate) never unwinds through OnLaunched's catch: XAML stows it and fail-fasts the
            // process (0xC000027B) with NO stderr. This hook logs the code + message first, so a boot
            // crash is diagnosable from the console (the port's analog of MAUI's
            // MauiWinUIApplication.OnApplicationUnhandledException log-then-rethrow).
            UnhandledException([](winrt::Windows::Foundation::IInspectable const&,
                                  mux::UnhandledExceptionEventArgs const& args) {
                std::fprintf(stderr, "[host_run] unhandled XAML exception (hresult 0x%08X): %s\n",
                             static_cast<unsigned>(args.Exception().value),
                             winrt::to_string(args.Message()).c_str());
                std::fflush(stderr);
            });
        }

        muxm::IXamlType GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const& type)
        {
            return provider_.GetXamlType(type);
        }
        muxm::IXamlType GetXamlType(winrt::hstring const& name)
        {
            return provider_.GetXamlType(name);
        }
        winrt::com_array<muxm::XmlnsDefinition> GetXmlnsDefinitions()
        {
            return provider_.GetXmlnsDefinitions();
        }

        void OnLaunched(mux::LaunchActivatedEventArgs const&)
        {
            try
            {
                // The WinUI control default styles (generic.xaml) — merged programmatically, in
                // OnLaunched, NOT the constructor (the framework resources.pri beside the exe backs it).
                Resources().MergedDictionaries().Append(muxc::XamlControlsResources{});
                if (!boot_window())
                {
                    Exit();
                }
            }
            catch (const std::exception& error)
            {
                std::fprintf(stderr, "[host_run] boot failed: %s\n", error.what());
                Exit();
            }
            catch (winrt::hresult_error const& error)
            {
                std::fprintf(stderr, "[host_run] boot failed (hresult 0x%08X): %s\n",
                             static_cast<unsigned>(error.code().value),
                             winrt::to_string(error.message()).c_str());
                Exit();
            }
        }

    private:
        mux::XamlTypeInfo::XamlControlsXamlMetaDataProvider provider_;
    };
} // namespace

namespace maui::hosting
{
    int run_app(int /*argc*/, char** /*argv*/, app_configurator configure)
    {
        // Unbuffered stderr: redirected-to-file stderr is FULLY buffered on the MSVC CRT, and a XAML
        // fail-fast (0xC000027B) kills the process without the CRT shutdown flush — every breadcrumb
        // before the crash would be lost. The boot log is small; unbuffered keeps it truthful.
        std::setvbuf(stderr, nullptr, _IONBF, 0);

        // Per-monitor-V2 DPI awareness before any HWND exists — crisp native rendering for the parity
        // captures without an embedded manifest (the runtime call is the manifest's equivalent).
        ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // Windows App Runtime for the unpackaged process: match the packaged framework by
        // major.minor + tag; any installed runtime ≥ the floor satisfies (PACKAGE_VERSION{} = no floor).
        const HRESULT bootstrap = ::MddBootstrapInitialize2(
            WINDOWSAPPSDK_RELEASE_MAJORMINOR, WINDOWSAPPSDK_RELEASE_VERSION_TAG_W, PACKAGE_VERSION{},
            MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
        if (FAILED(bootstrap))
        {
            std::fprintf(stderr, "[host_run] MddBootstrapInitialize2 failed (0x%08lX) — is the Windows App "
                                 "Runtime %u.%u installed?\n",
                         static_cast<unsigned long>(bootstrap), WINDOWSAPPSDK_RELEASE_MAJOR,
                         WINDOWSAPPSDK_RELEASE_MINOR);
            return 1;
        }

        try
        {
            state().configure = configure;
            winrt::init_apartment(winrt::apartment_type::single_threaded);
            // Creates the UI thread's DispatcherQueue and pumps until the last window closes.
            mux::Application::Start([](auto&&) { winrt::make<maui_host_app>(); });
            ::MddBootstrapShutdown();
            return 0;
        }
        catch (const std::exception& error)
        {
            std::fprintf(stderr, "[host_run] boot failed: %s\n", error.what());
            ::MddBootstrapShutdown();
            return 1;
        }
        catch (winrt::hresult_error const& error)
        {
            std::fprintf(stderr, "[host_run] boot failed (hresult 0x%08X): %s\n",
                         static_cast<unsigned>(error.code().value), winrt::to_string(error.message()).c_str());
            ::MddBootstrapShutdown();
            return 1;
        }
    }
} // namespace maui::hosting

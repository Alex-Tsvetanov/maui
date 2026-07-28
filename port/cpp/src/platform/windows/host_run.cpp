// run_app — the WINDOWS (WinUI 3) backend body of maui::hosting::run_app (host_run.hpp).
//
// The same mount_window + drive_layout the headless lane runs, wrapped in the WinUI 3 application
// lifecycle. Everything structural here was proven first by tools/parity/windows/winui_probe (a
// standalone code-only C++/WinRT app) precisely so that a failure in THIS file would be about the port
// and not about the toolchain. The three things that probe established, all of which are load-bearing:
//
//   1. THE BOOTSTRAPPER. An UNPACKAGED app must call MddBootstrapInitialize2 before touching any WinUI
//      type, or every activation fails with REGDB_E_CLASSNOTREG - an error that says nothing about
//      bootstrapping.
//   2. IXamlMetadataProvider. Normally generated from App.xaml by the XAML compiler. A code-only app
//      supplies it by delegating to the WinUI controls' own provider, or control activation throws.
//   3. XamlControlsResources MUST BE MERGED IN OnLaunched, NOT IN THE CONSTRUCTOR. Activating a XAML
//      type from the Application ctor dies with a stowed exception (0xC000027B) inside combase, because
//      the App object is not yet fully constructed as far as the XAML framework is concerned.

#include "maui/hosting/host_run.hpp"

#include <windows.h>
// windows.h defines GetCurrentTime as a function-like macro (it aliases GetTickCount). The C++/WinRT
// projection for Microsoft.UI.Xaml.Media.Animation declares a Timeline::GetCurrentTime method, and the
// macro then eats its argument list: "C4002 too many arguments for function-like macro invocation".
// Every WinUI C++ app has to undo it; there is no NOxxx guard that suppresses it.
#undef GetCurrentTime

#include <MddBootstrap.h>

#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <memory>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;

    // The fallback content size, used only until the real window reports one. A desktop-ish default
    // rather than the headless lane's phone viewport: this backend always has a real window, so this is
    // just the value for the single frame before the first SizeChanged arrives.
    constexpr double k_default_width = 1024.0;
    constexpr double k_default_height = 800.0;

    // The Windows App SDK major/minor the app is built against (1.7). It must match the package restored
    // by tools/parity/windows/build_winui_probe.ps1; a mismatch fails the bootstrap at startup.
    constexpr UINT32 k_wasdk_major_minor = 0x00010007;

    // The native Window behind a mounted maui window, or a null projected object if it is not attached.
    winui::Window native_window_of(maui::controls::window& window)
    {
        auto* handler = dynamic_cast<maui::core::window_handler*>(window.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        auto* platform = handler->typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::Window>(platform->native);
    }

    // The code-only WinUI application object. ApplicationT supplies the Application base; the extra
    // IXamlMetadataProvider is what the XAML compiler would otherwise have generated.
    struct maui_winui_app : winui::ApplicationT<maui_winui_app, winui::Markup::IXamlMetadataProvider>
    {
        explicit maui_winui_app(maui::hosting::app_configurator configure) : configure_(std::move(configure))
        {
            // Deliberately EMPTY of XAML work - see note 3 in the file header.
        }

        void OnLaunched(const winui::LaunchActivatedEventArgs&)
        {
            // Merge the WinUI control styles. Without them the controls still activate but render
            // unstyled, which looks like a broken layout rather than a missing resource dictionary.
            Resources().MergedDictionaries().Append(winui::Controls::XamlControlsResources{});

            // (1) Build the app from a FRESH builder the user's configurator populates.
            app_ = configure_(maui::hosting::maui_app::create_builder()).build();
            const std::shared_ptr<maui::controls::application>& application = app_->application();
            if (application == nullptr)
            {
                return;
            }
            window_ = dynamic_cast<maui::controls::window*>(application->create_window());
            if (window_ == nullptr)
            {
                return;
            }

            // (2) Generic mount: handlers across the tree (children before parents), the window handler
            //     last - which creates the native Window and hosts the page as its Content.
            maui::hosting::mount_window(*app_, *window_);

            const winui::Window native = native_window_of(*window_);
            if (native == nullptr)
            {
                return;
            }

            // (3) Re-layout on every resize. This is not a nicety: the E2E runner pins the window to an
            //     explicit rect AFTER launching the process, so the size the app boots at is never the
            //     size it is captured at. Without this the capture would show the boot layout.
            native.SizeChanged(
                [this](const winrt::Windows::Foundation::IInspectable&, const winui::WindowSizeChangedEventArgs& args) {
                    if (window_ != nullptr)
                    {
                        maui::hosting::drive_layout(*window_, args.Size().Width, args.Size().Height);
                    }
                });

            // (4) Show it, then lay out at the size it actually got. Activate first because a WinUI
            //     window has no client size until it is shown - laying out before would use the fallback
            //     and then immediately be corrected by the SizeChanged above.
            native.Activate();
            double width = k_default_width;
            double height = k_default_height;
            if (const auto app_window = native.AppWindow())
            {
                const auto client = app_window.ClientSize();
                if (client.Width > 0 && client.Height > 0)
                {
                    width = client.Width;
                    height = client.Height;
                }
            }
            maui::hosting::drive_layout(*window_, width, height);
        }

        // ---- IXamlMetadataProvider: delegate to the WinUI controls' own provider ---------------------
        // Every XAML type resolved at runtime comes through here. Returning empty (the tempting stub)
        // makes control activation fail later with an opaque error.
        winui::Markup::IXamlType GetXamlType(const winrt::Windows::UI::Xaml::Interop::TypeName& type)
        {
            return provider_.GetXamlType(type);
        }
        winui::Markup::IXamlType GetXamlType(const winrt::hstring& full_name)
        {
            return provider_.GetXamlType(full_name);
        }
        winrt::com_array<winui::Markup::XmlnsDefinition> GetXmlnsDefinitions()
        {
            return provider_.GetXmlnsDefinitions();
        }

    private:
        maui::hosting::app_configurator configure_;
        std::unique_ptr<maui::hosting::maui_app> app_;
        // NON-owning: the window is owned by the application (IApplication.CreateWindow), which the
        // maui_app above owns - so it outlives every use here.
        maui::controls::window* window_ = nullptr;
        winui::XamlTypeInfo::XamlControlsXamlMetaDataProvider provider_;
    };
} // namespace

namespace maui::hosting
{
    int run_app(int /*argc*/, char** /*argv*/, app_configurator configure)
    {
        // The bootstrapper FIRST: unpackaged apps have no package graph, so without it every WinUI
        // activation below fails with REGDB_E_CLASSNOTREG.
        if (FAILED(MddBootstrapInitialize2(k_wasdk_major_minor, L"", PACKAGE_VERSION{},
                                           MddBootstrapInitializeOptions_OnNoMatch_ShowUI)))
        {
            ::MessageBoxW(nullptr, L"The Windows App Runtime is not installed.", L"maui", MB_ICONERROR);
            return 1;
        }

        int exit_code = 0;
        try
        {
            winrt::init_apartment(winrt::apartment_type::single_threaded);
            // Application::Start owns the message loop and returns only when the app exits, so this call
            // IS the run loop - the Windows analogue of UIApplicationMain.
            winui::Application::Start([configure = std::move(configure)](auto&&) mutable {
                winrt::make<maui_winui_app>(std::move(configure));
            });
        }
        catch (const winrt::hresult_error&)
        {
            exit_code = 2;
        }
        catch (const std::exception&)
        {
            exit_code = 3;
        }
        MddBootstrapShutdown();
        return exit_code;
    }
} // namespace maui::hosting

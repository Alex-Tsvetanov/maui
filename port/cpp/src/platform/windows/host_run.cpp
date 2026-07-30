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

#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
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

    // Per-step boot logging, OFF unless MAUI_WINUI_LOG names a file. This exists because a WinUI
    // failure gives you nothing: a stowed exception (0xC000027B) unwinds through combase with no
    // message, no stack and no console -- the process is simply gone, and WER names a system DLL
    // rather than the call that provoked it. The winui_probe carried the same logging for the same
    // reason; without it you are reduced to guessing, and two guesses have already been wrong on the
    // `device` page. Env-gated so a normal run pays one getenv and writes nothing.
    //
    // Opened and closed per line, deliberately: the whole point is to survive a process that dies
    // mid-call, and a buffered stream would lose exactly the last line -- the one naming the step
    // that killed it.
    void boot_log(std::string_view step)
    {
        const char* const path = std::getenv("MAUI_WINUI_LOG");
        if (path == nullptr)
        {
            return;
        }
        std::FILE* file = nullptr;
        if (fopen_s(&file, path, "a") != 0 || file == nullptr)
        {
            return;
        }
        std::fwrite(step.data(), 1, step.size(), file);
        std::fputc('\n', file);
        std::fclose(file);
    }

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

    // The port's own layout viewport: `raw_height` (Window.Bounds.Height) minus the reserved title-bar
    // band, when there is one (winui_interop.hpp's has_extended_title_bar/k_app_title_bar_height).
    // Bounds now covers the WHOLE window rect (window_handler.cpp's create_platform_view extends
    // content into the title bar), so handing it to drive_layout unadjusted would lay out 32 DIPs past
    // where MAUI's own page viewport ends (WindowRootView's content is inset by that same band).
    double content_viewport_height(const winui::Window& native, double raw_height)
    {
        const double inset = maui::platform::windows::has_extended_title_bar(native)
                                 ? maui::platform::windows::k_app_title_bar_height
                                 : 0.0;
        return std::max(0.0, raw_height - inset);
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
            boot_log("OnLaunched: entered");
            Resources().MergedDictionaries().Append(winui::Controls::XamlControlsResources{});
            boot_log("OnLaunched: XamlControlsResources merged");

            // (1) Build the app from a FRESH builder the user's configurator populates.
            boot_log("build: configure_ + build");
            app_ = configure_(maui::hosting::maui_app::create_builder()).build();
            boot_log("build: maui_app built");
            const std::shared_ptr<maui::controls::application>& application = app_->application();
            if (application == nullptr)
            {
                // EXIT, do not just return: Application::Start owns the message loop and keeps pumping
                // after OnLaunched returns, so a bare return leaves a windowless process running forever
                // - invisible in the E2E runner, which then reports a capture timeout rather than a
                // startup failure. The headless lane can `return 0` here because it has no loop to leave.
                Exit();
                return;
            }
            boot_log("window: create_window");
            window_ = dynamic_cast<maui::controls::window*>(application->create_window());
            boot_log("window: created");
            if (window_ == nullptr)
            {
                Exit();
                return;
            }

            // (1b) The APPLICATION theme, set here and nowhere else: Application.RequestedTheme may only
            //      be assigned before the first window exists, and mount_window below is what creates it.
            //      This slot is the only point where both are true - the app has been built (so
            //      requested_theme() is known) and no Window has been made yet.
            //
            //      Why this and not just the content root's FrameworkElement.RequestedTheme (which the
            //      first attempt used): a per-element theme correctly restyles the CONTROLS, but every
            //      manual `Application.Resources.Lookup` still resolves against the APP theme. The dark
            //      capture proved it - white text on a light page, 97% differing. Setting the app theme
            //      makes every theme-resource resolution, mine and the framework's, agree.
            switch (application->requested_theme())
            {
                case maui::core::app_theme::dark:
                    winui::Application::Current().RequestedTheme(winui::ApplicationTheme::Dark);
                    break;
                case maui::core::app_theme::light:
                    winui::Application::Current().RequestedTheme(winui::ApplicationTheme::Light);
                    break;
                case maui::core::app_theme::unspecified:
                    break; // leave the system default
            }
            boot_log("theme: application theme set");

            // (2) Generic mount: handlers across the tree (children before parents), the window handler
            //     last - which creates the native Window and hosts the page as its Content.
            boot_log("mount: mount_window");
            maui::hosting::mount_window(*app_, *window_);
            boot_log("mount: mounted");

            const winui::Window native = native_window_of(*window_);
            if (native == nullptr)
            {
                Exit(); // see the note above: never leave the loop pumping with nothing on screen
                return;
            }

            // (3) Re-layout on every resize. This is not a nicety: the E2E runner pins the window to an
            //     explicit rect AFTER launching the process, so the size the app boots at is never the
            //     size it is captured at. Without this the capture would show the boot layout.
            native.SizeChanged([this, native](const winrt::Windows::Foundation::IInspectable&,
                                              const winui::WindowSizeChangedEventArgs& args) {
                if (window_ != nullptr)
                {
                    maui::hosting::drive_layout(*window_, args.Size().Width,
                                                content_viewport_height(native, args.Size().Height));
                }
            });

            // (3b) Native theme from the app's REQUESTED theme - the parity-capture dark/light path, and
            //      the twin of what the iOS host does with overrideUserInterfaceStyle. The gallery sets
            //      the cross-platform theme in pure C++ (application::set_platform_app_theme, seeded from
            //      MAUI_APPEARANCE); without pushing it to XAML here, every native control would keep
            //      rendering in the system theme and a "dark" capture would come back LIGHT - a
            //      plausible-looking but entirely wrong board row.
            //
            //      Belt-and-braces on top of the APPLICATION theme set at (1b), which is the actual
            //      mechanism. Kept because it makes the root's theme explicit in the visual tree and
            //      costs one property set; if (1b) ever cannot run (an already-initialised Application),
            //      this still restyles the controls even though resource lookups would go light.
            if (const auto themed = native.Content().try_as<winui::FrameworkElement>())
            {
                switch (application->requested_theme())
                {
                    case maui::core::app_theme::dark:
                        themed.RequestedTheme(winui::ElementTheme::Dark);
                        break;
                    case maui::core::app_theme::light:
                        themed.RequestedTheme(winui::ElementTheme::Light);
                        break;
                    case maui::core::app_theme::unspecified:
                        themed.RequestedTheme(winui::ElementTheme::Default);
                        break;
                }
            }

            // (3c) The themed PAGE BACKGROUND, painted opaquely on the content root.
            //
            //      MauiWinUIWindow.cs:52-54 DOES set a Mica BaseAlt SystemBackdrop, and I once read
            //      that as the mechanism and switched to it. MEASURED RESULT: greens 10 -> 0, and the
            //      light page went grey (~#E9E9E9) where MAUI is near-white. The oracle line is real;
            //      the conclusion was not. MAUI's WindowRootView paints an OPAQUE themed background
            //      over the backdrop, so the Mica never shows through - setting it without that
            //      overpaint exposes a tint MAUI never displays. Reverted, deliberately.
            //
            //      Applied ONLY when the page set no Background of its own: ReadLocalValue returns
            //      UnsetValue exactly when the mapper did not push one, so an explicit page colour
            //      still wins. Looked up AFTER RequestedTheme, so it is the right theme's brush.
            //
            //      `native.Content()` is now the title-bar-band wrapper Grid when the window extends
            //      content into the title bar (window_handler.cpp's host_content), not the page itself
            //      — the page's own Background (if any) is set independently, on the page's native view
            //      one level down, and paints over this default everywhere except the reserved band.
            //      So `panel` here never carries its own local value and this always paints the Grid —
            //      which is exactly right: it is what shows through the exposed band, matching MAUI's
            //      title bar having its own theme-coloured strip independent of the page's background.
            if (const auto panel = native.Content().try_as<winui::Controls::Panel>())
            {
                const bool has_own_background = panel.ReadLocalValue(winui::Controls::Panel::BackgroundProperty()) !=
                                                winui::DependencyProperty::UnsetValue();
                if (!has_own_background)
                {
                    const auto resources = Resources();
                    const auto key = winrt::box_value(winrt::hstring{L"ApplicationPageBackgroundThemeBrush"});
                    if (resources.HasKey(key))
                    {
                        panel.Background(resources.Lookup(key).as<winui::Media::Brush>());
                    }
                }
            }

            // (4) Show it, then lay out at the size it actually got. Activate first because a WinUI
            //     window has no client size until it is shown - laying out before would use the fallback
            //     and then immediately be corrected by the SizeChanged above.
            boot_log("activate: before");
            native.Activate();
            boot_log("activate: after");
            // Force a XAML layout pass BEFORE the port measures anything. Until one runs, a templated
            // control (a Button) has no template applied, so UIElement::Measure reports only its bare
            // content size - the port then arranges every button ~19px tall with no chrome, which is
            // exactly what the first Windows capture showed. UpdateLayout() applies templates and
            // settles the tree synchronously, so the DesiredSize the port reads afterwards is the real
            // themed one.
            if (const auto root = native.Content().try_as<winui::FrameworkElement>())
            {
                boot_log("layout: UpdateLayout");
                root.UpdateLayout();
                boot_log("layout: UpdateLayout done");
            }
            // Window.Bounds, NOT AppWindow.ClientSize: Bounds is in DIPs, which is the space XAML lays
            // out in and therefore the space the port's own measure/arrange works in. ClientSize is in
            // PHYSICAL PIXELS, so on any display above 100% scale it would hand the layout a viewport
            // 1.25x/1.5x too large -- correct-looking on the 100% parity guest and quietly wrong
            // everywhere else.
            double width = k_default_width;
            double height = k_default_height;
            const auto bounds = native.Bounds();
            if (bounds.Width > 0 && bounds.Height > 0)
            {
                width = bounds.Width;
                height = content_viewport_height(native, bounds.Height);
            }
            boot_log("layout: drive_layout");
            maui::hosting::drive_layout(*window_, width, height);
            boot_log("layout: drive_layout done -- boot complete");

            // Install the relayout hook (window::request_relayout) AFTER the first pass -- mirrors the
            // Android-only jni/relayout.hpp precedent, generalized to every backend (see window.hpp's
            // header comment). Re-reads native.Bounds() at call time, like the SizeChanged handler above,
            // rather than replaying the captured boot size -- so a leaf's invalidate_measure() (e.g. a
            // margin/orientation/SafeAreaEdges change post-boot) lays out at whatever size the window is
            // CURRENTLY showing. Deliberately does NOT wire anything new to CALL this hook (e.g. Image's
            // ImageOpened) -- that is the consumer, out of scope for this seam.
            window_->set_relayout_hook([this, native] {
                if (window_ == nullptr)
                {
                    return;
                }
                const auto current_bounds = native.Bounds();
                maui::hosting::drive_layout(*window_, current_bounds.Width,
                                            content_viewport_height(native, current_bounds.Height));
            });
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
            boot_log("main: bootstrap ok");
            winrt::init_apartment(winrt::apartment_type::single_threaded);
            boot_log("main: apartment initialised");
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

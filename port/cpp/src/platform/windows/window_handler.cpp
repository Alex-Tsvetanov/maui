// window_handler — Windows (WinUI 3) platform recipe: a REAL Microsoft.UI.Xaml.Window host, the first
// window partial of the windows per-control fan-out (the ios/android bring-up pattern replayed over
// C++/WinRT). The managed platform view is a mux::Window (one strong ref detached into
// window_platform::native — the wnative ownership doctrine); host_content resolves the root page's
// native FrameworkElement through the page handler's native_view() and sets it as Window.Content,
// exactly as the apple twin sets NSWindow.contentView. host_run.cpp borrows the same Window back out of
// the native slot to Activate it and drive the AppWindow client sizing.
//
// Ported from WindowHandler.Windows.cs + Platform/Windows/WindowExtensions.cs:
//   - CreatePlatformView: C# receives the Window from MauiWinUIWindow (the generated XamlApp); the
//     port's code-only host has no XAML Window subclass, so the handler CONSTRUCTS the mux::Window —
//     mirroring where the apple twin allocs its NSWindow (create_platform_view).
//   - MapContent: C# routes through NavigationRootManager.Connect → WindowRootViewContainer →
//     Window.Content. The port carries no navigation-root chrome (MauiToolbar / AppTitleBar), so the
//     page's native FrameworkElement IS the Window.Content — the library-independent shape of
//     SetContentView (DEVIATION, same scope as the android FrameLayout stand-in).
//   - MapTitle: WindowExtensions.UpdateTitle → platformWindow.Title = window.Title (the
//     NavigationRootManager.SetTitle half is chrome the port does not carry).
//   - MapX/Y/Width/Height: WindowExtensions.UpdatePosition/UpdateSize move the AppWindow. The port's
//     host (host_run.cpp) owns the client sizing — it ResizeClients to the 480×800 parity canvas and
//     re-drives layout on SizeChanged — so apply_frame is a documented no-op in this cut (deferred:
//     AppWindow.Move/Resize from the window's explicit x/y/width/height).
//   - ConnectHandler/DisconnectHandler: C# wires AppWindow.Changed → FrameChanged and the
//     NavigationRootManager template events. The port's window lifecycle (created/activated/destroying)
//     is driven by the application host (maui_app::open_window), exactly as the android partial
//     documents, so connect/disconnect stay the headless no-ops (deferred: AppWindow.Changed →
//     frame_changed).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists (no COM apartment, no
// Application/DispatcherQueue), and constructing any WinUI type throws hresult_error there.
// create_platform_view catches and keeps native null, while the headless mirrors (title /
// content_hosted / hosted_*) are ALWAYS maintained — so that suite observes exactly the headless
// partial's behavior, and the real app (host_run's XAML thread) additionally drives the real Window.

#include "maui/core/window_handler.hpp"

#include <memory>
#include <string>

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/base.h>

#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace wnative = maui::platform::win;

    // The page's native FrameworkElement, via its view-handler's native_view() (null if the page is
    // unattached or its handler has no native view). native_view() is C#'s ToPlatform() = ContainerView
    // ?? PlatformView. Mirrors the apple twin's page_native_view / the android window partial's helper.
    [[nodiscard]] void* page_native_view(maui::core::i_element& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        return handler != nullptr ? handler->native_view() : nullptr;
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the mux::Window (the wnative shape of the pimpl-owned-native
    // doctrine: the apple twin CFReleases its NSWindow here, the android twin drops its global ref).
    window_platform::~window_platform()
    {
        wnative::release(native);
    }

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        auto platform = std::make_unique<window_platform>();
        try
        {
            // The real Microsoft.UI.Xaml.Window (C#'s MauiWinUIWindow, minus the XAML subclass — see the
            // header). Created but NOT activated; host_run borrows it out of `native` and Activates it
            // after the mount, the apple makeKeyAndOrderFront: split.
            const mux::Window window;
            platform->native = wnative::store(window); // released in ~window_platform
        }
        catch (const winrt::hresult_error&)
        {
            // XAML-less degradation (header note): no XAML runtime on this thread — keep native null and
            // drive only the headless mirrors, exactly like the android VM-less fallback.
            platform->native = nullptr;
        }
        return platform;
    }

    // C# ConnectHandler — Windows: the port's window lifecycle is driven by the application host
    // (maui_app::open_window → created/activated), and host_run owns the AppWindow sizing + SizeChanged
    // re-layout, so there is nothing to wire here (header deviation; the android twin is the same shape).
    // deferred: AppWindow.Changed → UpdateVirtualViewFrame/FrameChanged (WindowHandler.Windows.cs).
    void window_handler::connect()
    {
    }

    // C# DisconnectHandler — Windows: nothing native was wired in connect (see above), and the content
    // teardown is the platform dtor's ref release, so this stays the headless no-op.
    void window_handler::disconnect() const
    {
    }

    void window_handler::host_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (the XAML-less cross-platform suite observes it):
        // whether a page with an attached view-handler IS the content (C#'s MapContent hosts the page's
        // platform view as the window root).
        platform->content_hosted = false;
        if (window_view_ == nullptr)
        {
            return;
        }
        auto* page = window_view_->content();
        if (page != nullptr && dynamic_cast<i_view_handler*>(page->handler().get()) != nullptr)
        {
            platform->content_hosted = true;
        }

        // The real Window.Content swap (when the XAML runtime + the Window exist): the page's native
        // FrameworkElement becomes the window root — C#'s NavigationRootManager.Connect →
        // Window.Content, minus the WindowRootViewContainer chrome (header deviation).
        if (platform->native == nullptr)
        {
            return;
        }
        auto window = wnative::borrow<mux::Window>(platform->native);
        if (window == nullptr)
        {
            return;
        }
        if (page == nullptr)
        {
            window.Content(nullptr); // an empty content host (the previous root is dropped)
            return;
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(page_native_view(*page)))
        {
            window.Content(element);
        }
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || window_view_ == nullptr)
        {
            return;
        }
        const std::string title(window_view_->title());
        platform->title = title; // keep the mirror in sync (tests read it)
        if (platform->native == nullptr)
        {
            return;
        }
        // WindowExtensions.UpdateTitle: platformWindow.Title = window.Title (the
        // NavigationRootManager.SetTitle chrome half is not carried — header).
        if (auto window = wnative::borrow<mux::Window>(platform->native))
        {
            window.Title(wnative::to_hstring_utf8(title));
        }
    }

    void window_handler::apply_frame()
    {
        // Windows: host_run owns the AppWindow client sizing (the 480×800 parity canvas + the
        // SizeChanged re-layout), so the handler does not move/size the native window in this cut —
        // the geometry lives in the window's properties, like the headless/android twins.
        // deferred: WindowExtensions.UpdatePosition/UpdateSize (AppWindow.Move / ResizeClient) from the
        // window's explicit x/y/width/height.
    }

    // --- chrome (W1-11): C# materializes the window chrome through the NavigationRootManager
    // (MauiToolbar / MenuBar / the custom AppTitleBar), which this backend does not carry (the same
    // AAR-less scope the android partial documents). Record the borrows in the portable mirrors so the
    // seam stays observable and skip the native build — the documented no-op. ---
    void window_handler::apply_toolbar(i_toolbar* toolbar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_toolbar = toolbar;
        }
    }

    void window_handler::apply_menu_bar(i_menu_bar* menu_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_menu_bar = menu_bar;
        }
    }

    void window_handler::apply_title_bar(i_title_bar* title_bar) const
    {
        if (auto* platform = typed_platform_view())
        {
            platform->hosted_title_bar = title_bar;
        }
    }
} // namespace maui::core

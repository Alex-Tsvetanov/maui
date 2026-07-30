// window_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Window whose Content is the root
// page's native element. The real-native twin of the headless partial, adapted from WindowHandler.cs +
// MauiWinUIWindow.cs (src/Core/src/Platform/Windows).
//
// Note the type asymmetry the void*-slot helpers hide: every OTHER handler boxes a UIElement, but a
// Window is NOT a UIElement in WinUI (it is not part of the visual tree — it HOSTS one), so this slot
// boxes a Window. Nothing may try to host a window as a child.

#include "maui/core/window_handler.hpp"

#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>

#include <cmath>
#include <memory>
#include <string>

#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;

    winui::Window as_window(void* native)
    {
        return maui::platform::windows::ref<winui::Window>(native);
    }
} // namespace

namespace maui::core
{
    window_platform::~window_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::Window>(native);
        }
    }

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        auto platform = std::make_unique<window_platform>();
        // Constructed but NOT activated: WinUI windows, like AppKit ones, exist before they are shown.
        // host_run.cpp activates it once the tree is mounted and laid out, so the first frame the user
        // (and the parity capture) sees is already complete rather than an empty window that fills in.
        platform->native = maui::platform::windows::take(winui::Window{});

        // MauiWinUIWindow.cs:39-46, ported verbatim including its ordering: extend content into the
        // title bar so the client area starts at the window-rect top instead of below the OS-drawn
        // caption. Done HERE, at window construction — not in apply_frame — because that is where the
        // oracle does it too (its own constructor, unconditionally, before any geometry is ever set).
        // apply_frame is the wrong home for this: it is about position/size, and its NaN geometry guard
        // early-returns for every app that never calls SetWindowSize (most of them), which would make a
        // chrome concern silently depend on frame-application call order.
        if (winrt::Microsoft::UI::Windowing::AppWindowTitleBar::IsCustomizationSupported())
        {
            const auto app_window = as_window(platform->native).AppWindow();
            if (app_window != nullptr)
            {
                const auto title_bar = app_window.TitleBar();
                if (title_bar != nullptr)
                {
                    title_bar.ExtendsContentIntoTitleBar(true);
                }
            }
        }
        return platform;
    }

    // C# ConnectHandler. WinUI raises Closed on the Window itself, so unlike AppKit there is no delegate
    // object to retain — the token lives in the platform mirror's activated flag only. The window
    // lifecycle events (Activated / Closed -> send_activated / send_destroying) are deliberately NOT
    // wired yet: they need a revocation slot on window_platform, and nothing in the parity lane observes
    // them (the runner drives one page per process launch).
    void window_handler::connect()
    {
    }

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
        platform->content_hosted = false;
        if (window_view_ == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto* page = window_view_->content();
        if (page == nullptr)
        {
            return;
        }
        auto* handler = dynamic_cast<i_view_handler*>(page->handler().get());
        if (handler == nullptr || handler->native_view() == nullptr)
        {
            return;
        }
        const winui::UIElement page_view = maui::platform::windows::ref<winui::UIElement>(handler->native_view());
        const winui::Window native = as_window(platform->native);

        // C# MapContent sets the window's root content to the page's platform view — but the real MAUI
        // root is WindowRootView, which insets its content by _appTitleBarHeight (WindowRootView.cs:280
        // `contentMargin`) precisely BECAUSE the window also extends content into the title bar
        // (create_platform_view above). Reproduce both halves together: wrap the page in a Grid (a
        // Panel, so host_run.cpp's themed-background block still finds it via try_as<Panel> and paints
        // the strip the margin exposes — an unpainted strip would trade a 1px offset for a 32-row
        // regression) whose single child carries the top margin. Only when the title bar is actually
        // extended: on an unsupported system it never took effect, and MAUI reserves no band there
        // either (NavigationRootManager.SetTitleBarVisibility's `isVisible` stays false), so the page
        // should host flush — exactly as before this fix.
        if (maui::platform::windows::has_extended_title_bar(native))
        {
            page_view.as<winui::FrameworkElement>().Margin(
                winui::Thickness{0, maui::platform::windows::k_app_title_bar_height, 0, 0});
            winui::Controls::Grid root{};
            root.Children().Append(page_view);
            native.Content(root);
        }
        else
        {
            native.Content(page_view);
        }
        platform->content_hosted = true;
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || window_view_ == nullptr)
        {
            return;
        }
        platform->title = std::string(window_view_->title());
        if (platform->native != nullptr)
        {
            as_window(platform->native).Title(maui::platform::windows::to_hstring(platform->title));
        }
    }

    void window_handler::apply_frame()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || window_view_ == nullptr)
        {
            return;
        }
        // Geometry goes through AppWindow, not the XAML Window: Window itself exposes no position/size in
        // WinUI 3. MoveAndResize works in PHYSICAL PIXELS while the window's properties are in DIPs, so a
        // non-100% display scale would need the monitor DPI applied here. The parity guest runs at 100%,
        // and getting that wrong silently produces correctly-laid-out but wrongly-SIZED captures, so it is
        // called out rather than silently assumed.
        const auto app_window = as_window(platform->native).AppWindow();
        if (app_window == nullptr)
        {
            return;
        }
        // UNSET geometry is NaN, not zero (IWindow's X/Y/Width/Height are unset until the developer or a
        // platform sets them). Casting NaN to int32 is undefined and in practice yields INT_MIN, so an
        // unguarded MoveAndResize would fling the window off-screen or collapse it to 0x0 -- and the
        // MAPPER RUNS ON MOUNT, i.e. on every app that never sets a window size, which is most of them.
        const double x = window_view_->x();
        const double y = window_view_->y();
        const double w = window_view_->width();
        const double h = window_view_->height();
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || !std::isfinite(h) || w <= 0 || h <= 0)
        {
            return; // leave the window where the shell put it
        }
        app_window.MoveAndResize(
            winrt::Windows::Graphics::RectInt32{static_cast<std::int32_t>(x), static_cast<std::int32_t>(y),
                                                static_cast<std::int32_t>(w), static_cast<std::int32_t>(h)});
    }

    // --- chrome (W1-11): mirror-only on this backend for now. C# DOES materialize a real toolbar and
    // menu bar on Windows (MauiToolbar / WindowRootView), unlike iOS where they are no-ops by design —
    // so these are an unfinished slice, not a platform limitation. Recorded here so the distinction is
    // not lost when the chrome fan-out lands.
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

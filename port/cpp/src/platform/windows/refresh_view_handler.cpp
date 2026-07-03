// refresh_view_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// that HOSTS the single scrollable Content child of a RefreshView. The windows twin of
// src/platform/android/refresh_view_handler.cpp (a plain MauiLayout host) /
// src/platform/apple/refresh_view_handler.mm (a plain NSView host) and the real-native sibling of the
// headless mirror (src/platform/headless/refresh_view_handler.cpp).
//
// Ported from RefreshViewHandler.cs + RefreshViewHandler.Windows.cs, with the same content-hosting shape
// as this backend's content_page partial: set_content re-parents the single Content's native element as
// the Canvas child, and platform_arrange frames the host via the shared Canvas recipe — the content's
// own host-relative platform_arrange then frames it within.
//
// PULL GESTURE + SPINNER are the DOCUMENTED DEVIATION (the same scope the android/apple twins document).
// C#'s Windows RefreshView is a Microsoft.UI.Xaml.Controls.RefreshContainer (WinUI's pull-to-refresh
// control, whose RefreshRequested → the deferral completes on IsRefreshing=false, and whose Visualizer
// carries the spinner + RefreshColor foreground) — deferred in this first cut: the host is a plain
// Canvas with NO pull recognizer and NO native spinner visual. The captures are STATIC and show the
// RefreshView resting (its content visible, no spinner), so the closed/resting Content is the faithful
// static render. The cross-platform refresh surface is still mirrored beside the (absent) native pushes:
// IsRefreshing / IsRefreshEnabled / the spinner color land on the platform mirror, and request_refresh()
// still writes IsRefreshing=true back through the virtual view (the C# RefreshContainer.RefreshRequested
// twin) so the programmatic/test path matches every backend. A future RefreshContainer host can drive
// the same request_refresh() entry point.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless mirrors (hosted_content + the refresh
// mirrors + the view_platform_base mirrors) are ALWAYS maintained so that suite observes exactly the
// headless partial's behavior.

#include "maui/core/refresh_view_handler.hpp"

#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    // The content's native FrameworkElement, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Mirrors the content_page partial's helper.
    [[nodiscard]] void* content_native_view(maui::core::i_view& content)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(content.handler().get());
        return handler != nullptr ? handler->native_view() : nullptr;
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Canvas host (the wnative shape of the pimpl-owned-native
    // doctrine; the android twin deletes its JNI global ref here).
    refresh_view_platform::~refresh_view_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Canvas when one exists.

    void refresh_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void refresh_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void refresh_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void refresh_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto panel = wnative::borrow<muxc::Canvas>(native);
        if (panel == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Panel branch: panel.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            panel.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        panel.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<refresh_view_platform>();
        try
        {
            // RefreshViewHandler.Windows CreatePlatformView: new RefreshContainer { PullDirection =
            // TopToBottom } — the port's manual-frame Canvas host (the pull control is deferred; header).
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~refresh_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# RefreshViewHandler.UpdateContent reads
        // VirtualView.Content) — the XAML-less cross-platform suite observes it.
        i_view* content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        platform->hosted_content = content;

        // The real single-child re-host (when the XAML runtime + the Canvas exist): C# UpdateContent —
        // RefreshContainer.Content = content.ToPlatform(); here Children().Clear() + Append (the same
        // swap the content_page partial does). The content is then framed by its own platform_arrange.
        if (platform->native == nullptr)
        {
            return;
        }
        auto host = wnative::borrow<muxc::Canvas>(platform->native);
        if (host == nullptr)
        {
            return;
        }
        host.Children().Clear();
        if (content == nullptr)
        {
            return; // an empty refresh host (the previous child was just removed)
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(content_native_view(*content)))
        {
            host.Children().Append(element);
        }
    }

    // C# MapIsRefreshing → RefreshContainer.RequestRefresh / the deferral completion. No native pull
    // control on this first cut (deferred: RefreshContainer — header), so IsRefreshing is mirrored only,
    // the same documented body as the android/apple twins.
    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refreshing = virtual_view()->is_refreshing();
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# UpdateRefreshColor: the RefreshVisualizer.Foreground follows RefreshColor (null = platform
        // default). No native spinner visual here (deferred: RefreshContainer — header), so the tint is
        // mirrored only (the headless/android spinner mirror).
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        if (paint != nullptr)
        {
            platform->has_refresh_color = true;
            platform->refresh_color_argb = paint->background_color().to_uint();
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
    }

    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# MapIsRefreshEnabled gates the pull gesture; no pull recognizer here (deferred — header), so
        // the flag is mirrored only.
        platform->refresh_enabled = virtual_view()->is_refresh_enabled();
    }

    // The native pull stand-in (C# RefreshContainer.RefreshRequested → OnRefresh): write
    // IsRefreshing=true back through the virtual view (which re-enters the control's coercion →
    // Refreshing + command). Identical to every other backend so the programmatic/test path matches; a
    // future RefreshContainer host calls this from its RefreshRequested.
    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the host to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        // The single Content child keeps the host-relative frame its own platform_arrange set.
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core

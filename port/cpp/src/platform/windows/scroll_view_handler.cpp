// scroll_view_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.
// ScrollViewer whose Content is a Canvas shim carrying the content child at its manual frame. The
// windows twin of src/platform/apple/scroll_view_handler.mm (NSScrollView + documentView) / the
// android ScrollView partial, and the real-native sibling of the headless mirror
// (src/platform/headless/scroll_view_handler.cpp).
//
// Ported from ScrollViewHandler.cs + ScrollViewHandler.Windows.cs + Platform/Windows/
// ScrollViewerExtensions.cs:
//   - CreatePlatformView: new ScrollViewer(). C# inserts a ContentPanel "padding shim" between the
//     ScrollViewer and the content (the ScrollViewer forces its Content to the origin, defeating
//     margins); the port's shim is a Canvas (the manual-frame host every windows partial shares) whose
//     explicit Width/Height IS the scrollable extent — the ContentPanel role collapsed onto the
//     existing Canvas layout model.
//   - MapContent → UpdateContentPanel: clear + re-parent the content's native view into the shim.
//   - Map*ScrollBarVisibility / MapOrientation → ScrollViewerExtensions.UpdateScrollBarVisibility
//     (Neither disables both axes; Default derives each axis from the orientation; an explicit
//     preference converts through ToWindowsScrollBarVisibility, still gated by the orientation).
//   - MapRequestScrollTo: clamp to ScrollableWidth/Height; equal-to-current acknowledges
//     ScrollFinished immediately, else ChangeView(h, v, null, request.Instant).
//   - ConnectHandler: ViewChanged → the offsets write back on every change; ScrollFinished fires when
//     !IsIntermediate (the ViewChanged body, routed through the platform's on_scrolled hook).
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - The shim is a stock Canvas, not C#'s ContentPanel (no CrossPlatformLayout back-ref — the port's
//     control owns measure/arrange; no Padding/Margin re-application — the control arranges the
//     content inside the padding itself, so the C# padding-shim motivation collapses).
//   - The scrollable extent is pushed in platform_arrange from the control's content_size() (the
//     MauiScrollView LayoutSubviews → ContentSize push), not re-measured by XAML.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, and scroll_to falls back to the headless synchronous
// clamp + write-back + ScrollFinished body, while the headless mirrors (orientation / bar visibilities
// / hosted content / offsets / scroll_requests) are ALWAYS maintained — so that suite observes exactly
// the headless partial's behavior.

#include "maui/core/scroll_view_handler.hpp"

#include <algorithm>
#include <memory>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/Windows.Foundation.h>             // IReference<double> (the ChangeView offsets)
#include <winrt/base.h>

#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    [[nodiscard]] muxc::ScrollViewer scroller_of(const maui::core::scroll_view_platform& platform)
    {
        return wnative::borrow<muxc::ScrollViewer>(platform.native);
    }

    // The content's native FrameworkElement, via its view-handler's native_view() (C#'s ToPlatform()).
    [[nodiscard]] void* content_native_view(maui::core::i_view& content)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(content.handler().get());
        return handler != nullptr ? handler->native_view() : nullptr;
    }

    // ScrollViewerExtensions.ToWindowsScrollBarVisibility: Always → Visible; Default → Auto;
    // Never → Hidden.
    [[nodiscard]] muxc::ScrollBarVisibility to_native_bar_visibility(maui::core::scroll_bar_visibility value)
    {
        switch (value)
        {
            case maui::core::scroll_bar_visibility::always:
                return muxc::ScrollBarVisibility::Visible;
            case maui::core::scroll_bar_visibility::never:
                return muxc::ScrollBarVisibility::Hidden;
            case maui::core::scroll_bar_visibility::default_:
            default:
                return muxc::ScrollBarVisibility::Auto;
        }
    }

    // ScrollViewerExtensions.UpdateScrollBarVisibility: Neither disables BOTH axes (a WinUI
    // ScrollViewer's Disabled bar visibility also disables scrolling on that axis — the orientation
    // gate rides the same property); Default derives each axis from the orientation (Auto on the
    // scrolling axes, Disabled otherwise); an explicit preference converts through
    // to_native_bar_visibility on the axes the orientation allows.
    void apply_scroll_bar_visibility(const muxc::ScrollViewer& scroller, maui::core::scroll_orientation orientation,
                                     maui::core::scroll_bar_visibility visibility)
    {
        using maui::core::scroll_orientation;
        if (orientation == scroll_orientation::neither)
        {
            scroller.HorizontalScrollBarVisibility(muxc::ScrollBarVisibility::Disabled);
            scroller.VerticalScrollBarVisibility(muxc::ScrollBarVisibility::Disabled);
            return;
        }
        const bool scrolls_horizontally =
            orientation == scroll_orientation::horizontal || orientation == scroll_orientation::both;
        const bool scrolls_vertically =
            orientation == scroll_orientation::vertical || orientation == scroll_orientation::both;
        const muxc::ScrollBarVisibility resolved = visibility == maui::core::scroll_bar_visibility::default_
                                                       ? muxc::ScrollBarVisibility::Auto
                                                       : to_native_bar_visibility(visibility);
        scroller.HorizontalScrollBarVisibility(scrolls_horizontally ? resolved : muxc::ScrollBarVisibility::Disabled);
        scroller.VerticalScrollBarVisibility(scrolls_vertically ? resolved : muxc::ScrollBarVisibility::Disabled);
    }
} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the ScrollViewer + the Canvas content shim (the wnative shape
    // of the pimpl-owned-native doctrine; the apple twin CFReleases its NSScrollView here).
    scroll_view_platform::~scroll_view_platform()
    {
        wnative::release(content_host);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real scroller when one exists.

    void scroll_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void scroll_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void scroll_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void scroll_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto scroller = wnative::borrow<muxc::ScrollViewer>(native);
        if (scroller == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Control branch: control.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            scroller.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        scroller.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<scroll_view_platform>();
        try
        {
            // ScrollViewHandler.Windows.CreatePlatformView: new ScrollViewer(). The Canvas shim stands
            // in for C#'s inserted ContentPanel (header deviations): it hosts the absolutely
            // positioned content, and its explicit size is the scrollable extent.
            const muxc::ScrollViewer scroller;
            const muxc::Canvas shim;
            scroller.Content(shim);
            platform->native = wnative::store(scroller);   // released in ~scroll_view_platform
            platform->content_host = wnative::store(shim); // released in ~scroll_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
            platform->content_host = nullptr;
        }
        return platform;
    }

    void scroll_view_handler::on_connect_handler(scroll_view_platform& platform)
    {
        // The ViewChanged body (ScrollViewHandler.Windows.cs): the virtual offsets follow the native
        // ones on EVERY change, and ScrollFinished fires when the change is final (!IsIntermediate).
        // The callback stays wired even XAML-less so the cross-platform suite can drive it (the
        // android partial's shape).
        platform.on_scrolled = [this](double offset_x, double offset_y, bool is_final) {
            auto* view = virtual_view();
            if (view == nullptr)
            {
                return;
            }
            view->set_horizontal_offset(offset_x);
            view->set_vertical_offset(offset_y);
            if (is_final)
            {
                view->scroll_finished();
            }
        };
        auto scroller = scroller_of(platform);
        if (scroller == nullptr)
        {
            return;
        }
        // ConnectHandler: platformView.ViewChanged += ViewChanged. The native event routes through the
        // platform callback (the peer is the platform struct, whose heap address is stable until
        // disconnect revokes this handler).
        auto* peer = &platform;
        const winrt::event_token view_changed_token = scroller.ViewChanged(
            [peer](const winrt::Windows::Foundation::IInspectable&, const muxc::ScrollViewerViewChangedEventArgs& e) {
                auto live = wnative::borrow<muxc::ScrollViewer>(peer->native);
                if (live == nullptr)
                {
                    return;
                }
                // Keep the shared offset mirrors current (the headless convention: the native partials
                // append beside the real pushes) before the write-back runs.
                peer->offset_x = live.HorizontalOffset();
                peer->offset_y = live.VerticalOffset();
                if (peer->on_scrolled)
                {
                    peer->on_scrolled(peer->offset_x, peer->offset_y, !e.IsIntermediate());
                }
            });
        platform.view_changed_token = view_changed_token.value;
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& platform)
    {
        // DisconnectHandler: ViewChanged -= ViewChanged; the C++ callback is cleared like the headless
        // twin's teardown.
        platform.on_scrolled = nullptr;
        if (auto scroller = scroller_of(platform))
        {
            if (platform.view_changed_token != 0)
            {
                scroller.ViewChanged(winrt::event_token{platform.view_changed_token});
            }
        }
        platform.view_changed_token = 0;
    }

    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C#'s UpdateContentPanel reads
        // VirtualView.PresentedContent) — the XAML-less cross-platform suite observes it.
        i_view* content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        platform->hosted_content = content;

        // The real single-child re-host (C# UpdateContentPanel: clear the shim's CachedChildren + Add
        // the content's native view).
        auto shim = wnative::borrow<muxc::Canvas>(platform->content_host);
        if (shim == nullptr)
        {
            return; // XAML-less
        }
        shim.Children().Clear();
        if (content == nullptr)
        {
            return;
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(content_native_view(*content)))
        {
            shim.Children().Append(element);
        }
    }

    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->orientation = virtual_view()->orientation(); // the headless mirror
        auto scroller = scroller_of(*platform);
        if (scroller == nullptr)
        {
            return;
        }
        // MapOrientation: the visibility re-derived for the NEW orientation's primary axis
        // (Horizontal reads the horizontal preference, everything else the vertical one), then
        // UpdateScrollBarVisibility applies both axes.
        const scroll_bar_visibility visibility = platform->orientation == scroll_orientation::horizontal
                                                     ? virtual_view()->horizontal_scroll_bar_visibility()
                                                     : virtual_view()->vertical_scroll_bar_visibility();
        apply_scroll_bar_visibility(scroller, platform->orientation, visibility);
    }

    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->horizontal_bar_visibility = virtual_view()->horizontal_scroll_bar_visibility();
        if (auto scroller = scroller_of(*platform))
        {
            // MapHorizontalScrollBarVisibility → UpdateScrollBarVisibility(orientation, value) — the
            // C# extension writes BOTH axes from the one preference, gated by the orientation.
            apply_scroll_bar_visibility(scroller, virtual_view()->orientation(), platform->horizontal_bar_visibility);
        }
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->vertical_bar_visibility = virtual_view()->vertical_scroll_bar_visibility();
        if (auto scroller = scroller_of(*platform))
        {
            apply_scroll_bar_visibility(scroller, virtual_view()->orientation(), platform->vertical_bar_visibility);
        }
    }

    // C# MapRequestScrollTo. With a real ScrollViewer the exact C# body runs: clamp the target to the
    // native ScrollableWidth/Height, acknowledge immediately when already there, else ChangeView (the
    // ViewChanged wiring above delivers the offsets write-back + ScrollFinished). XAML-less the
    // headless synchronous body runs instead (clamp against content vs frame, write back, acknowledge)
    // so the cross-platform suite observes exactly the headless behavior.
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request); // the shared trail (headless convention)

        auto scroller = scroller_of(*platform);
        if (scroller == nullptr)
        {
            // XAML-less degradation: the headless partial's synchronous body.
            const maui::graphics::size content = view->content_size();
            const maui::graphics::rect frame = view->frame();
            const double available_x = std::max(content.width - frame.width, 0.0);
            const double available_y = std::max(content.height - frame.height, 0.0);
            const double target_x = std::clamp(request.horizontal_offset, 0.0, available_x);
            const double target_y = std::clamp(request.vertical_offset, 0.0, available_y);
            platform->offset_x = target_x;
            platform->offset_y = target_y;
            view->set_horizontal_offset(target_x);
            view->set_vertical_offset(target_y);
            view->scroll_finished();
            return;
        }
        // MapRequestScrollTo: Math.Clamp(offset, 0, Scrollable{Width,Height}).
        const double target_x = std::clamp(request.horizontal_offset, 0.0, scroller.ScrollableWidth());
        const double target_y = std::clamp(request.vertical_offset, 0.0, scroller.ScrollableHeight());
        if (target_x == scroller.HorizontalOffset() && target_y == scroller.VerticalOffset())
        {
            view->scroll_finished(); // already there — acknowledge without moving (the C# early-out)
            return;
        }
        // ChangeView(h, v, null zoom, disableAnimation: request.Instant); the offsets write back
        // through ViewChanged.
        scroller.ChangeView(winrt::Windows::Foundation::IReference<double>{target_x},
                            winrt::Windows::Foundation::IReference<double>{target_y}, nullptr, request.instant);
    }

    void scroll_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native scroller to frame
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the scroller to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
        // The MauiScrollView LayoutSubviews → ContentSize push: the shim's explicit size is the
        // scrollable extent, re-derived from the control's freshly-arranged content size.
        auto shim = wnative::borrow<muxc::Canvas>(platform->content_host);
        if (shim == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const maui::graphics::size content = virtual_view()->content_size();
        shim.Width(std::max(content.width, 0.0));
        shim.Height(std::max(content.height, 0.0));
    }
} // namespace maui::core

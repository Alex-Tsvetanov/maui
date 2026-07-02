// flyout_page_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// that HOSTS the flyout page's DETAIL pane (the visible content). The windows twin of
// src/platform/android/flyout_page_handler.cpp (a MauiLayout hosting the detail; drawer deferred) and
// the real-native sibling of the headless two-pane mirror
// (src/platform/headless/flyout_page_handler.cpp).
//
// C# ORIGIN: FlyoutViewHandler.Windows.cs hosts through a RootNavigationView (a NavigationView
// subclass): MapDetail sets PlatformView.Content = Detail.ToPlatform(), MapFlyout sets the pane's
// FlyoutCustomContent, MapIsPresented drives IsPaneOpen, and the PaneOpened/PaneClosed events sync
// IsPresented back. The port keeps the library-independent half — host the DETAIL as the content — on a
// plain Canvas; the flyout PANE (the NavigationView drawer chrome, IsPaneOpen, the pane-open gesture,
// and the native→virtual PaneOpened/PaneClosed sync) is // deferred with the RootNavigationView infra
// (the android DrawerLayout deviation, translated).
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - The host is a plain Canvas showing the DETAIL pane only. The flyout pane is still MOUNTED (its
//     handler attaches, its native element builds) so a future NavigationView host can present it; it is
//     simply not added as a child. IsPresented / FlyoutBehavior / IsGestureEnabled are mirrored beside
//     the (absent) pane for the XAML-less suite — when IsPresented flips true a future host would open
//     the pane via the same update_presentation seam (C# MapIsPresented → IsPaneOpen).
//   - MapFlyoutWidth (OpenPaneLength) has no pane to size — deferred with the pane chrome.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless two-pane mirror (hosted_flyout /
// hosted_detail / presented / behavior / gesture_enabled) is ALWAYS maintained so that suite observes
// exactly the headless partial's pane + presentation tracking.

#include "maui/core/flyout_page_handler.hpp"

#include <cstdint>
#include <memory>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_flyout_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
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

    // The pane's native UIElement, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Empty when the pane is unattached / has no native (XAML-less).
    [[nodiscard]] mux::UIElement native_child(maui::core::i_view& pane)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(pane.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return wnative::borrow_as<mux::UIElement>(handler->native_view());
    }

    // Detach `element` from any current Panel parent (the re-parent guard — XAML throws on a
    // double-parent Append; the android twin's detach_from_parent).
    void detach_from_parent(const mux::UIElement& element)
    {
        const auto framework = element.try_as<mux::FrameworkElement>();
        if (framework == nullptr)
        {
            return;
        }
        const auto parent = framework.Parent();
        if (parent == nullptr)
        {
            return;
        }
        if (const auto panel = parent.try_as<muxc::Panel>())
        {
            std::uint32_t index = 0;
            if (panel.Children().IndexOf(element, index))
            {
                panel.Children().RemoveAt(index);
            }
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Canvas flyout host (the wnative shape of the
    // pimpl-owned-native doctrine; the android twin deletes its MauiLayout global ref here). The hosted
    // detail / flyout panes are owned by their own page handlers (non-owning children) — nothing to
    // release for those.
    flyout_page_platform::~flyout_page_platform()
    {
        wnative::release(native);
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Base body FIRST (the XAML-less suite observes the headless mirror), then the
    // real host (the content_page dual-path pattern). ----

    void flyout_page_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void flyout_page_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void flyout_page_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void flyout_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto host = wnative::borrow<muxc::Canvas>(native);
        if (host == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Panel branch: panel.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            host.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            host.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints — the base mirror above keeps the borrow observable.
    }

    std::unique_ptr<flyout_page_platform> flyout_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<flyout_page_platform>();
        try
        {
            // FlyoutViewHandler.CreatePlatformView: new RootNavigationView() — the port's manual-frame
            // Canvas detail host (the pane chrome is deferred; header deviations).
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~flyout_page_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void flyout_page_handler::set_panes(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless two-pane mirror is ALWAYS maintained (C# FlyoutViewHandler reads Flyout/Detail).
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->hosted_flyout = flyout->flyout_view();
            platform->hosted_detail = flyout->flyout_detail();
        }

        // The real single-child re-host (C# UpdateDetail: PlatformView.Content = Detail.ToPlatform()):
        // clear the old content, then add the DETAIL pane's native element filling the host. The flyout
        // pane stays MOUNTED (its native element exists) for a future NavigationView pane host —
        // UpdateFlyout's FlyoutCustomContent half is deferred (header deviations).
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
        if (platform->hosted_detail == nullptr)
        {
            return; // no detail pane set yet — an empty flyout host
        }
        if (auto element = native_child(*platform->hosted_detail))
        {
            detach_from_parent(element);
            host.Children().Append(element);
        }
    }

    // IsPresented / FlyoutBehavior realize the pane state. No native NavigationView pane on this cut, so
    // the presentation is mirrored only — the documented deviation (header). A future RootNavigationView
    // host would drive IsPaneOpen / UpdateFlyoutBehavior here from `presented` / `behavior`
    // (C# MapIsPresented / MapFlyoutBehavior).
    void flyout_page_handler::update_presentation(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->presented = flyout->flyout_is_presented();
            platform->behavior = flyout->flyout_behavior_value();
        }
    }

    maui::graphics::size flyout_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The flyout page sizes from its panes, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void flyout_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the host to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        // The detail child keeps the host-relative frame the flyout_page control's arrange set.
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core

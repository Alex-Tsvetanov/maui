// navigation_page_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// standing in for the page-stack container: host_current swaps the Canvas's CONTENT child to the
// navigation stack's current (top-most) page's native element on each push/pop, and host_modal overlays
// the top modal's native element on top of the whole container (removing it when the modal stack
// empties). The windows twin of src/platform/android/navigation_handler.cpp (a FrameLayout page-stack
// swap) and the real-native sibling of the headless mirror partial
// (src/platform/headless/navigation_page_handler.cpp).
//
// Ported from NavigationViewHandler.Windows.cs + StackNavigationManager.Windows.cs (the WinUI Frame
// navigation stack):
//   - C# CreatePlatformView returns a Microsoft.UI.Xaml.Controls.Frame; the StackNavigationManager
//     Navigates the Frame between pages, keeping one live content at a time. The port keeps that
//     library-independent shape — "what is the currently VISIBLE page" — as a plain Canvas whose content
//     child is the current page's native view; the real Frame + StackNavigationManager (page-type
//     registration, back-stack sync, transition animation) is // deferred (the android twin's plain
//     FrameLayout deviation, translated).
//   - NavigationFinished is reported synchronously by the cross-platform handler after host_current
//     returns (the Canvas child swap is synchronous), standing in for the manager's OnNavigated-driven
//     completion. The animated flag is mirrored (last_animated); the port runs no transition animation
//     here. (DEVIATION, below.)
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - The container is a plain Canvas, NOT a Frame + StackNavigationManager: the manager's page-mapping
//     infra has not reached this backend. The current page's native element is re-parented directly as
//     the Canvas's content child, which is the visible result of a Frame navigation — the page's handler
//     + native element survive the swap (the page is non-owning; only its element is re-parented).
//   - No transition ANIMATION: the swap is instant; last_animated is mirrored for parity. The navigation
//     BAR (title / back button) is windows chrome C# builds in the Controls layer (the NavigationRoot
//     manager's title bar) — host_current mirrors the chrome state into the platform so the seam stays
//     observable, but builds no native bar (the android twin's shape).
//   - The MODAL overlay is a plain re-parent of the modal page's native element as the Canvas's TOP-MOST
//     child (a Canvas stacks children; the last appended is on top). C#'s modal stack lives on the
//     Window's ModalNavigationManager; the port keeps it on the navigation_page (the documented
//     controls-layer simplification), so this partial just overlays/clears the modal element.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless mirrors (hosted_page / hosted_modal /
// bar chrome / last_animated / bar_translucent) are ALWAYS maintained so that suite observes exactly the
// headless partial's behavior.

#include "maui/core/navigation_page_handler.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_stack_navigation.hpp"
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

    // The page's native UIElement, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Empty when the page is unattached / has no native (XAML-less).
    // Mirrors the android twin's native_child helper.
    [[nodiscard]] mux::UIElement native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return wnative::borrow_as<mux::UIElement>(handler->native_view());
    }

    // Append `element` to the container as its top-most child, re-parenting it first (the android
    // add_filling_child shape — the element's frame comes from its own platform_arrange, the Canvas
    // manual-frame model).
    void append_child(const muxc::Canvas& container, const mux::UIElement& element)
    {
        wnative::detach_from_parent(element);
        container.Children().Append(element);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Canvas container (the wnative shape of the
    // pimpl-owned-native doctrine; the android twin deletes its FrameLayout global ref here). The hosted
    // page / modal elements are owned by their own page handlers (non-owning children) — nothing to
    // release for those.
    navigation_page_platform::~navigation_page_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real container when one exists.

    void navigation_page_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void navigation_page_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void navigation_page_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void navigation_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto container = wnative::borrow<muxc::Canvas>(native);
        if (container == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Panel branch: panel.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            container.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            container.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform's LinearGradientBrush et al.) —
        // the base mirror above keeps the borrow observable.
    }

    std::unique_ptr<navigation_page_platform> navigation_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<navigation_page_platform>();
        try
        {
            // NavigationViewHandler.CreatePlatformView: new Frame() + the StackNavigationManager — the
            // port's manual-frame Canvas page-stack stand-in (header deviations; real Frame navigation
            // is deferred).
            const muxc::Canvas container;
            platform->native = wnative::store(container); // released in ~navigation_page_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void navigation_page_handler::on_connect_handler(navigation_page_platform& /*platform*/)
    {
        // Windows has no native bar / back button to wire in this cut (the bar chrome is Controls-layer
        // NavigationRoot chrome, deferred — header deviations); the back-button routing is exercised
        // through navigation_page::send_back_button_pressed() directly in the unit tests, like the
        // headless twin.
    }

    void navigation_page_handler::host_current(i_view* top, i_view& view, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirrors are ALWAYS maintained (the XAML-less cross-platform suite observes them):
        // the current page + animation flag + the bar chrome state read off the navigation view.
        platform->hosted_page = top;
        platform->last_animated = animated;
        if (auto* navigation = dynamic_cast<i_stack_navigation*>(&view))
        {
            platform->bar_title = std::string(navigation->navigation_bar_title());
            platform->back_button_visible = navigation->navigation_back_button_visible();
            platform->bar_background_color = navigation->navigation_bar_background_color();
            platform->bar_text_color = navigation->navigation_bar_text_color();
            platform->hosted_title_view = navigation->navigation_bar_title_view();
            platform->toolbar_items = navigation->navigation_toolbar_items();
        }

        // The real Canvas content swap (when the XAML runtime + container exist): remove every child,
        // re-append the new current page's native element, then re-append the modal overlay (when
        // present) so it stays the top-most child — navigating the underlying stack while a modal covers
        // it must keep the modal on top (the android twin's preserve-the-overlay rule; the retained
        // pointers make clear-and-rebuild equivalent and simpler). No cross-fade — // deferred with the
        // Frame transition animation (header).
        if (platform->native == nullptr)
        {
            return;
        }
        auto container = wnative::borrow<muxc::Canvas>(platform->native);
        if (container == nullptr)
        {
            return;
        }
        container.Children().Clear();
        if (top != nullptr)
        {
            if (auto element = native_child(*top))
            {
                append_child(container, element);
            }
        }
        if (platform->hosted_modal != nullptr)
        {
            if (auto modal_element = native_child(*platform->hosted_modal))
            {
                append_child(container, modal_element); // restore the overlay on top
            }
        }
    }

    void navigation_page_handler::host_modal(i_view* top_modal, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The previous modal (if any) — its native element is the overlay to tear down before presenting
        // the new top modal (or clearing). Read it BEFORE updating the mirror.
        i_view* const previous_modal = platform->hosted_modal;
        platform->hosted_modal = top_modal;
        platform->last_animated = animated;

        if (platform->native == nullptr)
        {
            return;
        }
        auto container = wnative::borrow<muxc::Canvas>(platform->native);
        if (container == nullptr)
        {
            return;
        }
        // Tear down the previous overlay (clearing or replacing the presented modal): detach the
        // dismissed modal page's native element so the underlying content (never removed) is revealed.
        if (previous_modal != nullptr && previous_modal != top_modal)
        {
            if (auto old_element = native_child(*previous_modal))
            {
                wnative::detach_from_parent(old_element);
            }
        }
        if (top_modal == nullptr)
        {
            return; // the modal stack emptied — the underlying content is revealed
        }
        // Overlay the modal's native element as the Canvas's TOP-MOST child (appended last = top of the
        // z-order), so it covers the content (the windows analog of a presented modal page).
        if (auto element = native_child(*top_modal))
        {
            append_child(container, element);
        }
    }

    maui::graphics::size navigation_page_handler::get_desired_size(double /*width_constraint*/,
                                                                   double /*height_constraint*/) const
    {
        // The navigation page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void navigation_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native container to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the container to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout
        // model). The current page is arranged by the control through the page's own platform_arrange.
        wnative::arrange_native(platform->native, frame);
    }

    // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent push — Windows keeps
    // the cross-platform mirror only (an iOS-only knob; nothing native to drive on WinUI).
    void navigation_page_handler::update_bar_translucent(bool value)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->bar_translucent = value;
        }
    }
} // namespace maui::core

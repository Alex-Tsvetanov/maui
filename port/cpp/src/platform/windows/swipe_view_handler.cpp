// swipe_view_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// that HOSTS the single swipe Content child, plus the swipe state machine driven by the shared
// cross-platform maui::core::swipe_machine. The windows twin of
// src/platform/android/swipe_view_handler.cpp (a plain MauiLayout host) /
// src/platform/apple/swipe_view_handler.mm (a plain NSView host) and the real-native sibling of the
// headless mirror (src/platform/headless/swipe_view_handler.cpp).
//
// STATIC RENDER FIRST (the android partial's cut). The load-bearing requirement is Content visibility,
// so this cut renders the Content CLOSED (hosted as the Canvas child, framed by its own
// platform_arrange). The cross-platform swipe_machine still runs the FULL behavior (the open/close
// thresholds, the directional item-set selection, the SwipeStarted/Changing/Ended fan-out + IsOpen
// write-back) so the cross-platform suite observes the same machine on this backend — but the live
// drag-to-reveal pan + the open/close ANIMATION are the DOCUMENTED DEVIATION: C#'s Windows SwipeView is
// a Microsoft.UI.Xaml.Controls.SwipeControl (whose LeftItems/RightItems/... are SwipeItems the control
// materializes and pans natively) — deferred in this first cut; the host is a plain Canvas with no pan
// recognizer and no revealed-item subviews, the same scope the apple AppKit twin documents (it reuses
// the machine, driven by the programmatic open/close + the cross-platform synthetic offsets; a future
// SwipeControl host — or a pointer-tracking pan on the Canvas — can call the same begin/swipe/end entry
// points to add the real drag visual). The directional swipe-item BUTTONS are not realized as native
// subviews here: they are revealed only on a drag, which is deferred, and the reference resting state
// shows none of them — so the closed Content is the faithful static render.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless `hosted_content` mirror + the
// swipe_machine state are ALWAYS maintained so that suite observes exactly the headless partial's
// content tracking + machine.

#include "maui/core/swipe_view_handler.hpp"

#include <memory>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_swipe_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_machine.hpp"
#include "maui/core/swipe_view_requests.hpp"
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
    // ContainerView ?? PlatformView). Mirrors the content_page/refresh_view partials' helper.
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
    swipe_view_platform::~swipe_view_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Canvas when one exists.

    void swipe_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void swipe_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void swipe_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void swipe_view_platform::update_background(const maui::graphics::paint* value)
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
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            panel.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<swipe_view_platform> swipe_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_view_platform>();
        try
        {
            // SwipeViewHandler.Windows CreatePlatformView: new SwipeControl — the port's manual-frame
            // Canvas host (the native pan/reveal control is deferred; header).
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~swipe_view_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void swipe_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MauiSwipeView.UpdateContent reads
        // VirtualView.PresentedContent) — the XAML-less cross-platform suite observes it.
        i_view* content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        platform->hosted_content = content;

        // The real single-child re-host (when the XAML runtime + the Canvas exist): C# UpdateContent —
        // SwipeControl.Content = content.ToPlatform(); here Children().Clear() + Append (the same swap
        // the content_page/refresh_view partials do). The content is then framed by its own
        // platform_arrange (the closed resting render — header).
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
            return; // an empty swipe host (the previous child was just removed)
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(content_native_view(*content)))
        {
            host.Children().Append(element);
        }
    }

    void swipe_view_handler::update_transition_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // C# MapSwipeTransitionMode — cached for the machine (no native reveal to re-mode here; the live
        // pan is deferred — header).
        platform->transition = virtual_view()->transition_mode();
    }

    void swipe_view_handler::update_items()
    {
        // C# MapLeftItems/... are empty (the gesture-time UpdateSwipeItems re-reads the live collection);
        // the machine reads the collections directly on each swipe, so there is nothing to cache here —
        // the headless/android twins' body.
    }

    // The state-machine drivers — the SHARED cross-platform swipe_machine (the pure MauiSwipeView.cs
    // port) runs identically on every backend; this partial only wires the handler entry points to it
    // (the headless twin verbatim). The revealed-item native visual is deferred with the pan (header).

    void swipe_view_handler::programmatically_open(const swipe_view_open_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::programmatically_open(platform->state, *view, request);
    }

    void swipe_view_handler::reset_swipe(bool /*animated*/)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::reset_swipe(platform->state, *view);
    }

    void swipe_view_handler::begin_swipe(swipe_direction direction)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        swipe_machine::begin_swipe(platform->state, direction);
    }

    void swipe_view_handler::swipe_to(double offset)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::swipe_to(platform->state, *view, offset);
    }

    void swipe_view_handler::end_swipe()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::end_swipe(platform->state, *view);
    }

    void swipe_view_handler::platform_arrange(const maui::graphics::rect& frame)
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

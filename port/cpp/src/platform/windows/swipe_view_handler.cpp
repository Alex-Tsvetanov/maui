// swipe_view_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.SwipeControl whose
// Content hosts a Canvas panel, which in turn hosts the content child's native view. The real-native twin
// of the headless partial, ported from SwipeViewHandler.Windows.cs + SwipeViewExtensions.cs.
//
// LAYOUT SEAM DECISION (scroll_view_handler.cpp's exact tension): the oracle sets
// `handler.PlatformView.Content = presentedView.ToPlatform(...)` directly (SwipeViewHandler.Windows.cs:31)
// — SwipeControl derives ContentControl, so that Content slot is presented through a ContentPresenter,
// which measures/positions its child by its own alignment logic. The port's cross-platform layer instead
// runs its OWN measure/arrange and hands the content child an absolute-in-parent frame via
// Canvas.SetLeft/Top (every content-host partial in this backend relies on that convention — see
// content_page_handler.cpp / scroll_view_handler.cpp) — those attached properties only affect an element
// whose ACTUAL visual parent is a Canvas, which a ContentPresenter is not. So this partial does not set
// `.Content` to the child directly (the literal-oracle approach would silently drop the child's absolute
// frame): it wraps a plain Canvas panel in the ContentControl's Content slot (SwipeControl.Content =
// panel, the same single-child-host shape content_page_handler.cpp uses), and platform_arrange stamps
// BOTH the SwipeControl's own Canvas.Left/Top + Width/Height (so ITS parent positions it) AND the panel's
// Width/Height to the same frame (so the content child's own Canvas.Left/Top — set by the content's own
// platform_arrange — lands in a Canvas that actually has that coordinate space). No scrollable-extent
// oversizing is needed here (unlike scroll_view_handler.cpp's `max(content, frame)`): a SwipeView's
// Content always fills the view, so the panel is sized to the frame exactly.
//
// SWIPE ITEMS (LeftItems/TopItems/RightItems/BottomItems): NOT realized as native WSwipeItem buttons here.
// SwipeViewHandler.Windows.cs's MapLeftItems/MapTopItems/MapRightItems/MapBottomItems (lines 81-105) each
// funnel into UpdateSwipeItems -> CreateSwipeItems (lines 107-192), which materializes every
// ISwipeItemMenuItem as a real native WSwipeItem via `item.ToHandler(handler.MauiContext!).PlatformView is
// WSwipeItem` (line 184) — that requires a WINDOWS partial for swipe_item_menu_item_handler (the WSwipeItem
// recipe) to produce a native_view() for each item. Only headless/apple/ios have that partial today (see
// CMakeLists.txt's MAUI_WINDOWS_SWAPS); adding a sibling handler file is out of this task's scope (windows
// swipe_view_handler.cpp only). update_items() is left as the shared machine's live-collection read
// instead, matching EVERY OTHER backend's identical comment ("the machine reads the collections directly on
// each swipe, so there is nothing to cache here") — and a static capture shows the CLOSED resting state
// regardless, where the swipe items are not visible on any backend (the android partial's header comment
// documents the same reasoning: "the iOS reference resting state shows none of them").
//
// PROGRAMMATIC OPEN/CLOSE: MapRequestOpen (lines 38-49) is a documented no-op on Windows — the oracle's own
// comment says SwipeControl exposes no API to programmatically reveal its content, citing
// https://learn.microsoft.com/en-us/windows/winui/api/microsoft.ui.xaml.controls.swipecontrol.close — so
// programmatically_open() below does nothing, UNLIKE the apple/android twins (which have no native control
// at all and so drive the full synthetic swipe_machine as their only behavior model). MapRequestClose
// (lines 51-54) DOES have a real native call (`PlatformView.Close()`), ported 1:1 in reset_swipe().
//
// GESTURE PIPELINE (begin_swipe/swipe_to/end_swipe): WinUI's SwipeControl handles the actual pan-to-reveal
// gesture internally (there is no ProcessTouchMove/ProcessTouchUp equivalent anywhere in
// SwipeViewHandler.Windows.cs or SwipeViewExtensions.cs — unlike iOS's hand-rolled UIPanGestureRecognizer,
// WinUI ships the behavior natively). Per this task's instruction not to invent gesture plumbing where the
// oracle has no surface for it, these three entry points keep delegating to the shared cross-platform
// swipe_machine — the SAME posture the apple (AppKit, no native swipe control) and android (plain ViewGroup,
// no native swipe control) partials already take for their own "no native pan yet" gap, so a future
// synthetic-offset-driven test observes the same machine on every backend. The REAL visual reveal on a live
// Windows app comes entirely from WinUI's own native gesture handling on the SwipeControl, outside the
// port's control.

#include "maui/core/swipe_view_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <cmath>
#include <memory>
#include <string_view>

#include "maui/core/i_swipe_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_machine.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/graphics/rect.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and inside
    // namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here would resolve
    // to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using swipe_control = winui::Controls::SwipeControl;
    using canvas = winui::Controls::Canvas;

    swipe_control as_swipe_control(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<swipe_control>();
    }

    // The Canvas panel inside the SwipeControl's Content slot (created in create_platform_view). SwipeControl
    // derives from ContentControl, so Content() reads back as `object`/IInspectable — the same reason
    // scroll_view_handler.cpp's as_panel needs `.as<>()` rather than a direct UIElement read.
    canvas as_panel(void* native)
    {
        return as_swipe_control(native).Content().as<canvas>();
    }

    // The content child's native UIElement, via its view-handler's native_view() — the same helper every
    // single-content host partial defines (content_page_handler.cpp's native_of, scroll_view_handler.cpp's
    // native_of). Null when the child is unattached or its handler has no native view yet.
    winui::UIElement native_of(maui::core::i_view* view)
    {
        if (view == nullptr)
        {
            return nullptr;
        }
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(view->handler().get());
        if (handler == nullptr || handler->native_view() == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(handler->native_view());
    }
} // namespace

namespace maui::core
{
    swipe_view_platform::~swipe_view_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<swipe_view_platform> swipe_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_view_platform>();
        swipe_control control;
        // See the LAYOUT SEAM DECISION in this file's header comment: the Canvas panel is the single
        // child every content-host partial hosts (content_page_handler's Canvas, scroll_view_handler's
        // Canvas), just wrapped in the SwipeControl's ContentControl Content slot instead of set directly.
        canvas panel;
        control.Content(panel);
        platform->native = maui::platform::windows::take<winui::UIElement>(control);
        return platform;
    }

    void swipe_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->native == nullptr)
        {
            return;
        }
        // Single-content host: replace whatever is there rather than appending (content_page_handler's
        // exact recipe), so a re-set (the mapper re-runs on every Content change) cannot stack two
        // generations of content on top of each other.
        const canvas panel = as_panel(platform->native);
        panel.Children().Clear();
        if (const winui::UIElement element = native_of(platform->hosted_content))
        {
            panel.Children().Append(element);
        }
    }

    // C# MapSwipeTransitionMode is EMPTY on Windows too (SwipeViewHandler.Windows.cs:34-36) — SwipeControl
    // has no transition-mode knob to push. Cache mirror only, matching every other backend.
    void swipe_view_handler::update_transition_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->transition = virtual_view()->transition_mode();
    }

    // See the SWIPE ITEMS section of this file's header comment: realizing native WSwipeItem buttons needs
    // a windows swipe_item_menu_item_handler partial that doesn't exist yet (out of this task's scope).
    // The shared swipe_machine reads left_items()/right_items()/top_items()/bottom_items() directly off
    // i_swipe_view on each swipe, so there is nothing to cache here — matching every other backend.
    void swipe_view_handler::update_items()
    {
    }

    // C# MapRequestOpen (SwipeViewHandler.Windows.cs:38-49): documented no-op — SwipeControl has no API to
    // programmatically reveal its content (see the header comment's learn.microsoft.com citation). UNLIKE
    // apple/android (no native control, so the synthetic swipe_machine IS the behavior), Windows has a real
    // native control that genuinely cannot do this, so nothing is invented here.
    void swipe_view_handler::programmatically_open(const swipe_view_open_request& /*request*/)
    {
    }

    // C# MapRequestClose (SwipeViewHandler.Windows.cs:51-54): `handler.PlatformView.Close()`.
    void swipe_view_handler::reset_swipe(bool /*animated*/)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        // Keep the shared state-machine mirror in sync (state -> Idle, IsOpen false written back through
        // the view) — the same observable contract every other backend's reset_swipe provides — THEN push
        // to the real control, which is the oracle's actual mechanism on this backend.
        swipe_machine::reset_swipe(platform->state, *view);
        if (platform->native != nullptr)
        {
            as_swipe_control(platform->native).Close();
        }
    }

    // See the GESTURE PIPELINE section of this file's header comment: WinUI's SwipeControl drives the real
    // pan-to-reveal natively (no Windows.cs surface to port), so these three keep the shared swipe_machine
    // mirror alive — the same posture the apple/android partials take for their own no-native-gesture gap.
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
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite (content_page_handler.cpp /
        // scroll_view_handler.cpp's exact rationale: a NaN reaching XAML here is an unrecoverable stowed
        // exception with no message and no stack, 0xC000027B, so a skipped arrange is strictly better).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const swipe_control control = as_swipe_control(platform->native);
        canvas::SetLeft(control, frame.x);
        canvas::SetTop(control, frame.y);
        control.Width(frame.width);
        control.Height(frame.height);

        // LAYOUT SEAM (see this file's header comment): the panel is sized to the SAME frame (no
        // scrollable-extent oversizing — a SwipeView's Content always fills the view), so the content
        // child's own Canvas.Left/Top (stamped by ITS platform_arrange) lands in a Canvas that actually has
        // that coordinate space.
        const canvas panel = as_panel(platform->native);
        panel.Width(frame.width);
        panel.Height(frame.height);
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void swipe_view_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void swipe_view_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void swipe_view_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void swipe_view_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void swipe_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core

// layout_handler — WinUI 3 platform recipe: a Microsoft.UI.Xaml.Controls.Canvas that HOSTS the arranged
// children. The real-native twin of the headless partial, ported from LayoutHandler.Windows.cs.
//
// MAUI's Windows panel is LayoutPanel, a custom Panel whose MeasureOverride/ArrangeOverride call back
// into CrossPlatformMeasure/CrossPlatformArrange. The port already runs measure and arrange in its own
// cross-platform layout_manager and hands each child an absolute-in-parent frame, so the panel here only
// needs to (a) hold the children and (b) not fight XAML's own layout — which is exactly a Canvas: it
// arranges every child at its Canvas.Left/Top with its own Width/Height and never re-measures. This is
// the same simplification the iOS twin makes when it collapses C#'s LayoutView down to a plain UIView.
//
// Consequence worth stating: each child's platform_arrange writes Canvas.Left/Top + Width/Height on
// ITSELF (see label_handler::platform_arrange), so the panel does not position children at all — it only
// owns the child list and its z-order.

#include "maui/core/layout_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_z_order.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using canvas = winui::Controls::Canvas;

    canvas as_panel(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<canvas>();
    }

    // The child's native UIElement, via its view-handler's native_view() — the Windows analogue of the
    // iOS twin's native_child helper (C#'s ToPlatform()). Null when the child is unattached or its
    // handler produced no native view (every control still on the headless mirror in this first slice).
    winui::UIElement native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr || handler->native_view() == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(handler->native_view());
    }

    // Insert into the panel's Children at `target_index`, clamped to [0, size]. A negative index (an
    // unfound child) drops to the bottom, matching the iOS twin's insert_subview_at and the headless
    // insert_at. XAML throws on an out-of-range index, so the clamp is load-bearing, not defensive.
    //
    // A child ALREADY in this collection is moved, not re-inserted: XAML throws E_INVALIDARG when an
    // element that already has a parent is added to a UIElementCollection, and the port re-fires each
    // container's "add" on every mount replay (element::mount_into_handler, driven by the collection
    // view's ensure_mounted), so the same child legitimately arrives twice. Pass 1 hosts it; pass 2 used
    // to kill the process inside drive_layout -- that is the header_footer_grid crash. Remove-then-insert
    // keeps the replay's INDEX meaning intact instead of making a re-add a silent no-op.
    void insert_child_at(const canvas& panel, const winui::UIElement& child, int target_index)
    {
        const auto children = panel.Children();
        std::uint32_t existing = 0;
        if (children.IndexOf(child, existing))
        {
            children.RemoveAt(existing);
        }
        const auto count = static_cast<int>(children.Size());
        children.InsertAt(static_cast<std::uint32_t>(std::clamp(target_index, 0, count)), child);
    }

    void remove_child(const canvas& panel, const winui::UIElement& child)
    {
        const auto children = panel.Children();
        std::uint32_t index = 0;
        if (children.IndexOf(child, index))
        {
            children.RemoveAt(index);
        }
    }
} // namespace

namespace maui::core
{
    layout_platform::~layout_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    // ClipsToBounds is applied in platform_arrange, where the panel's size is finally known: WinUI clips
    // through a Clip GEOMETRY (a rectangle), not a boolean, so there is nothing to push until then.
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value;
    }

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        auto platform = std::make_unique<layout_platform>();
        platform->native = maui::platform::windows::take<winui::UIElement>(canvas{});
        return platform;
    }

    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // The z-ordered position, not the end — C# LayoutHandler.Add inserts at GetLayoutHandlerIndex.
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
        if (const winui::UIElement element = native_child(child))
        {
            insert_child_at(as_panel(platform->native), element, target);
        }
        // The mirror tracks EVERY child, including one whose handler has no native view yet, so the
        // logical child list stays a faithful record of the layout even during the backend fan-out.
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    void layout_handler::remove(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        if (const winui::UIElement element = native_child(child))
        {
            remove_child(as_panel(platform->native), element);
        }
        std::erase(platform->children, &child);
    }

    void layout_handler::clear()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_panel(platform->native).Children().Clear();
        platform->children.clear();
    }

    void layout_handler::insert(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // As in add(): the NATIVE slot is z-ordered, so the logical `index` only drives the mirror when
        // there is no virtual view to compute a z-order from.
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : index;
        if (const winui::UIElement element = native_child(child))
        {
            insert_child_at(as_panel(platform->native), element, target);
        }
        auto& children = platform->children;
        const auto position = std::min(static_cast<std::size_t>(std::max(target, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        if (index < 0 || static_cast<std::size_t>(index) >= children.size())
        {
            return;
        }
        // Replace in place: the child COUNT is unchanged, so the old element is swapped for the new one at
        // the same slot rather than removed and appended (which would silently reorder the panel).
        const canvas panel = as_panel(platform->native);
        const winui::UIElement incoming = native_child(child);
        std::uint32_t slot = 0;
        bool replaced = false;
        if (const winui::UIElement outgoing = native_child(*children[static_cast<std::size_t>(index)]))
        {
            if (panel.Children().IndexOf(outgoing, slot))
            {
                if (incoming)
                {
                    panel.Children().SetAt(slot, incoming);
                }
                else
                {
                    panel.Children().RemoveAt(slot);
                }
                replaced = true;
            }
        }
        // The OUTGOING child may have had no native view at all -- the common case while the backend
        // fan-out is incomplete, since most controls are still on the headless mirror. Replacing nothing
        // must still HOST the incoming child, or a swap from an unported control to a ported one would
        // silently render nothing. Its z-ordered slot is recomputed, exactly as add() does.
        if (!replaced && incoming)
        {
            const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : -1;
            insert_child_at(panel, incoming, target);
        }
        children[static_cast<std::size_t>(index)] = &child;
    }

    void layout_handler::update_z_index(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        auto& children = platform->children;
        const auto current = std::ranges::find(children, &child);
        if (current == children.end())
        {
            return; // not hosted (C#'s currentIndex == -1)
        }
        const int target = get_layout_handler_index(*virtual_view(), child);
        if (target < 0)
        {
            return;
        }
        if (const winui::UIElement element = native_child(child))
        {
            const canvas panel = as_panel(platform->native);
            remove_child(panel, element);
            insert_child_at(panel, element, target);
        }
        children.erase(current);
        const auto position = std::min(static_cast<std::size_t>(target), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    maui::graphics::size layout_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // A layout sizes itself through its layout_manager (the control overrides measure to delegate to
        // the manager, not to the handler), so the handler reports nothing — same as every other backend.
        return {0, 0};
    }

    void layout_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite. C# only tests `< 0` because its
        // cross-platform arrange never yields NaN; if one ever reaches XAML here it is an unrecoverable
        // stowed exception with no message and no stack (0xC000027B), so a skipped arrange is strictly
        // better than a dead process. A NaN arriving here is an upstream layout bug worth chasing, not
        // a value with a meaning.
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const canvas panel = as_panel(platform->native);
        canvas::SetLeft(panel, frame.x);
        canvas::SetTop(panel, frame.y);
        panel.Width(frame.width);
        panel.Height(frame.height);
        // ClipsToBounds: a Canvas does NOT clip its children by default (unlike a UIView with
        // masksToBounds), so the rectangle has to be rebuilt on every arrange — its size is the only
        // thing that changes, and there is no "clip to my own bounds" flag in XAML.
        if (platform->clips_to_bounds)
        {
            winui::Media::RectangleGeometry clip;
            clip.Rect(winrt::Windows::Foundation::Rect{0.0F, 0.0F, static_cast<float>(frame.width),
                                                       static_cast<float>(frame.height)});
            panel.Clip(clip);
        }
        else
        {
            panel.Clip(nullptr);
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void layout_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void layout_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void layout_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void layout_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void layout_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core

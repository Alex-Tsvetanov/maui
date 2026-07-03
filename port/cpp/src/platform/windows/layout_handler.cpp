// layout_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas panel
// whose Children track the layout control's logical children. The windows twin of
// src/platform/apple/layout_handler.mm (real NSView subviews) and the real-native sibling of the
// headless child-count mirror (src/platform/headless/layout_handler.cpp).
//
// Ported from LayoutHandler.cs + LayoutHandler.Windows.cs (MAUI's LayoutPanel → the port's Canvas
// manual-frame panel — the apple flipped_container twin; children are absolutely positioned by each
// child's own platform_arrange, and the C++ layout_manager owns measure/arrange, so the panel never
// lays out anything itself — exactly LayoutPanel, whose Measure/ArrangeOverride delegate to
// CrossPlatformLayout):
//   - Add/Insert: CachedChildren.Insert(GetLayoutHandlerIndex(child), child.ToPlatform()) — the
//     z-ordered position, not the logical index (both C# bodies are identical).
//   - Remove: CachedChildren.Remove(child.ToPlatform()); Clear: CachedChildren.Clear().
//   - Update: CachedChildren[index] = child.ToPlatform() (replace in place) + EnsureZIndexOrder.
//   - UpdateZIndex → EnsureZIndexOrder: move the child's element to its z-ordered position (a re-order
//     only, no count change).
//   - MapClipsToBounds → LayoutPanel.ClipsToBounds, applied in ArrangeOverride as a full-frame
//     RectangleGeometry Clip — ported in update_clips_to_bounds + platform_arrange below.
//
// The headless children/clips_to_bounds mirrors are ALWAYS maintained (the same dual-drive the android
// partial documents): the XAML-less cross-platform suite (no COM apartment / no XAML runtime on the
// windows preset's test host) observes exactly the headless partial's behavior, while the real app
// additionally drives the real Canvas.Children.

#include "maui/core/layout_handler.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_z_order.hpp"
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
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wnative = maui::platform::win;

    // Insert `child` into the subview mirror at `index` (clamped to [0, size]) — the headless twin's
    // helper, kept verbatim so both partials order identically. A negative index appends.
    void insert_at(std::vector<maui::core::i_view*>& children, int index, maui::core::i_view& child)
    {
        const auto position = std::min(static_cast<std::size_t>(std::max(index, 0)), children.size());
        children.insert(children.begin() + static_cast<std::ptrdiff_t>(position), &child);
    }

    // The child's native UIElement to host, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Empty when the child is unattached / has no native (XAML-less).
    [[nodiscard]] mux::UIElement native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return wnative::borrow_as<mux::UIElement>(handler->native_view());
    }

    // The panel's Children vector, or empty when there is no native panel (XAML-less degradation).
    [[nodiscard]] muxc::UIElementCollection panel_children(void* native)
    {
        auto panel = wnative::borrow<muxc::Canvas>(native);
        return panel != nullptr ? panel.Children() : nullptr;
    }

    // InsertAt with the C# CachedChildren.Insert clamp (the mirror's insert_at semantics on the native
    // vector): a target beyond the native count appends. Detaches the element from any prior parent
    // first (the android add_view_at twin, wnative::detach_from_parent): the mount replay re-fires
    // "add" for an already-hosted child on every boxed-chrome realize pass, and a WinUI InsertAt of a
    // parented element throws E_CHILD_ALREADY_EXISTS — the silent header_footer_* boot fail-fast.
    void native_insert_at(const muxc::UIElementCollection& children, int index, const mux::UIElement& element)
    {
        // Already in THIS collection (the mount replay re-firing "add" for a hosted child): move it,
        // don't double-insert. Checked directly against the target — parent-based detach alone can
        // miss it (a never-loaded subtree reports no logical parent — the wnative helper's note).
        std::uint32_t existing = 0;
        if (children.IndexOf(element, existing))
        {
            children.RemoveAt(existing);
        }
        wnative::detach_from_parent(element); // a child arriving from ANOTHER panel
        const auto count = children.Size();
        const auto position = std::min(static_cast<std::uint32_t>(std::max(index, 0)), count);
        children.InsertAt(position, element);
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Canvas panel (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSView here).
    layout_platform::~layout_platform()
    {
        wnative::release(native);
    }

    // ILayout.ClipsToBounds → LayoutPanel.ClipsToBounds. C# applies/clears the actual Clip geometry in
    // LayoutPanel.ArrangeOverride (the frame is only known there); the port mirrors that split — the
    // flag lands here (and clearing is immediate), the full-frame RectangleGeometry install happens in
    // platform_arrange.
    void layout_platform::update_clips_to_bounds(bool value)
    {
        clips_to_bounds = value; // the headless mirror (the XAML-less suite observes it)
        if (!value)
        {
            if (auto panel = wnative::borrow<muxc::Canvas>(native))
            {
                panel.Clip(nullptr); // LayoutPanel.ArrangeOverride's `Clip = null` branch
            }
        }
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real panel when one exists.

    void layout_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void layout_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void layout_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void layout_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto panel = wnative::borrow<muxc::Canvas>(native);
        if (panel == nullptr)
        {
            return;
        }
        // LayoutHandler.Windows.MapBackground → UpdatePlatformViewBackground's Panel branch:
        // panel.Background = paint.ToPlatform(); null clears the value.
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
        // deferred: gradient / image-source paints (Paint.ToPlatform's LinearGradientBrush et al.) —
        // the base mirror above keeps the borrow observable.
    }

    std::unique_ptr<layout_platform> layout_handler::create_platform_view()
    {
        auto platform = std::make_unique<layout_platform>();
        try
        {
            // LayoutHandler.CreatePlatformView: new LayoutPanel { CrossPlatformLayout = VirtualView } —
            // the port's layout_manager lives on the control, so the panel carries no back-ref (header).
            const muxc::Canvas panel;
            platform->native = wnative::store(panel); // released in ~layout_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    // C# LayoutHandler.Add: CachedChildren.Insert(GetLayoutHandlerIndex(child), child.ToPlatform()) —
    // the z-ordered position, not the end. The child is already in the layout's logical list when this
    // runs (the control appends before invoking "add").
    void layout_handler::add(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child)
                                                     : static_cast<int>(platform->children.size());
        insert_at(platform->children, target, child);
        if (auto children = panel_children(platform->native))
        {
            if (auto element = native_child(child))
            {
                native_insert_at(children, target, element);
            }
        }
    }

    // C# LayoutHandler.Remove: CachedChildren.Remove(child.ToPlatform()).
    void layout_handler::remove(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        std::erase(platform->children, &child);
        if (auto children = panel_children(platform->native))
        {
            if (auto element = native_child(child))
            {
                std::uint32_t index = 0;
                if (children.IndexOf(element, index))
                {
                    children.RemoveAt(index);
                }
            }
        }
    }

    // C# LayoutHandler.Clear: CachedChildren.Clear().
    void layout_handler::clear()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->children.clear();
        if (auto children = panel_children(platform->native))
        {
            children.Clear();
        }
    }

    // C# LayoutHandler.Insert also places the element at GetLayoutHandlerIndex (the z-ordered position),
    // not the logical `index` — the panel's child order is z-index-driven.
    void layout_handler::insert(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const int target = virtual_view() != nullptr ? get_layout_handler_index(*virtual_view(), child) : index;
        insert_at(platform->children, target, child);
        if (auto children = panel_children(platform->native))
        {
            if (auto element = native_child(child))
            {
                native_insert_at(children, target, element);
            }
        }
    }

    // C# LayoutHandler.Update: CachedChildren[index] = child.ToPlatform() (replace in place — the count
    // is unchanged; the follow-up EnsureZIndexOrder re-order arrives through update_z_index).
    void layout_handler::update(int index, i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        auto& mirror = platform->children;
        if (index >= 0 && static_cast<std::size_t>(index) < mirror.size())
        {
            mirror[static_cast<std::size_t>(index)] = &child;
        }
        if (auto children = panel_children(platform->native))
        {
            if (auto element = native_child(child))
            {
                if (index >= 0 && static_cast<std::uint32_t>(index) < children.Size())
                {
                    // Swap the slot in place as RemoveAt + detach-and-insert (the android update's
                    // removeViewAt + addView, the apple removeFromSuperview + reinsert): a plain SetAt
                    // throws E_CHILD_ALREADY_EXISTS when the incoming element is parented anywhere —
                    // including already sitting in this very slot.
                    children.RemoveAt(static_cast<std::uint32_t>(index));
                    native_insert_at(children, index, element);
                }
            }
        }
    }

    // C# LayoutHandler.EnsureZIndexOrder: move `child`'s element to its z-ordered position. Re-order
    // only (no count change); a child not currently hosted is left alone (currentIndex == -1).
    void layout_handler::update_z_index(i_view& child)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        auto& mirror = platform->children;
        const auto current = std::ranges::find(mirror, &child);
        if (current == mirror.end())
        {
            return;
        }
        const int target = get_layout_handler_index(*virtual_view(), child);
        if (target < 0)
        {
            return;
        }
        mirror.erase(current);
        insert_at(mirror, target, child);
        if (auto children = panel_children(platform->native))
        {
            if (auto element = native_child(child))
            {
                std::uint32_t native_current = 0;
                if (children.IndexOf(element, native_current) && static_cast<int>(native_current) != target)
                {
                    children.RemoveAt(native_current); // C# children.Move(current, target)
                    native_insert_at(children, target, element);
                }
            }
        }
    }

    maui::graphics::size layout_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // A layout computes its own size through its layout_manager (the control overrides measure to
        // delegate to the manager, not the handler) — LayoutPanel.MeasureOverride is likewise pure
        // CrossPlatformMeasure delegation, so the handler reports nothing here.
        return {0, 0};
    }

    void layout_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native panel to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the panel to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
        // LayoutPanel.ArrangeOverride's ClipsToBounds branch: Clip = new RectangleGeometry { Rect =
        // (0, 0, finalSize) } when set (the clear-on-false half lives in update_clips_to_bounds).
        if (platform->clips_to_bounds)
        {
            if (auto panel = wnative::borrow<muxc::Canvas>(platform->native))
            {
                const muxm::RectangleGeometry geometry;
                geometry.Rect(winrt::Windows::Foundation::Rect{0.0F, 0.0F, static_cast<float>(frame.width),
                                                               static_cast<float>(frame.height)});
                panel.Clip(geometry);
            }
        }
    }
} // namespace maui::core

#pragma once
// maui::controls::layout<LayoutInterface>  <=  Microsoft.Maui.Controls.Layout
//
// The reusable base for the concrete layout controls (vertical/horizontal stack, …). It adds, on top of
// view<LayoutInterface> (which already supplies the i_view bodies), the three things a layout needs:
//   - the i_container surface over a list of NON-owning child pointers (the caller owns child lifetimes,
//     PROFILE §8) — count/at/add/insert/remove_at/clear/index_of;
//   - i_padding via a bindable padding_ (property<thickness>), whose descriptor each concrete control
//     supplies through the constructor (its own static padding_property());
//   - a lazily-created layout_manager (from the pure-virtual create_layout_manager() each concrete
//     layout overrides to return its M3 manager), and measure/arrange overrides that delegate to it —
//     a layout computes its OWN geometry (unlike a leaf control, which delegates to its handler).
//
// On each child mutation the control notifies its handler (if attached) so the native panel can sync its
// subviews, by invoking the layout command ("add"/"remove"/"clear"/"insert") with a layout_handler_update
// payload — mirroring C# Layout, which routes through the LayoutHandler CommandMapper.
//
// Why the template parameter (cf. view<>): a concrete control is e.g. vertical_stack_layout :
// i_stack_layout, and i_stack_layout already derives i_view (via i_layout). Parameterizing on the
// control's view-interface keeps a single i_view subobject. i_container/i_padding likewise come in once
// (through i_layout) and are implemented here.

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_cross_platform_layout.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::controls
{
    // The shared bindable descriptor for Layout.IsClippedToBounds (ILayout.ClipsToBounds). NON-template
    // free-function descriptor — one descriptor shared across every layout<LayoutInterface>, so the
    // mapper key ("clips_to_bounds") is identical for every layout control — defined out-of-line in
    // src/controls/layout.cpp. Default false (Layout.IsClippedToBoundsProperty).
    const maui::core::bindable_property<bool>& clips_to_bounds_property();

    template <class LayoutInterface>
    class layout : public view<LayoutInterface>,
                   public maui::core::i_cross_platform_layout,
                   public maui::core::i_safe_area_view
    {
        static_assert(std::is_base_of_v<maui::core::i_layout, LayoutInterface>,
                      "LayoutInterface must derive maui::core::i_layout");

    public:
        // ---- i_container (children are referenced, not owned) ----
        [[nodiscard]] int count() const override
        {
            return static_cast<int>(children_.size());
        }
        [[nodiscard]] maui::core::i_view& at(int index) const override
        {
            return *children_[static_cast<std::size_t>(index)];
        }
        void add(maui::core::i_view& child) override
        {
            children_.push_back(&child);
            attach_child(child);
            notify_handler("add", static_cast<int>(children_.size()) - 1, child);
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children_.insert(children_.begin() + index, &child);
            attach_child(child);
            notify_handler("insert", index, child);
        }
        void remove_at(int index) override
        {
            maui::core::i_view& removed = *children_[static_cast<std::size_t>(index)];
            children_.erase(children_.begin() + index);
            detach_child(removed);
            notify_handler("remove", index, removed);
        }
        void clear() override
        {
            for (auto* child : children_)
            {
                detach_child(*child);
            }
            children_.clear();
            notify_handler("clear", 0, nullptr);
        }
        [[nodiscard]] int index_of(const maui::core::i_view& child) const override
        {
            for (std::size_t i = 0; i < children_.size(); ++i)
            {
                if (children_[i] == &child)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // ---- i_padding ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }

        // ---- i_layout: ClipsToBounds (Layout.IsClippedToBounds) ----
        // Whether the layout clips its children to its bounds. Bindable; a change re-runs the layout
        // handler's clips_to_bounds map (Apple: layer.masksToBounds). The arranged frame stays in frame_.
        [[nodiscard]] bool clips_to_bounds() const override
        {
            return clips_to_bounds_.get();
        }
        void set_clips_to_bounds(bool value)
        {
            clips_to_bounds_.set(value);
        }

        // ---- layout size requests (C# IView.Width/Height/Minimum*/Maximum*) ----
        // No override needed: view<>'s base now derives these from the bindable WidthRequest/HeightRequest/
        // Minimum*/Maximum* requests (VisualElement's IView mapping) — an unset width/height reads Unset
        // ("size to content"), an unset minimum reads Unset (a no-op floor), and the maximum defaults to
        // +inf. That is exactly the request semantics the M3 managers' resolve_constraints expects, and it
        // lets a layout honor an explicit size request like any other view. The arranged frame stays in
        // frame_; measure/arrange below delegate to the layout's own manager (a layout sizes itself).

        // ---- layout pass: the layout computes its OWN geometry via its manager (unlike a leaf control,
        // which delegates measure/arrange to its handler). arrange resolves the layout's own FRAME within the
        // allotted bounds — compute_frame honors THIS layout's HorizontalOptions/VerticalOptions so a nested
        // Start/Center/End layout is positioned within its parent's cell, exactly as C# VisualElement.
        // ArrangeOverride (Frame = ComputeFrame(bounds); Handler.PlatformArrange(Frame)) does before the
        // handler's CrossPlatformArrange runs — then sizes the native host panel to that frame and positions
        // the children into it via the manager (C# Layout.CrossPlatformArrange(Frame)).
        maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            const maui::graphics::size measured = ensure_manager().measure(width_constraint, height_constraint);
            this->desired_size_ = measured;
            return measured;
        }
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            this->frame_ = this->compute_frame(bounds);
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(this->handler().get()))
            {
                view_handler->platform_arrange(this->frame_); // size/position the host panel to the frame
            }
            // Position the children WITHIN the native host panel, whose origin is (0,0) in its own
            // coordinate space — the host was framed at `frame_` (relative to the PARENT host) by
            // platform_arrange above, and a native subview's frame is expressed relative to its superview's
            // 0-origin bounds. So the children are arranged HOST-RELATIVE: `frame_`'s absolute origin is
            // dropped, exactly mirroring C#'s native LayoutSubviews, which hands the layout manager its own
            // 0-origin Bounds. Carrying `frame_`'s origin into arrange_children would double-offset every
            // child (host frame origin + the same origin re-added by the manager's `bounds.Left/Top` term),
            // which is the nested-layout overlap bug: a layout sitting at a non-zero offset inside another
            // layout pushed its children down/right by that offset. Mirrors the single-content sibling hosts
            // border::arrange / templated_view::arrange (which likewise arrange their content host-relative).
            // (cross_platform_arrange below keeps the raw bounds — the native-driven path passes the host's
            // own already-0-origin bounds, so it must not be flattened.)
            const maui::graphics::rect host_relative{0, 0, this->frame_.width, this->frame_.height};
            return ensure_manager().arrange_children(host_relative); // position the children within the host
        }

        // ---- i_cross_platform_layout (Layout.CrossPlatformMeasure / CrossPlatformArrange) ----
        // The cross-platform face a native host calls back into: measure/arrange the children through
        // the layout manager, exactly as C# routes LayoutManager.Measure / ArrangeChildren. These are
        // the pure cross-platform half — unlike arrange() above, cross_platform_arrange does NOT size
        // the native host panel (the native side already owns its own frame when it calls this).
        [[nodiscard]] maui::graphics::size cross_platform_measure(double width_constraint,
                                                                  double height_constraint) override
        {
            return ensure_manager().measure(width_constraint, height_constraint);
        }
        maui::graphics::size cross_platform_arrange(const maui::graphics::rect& bounds) override
        {
            return ensure_manager().arrange_children(bounds);
        }

        // ---- i_safe_area_view (Layout.IgnoreSafeArea) ----
        // C# Layout.IgnoreSafeArea is a plain auto-property defaulting to false (the obsolete
        // pre-SafeAreaEdges knob). Honored by the Apple layout host only (a native deferral — the
        // headless backend has no safe area); see i_safe_area_view.hpp.
        [[nodiscard]] bool ignore_safe_area() const override
        {
            return ignore_safe_area_;
        }
        void set_ignore_safe_area(bool value)
        {
            ignore_safe_area_ = value;
        }

    protected:
        explicit layout(const maui::core::bindable_property<maui::core::thickness>& padding_descriptor)
            : padding_(*this, padding_descriptor)
        {
        }

        // Each concrete layout returns its M3 manager (e.g. vertical_stack_layout_manager) over *this.
        [[nodiscard]] virtual std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() = 0;

        // Each child is a logical child, so BindingContext + Window inherit down to it. children_ are the
        // i_view contract; cross-cast to the element base every control shares.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (auto* child : children_)
            {
                if (auto* child_element = dynamic_cast<element*>(child))
                {
                    visit(*child_element);
                }
            }
        }

        // Generic mount (app_host): re-fire the "add" command for each existing child so the now-attached
        // handler's native panel hosts every child's native view — the construction-order replay of the per-
        // child "add" that add() runs once a handler is present (the generic form of gallery_rehost_layout).
        void mount_into_handler() override
        {
            for (int index = 0; index < count(); ++index)
            {
                notify_handler("add", index, at(index));
            }
        }

    private:
        // Attach / detach a child from this layout's logical tree (so it inherits / loses BindingContext +
        // Window). The cast bridges the i_view child pointer to the shared element base.
        void attach_child(maui::core::i_view& child)
        {
            if (auto* child_element = dynamic_cast<element*>(&child))
            {
                this->attach_logical_child(*child_element);
            }
        }
        void detach_child(maui::core::i_view& child)
        {
            if (auto* child_element = dynamic_cast<element*>(&child))
            {
                this->detach_logical_child(*child_element);
            }
        }

        [[nodiscard]] maui::layouts::i_layout_manager& ensure_manager()
        {
            if (!manager_)
            {
                manager_ = create_layout_manager();
            }
            return *manager_;
        }

        // Tell the handler (if attached) to mirror the mutation onto the native panel. `child` may be
        // null for "clear" (no specific view). The payload is the C# LayoutHandlerUpdate (index + view).
        void notify_handler(const char* command, int index, maui::core::i_view* child)
        {
            if (const auto& element_handler = this->handler())
            {
                element_handler->invoke(command, maui::core::layout_handler_update{.index = index, .view = child});
            }
        }
        void notify_handler(const char* command, int index, maui::core::i_view& child)
        {
            notify_handler(command, index, &child);
        }

        std::vector<maui::core::i_view*> children_; // NON-owning: the caller owns the child lifetimes
        maui::core::property<maui::core::thickness> padding_;
        // Layout.IsClippedToBounds (default false). A change re-runs the handler's clips_to_bounds map.
        maui::core::property<bool> clips_to_bounds_{*this, clips_to_bounds_property()};
        std::unique_ptr<maui::layouts::i_layout_manager> manager_;
        bool ignore_safe_area_ = false; // Layout.IgnoreSafeArea (the obsolete auto-property, default false)
    };
} // namespace maui::controls

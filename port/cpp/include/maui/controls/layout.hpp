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
#include <memory>
#include <type_traits>
#include <vector>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_layout.hpp"
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
    template <class LayoutInterface> class layout : public view<LayoutInterface>
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
            notify_handler("add", static_cast<int>(children_.size()) - 1, child);
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children_.insert(children_.begin() + index, &child);
            notify_handler("insert", index, child);
        }
        void remove_at(int index) override
        {
            maui::core::i_view& removed = *children_[static_cast<std::size_t>(index)];
            children_.erase(children_.begin() + index);
            notify_handler("remove", index, removed);
        }
        void clear() override
        {
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

        // ---- layout size requests (C# IView.Width/Height/Minimum*/Maximum*) ----
        // The layout manager's resolve_constraints reads these as the *explicit request* (not the laid-out
        // frame): unset width/height means "size to content", and no min/max means [0, +inf]. view<>'s
        // base returns the frame-derived width/height (an M2/M3 simplification), so layouts override these
        // to the request semantics the M3 managers were written against. The full bindable WidthRequest/
        // HeightRequest surface is part of the deferred VisualElement/ViewMapper work.
        [[nodiscard]] double width() const override
        {
            return maui::core::dimension::unset;
        }
        [[nodiscard]] double height() const override
        {
            return maui::core::dimension::unset;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return maui::core::dimension::minimum;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return maui::core::dimension::minimum;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return maui::core::dimension::maximum;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return maui::core::dimension::maximum;
        }

        // ---- layout pass: the layout computes its OWN geometry via its manager (unlike a leaf control,
        // which delegates measure/arrange to its handler). arrange additionally sizes the native host
        // panel to its bounds (C# VisualElement.ArrangeOverride: Frame = bounds; Handler.PlatformArrange)
        // and positions the children via the manager (C# Layout.CrossPlatformArrange).
        maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            const maui::graphics::size measured = ensure_manager().measure(width_constraint, height_constraint);
            this->desired_size_ = measured;
            return measured;
        }
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            this->frame_ = bounds;
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(this->handler().get()))
            {
                view_handler->platform_arrange(bounds); // size/position the host panel
            }
            return ensure_manager().arrange_children(bounds); // position the children
        }

    protected:
        explicit layout(const maui::core::bindable_property<maui::core::thickness>& padding_descriptor)
            : padding_(*this, padding_descriptor)
        {
        }

        // Each concrete layout returns its M3 manager (e.g. vertical_stack_layout_manager) over *this.
        [[nodiscard]] virtual std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() = 0;

    private:
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
        std::unique_ptr<maui::layouts::i_layout_manager> manager_;
    };
} // namespace maui::controls

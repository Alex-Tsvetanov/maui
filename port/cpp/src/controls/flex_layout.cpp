// maui::controls::flex_layout — the FlexLayout control: drives the vendored flex engine
// (src/layouts/detail/flex) from the children + attached flex values. A faithful port of FlexLayout.cs
// (engine wiring: InitItemProperties / AddFlexItem's SelfSizing / Layout / the measure-mode hack) +
// FlexExtensions.cs (GetConstraints / ToFlexBasis / GetFrame). See flex_layout.hpp.

#include "maui/controls/flex_layout.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

#include "maui/controls/layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_flex_layout.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"

// The vendored flex engine is a maui::layouts internal (src/layouts/detail/, PROFILE §3 — never in the
// public include/ tree). Reached by a relative path so no extra include dir is needed on maui_controls.
#include "../layouts/detail/flex.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();

        // C# FlexExtensions.ToFlexBasis: public FlexBasis -> internal Flex.Basis.
        maui::layouts::flex::basis to_engine_basis(const maui::layouts::flex_basis& basis)
        {
            if (basis.is_auto())
            {
                return maui::layouts::flex::basis::auto_value;
            }
            return maui::layouts::flex::basis{basis.length(), basis.is_relative()};
        }

        // C# FlexExtensions.GetConstraints: walk up the parent items collecting the first finite Width and
        // Height (a constraint of -1 means "still unconstrained on that axis").
        maui::graphics::size engine_constraints(const maui::layouts::flex::item& item)
        {
            double width_constraint = -1;
            double height_constraint = -1;
            const maui::layouts::flex::item* parent = item.parent();
            while (parent != nullptr && (width_constraint < 0 || height_constraint < 0))
            {
                if (width_constraint < 0 && !std::isnan(parent->width()))
                {
                    width_constraint = static_cast<double>(parent->width());
                }
                if (height_constraint < 0 && !std::isnan(parent->height()))
                {
                    height_constraint = static_cast<double>(parent->height());
                }
                parent = parent->parent();
            }
            return {width_constraint, height_constraint};
        }
    } // namespace

    flex_layout::flex_layout() : maui::controls::layout<maui::core::i_flex_layout>(padding_property())
    {
        this->set_style_target_type<flex_layout>(); // implicit / class style match
        root_ = std::make_unique<maui::layouts::flex::item>();
    }

    flex_layout::~flex_layout() = default;

    maui::layouts::flex::item* flex_layout::item_for(const maui::core::i_view& view) const
    {
        const auto found = items_.find(&view);
        return found == items_.end() ? nullptr : found->second.get();
    }

    void flex_layout::init_item_properties(const maui::core::i_view& view, maui::layouts::flex::item& item) const
    {
        item.set_order(get_order(view));
        item.grow = get_grow(view);
        item.shrink = get_shrink(view);
        item.basis = to_engine_basis(get_basis(view));
        item.align_self = static_cast<maui::layouts::flex::align_self>(get_align_self(view));

        const maui::core::thickness margin = view.margin();
        item.margin_left = static_cast<float>(margin.left);
        item.margin_top = static_cast<float>(margin.top);
        item.margin_right = static_cast<float>(margin.right);
        item.margin_bottom = static_cast<float>(margin.bottom);

        // C# GetWidth/GetHeight read WidthRequest/HeightRequest (i_view::width()/height() in the port):
        // a negative (unset) request maps to NaN ("size to content").
        const double width = view.width();
        item.set_width(width < 0 ? nan_f : static_cast<float>(width));
        const double height = view.height();
        item.set_height(height < 0 ? nan_f : static_cast<float>(height));

        item.is_visible = view.visibility() != maui::core::visibility::collapsed;

        if (const auto* with_padding = dynamic_cast<const maui::core::i_padding*>(&view))
        {
            const maui::core::thickness padding = with_padding->padding();
            item.padding[0] = static_cast<float>(padding.left);
            item.padding[1] = static_cast<float>(padding.top);
            item.padding[2] = static_cast<float>(padding.right);
            item.padding[3] = static_cast<float>(padding.bottom);
        }
    }

    void flex_layout::rebuild_items()
    {
        // C# OnAdd/OnRemove maintain the tree incrementally; the port rebuilds it from scratch each layout
        // pass (cheaper than tracking add/insert/remove against the non-owning child list, and behavior is
        // identical — InitItemProperties is re-run every layout in C# via EnsureFlexItemPropertiesUpdated).
        for (std::size_t i = root_->count(); i > 0; --i)
        {
            root_->remove(root_->child_at(i - 1));
        }
        items_.clear();

        // Root knobs (C# InitLayoutProperties).
        root_->align_content = static_cast<maui::layouts::flex::align_content>(align_content());
        root_->align_items = static_cast<maui::layouts::flex::align_items>(align_items());
        root_->direction = static_cast<maui::layouts::flex::direction>(direction());
        root_->justify_content = static_cast<maui::layouts::flex::justify>(justify_content());
        root_->wrap = static_cast<maui::layouts::flex::wrap>(wrap());

        for (int n = 0; n < this->count(); ++n)
        {
            maui::core::i_view& child = this->at(n);
            auto item = std::make_unique<maui::layouts::flex::item>();
            init_item_properties(child, *item);

            // self_sizing measures the child (C# AddFlexItem's SelfSizing delegate). A nested flex_layout
            // would supply its own root as the item and skip this; the port treats every child as a leaf
            // for measurement (nested flex layouts still measure correctly through the child's own manager).
            maui::core::i_view* child_ptr = &child;
            item->self_sizing = [child_ptr](maui::layouts::flex::item& it, float& w, float& h, bool measure_mode) {
                maui::graphics::size request;
                if (measure_mode)
                {
                    maui::graphics::size constraints = engine_constraints(it);
                    // C#: a 0 constraint in measure mode becomes +inf (measure unconstrained).
                    constraints.width =
                        (constraints.width == 0) ? std::numeric_limits<double>::infinity() : constraints.width;
                    constraints.height =
                        (constraints.height == 0) ? std::numeric_limits<double>::infinity() : constraints.height;
                    request = child_ptr->measure(constraints.width, constraints.height);
                }
                else
                {
                    // Arrange pass: never measure; use the already-computed DesiredSize. When an explicit
                    // Width/Height is set on the item, return NaN so layout_item keeps that value (issue
                    // #31109).
                    request = child_ptr->desired_size();
                }
                w = (!measure_mode && !std::isnan(it.width())) ? nan_f : static_cast<float>(request.width);
                h = (!measure_mode && !std::isnan(it.height())) ? nan_f : static_cast<float>(request.height);
            };

            root_->add(*item);
            items_.emplace(&child, std::move(item));
        }
    }

    maui::graphics::rect flex_layout::get_flex_frame(const maui::core::i_view& view) const
    {
        const maui::layouts::flex::item* item = item_for(view);
        if (item == nullptr)
        {
            return {};
        }
        return {item->frame[0], item->frame[1], item->frame[2], item->frame[3]};
    }

    void flex_layout::layout(double width, double height)
    {
        rebuild_items();

        // C# NeedsMeasureHack: an infinite constraint means Shrink/Stretch can't be sized sensibly, so the
        // engine temporarily zeroes Shrink and forces AlignSelf=Start for every child. We apply the same
        // adjustment directly on the (freshly rebuilt) items — RestoreValues is unnecessary because the
        // items are rebuilt on the next pass anyway.
        const bool measure_hack = std::isinf(width) || std::isinf(height);
        if (measure_hack)
        {
            for (int n = 0; n < this->count(); ++n)
            {
                if (auto* item = item_for(this->at(n)))
                {
                    item->shrink = 0;
                    item->align_self = maui::layouts::flex::align_self::start;
                }
            }
        }

        // C#: a positive-infinite available dimension is treated as 0 for the root size.
        root_->set_width(!std::isinf(width) ? static_cast<float>(width) : 0.0F);
        root_->set_height(!std::isinf(height) ? static_cast<float>(height) : 0.0F);
        root_->layout(in_measure_mode_);
    }

    maui::graphics::size flex_layout::measure(double width_constraint, double height_constraint)
    {
        in_measure_mode_ = true;
        const maui::graphics::size result =
            maui::controls::layout<maui::core::i_flex_layout>::measure(width_constraint, height_constraint);
        in_measure_mode_ = false;
        return result;
    }

    maui::graphics::size flex_layout::arrange(const maui::graphics::rect& bounds)
    {
        in_measure_mode_ = false;
        return maui::controls::layout<maui::core::i_flex_layout>::arrange(bounds);
    }

    // ---- shared bindable-property descriptors (defaults match the C# *Property defaults) ----
    const maui::core::bindable_property<maui::layouts::flex_direction>& flex_layout::direction_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_direction> descriptor{
            "direction", maui::layouts::flex_direction::row};
        return descriptor;
    }
    const maui::core::bindable_property<maui::layouts::flex_justify>& flex_layout::justify_content_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_justify> descriptor{
            "justify_content", maui::layouts::flex_justify::start};
        return descriptor;
    }
    const maui::core::bindable_property<maui::layouts::flex_align_content>& flex_layout::align_content_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_align_content> descriptor{
            "align_content", maui::layouts::flex_align_content::stretch};
        return descriptor;
    }
    const maui::core::bindable_property<maui::layouts::flex_align_items>& flex_layout::align_items_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_align_items> descriptor{
            "align_items", maui::layouts::flex_align_items::stretch};
        return descriptor;
    }
    const maui::core::bindable_property<maui::layouts::flex_position>& flex_layout::position_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_position> descriptor{
            "position", maui::layouts::flex_position::relative};
        return descriptor;
    }
    const maui::core::bindable_property<maui::layouts::flex_wrap>& flex_layout::wrap_property()
    {
        static const maui::core::bindable_property<maui::layouts::flex_wrap> descriptor{
            "wrap", maui::layouts::flex_wrap::no_wrap};
        return descriptor;
    }
    const maui::core::bindable_property<maui::core::thickness>& flex_layout::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for flex_layout — a flex_layout is an i_layout, so it reuses
// layout_handler (the same handler the stack/grid layouts use). Opt-in, PROFILE §6.
MAUI_REGISTER_HANDLER(maui::controls::flex_layout, maui::core::layout_handler)

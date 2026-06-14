#pragma once
// maui::controls::flex_layout  <=  Microsoft.Maui.Controls.FlexLayout
//
// A flexbox-like layout control. It is a layout<> over i_flex_layout: the children + padding come from the
// base, and this adds the container-level flex knobs (direction / justify / align-content / align-items /
// position / wrap) + the per-child flex attached values (order / grow / shrink / align-self / basis),
// drives the vendored flex engine (src/layouts/detail/flex), and supplies the flex_layout_manager. Ported
// from FlexLayout.cs.
//
// Engine model: C# keeps a Flex.Item tree — a root item mirroring the layout's knobs, plus one child item
// per view (tracked via the FlexItem attached property / _viewInfo). The port owns that tree here: a root
// flex::item plus an owned flex::item per child, rebuilt lazily, keyed on the child pointer. Each child's
// self_sizing callback measures the view on demand (the measure-mode hack for infinite constraints is
// reproduced). get_flex_frame returns the child item's computed frame.
//
// Attached values: stored in a per-child store (the uniform-map shape grid/absolute_layout use). Defaults
// match the C# attached-property defaults: order 0, grow 0, shrink 1, align-self auto, basis auto. The
// grow/shrink validators (>= 0) match C#'s validateValue (an invalid value is silently ignored).

#include <memory>
#include <unordered_map>

#include "maui/controls/layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_flex_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"
#include "maui/layouts/flex_layout_manager.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::layouts::flex
{
    class item; // the vendored engine node (src/layouts/detail/flex.hpp); pimpl-owned here
} // namespace maui::layouts::flex

namespace maui::controls
{
    class flex_layout : public layout<maui::core::i_flex_layout>
    {
    public:
        flex_layout();
        ~flex_layout() override; // out-of-line: the owned flex::item tree is incomplete in this header
        flex_layout(const flex_layout&) = delete;
        flex_layout(flex_layout&&) = delete;
        flex_layout& operator=(const flex_layout&) = delete;
        flex_layout& operator=(flex_layout&&) = delete;

        // Shared bindable-property descriptors (one instance per type, like FlexLayout.*Property).
        static const maui::core::bindable_property<maui::layouts::flex_direction>& direction_property();
        static const maui::core::bindable_property<maui::layouts::flex_justify>& justify_content_property();
        static const maui::core::bindable_property<maui::layouts::flex_align_content>& align_content_property();
        static const maui::core::bindable_property<maui::layouts::flex_align_items>& align_items_property();
        static const maui::core::bindable_property<maui::layouts::flex_position>& position_property();
        static const maui::core::bindable_property<maui::layouts::flex_wrap>& wrap_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- container-level flex properties (i_flex_layout getters override; setters public) ----
        [[nodiscard]] maui::layouts::flex_direction direction() const override
        {
            return direction_.get();
        }
        void set_direction(maui::layouts::flex_direction value)
        {
            direction_.set(value);
        }
        [[nodiscard]] maui::layouts::flex_justify justify_content() const override
        {
            return justify_content_.get();
        }
        void set_justify_content(maui::layouts::flex_justify value)
        {
            justify_content_.set(value);
        }
        [[nodiscard]] maui::layouts::flex_align_content align_content() const override
        {
            return align_content_.get();
        }
        void set_align_content(maui::layouts::flex_align_content value)
        {
            align_content_.set(value);
        }
        [[nodiscard]] maui::layouts::flex_align_items align_items() const override
        {
            return align_items_.get();
        }
        void set_align_items(maui::layouts::flex_align_items value)
        {
            align_items_.set(value);
        }
        [[nodiscard]] maui::layouts::flex_position position() const override
        {
            return position_.get();
        }
        void set_position(maui::layouts::flex_position value)
        {
            position_.set(value);
        }
        [[nodiscard]] maui::layouts::flex_wrap wrap() const override
        {
            return wrap_.get();
        }
        void set_wrap(maui::layouts::flex_wrap value)
        {
            wrap_.set(value);
        }

        // ---- per-child flex attached values (C# FlexLayout.Get/Set*(IView)) ----
        [[nodiscard]] int get_order(const maui::core::i_view& view) const override
        {
            return info_for(view).order;
        }
        [[nodiscard]] float get_grow(const maui::core::i_view& view) const override
        {
            return info_for(view).grow;
        }
        [[nodiscard]] float get_shrink(const maui::core::i_view& view) const override
        {
            return info_for(view).shrink;
        }
        [[nodiscard]] maui::layouts::flex_align_self get_align_self(const maui::core::i_view& view) const override
        {
            return info_for(view).align_self;
        }
        [[nodiscard]] maui::layouts::flex_basis get_basis(const maui::core::i_view& view) const override
        {
            return info_for(view).basis;
        }
        void set_order(maui::core::i_view& view, int order)
        {
            view_info_[&view].order = order;
        }
        void set_grow(maui::core::i_view& view, float grow)
        {
            if (grow >= 0) // C# GrowProperty validateValue: (float)value >= 0
            {
                view_info_[&view].grow = grow;
            }
        }
        void set_shrink(maui::core::i_view& view, float shrink)
        {
            if (shrink >= 0) // C# ShrinkProperty validateValue: (float)value >= 0
            {
                view_info_[&view].shrink = shrink;
            }
        }
        void set_align_self(maui::core::i_view& view, maui::layouts::flex_align_self align_self)
        {
            view_info_[&view].align_self = align_self;
        }
        void set_basis(maui::core::i_view& view, maui::layouts::flex_basis basis)
        {
            view_info_[&view].basis = basis;
        }

        // ---- i_flex_layout: engine bridge ----
        [[nodiscard]] maui::graphics::rect get_flex_frame(const maui::core::i_view& view) const override;
        void layout(double width, double height) override;

        // measure runs the engine in "measure mode" (C# FlexLayout.CrossPlatformMeasure sets InMeasureMode
        // around the manager.Measure call — the engine then measures children unconstrained on infinite
        // axes instead of stretching them to zero). arrange uses the non-measure path (DesiredSize). We
        // override these to toggle the flag around the base layout<> pass; the base still delegates to the
        // flex_layout_manager, which calls back into layout(...).
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        [[nodiscard]] std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() override
        {
            return std::make_unique<maui::layouts::flex_layout_manager>(*this);
        }

    private:
        // Per-child attached values (C#'s FlexInfo, minus the FlexItem pointer which the port tracks
        // separately in items_). Defaults match the C# attached-property defaults.
        struct flex_info
        {
            int order = 0;
            float grow = 0;
            float shrink = 1.0F;
            maui::layouts::flex_align_self align_self = maui::layouts::flex_align_self::auto_;
            maui::layouts::flex_basis basis = maui::layouts::flex_basis::auto_value;
        };

        [[nodiscard]] flex_info info_for(const maui::core::i_view& view) const
        {
            const auto found = view_info_.find(&view);
            return found == view_info_.end() ? flex_info{} : found->second;
        }

        // (Re)build the engine tree from the current children + attached values, then run the algorithm.
        void rebuild_items();
        void init_item_properties(const maui::core::i_view& view, maui::layouts::flex::item& item) const;
        [[nodiscard]] maui::layouts::flex::item* item_for(const maui::core::i_view& view) const;

        std::unordered_map<const maui::core::i_view*, flex_info> view_info_;

        std::unique_ptr<maui::layouts::flex::item> root_;
        std::unordered_map<const maui::core::i_view*, std::unique_ptr<maui::layouts::flex::item>> items_;
        bool in_measure_mode_ = false;

        maui::core::property<maui::layouts::flex_direction> direction_{*this, direction_property()};
        maui::core::property<maui::layouts::flex_justify> justify_content_{*this, justify_content_property()};
        maui::core::property<maui::layouts::flex_align_content> align_content_{*this, align_content_property()};
        maui::core::property<maui::layouts::flex_align_items> align_items_{*this, align_items_property()};
        maui::core::property<maui::layouts::flex_position> position_{*this, position_property()};
        maui::core::property<maui::layouts::flex_wrap> wrap_{*this, wrap_property()};
    };
} // namespace maui::controls

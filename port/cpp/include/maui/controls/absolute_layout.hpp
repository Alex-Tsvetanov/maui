#pragma once
// maui::controls::absolute_layout  <=  Microsoft.Maui.Controls.AbsoluteLayout
//
// A layout control that positions children at explicit (optionally proportional) bounds. It is a
// layout<> over i_absolute_layout: the children + padding come from the base, and this adds the per-child
// LayoutBounds / LayoutFlags attached storage, then supplies the absolute_layout_manager. Ported from
// AbsoluteLayout.cs.
//
// Attached storage: C# keeps LayoutBounds/LayoutFlags as attached BindableProperties on BindableObject
// children and, for non-bindable (virtual) children, in an internal _viewInfo dictionary. The port uses
// a single uniform store: a std::unordered_map<const i_view*, layout_info> keyed on the child pointer
// (the same shape grid uses — it avoids the index-sync a parallel vector would need). The defaults match
// C#: bounds (0, 0, AutoSize, AutoSize), flags none. AutoSize == -1 (a child sized to its own measure).

#include <memory>
#include <unordered_map>

#include "maui/controls/layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_absolute_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"
#include "maui/layouts/absolute_layout_manager.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::controls
{
    class absolute_layout : public layout<maui::core::i_absolute_layout>
    {
    public:
        // C# AbsoluteLayout.AutoSize: width/height sized to the child's own measure.
        static constexpr double auto_size = -1;

        absolute_layout() : layout(padding_property())
        {
            this->set_style_target_type<absolute_layout>(); // implicit / class style match
        }

        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // The per-child LayoutBounds + LayoutFlags (C#'s AbsoluteLayoutInfo). Defaults match the C#
        // attached-property defaults: bounds (0, 0, AutoSize, AutoSize), flags none.
        struct layout_info
        {
            maui::graphics::rect bounds{0, 0, auto_size, auto_size};
            maui::layouts::absolute_layout_flags flags = maui::layouts::absolute_layout_flags::none;
        };

        // ---- attached LayoutBounds / LayoutFlags (C# AbsoluteLayout.Get/SetLayoutBounds/Flags(IView)) ----
        [[nodiscard]] maui::graphics::rect get_layout_bounds(const maui::core::i_view& view) const override
        {
            return info_for(view).bounds;
        }
        [[nodiscard]] maui::layouts::absolute_layout_flags get_layout_flags(
            const maui::core::i_view& view) const override
        {
            return info_for(view).flags;
        }
        void set_layout_bounds(maui::core::i_view& view, maui::graphics::rect bounds)
        {
            view_info_[&view].bounds = bounds;
            this->invalidate_measure(); // C# LayoutBoundsPropertyChanged → layout.InvalidateMeasure()
        }
        void set_layout_flags(maui::core::i_view& view, maui::layouts::absolute_layout_flags flags)
        {
            view_info_[&view].flags = flags;
        }

        // ---- i_container overrides: prune the per-child store for departed children (C# OnRemove/OnClear)
        void remove_at(int index) override
        {
            const maui::core::i_view* const removed = &this->at(index);
            layout::remove_at(index);
            view_info_.erase(removed);
        }
        void clear() override
        {
            layout::clear();
            view_info_.clear();
        }

    protected:
        [[nodiscard]] std::unique_ptr<maui::layouts::i_layout_manager> create_layout_manager() override
        {
            return std::make_unique<maui::layouts::absolute_layout_manager>(*this);
        }

    private:
        [[nodiscard]] layout_info info_for(const maui::core::i_view& view) const
        {
            const auto found = view_info_.find(&view);
            return found == view_info_.end() ? layout_info{} : found->second;
        }

        std::unordered_map<const maui::core::i_view*, layout_info> view_info_; // keyed on the child pointer
    };
} // namespace maui::controls

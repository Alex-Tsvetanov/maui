#pragma once
// maui::controls::grid_items_layout  <=  Microsoft.Maui.Controls.GridItemsLayout
//
// A multi-column (vertical orientation) / multi-row (horizontal) grid of items: a bindable Span
// (validated >= 1) plus the two inter-item spacings (validated >= 0). Invalid sets are silently
// ignored (the property<T> validate_value convention).

#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class grid_items_layout : public items_layout
    {
    public:
        explicit grid_items_layout(items_layout_orientation orientation) : items_layout(orientation)
        {
        }
        grid_items_layout(int span, items_layout_orientation orientation) : items_layout(orientation)
        {
            set_span(span);
        }

        static const maui::core::bindable_property<int>& span_property();
        static const maui::core::bindable_property<double>& vertical_item_spacing_property();
        static const maui::core::bindable_property<double>& horizontal_item_spacing_property();

        [[nodiscard]] int span() const
        {
            return span_.get();
        }
        void set_span(int value)
        {
            span_.set(value);
        }

        [[nodiscard]] double vertical_item_spacing() const
        {
            return vertical_item_spacing_.get();
        }
        void set_vertical_item_spacing(double value)
        {
            vertical_item_spacing_.set(value);
        }

        [[nodiscard]] double horizontal_item_spacing() const
        {
            return horizontal_item_spacing_.get();
        }
        void set_horizontal_item_spacing(double value)
        {
            horizontal_item_spacing_.set(value);
        }

    private:
        maui::core::property<int> span_{*this, span_property()};
        maui::core::property<double> vertical_item_spacing_{*this, vertical_item_spacing_property()};
        maui::core::property<double> horizontal_item_spacing_{*this, horizontal_item_spacing_property()};
    };
} // namespace maui::controls

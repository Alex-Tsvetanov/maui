#pragma once
// maui::controls::linear_items_layout  <=  Microsoft.Maui.Controls.LinearItemsLayout
//
// A single row/column of items with a bindable ItemSpacing (validated >= 0 — the port's
// validate_value silently ignores an invalid set, the property<T> convention). The C# static
// Vertical/Horizontal singletons are the vertical()/horizontal() accessors; the per-ItemsView
// defaultValueCreator is create_vertical_default()/create_horizontal_default() (a FRESH instance per
// call, like C#). The carousel snap-point presets land with the carousel (out of scope here).

#include <memory>

#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class linear_items_layout : public items_layout
    {
    public:
        explicit linear_items_layout(items_layout_orientation orientation) : items_layout(orientation)
        {
        }

        // LinearItemsLayout.ItemSpacingProperty (default 0; values < 0 ignored).
        static const maui::core::bindable_property<double>& item_spacing_property();

        [[nodiscard]] double item_spacing() const
        {
            return item_spacing_.get();
        }
        void set_item_spacing(double value)
        {
            item_spacing_.set(value);
        }

        // The shared static instances (LinearItemsLayout.Vertical / .Horizontal).
        [[nodiscard]] static const std::shared_ptr<linear_items_layout>& vertical();
        [[nodiscard]] static const std::shared_ptr<linear_items_layout>& horizontal();
        // The per-view defaults (CreateVerticalDefault / CreateHorizontalDefault — fresh instances).
        [[nodiscard]] static std::shared_ptr<linear_items_layout> create_vertical_default();
        [[nodiscard]] static std::shared_ptr<linear_items_layout> create_horizontal_default();

    private:
        maui::core::property<double> item_spacing_{*this, item_spacing_property()};
    };
} // namespace maui::controls

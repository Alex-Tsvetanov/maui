// grid_items_layout — the three descriptors (GridItemsLayout.cs: Span default 1 / >= 1; the two
// spacings default 0 / >= 0).

#include "maui/controls/items/grid_items_layout.hpp"

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<int>& grid_items_layout::span_property()
    {
        static const maui::core::bindable_property<int> descriptor{
            "span", 1, {.validate_value = [](maui::core::bindable_object&, const int& value) { return value >= 1; }}};
        return descriptor;
    }

    const maui::core::bindable_property<double>& grid_items_layout::vertical_item_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "vertical_item_spacing", 0.0, {.validate_value = [](maui::core::bindable_object&, const double& value) {
                return value >= 0;
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<double>& grid_items_layout::horizontal_item_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "horizontal_item_spacing", 0.0, {.validate_value = [](maui::core::bindable_object&, const double& value) {
                return value >= 0;
            }}};
        return descriptor;
    }
} // namespace maui::controls

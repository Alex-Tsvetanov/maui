// linear_items_layout — descriptor + the static/default instances (LinearItemsLayout.cs).

#include "maui/controls/items/linear_items_layout.hpp"

#include <memory>

#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<double>& linear_items_layout::item_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "item_spacing", 0.0, {.validate_value = [](maui::core::bindable_object&, const double& value) {
                return value >= 0;
            }}};
        return descriptor;
    }

    const std::shared_ptr<linear_items_layout>& linear_items_layout::vertical()
    {
        static const std::shared_ptr<linear_items_layout> instance = create_vertical_default();
        return instance;
    }

    const std::shared_ptr<linear_items_layout>& linear_items_layout::horizontal()
    {
        static const std::shared_ptr<linear_items_layout> instance = create_horizontal_default();
        return instance;
    }

    std::shared_ptr<linear_items_layout> linear_items_layout::create_vertical_default()
    {
        return std::make_shared<linear_items_layout>(items_layout_orientation::vertical);
    }

    std::shared_ptr<linear_items_layout> linear_items_layout::create_horizontal_default()
    {
        return std::make_shared<linear_items_layout>(items_layout_orientation::horizontal);
    }
} // namespace maui::controls

// structured_items_view — the five descriptors (StructuredItemsView.cs).

#include "maui/controls/items/structured_items_view.hpp"

#include <memory>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<boxed_item>& structured_items_view::header_property()
    {
        static const maui::core::bindable_property<boxed_item> descriptor{"header"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& structured_items_view::
        header_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"header_template"};
        return descriptor;
    }

    const maui::core::bindable_property<boxed_item>& structured_items_view::footer_property()
    {
        static const maui::core::bindable_property<boxed_item> descriptor{"footer"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& structured_items_view::
        footer_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"footer_template"};
        return descriptor;
    }

    const maui::core::bindable_property<item_sizing_strategy>& structured_items_view::item_sizing_strategy_property()
    {
        static const maui::core::bindable_property<controls::item_sizing_strategy> descriptor{
            "item_sizing_strategy", controls::item_sizing_strategy::measure_all_items};
        return descriptor;
    }
} // namespace maui::controls

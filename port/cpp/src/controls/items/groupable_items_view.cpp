// groupable_items_view — the three descriptors (GroupableItemsView.cs).

#include "maui/controls/items/groupable_items_view.hpp"

#include <memory>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& groupable_items_view::is_grouped_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_grouped", false};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& groupable_items_view::
        group_header_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"group_header_template"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& groupable_items_view::
        group_footer_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"group_footer_template"};
        return descriptor;
    }
} // namespace maui::controls

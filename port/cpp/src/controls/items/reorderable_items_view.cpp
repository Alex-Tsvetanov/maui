// reorderable_items_view — the two descriptors (ReorderableItemsView.cs).

#include "maui/controls/items/reorderable_items_view.hpp"

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& reorderable_items_view::can_mix_groups_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"can_mix_groups", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& reorderable_items_view::can_reorder_items_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"can_reorder_items", false};
        return descriptor;
    }
} // namespace maui::controls

// maui::controls::toolbar_item — the shared bindable-property descriptors. See toolbar_item.hpp;
// ported from src/Controls/src/Core/Toolbar/ToolbarItem.cs.

#include "maui/controls/toolbar_item.hpp"

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<toolbar_item_order>& toolbar_item::order_property()
    {
        static const maui::core::bindable_property<toolbar_item_order> descriptor{"order",
                                                                                  toolbar_item_order::default_order};
        return descriptor;
    }

    const maui::core::bindable_property<int>& toolbar_item::priority_property()
    {
        static const maui::core::bindable_property<int> descriptor{"priority", 0};
        return descriptor;
    }
} // namespace maui::controls

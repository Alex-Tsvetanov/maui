// maui::controls::menu_bar_item — the shared bindable-property descriptors. See menu_bar_item.hpp;
// ported from src/Controls/src/Core/Menu/MenuBarItem.cs.

#include "maui/controls/menu_bar_item.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& menu_bar_item::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& menu_bar_item::is_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<int>& menu_bar_item::priority_property()
    {
        static const maui::core::bindable_property<int> descriptor{"priority", 0};
        return descriptor;
    }
} // namespace maui::controls

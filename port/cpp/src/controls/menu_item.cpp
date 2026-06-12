// maui::controls::menu_item — the shared bindable-property descriptors + the effective IsEnabled walk.
// See menu_item.hpp; ported from src/Controls/src/Core/Menu/MenuItem.cs.

#include "maui/controls/menu_item.hpp"

#include <memory>
#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& menu_item::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& menu_item::is_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& menu_item::is_destructive_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_destructive", false};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& menu_item::
        icon_image_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "icon_image_source"};
        return descriptor;
    }

    bool menu_item::is_enabled() const
    {
        // C# CoerceIsEnabledProperty: an explicit false wins outright; otherwise the parent chain decides
        // (`menuItem.Parent is MenuItem parentMenuItem && !parentMenuItem.IsEnabled` → false). C# coerces
        // the STORED value and re-coerces children via PropagatePropertyChanged on every parent change;
        // the port computes the identical observable result at read time (see the header note). The walk
        // recurses through is_enabled(), so a disabled grand-ancestor disables the whole sub-tree
        // (MenuHierarchyCanBeDisabled).
        if (!is_enabled_.get())
        {
            return false;
        }
        if (const auto* parent = dynamic_cast<const menu_item*>(logical_parent()))
        {
            return parent->is_enabled();
        }
        return true;
    }
} // namespace maui::controls

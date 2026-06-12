// maui::controls::menu_bar — the descriptor, the page-list reconciliation, and the no-parenting list
// hooks. See menu_bar.hpp; ported from src/Controls/src/Core/Menu/MenuBar.cs.

#include "maui/controls/menu_bar.hpp"

#include <cstddef>
#include <vector>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    menu_bar::menu_bar()
        : items_{// C# MenuBar.Add/Insert never call AddLogicalChild — the items stay parented to their
                 // pages (the menu bar is a chrome aggregate).
                 menu_element_list<menu_bar_item>::tree_hook{},
                 // C# RemoveAt: `if (item is Element e && e.Parent == this) e.Parent = null;` — only
                 // un-parent an item that was actually parented to this menu bar.
                 [this](menu_bar_item& child) {
                     if (child.logical_parent() == this)
                     {
                         detach_logical_child(child);
                     }
                 }}
    {
        this->set_style_target_type<menu_bar>();
    }

    const maui::core::bindable_property<bool>& menu_bar::is_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    void menu_bar::sync_menu_bar_items_from_pages(const std::vector<menu_bar_item*>& menu_bar_items)
    {
        // C# MenuBar.SyncMenuBarItemsFromPages, 1:1.
        for (std::size_t i = 0; i < menu_bar_items.size(); ++i)
        {
            menu_bar_item* const item = menu_bar_items[i];
            if (items_.count() > i && items_.at(i) == item)
            {
                continue;
            }
            if (items_.contains(*item))
            {
                items_.remove(*item);
            }
            items_.insert(i, *item);
        }
        while (items_.count() > menu_bar_items.size())
        {
            items_.remove_at(items_.count() - 1);
        }
    }
} // namespace maui::controls

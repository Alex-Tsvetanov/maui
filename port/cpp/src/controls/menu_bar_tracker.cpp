// maui::controls::menu_bar_tracker — the menu_bar_item collection lookup, the priority comparer, and
// the owned menu_bar sync. See menu_bar_tracker.hpp; ported from src/Controls/src/Core/Menu/
// MenuBarTracker.cs.

#include "maui/controls/menu_bar_tracker.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/controls/navigation_page.hpp"

namespace maui::controls
{
    menu_bar_tracker::menu_bar_tracker()
        : sync_token_(collection_changed, collection_changed.connect([this] { sync(); }))
    {
    }

    maui::controls::menu_bar* menu_bar_tracker::menu_bar()
    {
        // C# MenuBarTracker.MenuBar: an empty aggregate yields null (the window then shows no menu bar);
        // otherwise re-sync and hand back the chrome.
        if (toolbar_items().empty())
        {
            return nullptr;
        }
        sync();
        return &bar_;
    }

    void menu_bar_tracker::sync()
    {
        // C# OnMenuBarItemCollectionChanged → _menuBar.SyncMenuBarItemsFromPages(ToolbarItems). (The C#
        // re-parenting of the MenuBar under the window + the handler poke are the window's wiring here —
        // see the header.)
        bar_.sync_menu_bar_items_from_pages(toolbar_items());
    }

    menu_element_list<menu_bar_item>* menu_bar_tracker::get_menu_items(element& page) const
    {
        // C# GetMenuItems(Page) => page.MenuBarItems — the port's "Page" is either page type.
        if (auto* content = dynamic_cast<content_page*>(&page))
        {
            return &content->menu_bar_items();
        }
        if (auto* navigation = dynamic_cast<navigation_page*>(&page))
        {
            return &navigation->menu_bar_items();
        }
        return nullptr;
    }

    bool menu_bar_tracker::less(const menu_bar_item& lhs, const menu_bar_item& rhs) const
    {
        // C# MenuBarItemComparer: x.Priority.CompareTo(y.Priority).
        return lhs.priority() < rhs.priority();
    }
} // namespace maui::controls

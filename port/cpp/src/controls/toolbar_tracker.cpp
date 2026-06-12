// maui::controls::toolbar_tracker — the toolbar_item collection lookup + the priority comparer. See
// toolbar_tracker.hpp; ported from src/Controls/src/Core/Toolbar/ToolbarTracker.cs.

#include "maui/controls/toolbar_tracker.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/toolbar_item.hpp"

namespace maui::controls
{
    menu_element_list<toolbar_item>* toolbar_tracker::get_menu_items(element& page) const
    {
        // C# GetMenuItems(Page) => page.ToolbarItems — the port's "Page" is either page type.
        if (auto* content = dynamic_cast<content_page*>(&page))
        {
            return &content->toolbar_items();
        }
        if (auto* navigation = dynamic_cast<navigation_page*>(&page))
        {
            return &navigation->toolbar_items();
        }
        return nullptr;
    }

    bool toolbar_tracker::less(const toolbar_item& lhs, const toolbar_item& rhs) const
    {
        // C# ToolBarItemComparer: x.Priority.CompareTo(y.Priority).
        return lhs.priority() < rhs.priority();
    }
} // namespace maui::controls

#pragma once
// maui::controls::toolbar_tracker  <=  Microsoft.Maui.Controls.ToolbarTracker
//
// The toolbar_item specialization of menu_item_tracker: items come from a page's toolbar_items()
// collection and sort by ToolbarItem.Priority (ToolBarItemComparer). Ported from
// src/Controls/src/Core/Toolbar/ToolbarTracker.cs.

#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/menu_item_tracker.hpp"
#include "maui/controls/toolbar_item.hpp"

namespace maui::controls
{
    class toolbar_tracker : public menu_item_tracker<toolbar_item>
    {
    protected:
        [[nodiscard]] menu_element_list<toolbar_item>* get_menu_items(element& page) const override;
        [[nodiscard]] bool less(const toolbar_item& lhs, const toolbar_item& rhs) const override;
    };
} // namespace maui::controls

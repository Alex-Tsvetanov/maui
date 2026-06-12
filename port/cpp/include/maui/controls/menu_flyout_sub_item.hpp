#pragma once
// maui::controls::menu_flyout_sub_item  <=  Microsoft.Maui.Controls.MenuFlyoutSubItem
//
// A menu item that opens a nested sub-menu: a menu_flyout_item that is also a list of child menu
// elements. Ported from src/Controls/src/Core/Menu/MenuFlyoutSubItem.cs: Add/Insert parent the child
// into the logical tree (AddLogicalChild — so IsEnabled propagates down and BindingContext inherits),
// Remove/RemoveAt un-parent it, and every mutation notifies the handler seam with (action, index, item)
// — see menu_element_list.hpp for the collapse of C#'s per-element handler Invoke.

#include <cstddef>

#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_menu_flyout_sub_item.hpp"

namespace maui::controls
{
    class menu_flyout_sub_item : public menu_flyout_item, public virtual maui::core::i_menu_flyout_sub_item
    {
    public:
        menu_flyout_sub_item()
        {
            this->set_style_target_type<menu_flyout_sub_item>();
        }

        // The child list (C#'s IList<IMenuElement> face, narrowed to the controls menu_item base so the
        // children participate in the logical tree). Mutations notify the handler seam + `changed`.
        [[nodiscard]] menu_element_list<menu_item>& items()
        {
            return items_;
        }
        [[nodiscard]] const menu_element_list<menu_item>& items() const
        {
            return items_;
        }

        // ---- i_menu_flyout_sub_item ----
        [[nodiscard]] std::size_t item_count() const override
        {
            return items_.count();
        }
        [[nodiscard]] maui::core::i_menu_element* item_at(std::size_t index) const override
        {
            return items_.at(index);
        }

    protected:
        // Every child is a logical child (BindingContext + the IsEnabled chain flow down).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (menu_item* const child : items_.items())
            {
                visit(*child);
            }
        }

    private:
        menu_element_list<menu_item> items_{[this](menu_item& child) { attach_logical_child(child); },
                                            [](menu_item& child) { detach_logical_child(child); }};
    };
} // namespace maui::controls

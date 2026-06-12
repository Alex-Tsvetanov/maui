#pragma once
// maui::controls::menu_flyout  <=  Microsoft.Maui.Controls.MenuFlyout
//
// A flyout presenting a menu of commands — the type attached to a view as its ContextFlyout. Ported
// from src/Controls/src/Core/Menu/MenuFlyout.cs ("same pattern as MenuBarItem"): Add/Insert parent the
// child into the logical tree (so the flyout's BindingContext — inherited from the attached view —
// flows into the items), Remove/RemoveAt un-parent it, and every mutation notifies the handler seam
// with (action, index, item) — see menu_element_list.hpp.

#include <cstddef>

#include "maui/controls/flyout_base.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_menu_flyout.hpp"

namespace maui::controls
{
    class menu_flyout : public flyout_base, public maui::core::i_menu_flyout
    {
    public:
        menu_flyout()
        {
            this->set_style_target_type<menu_flyout>();
        }

        // The flyout's item list. Mutations notify the handler seam + `changed`.
        [[nodiscard]] menu_element_list<menu_item>& items()
        {
            return items_;
        }
        [[nodiscard]] const menu_element_list<menu_item>& items() const
        {
            return items_;
        }

        // ---- i_menu_flyout ----
        [[nodiscard]] std::size_t item_count() const override
        {
            return items_.count();
        }
        [[nodiscard]] maui::core::i_menu_element* item_at(std::size_t index) const override
        {
            return items_.at(index);
        }

    protected:
        // Every item is a logical child (BindingContext inherits down — ContextFlyoutTests).
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

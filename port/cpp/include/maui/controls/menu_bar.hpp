#pragma once
// maui::controls::menu_bar  <=  Microsoft.Maui.Controls.MenuBar
//
// The window-level menu bar chrome: the list of top-level menu_bar_items the native main menu
// materializes. Ported from src/Controls/src/Core/Menu/MenuBar.cs:
//   - Add/Insert do NOT parent the items (a menu_bar_item is parented by its page; the menu bar is a
//     chrome AGGREGATE the tracker syncs) — only Remove/RemoveAt clear the parent when the item was
//     parented to this menu bar;
//   - sync_menu_bar_items_from_pages reconciles the aggregate against the tracker's page-sourced list
//     (the SyncMenuBarItemsFromPages algorithm 1:1);
//   - every mutation notifies the handler seam with (action, index, item) — menu_element_list.hpp.

#include <cstddef>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_menu_bar.hpp"
#include "maui/core/i_menu_bar_item.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class menu_bar : public element, public maui::core::i_menu_bar
    {
    public:
        menu_bar();

        // Shared bindable-property descriptor (MenuBar.IsEnabledProperty).
        static const maui::core::bindable_property<bool>& is_enabled_property();

        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }

        // The top-level menu list. Mutations notify the handler seam + `changed`.
        [[nodiscard]] menu_element_list<menu_bar_item>& items()
        {
            return items_;
        }
        [[nodiscard]] const menu_element_list<menu_bar_item>& items() const
        {
            return items_;
        }

        // C# MenuBar.SyncMenuBarItemsFromPages(IList<MenuBarItem>): reconcile this aggregate against the
        // page-sourced list — skip items already in place, re-insert moved ones, append new ones, trim
        // the tail.
        void sync_menu_bar_items_from_pages(const std::vector<menu_bar_item*>& menu_bar_items);

        // ---- i_menu_bar ----
        [[nodiscard]] bool is_enabled() const override
        {
            return is_enabled_.get();
        }
        [[nodiscard]] std::size_t item_count() const override
        {
            return items_.count();
        }
        [[nodiscard]] maui::core::i_menu_bar_item* item_at(std::size_t index) const override
        {
            return items_.at(index);
        }

    private:
        maui::core::property<bool> is_enabled_{*this, is_enabled_property()};
        menu_element_list<menu_bar_item> items_; // hooks set in the ctor (detach-only — see above)
    };
} // namespace maui::controls

#pragma once
// maui::controls::menu_bar_tracker  <=  Microsoft.Maui.Controls.MenuBarTracker
//
// The menu_bar_item specialization of menu_item_tracker, additionally OWNING the menu_bar chrome it
// keeps in sync: every collection change reconciles the menu bar against the page-sourced aggregate
// (SyncMenuBarItemsFromPages), and menu_bar() returns null while the aggregate is empty. Ported from
// src/Controls/src/Core/Menu/MenuBarTracker.cs. The C# (parent, handlerProperty) pair — re-parenting
// the MenuBar under the window and poking the window handler — is the WINDOW's wiring in the port:
// controls::window subscribes collection_changed and pushes "menu_bar" to its handler itself
// (documented collapse; the menu bar element is not re-parented — items stay parented to their pages).

#include "maui/controls/menu_bar.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/menu_item_tracker.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class menu_bar_tracker : public menu_item_tracker<menu_bar_item>
    {
    public:
        menu_bar_tracker();

        // C# MenuBarTracker.MenuBar: null while the aggregate is empty; otherwise the synced chrome.
        [[nodiscard]] maui::controls::menu_bar* menu_bar();

    protected:
        [[nodiscard]] menu_element_list<menu_bar_item>* get_menu_items(element& page) const override;
        [[nodiscard]] bool less(const menu_bar_item& lhs, const menu_bar_item& rhs) const override;

    private:
        void sync();

        maui::controls::menu_bar bar_;             // the owned chrome (C# `_menuBar = new MenuBar()`)
        maui::core::scoped_connection sync_token_; // CollectionChanged → SyncMenuBarItemsFromPages
    };
} // namespace maui::controls

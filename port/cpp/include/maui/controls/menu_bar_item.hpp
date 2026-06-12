#pragma once
// maui::controls::menu_bar_item  <=  Microsoft.Maui.Controls.MenuBarItem
//
// A top-level menu in the menu bar (e.g. "File"): a titled, enableable element whose children are the
// drop-down's menu elements (menu_flyout_item / menu_flyout_sub_item / menu_flyout_separator). Ported
// from src/Controls/src/Core/Menu/MenuBarItem.cs: Add/Insert parent the child into the LOGICAL tree
// (the backing store IS the logical-children list in C#), Remove/RemoveAt un-parent it, and every
// mutation notifies the handler seam with (action, index, item) — see menu_element_list.hpp. A
// menu_bar_item itself is parented by the PAGE that lists it (Page.MenuBarItems), not by the menu bar
// chrome (C# MenuBar.Add never calls AddLogicalChild).

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/menu_element_list.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_menu_bar_item.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class menu_bar_item : public element, public maui::core::i_menu_bar_item
    {
    public:
        menu_bar_item()
        {
            this->set_style_target_type<menu_bar_item>();
        }

        // Shared bindable-property descriptors (one instance per type, like MenuBarItem.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<bool>& is_enabled_property();
        static const maui::core::bindable_property<int>& priority_property();

        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }
        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }

        // C# MenuBarItem.Priority (sorts the bar's items, like ToolbarItem.Priority).
        [[nodiscard]] int priority() const
        {
            return priority_.get();
        }
        void set_priority(int value)
        {
            priority_.set(value);
        }

        // The drop-down's child list. Mutations notify the handler seam + `changed`.
        [[nodiscard]] menu_element_list<menu_item>& items()
        {
            return items_;
        }
        [[nodiscard]] const menu_element_list<menu_item>& items() const
        {
            return items_;
        }

        // ---- i_menu_bar_item ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
        [[nodiscard]] bool is_enabled() const override
        {
            return is_enabled_.get();
        }
        [[nodiscard]] std::size_t item_count() const override
        {
            return items_.count();
        }
        [[nodiscard]] maui::core::i_menu_element* item_at(std::size_t index) const override
        {
            return items_.at(index);
        }

    protected:
        // Every drop-down child is a logical child (BindingContext inherits down to the flyout items).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (menu_item* const child : items_.items())
            {
                visit(*child);
            }
        }

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<bool> is_enabled_{*this, is_enabled_property()};
        maui::core::property<int> priority_{*this, priority_property()};
        menu_element_list<menu_item> items_{[this](menu_item& child) { attach_logical_child(child); },
                                            [](menu_item& child) { detach_logical_child(child); }};
    };
} // namespace maui::controls

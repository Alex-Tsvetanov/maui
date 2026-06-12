#pragma once
// maui::controls::menu_flyout_item  <=  Microsoft.Maui.Controls.MenuFlyoutItem
//
// A command entry in a menu flyout (a menu bar drop-down or a context menu): a menu_item that
// additionally carries keyboard accelerators. Ported from src/Controls/src/Core/Menu/MenuFlyoutItem.cs
// — the accelerator collection is a plain owned vector (C#'s ObservableCollection change notification
// drives nothing the port materializes incrementally; the native menus rebuild whole — STATUS.md W1-11).

#include <vector>

#include "maui/controls/menu_item.hpp"
#include "maui/core/i_menu_flyout_item.hpp"
#include "maui/core/keyboard_accelerator.hpp"

namespace maui::controls
{
    // i_menu_flyout_item is a VIRTUAL base (its own i_menu_element base already is — see the contract
    // headers): menu_flyout_sub_item / menu_flyout_separator reach it both through this class and
    // through their own core contracts. menu_item's text/is_enabled/send_clicked overrides DOMINATE the
    // shared virtual i_menu_element pures, so no forwarding is needed here.
    class menu_flyout_item : public menu_item, public virtual maui::core::i_menu_flyout_item
    {
    public:
        menu_flyout_item()
        {
            this->set_style_target_type<menu_flyout_item>();
        }

        // The mutable accelerator list (C# MenuFlyoutItem.KeyboardAccelerators, an IList).
        [[nodiscard]] std::vector<maui::core::keyboard_accelerator>& accelerators()
        {
            return accelerators_;
        }
        [[nodiscard]] const std::vector<maui::core::keyboard_accelerator>& accelerators() const
        {
            return accelerators_;
        }

        // ---- i_menu_flyout_item ----
        [[nodiscard]] std::vector<maui::core::keyboard_accelerator> keyboard_accelerators() const override
        {
            return accelerators_;
        }

    private:
        std::vector<maui::core::keyboard_accelerator> accelerators_;
    };
} // namespace maui::controls

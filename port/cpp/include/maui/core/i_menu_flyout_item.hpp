#pragma once
// maui::core::i_menu_flyout_item  <=  Microsoft.Maui.IMenuFlyoutItem
//
// A command entry in a menu flyout (a menu bar drop-down or a context menu): an i_menu_element that may
// carry keyboard accelerators (the shortcut the native menu item shows / registers). Ported from
// src/Core/src/Core/IMenuFlyoutItem.cs (KeyboardAccelerators).
//
// i_menu_element is a VIRTUAL base: the concrete controls::menu_flyout_item reaches it both through
// controls::menu_item and through this contract (C#'s class+interface diamond), so the bases must
// collapse to one subobject for dynamic_cast<i_menu_element*> to stay unambiguous (the same
// virtual-base technique view_handler uses for i_view_handler).

#include <vector>

#include "maui/core/i_menu_element.hpp"
#include "maui/core/keyboard_accelerator.hpp"

namespace maui::core
{
    class i_menu_flyout_item : public virtual i_menu_element
    {
    public:
        // C# IMenuFlyoutItem.KeyboardAccelerators — the shortcut keys (empty when none). Returned by
        // value (a snapshot), like C#'s read-only list projection of the control's collection.
        [[nodiscard]] virtual std::vector<keyboard_accelerator> keyboard_accelerators() const = 0;
    };
} // namespace maui::core

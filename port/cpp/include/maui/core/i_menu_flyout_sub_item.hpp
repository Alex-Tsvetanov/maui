#pragma once
// maui::core::i_menu_flyout_sub_item  <=  Microsoft.Maui.IMenuFlyoutSubItem
//
// A menu item that opens a nested sub-menu: an i_menu_flyout_item that is also a list of child menu
// elements. Ported from src/Core/src/Core/IMenuFlyoutSubItem.cs (IMenuFlyoutSubItem : IMenuFlyoutItem,
// IList<IMenuElement>); the C# IList face becomes the count/at pair the native menu builders walk
// (the mutating surface lives on the concrete controls type). i_menu_flyout_item is a VIRTUAL base
// (the concrete controls type reaches it through two paths — see i_menu_flyout_item.hpp).

#include <cstddef>

#include "maui/core/i_menu_flyout_item.hpp"

namespace maui::core
{
    class i_menu_flyout_sub_item : public virtual i_menu_flyout_item
    {
    public:
        [[nodiscard]] virtual std::size_t item_count() const = 0;
        // The child at `index` (non-owning; the controls layer owns the tree). Null when out of range.
        [[nodiscard]] virtual i_menu_element* item_at(std::size_t index) const = 0;
    };
} // namespace maui::core

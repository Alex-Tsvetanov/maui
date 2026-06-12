#pragma once
// maui::core::i_menu_flyout  <=  Microsoft.Maui.IMenuFlyout
//
// A flyout that displays a menu of commands (the type a context menu materializes from). Ported from
// src/Core/src/Core/IMenuFlyout.cs (IMenuFlyout : IList<IMenuElement>, IFlyout); the C# IList face
// becomes the count/at pair the native menu builders walk.

#include <cstddef>

#include "maui/core/i_flyout.hpp"
#include "maui/core/i_menu_element.hpp"

namespace maui::core
{
    // i_flyout is a VIRTUAL base: the concrete controls::menu_flyout reaches it both through
    // controls::flyout_base and through this contract (see i_menu_flyout_item.hpp for the technique).
    class i_menu_flyout : public virtual i_flyout
    {
    public:
        [[nodiscard]] virtual std::size_t item_count() const = 0;
        // The menu element at `index` (non-owning; the controls layer owns the tree). Null out of range.
        [[nodiscard]] virtual i_menu_element* item_at(std::size_t index) const = 0;
    };
} // namespace maui::core

#pragma once
// maui::core::i_menu_flyout_separator  <=  Microsoft.Maui.IMenuFlyoutSeparator
//
// The marker contract for a separator line between menu items (the natives materialize it as
// NSMenuItem.separatorItem / a UIMenu boundary). Ported from src/Core/src/Core/IMenuFlyoutSeparator.cs
// (IMenuFlyoutSeparator : IMenuFlyoutItem — an empty marker, detected by the native menu builders).
// i_menu_flyout_item is a VIRTUAL base (the concrete controls type reaches it through two paths —
// see i_menu_flyout_item.hpp).

#include "maui/core/i_menu_flyout_item.hpp"

namespace maui::core
{
    class i_menu_flyout_separator : public virtual i_menu_flyout_item
    {
    };
} // namespace maui::core

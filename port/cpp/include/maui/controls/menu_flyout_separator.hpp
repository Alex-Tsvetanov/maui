#pragma once
// maui::controls::menu_flyout_separator  <=  Microsoft.Maui.Controls.MenuFlyoutSeparator
//
// A horizontal line separating menu items. Ported from src/Controls/src/Core/Menu/
// MenuFlyoutSeparator.cs (an empty MenuFlyoutItem subclass); the native menu builders detect the
// i_menu_flyout_separator contract and materialize NSMenuItem.separatorItem (AppKit).

#include "maui/controls/menu_flyout_item.hpp"
#include "maui/core/i_menu_flyout_separator.hpp"

namespace maui::controls
{
    class menu_flyout_separator : public menu_flyout_item, public virtual maui::core::i_menu_flyout_separator
    {
    public:
        menu_flyout_separator()
        {
            this->set_style_target_type<menu_flyout_separator>();
        }
    };
} // namespace maui::controls

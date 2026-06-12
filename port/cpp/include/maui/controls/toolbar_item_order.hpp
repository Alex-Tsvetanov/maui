#pragma once
// maui::controls::toolbar_item_order  <=  Microsoft.Maui.Controls.ToolbarItemOrder
//
// Whether a ToolbarItem appears on the primary or the secondary (overflow) toolbar surface. Ported
// from src/Controls/src/Core/Toolbar/ToolbarItemOrder.cs ("default" is a C++ keyword, hence
// default_order; the values and meaning are 1:1).

namespace maui::controls
{
    enum class toolbar_item_order
    {
        default_order, // use the platform's default placement (the primary surface)
        primary,       // place on the primary toolbar surface
        secondary,     // place on the secondary (overflow) surface
    };
} // namespace maui::controls

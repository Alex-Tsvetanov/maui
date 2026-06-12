#pragma once
// maui::core::i_toolbar_item  <=  the toolbar-item face of Microsoft.Maui.Controls.ToolbarItem
//
// The narrow contract the native toolbar chrome materializes a toolbar entry from: the i_menu_element
// surface (text / enabled / clicked) plus the Primary-vs-Secondary placement. C# has no IToolbarItem —
// its per-platform Toolbar partials downcast to the concrete Controls.ToolbarItem and read Order; the
// port keeps the handler layer concrete-free, so the placement flag is surfaced on this small contract
// instead (documented deviation; the semantics are ToolbarItemOrder.cs's: Secondary items go to the
// secondary/overflow surface, Default and Primary to the primary surface).

// i_menu_element is a VIRTUAL base — the concrete controls::toolbar_item reaches it both through
// controls::menu_item and through this contract (see i_menu_flyout_item.hpp for the technique).

#include "maui/core/i_menu_element.hpp"

namespace maui::core
{
    class i_toolbar_item : public virtual i_menu_element
    {
    public:
        // True when the item belongs on the SECONDARY toolbar surface (ToolbarItemOrder.Secondary —
        // an overflow menu on the natives); Default/Primary items return false.
        [[nodiscard]] virtual bool is_secondary() const = 0;
    };
} // namespace maui::core

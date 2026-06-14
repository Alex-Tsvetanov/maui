#pragma once
// maui::core::i_swipe_item_menu_item  <=  Microsoft.Maui.ISwipeItemMenuItem
//
// A swipe item rendered as a coloured menu button (the SwipeItem control's contract). Ported from
// src/Core/src/Core/ISwipeItemMenuItem.cs (ISwipeItemMenuItem : IMenuElement, ISwipeItem): the Background
// paint the native button fills with and the Visibility that gates whether it participates. is_enabled()
// arrives through the IMenuElement base — the swipe state machine's ExecuteSwipeItem reads it to decide
// whether to invoke (MauiSwipeView.ExecuteSwipeItem).

#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_swipe_item.hpp"
#include "maui/core/visibility.hpp"

namespace maui::graphics
{
    class paint;
}

namespace maui::core
{
    // i_menu_element is a VIRTUAL base so it collapses to one subobject with the menu_item base (which
    // also derives i_menu_element virtually) — otherwise swipe_item would carry two i_menu_element
    // subobjects and dynamic_cast<i_menu_element*> would be ambiguous.
    class i_swipe_item_menu_item : public virtual i_menu_element, public i_swipe_item
    {
    public:
        // C# ISwipeItemMenuItem.Background — the paint filling the menu button's background (null = none).
        [[nodiscard]] virtual const maui::graphics::paint* background() const = 0;

        // C# ISwipeItemMenuItem.Visibility — whether the item is part of the visual tree.
        [[nodiscard]] virtual maui::core::visibility visibility() const = 0;
    };
} // namespace maui::core

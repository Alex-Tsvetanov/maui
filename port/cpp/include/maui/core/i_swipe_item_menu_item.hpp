#pragma once
// maui::core::i_swipe_item_menu_item  <=  Microsoft.Maui.ISwipeItemMenuItem
//
// A swipe item rendered as a coloured menu button (the SwipeItem control's contract). Ported from
// src/Core/src/Core/ISwipeItemMenuItem.cs (ISwipeItemMenuItem : IMenuElement, ISwipeItem): the Background
// paint the native button fills with and the Visibility that gates whether it participates. is_enabled()
// arrives through the IMenuElement base — the swipe state machine's ExecuteSwipeItem reads it to decide
// whether to invoke (MauiSwipeView.ExecuteSwipeItem).
//
// W7-U09: the swipe_item_menu_item_handler (the UIButton recipe) maps Text/TextColor/Font/
// CharacterSpacing/Background/Source onto the native button. C#'s IMenuElement : IImageSourcePart, IText
// — the port's i_menu_element collapsed those (text() only) — so the text-style face (i_text_style:
// text_color/font/character_spacing) and the icon source() the handler reads are added HERE, on the
// swipe-item contract that needs them, rather than reshaping the shared i_menu_element. On the concrete
// SwipeItem (: MenuItem) these are the INERT MenuItem.ITextStyle defaults (TextColor null, Font.Default,
// CharacterSpacing 0 — MenuItem.cs:156-160); the visible label colour comes from get_text_color() below.

#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_swipe_item.hpp"
#include "maui/core/i_text_style.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"

#include <memory>
#include <optional>

namespace maui::graphics
{
    class paint;
}

namespace maui::core
{
    // i_menu_element and i_text_style are VIRTUAL bases: i_menu_element collapses to one subobject with the
    // menu_item base (which also derives i_menu_element virtually) — otherwise swipe_item would carry two
    // i_menu_element subobjects and dynamic_cast<i_menu_element*> would be ambiguous; i_text_style is
    // virtual for symmetry (no second copy if a future base also reaches it).
    class i_swipe_item_menu_item : public virtual i_menu_element, public virtual i_text_style, public i_swipe_item
    {
    public:
        // C# ISwipeItemMenuItem.Background — the paint filling the menu button's background (null = none).
        [[nodiscard]] virtual const maui::graphics::paint* background() const = 0;

        // C# ISwipeItemMenuItem.Visibility — whether the item is part of the visual tree.
        [[nodiscard]] virtual maui::core::visibility visibility() const = 0;

        // C# IMenuElement.Source (IImageSourcePart.Source) — the icon image source the native button shows
        // (null when none). The handler's MapSource resizes + tints it (SwipeItemMenuItemHandler.iOS).
        [[nodiscard]] virtual std::shared_ptr<maui::core::i_image_source> source() const = 0;
    };

    // Microsoft.Maui.Platform.SwipeViewExtensions.GetTextColor (src/Core/src/Platform/SwipeViewExtensions.cs):
    // the effective title colour the swipe button's text is drawn in. Returns nullopt when there is no
    // background colour to read, or when the icon is a font-image source carrying its own colour (the icon
    // tint then drives the appearance). Otherwise picks white on a dark background / black on a light one,
    // using the C# luminosity coefficients (0.2126 R + 0.7152 G + 0.0722 B) against the 0.75 threshold.
    [[nodiscard]] std::optional<maui::graphics::color> get_text_color(const i_swipe_item_menu_item& item);
} // namespace maui::core

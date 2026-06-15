#pragma once
// maui::core::swipe_item_menu_item_handler  <=  Microsoft.Maui.Handlers.SwipeItemMenuItemHandler
//
// The handler for a swipe menu item (ISwipeItemMenuItem) — the coloured button the SwipeView reveals.
// Ported from SwipeItemMenuItemHandler.cs (+ .iOS.cs).
//
//   - Maps Text / TextColor / CharacterSpacing / Font / Background / Source / Visibility onto the native
//     button (SwipeItemMenuItemHandler.Mapper).
//   - The native platform view (SwipeItemMenuItemHandler.iOS) is a UIButton subclass with
//     UserInteractionEnabled=false (the SwipeView's own pan gesture drives activation, not the button),
//     RestorationIdentifier = Text, that observes its own Frame so the icon re-resizes when the swipe
//     item is laid out.
//
// PORT ADAPTATION (vs window_handler / the view handlers): C#'s SwipeItemMenuItemHandler is an
// ElementHandler<ISwipeItemMenuItem, UIButton> driven by a real IPropertyMapper, because C#'s
// ISwipeItemMenuItem : IMenuElement : IElement is a full IElement. The PORT's i_swipe_item_menu_item is
// deliberately NOT an i_element (i_menu_element/i_swipe_item carry only the activation surface — see
// i_swipe_item.hpp), and the concrete swipe_item (: menu_item) is not handler-hosted through the standard
// i_element/registry seam. So this handler is a STANDALONE handler (not an i_element_handler, not the
// generic property_mapper): it connects directly to the i_swipe_item_menu_item, runs the same property
// pushes (the apply_* mirror MapText/MapTextColor/...), and re-pushes one property via update_value(name).
// The C# MauiSwipeView creates this handler when it materializes the swipe items natively; the port's
// swipe machine activates the items through the contract, and this handler renders their button visual.
//
// Same partial-class split + single cross-platform swipe_item_menu_item_platform struct as the view
// handlers: the ctor + connect/update lifecycle + the get_text_color helper live in
// swipe_item_menu_item_handler.cpp; the platform recipe (create the UIButton, push title/colour/font/
// background/icon, the frame observer) lives per backend under
// src/platform/<backend>/swipe_item_menu_item_handler.{cpp,mm}.

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/font.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/visibility.hpp"

namespace maui::core
{
    // The pimpl that owns the native button (PROFILE §8). A single cross-platform struct (like the view
    // handlers' *_platform): the native slot + a headless mirror of what the native button tracks (the
    // last text / title colour / background / font / source / visibility pushed), so the headless tests can
    // observe the mapped values without a real UIButton. Backend-defined destructor releases the retained
    // native button on iOS/apple and detaches the frame observer.
    struct swipe_item_menu_item_platform
    {
        swipe_item_menu_item_platform() = default;
        ~swipe_item_menu_item_platform(); // backend-defined: releases the retained UI/NSButton + frame KVO
        swipe_item_menu_item_platform(const swipe_item_menu_item_platform&) = delete;
        swipe_item_menu_item_platform(swipe_item_menu_item_platform&&) = delete;
        swipe_item_menu_item_platform& operator=(const swipe_item_menu_item_platform&) = delete;
        swipe_item_menu_item_platform& operator=(swipe_item_menu_item_platform&&) = delete;

        void* native = nullptr; // the native UI/NSButton (iOS/apple) — null on headless

        // ---- headless mirror (the button's observable state; the iOS/apple build ALSO drives the real button) ----
        i_swipe_item_menu_item* hosted_item = nullptr; // the virtual view (set on connect)
        std::string title;                             // the last title pushed (MapText / RestorationIdentifier)
        bool has_title_color = false;                  // whether a title colour was applied (GetTextColor non-null)
        std::uint32_t title_color_argb = 0;            // the applied title colour (MapTextColor)
        bool has_background = false;                   // whether a background paint was applied (MapBackground)
        std::uint32_t background_argb = 0;             // the applied background colour
        double character_spacing = 0;                  // the last CharacterSpacing pushed (MapCharacterSpacing)
        maui::core::font item_font;                    // the last Font pushed (MapFont)
        bool has_source = false;                       // whether a non-empty icon source was set (MapSource)
        maui::core::visibility item_visibility = maui::core::visibility::visible; // the last Visibility pushed
    };

    class swipe_item_menu_item_handler
    {
    public:
        swipe_item_menu_item_handler();
        ~swipe_item_menu_item_handler();
        swipe_item_menu_item_handler(const swipe_item_menu_item_handler&) = delete;
        swipe_item_menu_item_handler(swipe_item_menu_item_handler&&) = delete;
        swipe_item_menu_item_handler& operator=(const swipe_item_menu_item_handler&) = delete;
        swipe_item_menu_item_handler& operator=(swipe_item_menu_item_handler&&) = delete;

        static std::unique_ptr<swipe_item_menu_item_platform> create_platform_view();

        // C# ElementHandler.SetVirtualView: create the native button on first connect, wire the frame
        // observer (ConnectHandler), then push every mapped property. Re-running with the same item is a
        // no-op.
        void set_virtual_view(i_swipe_item_menu_item& item);
        // C# UpdateValue(propertyName): re-push one property (by the port's snake_case key —
        // text/text_color/character_spacing/font/background/source/visibility). Unknown keys are ignored.
        void update_value(std::string_view property);
        // C# ElementHandler.DisconnectHandler: tear down the frame observer + drop the references.
        // Idempotent.
        void disconnect_handler();

        // The typed platform-view accessor (the pimpl) for tests + the per-backend recipe.
        [[nodiscard]] swipe_item_menu_item_platform* typed_platform_view() const
        {
            return platform_view_.get();
        }
        // The connected swipe-item contract, or null if disconnected.
        [[nodiscard]] i_swipe_item_menu_item* item_view() const
        {
            return item_view_;
        }
        // The native UI/NSButton (null on headless / when disconnected) — for the native seam tests.
        [[nodiscard]] void* native_view() const
        {
            return platform_view_ != nullptr ? platform_view_->native : nullptr;
        }

    private:
        // Push every property once (SetVirtualView's mapper pass).
        void apply_all() const;

        // The backend recipe: each pushes one property to the native button (defined per backend in
        // swipe_item_menu_item_handler.{cpp,mm}). const: each touches only the pimpl (+ the native button).
        void connect();          // create the frame observer (C# ConnectHandler / SwipeItemButtonProxy)
        void disconnect() const; // tear down the frame observer (C# DisconnectHandler)
        void apply_text() const;
        void apply_text_color() const;
        void apply_character_spacing() const;
        void apply_font() const;
        void apply_background() const;
        void apply_source() const;
        void apply_visibility() const;

        std::unique_ptr<swipe_item_menu_item_platform> platform_view_;
        i_swipe_item_menu_item* item_view_ = nullptr; // non-owning (the item owns the handler)
    };
} // namespace maui::core

#pragma once
// maui::core::i_menu_bar_element  <=  Microsoft.Maui.IMenuBarElement
//
// Marks an element (the window here) as carrying a menu bar. The window_handler cross-casts its
// i_window to this to reach the menu bar it materializes (the NSMenu main menu on AppKit). Ported
// from src/Core/src/Core/IMenuBarElement.cs.

namespace maui::core
{
    class i_menu_bar;

    class i_menu_bar_element
    {
    public:
        virtual ~i_menu_bar_element() = default;

        // The element's menu bar, or null when there are no menu bar items (C# MenuBarTracker.MenuBar
        // returns null for an empty tracker). Non-owning.
        [[nodiscard]] virtual i_menu_bar* menu_bar() const = 0;

    protected:
        i_menu_bar_element() = default;
        i_menu_bar_element(const i_menu_bar_element&) = default;
        i_menu_bar_element(i_menu_bar_element&&) = default;
        i_menu_bar_element& operator=(const i_menu_bar_element&) = default;
        i_menu_bar_element& operator=(i_menu_bar_element&&) = default;
    };
} // namespace maui::core

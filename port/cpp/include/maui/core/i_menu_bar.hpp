#pragma once
// maui::core::i_menu_bar  <=  Microsoft.Maui.IMenuBar
//
// The horizontal row of top-level menus at the top of an app window (the NSMenu main menu on macOS).
// Ported from src/Core/src/Core/IMenuBar.cs (IMenuBar : IList<IMenuBarItem>, IElement); the C# IList
// face becomes the count/at pair the native menu builder walks.

#include <cstddef>

#include "maui/core/i_menu_bar_item.hpp"

namespace maui::core
{
    class i_menu_bar
    {
    public:
        virtual ~i_menu_bar() = default;

        // C# IMenuBar.IsEnabled.
        [[nodiscard]] virtual bool is_enabled() const = 0;

        [[nodiscard]] virtual std::size_t item_count() const = 0;
        // The top-level menu at `index` (non-owning). Null when out of range.
        [[nodiscard]] virtual i_menu_bar_item* item_at(std::size_t index) const = 0;

    protected:
        i_menu_bar() = default;
        i_menu_bar(const i_menu_bar&) = default;
        i_menu_bar(i_menu_bar&&) = default;
        i_menu_bar& operator=(const i_menu_bar&) = default;
        i_menu_bar& operator=(i_menu_bar&&) = default;
    };
} // namespace maui::core

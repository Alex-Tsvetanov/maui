#pragma once
// maui::core::i_menu_bar_item  <=  Microsoft.Maui.IMenuBarItem
//
// A top-level menu in a menu bar (e.g. "File"): a titled, enableable list of menu elements (its
// drop-down). Ported from src/Core/src/Core/IMenuBarItem.cs (IMenuBarItem : IList<IMenuElement>,
// IElement); the C# IList face becomes the count/at pair the native menu builders walk.

#include <cstddef>
#include <string_view>

#include "maui/core/i_menu_element.hpp"

namespace maui::core
{
    class i_menu_bar_item
    {
    public:
        virtual ~i_menu_bar_item() = default;

        // C# IMenuBarItem.Text — the top-level menu title.
        [[nodiscard]] virtual std::string_view text() const = 0;
        // C# IMenuBarItem.IsEnabled.
        [[nodiscard]] virtual bool is_enabled() const = 0;

        [[nodiscard]] virtual std::size_t item_count() const = 0;
        // The drop-down element at `index` (non-owning). Null when out of range.
        [[nodiscard]] virtual i_menu_element* item_at(std::size_t index) const = 0;

    protected:
        i_menu_bar_item() = default;
        i_menu_bar_item(const i_menu_bar_item&) = default;
        i_menu_bar_item(i_menu_bar_item&&) = default;
        i_menu_bar_item& operator=(const i_menu_bar_item&) = default;
        i_menu_bar_item& operator=(i_menu_bar_item&&) = default;
    };
} // namespace maui::core

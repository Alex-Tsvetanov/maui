#pragma once
// maui::core::i_toolbar  <=  Microsoft.Maui.IToolbar
//
// The bar that may display the page title, navigation affordances, and the page's toolbar items —
// the contract the window chrome host materializes (an NSToolbar on AppKit). Ported from
// src/Core/src/Core/IToolbar.cs (BackButtonVisible / IsVisible / Title) PLUS the items walk: C#'s
// IToolbar carries no items (the per-platform Toolbar partials read the concrete Controls.Toolbar's
// ToolbarItems); the port keeps the handler layer concrete-free, so the count/at pair is folded onto
// this contract (documented deviation, same shape as the menu contracts).

#include <cstddef>
#include <string_view>

#include "maui/core/i_toolbar_item.hpp"

namespace maui::core
{
    class i_toolbar
    {
    public:
        virtual ~i_toolbar() = default;

        // C# IToolbar.BackButtonVisible.
        [[nodiscard]] virtual bool back_button_visible() const = 0;
        // C# IToolbar.IsVisible.
        [[nodiscard]] virtual bool is_visible() const = 0;
        // C# IToolbar.Title.
        [[nodiscard]] virtual std::string_view title() const = 0;

        // The aggregated toolbar items (priority-sorted by the toolbar tracker). Non-owning.
        [[nodiscard]] virtual std::size_t item_count() const = 0;
        [[nodiscard]] virtual i_toolbar_item* item_at(std::size_t index) const = 0;

    protected:
        i_toolbar() = default;
        i_toolbar(const i_toolbar&) = default;
        i_toolbar(i_toolbar&&) = default;
        i_toolbar& operator=(const i_toolbar&) = default;
        i_toolbar& operator=(i_toolbar&&) = default;
    };
} // namespace maui::core

#pragma once
// maui::core::i_menu_element  <=  Microsoft.Maui.IMenuElement
//
// The contract every menu entry implements (menu bar drop-down items, context-flyout items, toolbar
// items): the text + enabled state the native menu materializes, and the INBOUND clicked() channel the
// native menu item's action invokes (IMenuElement.Clicked() → MenuItem's IMenuItemController.Activate).
// Ported from src/Core/src/Core/IMenuElement.cs. C#'s IImageSourcePart/IText bases are collapsed to the
// text() the natives read; the icon image source stays on the concrete controls::menu_item (its native
// materialization is deferred — see STATUS.md).

#include <string_view>

namespace maui::core
{
    class i_menu_element
    {
    public:
        virtual ~i_menu_element() = default;

        // C# IText.Text — the label the native menu item shows.
        [[nodiscard]] virtual std::string_view text() const = 0;

        // C# IMenuElement.IsEnabled — the EFFECTIVE enabled state (a disabled ancestor menu disables it).
        [[nodiscard]] virtual bool is_enabled() const = 0;

        // C# IMenuElement.Clicked() — the inbound activation the native menu item's action calls (routes
        // to the control's clicked event, the port's command-as-clicked-event channel). Named send_* per
        // the port's inbound-channel convention (like i_button's send_clicked), freeing the plain name
        // for the control's observable `clicked` event.
        virtual void send_clicked() = 0;

    protected:
        i_menu_element() = default;
        i_menu_element(const i_menu_element&) = default;
        i_menu_element(i_menu_element&&) = default;
        i_menu_element& operator=(const i_menu_element&) = default;
        i_menu_element& operator=(i_menu_element&&) = default;
    };
} // namespace maui::core

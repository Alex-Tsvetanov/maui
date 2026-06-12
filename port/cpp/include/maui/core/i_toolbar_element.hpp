#pragma once
// maui::core::i_toolbar_element  <=  Microsoft.Maui.IToolbarElement
//
// Marks an element (the window here) as carrying a toolbar. The window_handler cross-casts its
// i_window to this to reach the chrome it materializes. Ported from src/Core/src/Core/IToolbarElement.cs.

namespace maui::core
{
    class i_toolbar;

    class i_toolbar_element
    {
    public:
        virtual ~i_toolbar_element() = default;

        // The element's toolbar, or null when it has none (no navigation chrome yet). Non-owning.
        [[nodiscard]] virtual i_toolbar* toolbar() const = 0;

    protected:
        i_toolbar_element() = default;
        i_toolbar_element(const i_toolbar_element&) = default;
        i_toolbar_element(i_toolbar_element&&) = default;
        i_toolbar_element& operator=(const i_toolbar_element&) = default;
        i_toolbar_element& operator=(i_toolbar_element&&) = default;
    };
} // namespace maui::core

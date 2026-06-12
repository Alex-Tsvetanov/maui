#pragma once
// maui::core::i_title_bar_element  <=  the TitleBar slot of Microsoft.Maui.IWindow
//
// Marks an element (the window) as carrying a custom title bar. C# puts TitleBar directly on IWindow
// (mapped on Windows + Mac Catalyst); the port surfaces it on this separate cast-interface instead so
// the existing i_window contract stays untouched (documented deviation — same dynamic_cast discovery
// shape as i_toolbar_element / i_menu_bar_element).

namespace maui::core
{
    class i_title_bar;

    class i_title_bar_element
    {
    public:
        virtual ~i_title_bar_element() = default;

        // The element's title bar, or null when none is set. Non-owning.
        [[nodiscard]] virtual i_title_bar* title_bar() const = 0;

    protected:
        i_title_bar_element() = default;
        i_title_bar_element(const i_title_bar_element&) = default;
        i_title_bar_element(i_title_bar_element&&) = default;
        i_title_bar_element& operator=(const i_title_bar_element&) = default;
        i_title_bar_element& operator=(i_title_bar_element&&) = default;
    };
} // namespace maui::core

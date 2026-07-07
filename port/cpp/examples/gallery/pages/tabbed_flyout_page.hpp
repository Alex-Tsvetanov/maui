#pragma once
// tabbed_flyout_page — the code-first twin of the shared port/maui-reference/pages/tabbed_flyout.xaml.
//
// The C# page is a FlyoutPage whose flyout pane is a titled menu (Home tab / Settings tab / Toggle flyout
// buttons + a status label) and whose detail pane is a TabbedPage (Home + Settings tabs). But FlyoutPage /
// TabbedPage / multi-page hosts are page CHROME outside the shared-XAML dialect (the loader only hosts a
// ContentPage), so the shared tabbed_flyout.xaml — and therefore MAUI's resting capture — DEGRADES to the
// at-rest VISIBLE content composed into one ContentPage: the flyout menu's three buttons + its status
// label, plus the detail's current (Home) tab label. This twin mirrors that degraded shape EXACTLY.
//
// (The FlyoutPage + TabbedPage CONTROLS themselves are exercised by their own unit tests —
// tests/controls/flyout_page_tests.cpp / tabbed_page_tests.cpp — and a real multi-page/Shell render root is
// tracked SEPARATELY from the board as a deliberate capability-expansion; this gallery board page is the
// degraded resting twin, matching MAUI like carousel_page / ios_scroll_view.)
//
// The page OWNS its whole element tree; it is backend-agnostic — a sample main attaches handlers bottom-up
// via the hosting layer and hosts page() in a window.

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

namespace maui::samples
{
    class tabbed_flyout_page
    {
    public:
        tabbed_flyout_page()
        {
            page_.set_title("Tabbed + flyout demo");

            // The degraded resting content (shared tabbed_flyout.xaml): the flyout menu's three buttons + its
            // status label, then the detail's current (Home) tab label — all in one VerticalStackLayout.
            home_button_.set_text("Home tab");
            settings_button_.set_text("Settings tab");
            toggle_button_.set_text("Toggle flyout");
            status_.set_text("Flyout dismissed");
            home_tab_label_.set_text("This is the Home tab.");

            stack_.set_spacing(8);
            stack_.add(home_button_);
            stack_.add(settings_button_);
            stack_.add(toggle_button_);
            stack_.add(status_);
            stack_.add(home_tab_label_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::button home_button_;
        maui::controls::button settings_button_;
        maui::controls::button toggle_button_;
        maui::controls::label status_;
        maui::controls::label home_tab_label_;
    };
} // namespace maui::samples

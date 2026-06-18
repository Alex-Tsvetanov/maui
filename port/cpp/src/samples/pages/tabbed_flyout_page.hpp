#pragma once
// tabbed_flyout_page — a self-contained demo page for the W1-10 tabbed + flyout vertical: a
// flyout_page whose FLYOUT pane is a titled menu (two buttons selecting the detail's tabs + a
// "Toggle flyout" presenting/dismissing itself) and whose DETAIL pane is a tabbed_page with two tabs
// (each a content_page hosting a label). Demonstrates:
//   - multi_page children + CurrentPage (the menu buttons drive set_current_page; the tab chrome's
//     own selection syncs back through i_tabbed_view::on_tab_selected),
//   - the tab-bar styling surface (a tinted selected-tab color),
//   - IsPresented + is_presented_changed (the toggle button + the status label),
//   - the FlyoutPage guards (the flyout carries a Title before it is set).
//
// The page OWNS its whole element tree (the value_controls_page pattern) and is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.

#include <cstdio>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class tabbed_flyout_page
    {
    public:
        tabbed_flyout_page()
        {
            // ---- the detail: a two-tab tabbed_page ----
            first_tab_.set_title("Home");
            first_label_.set_text("This is the Home tab.");
            first_tab_.set_content(first_label_);

            second_tab_.set_title("Settings");
            second_label_.set_text("This is the Settings tab.");
            second_tab_.set_content(second_label_);

            tabs_.set_title("Demo tabs");
            tabs_.add(first_tab_);
            tabs_.add(second_tab_);
            tabs_.set_selected_tab_color(maui::graphics::colors::royal_blue);

            // ---- the flyout: a titled menu page (FlyoutPage requires the Title) ----
            menu_page_.set_title("Menu");
            home_button_.set_text("Home tab");
            home_button_.clicked.connect([this] { tabs_.set_current_page(&first_tab_); });
            settings_button_.set_text("Settings tab");
            settings_button_.clicked.connect([this] { tabs_.set_current_page(&second_tab_); });
            toggle_button_.set_text("Toggle flyout");
            toggle_button_.clicked.connect([this] { page_.set_is_presented(!page_.is_presented()); });

            menu_stack_.set_spacing(8);
            menu_stack_.add(home_button_);
            menu_stack_.add(settings_button_);
            menu_stack_.add(toggle_button_);
            menu_stack_.add(status_);
            menu_page_.set_content(menu_stack_);

            // ---- the flyout page over both ----
            page_.set_title("Tabbed + flyout demo");
            page_.set_flyout(&menu_page_);
            page_.set_detail(&tabs_);
            page_.is_presented_changed.connect([this] { refresh_status(); });
            refresh_status(); // the initial state is platform-dependent (presented on classic macOS)
        }

        [[nodiscard]] maui::controls::flyout_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP through the nested multi-page tree: each tab's
        // label + the tab content_page, then the tabbed_page; the menu buttons + status, the menu_stack
        // and the menu content_page; finally the flyout_page root. Then re-host the tree built in the
        // ctor: each tab page's content, the menu_stack's children, the menu page, the tabbed_page's tabs,
        // and the flyout_page's two panes (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, first_label_, "first_label_");
            gallery_attach_one(app, first_tab_, "first_tab_");
            gallery_attach_one(app, second_label_, "second_label_");
            gallery_attach_one(app, second_tab_, "second_tab_");
            gallery_attach_one(app, tabs_, "tabs_");
            gallery_attach_one(app, home_button_, "home_button_");
            gallery_attach_one(app, settings_button_, "settings_button_");
            gallery_attach_one(app, toggle_button_, "toggle_button_");
            gallery_attach_one(app, status_, "status_");
            gallery_attach_one(app, menu_stack_, "menu_stack_");
            gallery_attach_one(app, menu_page_, "menu_page_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(first_tab_); // each tab content_page hosts its label
            gallery_rehost_content(second_tab_);
            gallery_rehost_layout(menu_stack_); // the menu stack hosts its buttons + status
            gallery_rehost_content(menu_page_); // the menu page hosts the menu stack
            gallery_rehost_pages(tabs_);        // the tabbed_page hosts its two tabs
            gallery_rehost_panes(page_);        // the flyout_page hosts the menu (flyout) + tabs (detail)
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment (and the
        // demo-page tests).
        [[nodiscard]] maui::controls::tabbed_page& tabs()
        {
            return tabs_;
        }
        [[nodiscard]] maui::controls::content_page& menu_page()
        {
            return menu_page_;
        }
        [[nodiscard]] maui::controls::content_page& first_tab()
        {
            return first_tab_;
        }
        [[nodiscard]] maui::controls::content_page& second_tab()
        {
            return second_tab_;
        }
        [[nodiscard]] maui::controls::button& home_button()
        {
            return home_button_;
        }
        [[nodiscard]] maui::controls::button& settings_button()
        {
            return settings_button_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }

    private:
        void refresh_status()
        {
            status_.set_text(std::string{"Flyout "} + (page_.is_presented() ? "presented" : "dismissed"));
        }

        maui::controls::flyout_page page_;
        maui::controls::tabbed_page tabs_;
        maui::controls::content_page first_tab_;
        maui::controls::content_page second_tab_;
        maui::controls::label first_label_;
        maui::controls::label second_label_;
        maui::controls::content_page menu_page_;
        maui::controls::vertical_stack_layout menu_stack_;
        maui::controls::button home_button_;
        maui::controls::button settings_button_;
        maui::controls::button toggle_button_;
        maui::controls::label status_;
    };
} // namespace maui::samples

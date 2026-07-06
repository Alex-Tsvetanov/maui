#pragma once
// maui::samples::table_view_page — the code-first builder twin of the shared page
// port/maui-reference/pages/table_view.xaml (MauiReference.Pages.TableViewPage).
//
// A TableView (Intent=Settings, HasUnevenRows) over a TableRoot with three sections that between them
// exercise the renderable cell family: a TextCell (Text+Detail+TextColor), an EntryCell
// (Label+Placeholder+Text), two SwitchCells (Text+On [+OnColor]), and a ViewCell hosting a small
// HorizontalStackLayout of two Labels. Built bottom-up and OWNED by the page (the collectionview_page
// pattern): the page owns the table_view, and the table_view co-owns its root/sections/cells via
// shared_ptr (the real ownership, unlike the XAML graph's aliasing handles).
//
// Structure-equivalence note: tests/support/view_tree_describe.hpp treats table_view as an opaque leaf
// "view" (it is neither an i_container nor a recognized content host, and cells are elements, not
// i_views), so the describe() tree is content_page("TableView") -> view. The cell tree below is for
// faithful rendering in the gallery, not for the structural gate.

#include <memory>
#include <string>

#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/cells/view_cell.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/table_intent.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class table_view_page
    {
    public:
        table_view_page()
        {
            page_.set_title("TableView");

            table_.set_intent(maui::controls::table_intent::settings);
            table_.set_has_uneven_rows(true);

            auto root = std::make_shared<maui::controls::table_root>();

            // Section 1 — Profile: a TextCell + an EntryCell.
            auto profile = std::make_shared<maui::controls::table_section>("Profile");
            auto account = std::make_shared<maui::controls::text_cell>();
            account->set_text("Account");
            account->set_detail("alex@example.com");
            account->set_text_color(maui::graphics::colors::navy);
            profile->add(account);
            auto name = std::make_shared<maui::controls::entry_cell>();
            name->set_label("Name");
            name->set_placeholder("Enter your name");
            name->set_text("Alex");
            profile->add(name);
            root->add(profile);

            // Section 2 — Preferences: two SwitchCells.
            auto preferences = std::make_shared<maui::controls::table_section>("Preferences");
            auto notifications = std::make_shared<maui::controls::switch_cell>();
            notifications->set_text("Notifications");
            notifications->set_on(true);
            preferences->add(notifications);
            auto dark_mode = std::make_shared<maui::controls::switch_cell>();
            dark_mode->set_text("Dark mode");
            dark_mode->set_on(false);
            dark_mode->set_on_color(maui::graphics::colors::medium_purple);
            preferences->add(dark_mode);
            root->add(preferences);

            // Section 3 — Custom: a ViewCell hosting a HorizontalStackLayout of two Labels.
            auto custom = std::make_shared<maui::controls::table_section>("Custom");
            auto view_cell = std::make_shared<maui::controls::view_cell>();
            auto row = std::make_shared<maui::controls::horizontal_stack_layout>();
            row->set_spacing(8);
            row->set_padding(maui::core::thickness(12, 6, 12, 6));
            auto status = std::make_shared<maui::controls::label>();
            status->set_text("Status");
            status->set_text_color(maui::graphics::colors::gray);
            row->add(*status);
            auto active = std::make_shared<maui::controls::label>();
            active->set_text("Active");
            active->set_text_color(maui::graphics::colors::green);
            row->add(*active);
            status_label_ = status;
            active_label_ = active;
            layout_row_ = row;
            view_cell->set_view(row);
            custom->add(view_cell);
            root->add(custom);

            table_.set_root(root);
            page_.set_content(table_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        [[nodiscard]] maui::controls::table_view& table()
        {
            return table_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::table_view table_;
        // The ViewCell's hosted layout + labels, kept alive by the page (the table co-owns them too, but
        // the page holds explicit handles so they outlive any collection churn — §8 publisher-before).
        std::shared_ptr<maui::controls::horizontal_stack_layout> layout_row_;
        std::shared_ptr<maui::controls::label> status_label_;
        std::shared_ptr<maui::controls::label> active_label_;
    };
} // namespace maui::samples

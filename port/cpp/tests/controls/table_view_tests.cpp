// Tests for the table_view control + its model layer + the headless handler seam. The control half
// ports src/Controls/tests/Core.UnitTests/TableViewUnitTests.cs (constructor / ModelChanged /
// BindingsContextChainsToModel / ParentsViewCells / ParentsAddedViewCells) plus the Cell parent-driven
// RenderHeight + ForceUpdateSize cases from CellTests.cs (which use a ListView parent in C# — exercised
// here through the table_view, since ListView is deferred → CollectionView). The seam half drives the
// headless row-realization simulator (realize / reuse / selection — the UITableView/NSTableView twins
// cover the real natives on their presets).

#include "maui/controls/table_view.hpp"

#include <memory>
#include <string>

#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/image_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/cells/view_cell.hpp"
#include "maui/controls/file_image_source.hpp" // image_source::from_file
#include "maui/controls/label.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_view_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::cell_content_kind;
    using maui::controls::entry_cell;
    using maui::controls::image_cell;
    using maui::controls::image_source;
    using maui::controls::label;
    using maui::controls::switch_cell;
    using maui::controls::table_root;
    using maui::controls::table_row_event_kind;
    using maui::controls::table_section;
    using maui::controls::table_view;
    using maui::controls::table_view_handler;
    using maui::controls::text_cell;
    using maui::controls::view_cell;

    // ---- the control + model (TableViewUnitTests.cs) ----

    TEST(table_view_, constructor_has_empty_root) // TestConstructor (layout-options part skipped — see hdr)
    {
        table_view table;
        ASSERT_NE(table.root(), nullptr);
        EXPECT_EQ(table.root()->count(), 0U);
    }

    TEST(table_view_, model_changed_on_root_set) // TestModelChanged
    {
        table_view table;
        bool changed = false;
        table.model_changed.connect([&changed] { changed = true; });
        table.set_root(std::make_shared<table_root>("NewRoot"));
        EXPECT_TRUE(changed);
    }

    TEST(table_view_, binding_context_chains_to_model) // BindingsContextChainsToModel
    {
        auto context = std::make_shared<std::string>("Context");

        table_view table;
        table.set_binding_context(context);
        table.set_root(std::make_shared<table_root>());
        EXPECT_EQ(table.root()->binding_context<std::string>(), context);

        // Reverse assignment order.
        table_view table2;
        table2.set_root(std::make_shared<table_root>());
        table2.set_binding_context(context);
        EXPECT_EQ(table2.root()->binding_context<std::string>(), context);
    }

    TEST(table_view_, parents_view_cells) // ParentsViewCells
    {
        auto view_cell_ptr = std::make_shared<view_cell>();
        view_cell_ptr->set_view(std::make_shared<label>());

        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>();
        section->add(view_cell_ptr);
        root->add(section);

        table_view table;
        table.set_root(root);

        EXPECT_EQ(view_cell_ptr->logical_parent(), &table);
        EXPECT_EQ(view_cell_ptr->view()->logical_parent(), view_cell_ptr.get());
    }

    TEST(table_view_, parents_added_view_cells) // ParentsAddedViewCells
    {
        auto section = std::make_shared<table_section>();
        auto root = std::make_shared<table_root>();
        root->add(section);

        table_view table;
        table.set_root(root);

        auto view_cell_ptr = std::make_shared<view_cell>();
        view_cell_ptr->set_view(std::make_shared<label>());
        section->add(view_cell_ptr); // added AFTER the root is hosted

        EXPECT_EQ(view_cell_ptr->logical_parent(), &table);
        EXPECT_EQ(view_cell_ptr->view()->logical_parent(), view_cell_ptr.get());
    }

    TEST(table_view_, section_title_change_reruns_model)
    {
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>("Old");
        root->add(section);
        table_view table;
        table.set_root(root);

        bool changed = false;
        table.model_changed.connect([&changed] { changed = true; });
        section->set_title("New");
        EXPECT_TRUE(changed);
    }

    // ---- the model surface (sections / rows / cell / section title) ----

    TEST(table_view_, model_reports_structure)
    {
        auto root = std::make_shared<table_root>();
        auto s0 = std::make_shared<table_section>("S0");
        auto cell0 = std::make_shared<text_cell>();
        cell0->set_text("c0");
        s0->add(cell0);
        s0->add(std::make_shared<text_cell>());
        auto s1 = std::make_shared<table_section>("S1");
        s1->add(std::make_shared<text_cell>());
        root->add(s0);
        root->add(s1);

        table_view table;
        table.set_root(root);
        auto* model = table.model();
        ASSERT_NE(model, nullptr);
        EXPECT_EQ(model->get_section_count(), 2);
        EXPECT_EQ(model->get_row_count(0), 2);
        EXPECT_EQ(model->get_row_count(1), 1);
        EXPECT_EQ(model->get_section_title(0), "S0");
        EXPECT_EQ(model->get_section_title(1), "S1");
        EXPECT_EQ(model->get_cell(0, 0), cell0);
        EXPECT_EQ(model->get_cell(5, 0), nullptr); // out of range
    }

    TEST(table_view_, row_selected_taps_cell)
    {
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>();
        auto cell0 = std::make_shared<text_cell>();
        bool tapped = false;
        cell0->tapped.connect([&tapped] { tapped = true; });
        section->add(cell0);
        root->add(section);

        table_view table;
        table.set_root(root);
        table.model()->row_selected(0, 0);
        EXPECT_TRUE(tapped);
    }

    // ---- Cell.RenderHeight + ForceUpdateSize via the table parent (CellTests.cs equivalents) ----

    TEST(cell_render_height, reads_row_height_from_table_parent) // RenderHeightINPCFromParent equivalent
    {
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>();
        auto c = std::make_shared<text_cell>();
        section->add(c);
        root->add(section);

        table_view table;
        table.set_root(root);

        // No uneven rows: RenderHeight = the table's RowHeight (Cell.RenderHeight).
        table.set_row_height(5);
        EXPECT_DOUBLE_EQ(c->render_height(), 5.0);
    }

    TEST(cell_render_height, uneven_rows_uses_cell_height)
    {
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>();
        auto c = std::make_shared<text_cell>();
        c->set_height(99);
        section->add(c);
        root->add(section);

        table_view table;
        table.set_has_uneven_rows(true);
        table.set_root(root);
        table.set_row_height(5);
        // HasUnevenRows && Height>0 → the cell's own Height.
        EXPECT_DOUBLE_EQ(c->render_height(), 99.0);
    }

    TEST(cell_force_update, gated_on_uneven_rows)
    {
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>();
        auto c = std::make_shared<view_cell>();
        section->add(c);
        root->add(section);

        table_view table;
        table.set_root(root);

        int calls = 0;
        c->force_update_size_requested.connect([&calls] { ++calls; });

        // Even rows: ForceUpdateSize is a no-op (Cell.ForceUpdateSize gates on HasUnevenRows).
        c->force_update_size();
        EXPECT_EQ(calls, 0);

        table.set_has_uneven_rows(true);
        c->force_update_size();
        EXPECT_EQ(calls, 1); // now it fires
    }

    // ---- the handler seam (realize / reuse / select) ----

    // table_view is non-copyable/non-movable (a view<>), so populate one held by shared_ptr.
    [[nodiscard]] std::shared_ptr<table_view> make_populated(int sections, int rows_per_section)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        for (int s = 0; s < sections; ++s)
        {
            auto section = std::make_shared<table_section>();
            for (int r = 0; r < rows_per_section; ++r)
            {
                auto c = std::make_shared<text_cell>();
                c->set_text("s" + std::to_string(s) + "r" + std::to_string(r));
                section->add(c);
            }
            root->add(section);
        }
        table->set_root(root);
        return table;
    }

    TEST(table_view_seam, connect_realizes_all_rows)
    {
        auto table = make_populated(2, 3);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->realized.size(), 6U); // 2 sections x 3 rows
        EXPECT_EQ(platform->realized.front().text, "s0r0");
        EXPECT_EQ(platform->realized.back().text, "s1r2");
    }

    TEST(table_view_seam, maps_row_metrics)
    {
        auto table = make_populated(1, 1);
        table->set_row_height(44);
        table->set_has_uneven_rows(true);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->row_height, 44);
        EXPECT_TRUE(platform->has_uneven_rows);
    }

    TEST(table_view_seam, reload_re_realizes_all_rows)
    {
        auto table = make_populated(1, 2);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler); // initial realize (2 fresh)

        auto* platform = handler->typed_platform_view();
        platform->events.clear();

        handler->reload(); // a second pass re-realizes every row

        EXPECT_EQ(platform->realized.size(), 2U);
#if !defined(MAUI_PLATFORM_APPLE) && !defined(MAUI_PLATFORM_IOS)
        // The headless simulator's reuse pool is DETERMINISTIC: the two same-type rows recycled by the
        // first reload's PrepareForReuse are dequeued, so both events are `reused`. (Native reuse is real
        // but window/scroll-timing dependent — asserted leniently on the native seam tests.)
        int reused = 0;
        for (const auto& ev : platform->events)
        {
            if (ev.kind == table_row_event_kind::reused)
            {
                ++reused;
            }
        }
        EXPECT_EQ(reused, 2);
#endif
    }

    TEST(table_view_seam, select_records_and_taps_cell)
    {
        auto table = make_populated(1, 2);
        // tap-observe the second cell
        auto cell1 = table->root()->at(0)->at(1);
        bool tapped = false;
        cell1->tapped.connect([&tapped] { tapped = true; });

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        handler->simulate_select(0, 1);
        auto* platform = handler->typed_platform_view();
        // Whole-optional compare (avoids unchecked optional deref): the selected path is exactly {0,1}.
        EXPECT_EQ(platform->selected_path, (maui::controls::table_row_path{.section = 0, .row = 1}));
        EXPECT_TRUE(tapped); // selection routes through the model → cell.OnTapped
    }

    // ---- rich per-cell-type content (switch / entry / image) + section group rows ----

    // A table with a switch, entry, image, and text cell in one titled section.
    [[nodiscard]] std::shared_ptr<table_view> make_rich_table()
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>("Settings");

        auto sw = std::make_shared<switch_cell>();
        sw->set_text("Wi-Fi");
        sw->set_on(true);
        section->add(sw);

        auto entry = std::make_shared<entry_cell>();
        entry->set_label("Name");
        entry->set_text("Ada");
        entry->set_placeholder("enter name");
        section->add(entry);

        auto image = std::make_shared<image_cell>();
        image->set_text("Avatar");
        image->set_detail("profile");
        image->set_image_source(image_source::from_file("/tmp/avatar.png"));
        section->add(image);

        auto text = std::make_shared<text_cell>();
        text->set_text("About");
        text->set_detail("v1.0");
        section->add(text);

        root->add(section);
        table->set_root(root);
        return table;
    }

    TEST(table_view_seam, switch_cell_realizes_toggle_content)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& rows = handler->typed_platform_view()->realized;
        ASSERT_GE(rows.size(), 1U);
        EXPECT_EQ(rows[0].content, cell_content_kind::toggle);
        EXPECT_EQ(rows[0].text, "Wi-Fi");
        EXPECT_TRUE(rows[0].toggle_on);
    }

    TEST(table_view_seam, entry_cell_realizes_entry_content)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& rows = handler->typed_platform_view()->realized;
        ASSERT_GE(rows.size(), 2U);
        EXPECT_EQ(rows[1].content, cell_content_kind::entry);
        EXPECT_EQ(rows[1].text, "Name"); // the label rides the primary text
        EXPECT_EQ(rows[1].entry_text, "Ada");
        EXPECT_EQ(rows[1].entry_placeholder, "enter name");
    }

    TEST(table_view_seam, image_cell_realizes_image_content)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& rows = handler->typed_platform_view()->realized;
        ASSERT_GE(rows.size(), 3U);
        EXPECT_EQ(rows[2].content, cell_content_kind::image);
        EXPECT_EQ(rows[2].text, "Avatar");
        EXPECT_EQ(rows[2].detail, "profile");
        EXPECT_TRUE(rows[2].has_image); // a non-empty ImageSource → an image view is populated
    }

    TEST(table_view_seam, text_cell_realizes_text_with_detail)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& rows = handler->typed_platform_view()->realized;
        ASSERT_GE(rows.size(), 4U);
        EXPECT_EQ(rows[3].content, cell_content_kind::text);
        EXPECT_EQ(rows[3].text, "About");
        EXPECT_EQ(rows[3].detail, "v1.0");
    }

    TEST(table_view_seam, section_headers_are_realized)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        auto s0 = std::make_shared<table_section>("Alpha");
        s0->add(std::make_shared<text_cell>());
        auto s1 = std::make_shared<table_section>("Beta");
        s1->add(std::make_shared<text_cell>());
        s1->add(std::make_shared<text_cell>());
        root->add(s0);
        root->add(s1);
        table->set_root(root);

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& headers = handler->typed_platform_view()->section_headers;
        ASSERT_EQ(headers.size(), 2U);
        EXPECT_EQ(headers[0].section, 0);
        EXPECT_EQ(headers[0].title, "Alpha");
        EXPECT_EQ(headers[1].section, 1);
        EXPECT_EQ(headers[1].title, "Beta");
    }

    TEST(table_view_seam, empty_sections_produce_no_header)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        root->add(std::make_shared<table_section>("Empty")); // no rows → no header
        auto s1 = std::make_shared<table_section>("HasRows");
        s1->add(std::make_shared<text_cell>());
        root->add(s1);
        table->set_root(root);

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& headers = handler->typed_platform_view()->section_headers;
        ASSERT_EQ(headers.size(), 1U);
        EXPECT_EQ(headers[0].title, "HasRows");
    }

    TEST(table_view_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<maui::core::i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<table_view>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<table_view_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        auto table = make_populated(1, 1);
        table->set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->realized.size(), 1U);
    }

    TEST(table_view_seam, clearing_handler_disconnects)
    {
        auto table = make_populated(1, 1);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        table->set_handler(nullptr);
        EXPECT_EQ(table->handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

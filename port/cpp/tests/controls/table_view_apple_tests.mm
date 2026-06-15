// table_view_handler — macOS (AppKit) seam tests: a real NSTableView realizes rows through the
// datasource/delegate (viewForTableColumn dequeues by reuse id), and the selection path routes back
// through the model to tap the cell. Asserts the same realize/reuse/selection oracle as the headless
// simulator, but on the genuine native table. Obj-C++ with ARC (the apple test exe).

#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/image_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_view.hpp"
#include "maui/controls/table_view_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::entry_cell;
    using maui::controls::image_cell;
    using maui::controls::image_source;
    using maui::controls::switch_cell;
    using maui::controls::table_root;
    using maui::controls::table_section;
    using maui::controls::table_view;
    using maui::controls::table_view_handler;
    using maui::controls::text_cell;

    NSTableView* native_table(const std::shared_ptr<table_view_handler>& handler)
    {
        NSScrollView* const scroll = (__bridge NSScrollView*)handler->typed_platform_view()->native;
        return scroll.documentView;
    }

    [[nodiscard]] std::shared_ptr<table_view> make_populated(int sections, int rows_per_section)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        for (int s = 0; s < sections; ++s)
        {
            auto section = std::make_shared<table_section>("S" + std::to_string(s));
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

    TEST(table_view_apple_seam, native_table_realizes_all_rows)
    {
        auto table = make_populated(2, 3);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->native, nullptr); // a real NSScrollView/NSTableView
        EXPECT_EQ(platform->realized.size(), 6U);
        EXPECT_EQ(platform->realized.front().text, "s0r0");
    }

    TEST(table_view_apple_seam, reload_re_realizes_all_rows)
    {
        auto table = make_populated(1, 3);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        handler->reload(); // a second pass re-realizes every row through the NSTableView datasource
        // The NSTableView view-based reuse queue serves same-identifier views WHEN rows scroll
        // offscreen / the table is window-hosted — not deterministic for an off-window unit table, so
        // the realization count is asserted here and the deterministic reuse pool is the headless
        // simulator's guarantee (table_view_tests.cpp).
        EXPECT_EQ(platform->realized.size(), 3U);
    }

    TEST(table_view_apple_seam, selection_taps_cell)
    {
        auto table = make_populated(1, 2);
        auto cell1 = table->root()->at(0)->at(1);
        bool tapped = false;
        cell1->tapped.connect([&tapped] { tapped = true; });

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        // The header-aware flat layout for 1 section x 2 rows is [header@0, cell(0,0)@1, cell(0,1)@2], so
        // flat index 2 selects section 0, row 1.
        auto* platform = handler->typed_platform_view();
        NSTableView* const native = native_table(handler);
        [native selectRowIndexes:[NSIndexSet indexSetWithIndex:2] byExtendingSelection:NO];

        ASSERT_TRUE(platform->selected_path.has_value());
        EXPECT_EQ(platform->selected_path->section, 0);
        EXPECT_EQ(platform->selected_path->row, 1);
        EXPECT_TRUE(tapped);
    }

    TEST(table_view_apple_seam, section_header_is_a_non_selectable_group_row)
    {
        auto table = make_populated(1, 2);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        NSTableView* const native = native_table(handler);
        // [header@0, cell@1, cell@2] — 3 native rows; the section header is recorded + is a group row.
        EXPECT_EQ(native.numberOfRows, 3);
        EXPECT_TRUE([(id<NSTableViewDelegate>)native.delegate tableView:native isGroupRow:0]);
        ASSERT_EQ(platform->section_headers.size(), 1U);
        EXPECT_EQ(platform->section_headers[0].title, "S0");

        // Selecting the header row is a no-op (it is not selectable) — no selection is recorded.
        [native selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
        EXPECT_FALSE(platform->selected_path.has_value());
    }

    TEST(table_view_apple_seam, rich_cells_build_native_subcontrols)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>("Mix");
        auto sw = std::make_shared<switch_cell>();
        sw->set_text("Wi-Fi");
        sw->set_on(true);
        section->add(sw);
        auto entry = std::make_shared<entry_cell>();
        entry->set_label("Name");
        entry->set_text("Ada");
        section->add(entry);
        auto image = std::make_shared<image_cell>();
        image->set_text("Avatar");
        image->set_image_source(image_source::from_file("/tmp/a.png"));
        section->add(image);
        root->add(section);
        table->set_root(root);

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        const auto& rows = handler->typed_platform_view()->realized;
        ASSERT_EQ(rows.size(), 3U);
        EXPECT_EQ(rows[0].content, maui::controls::cell_content_kind::toggle);
        EXPECT_TRUE(rows[0].toggle_on);
        EXPECT_EQ(rows[1].content, maui::controls::cell_content_kind::entry);
        EXPECT_EQ(rows[1].entry_text, "Ada");
        EXPECT_EQ(rows[2].content, maui::controls::cell_content_kind::image);
        EXPECT_TRUE(rows[2].has_image);

        // The native row views carry the embedded sub-controls (flat indices 1,2,3 after the header@0).
        NSTableView* const native = native_table(handler);
        NSView* const switchRow = [native viewAtColumn:0 row:1 makeIfNecessary:YES];
        EXPECT_NE([switchRow viewWithTag:1002], nil); // the embedded NSSwitch
        NSView* const entryRow = [native viewAtColumn:0 row:2 makeIfNecessary:YES];
        EXPECT_NE([entryRow viewWithTag:1003], nil); // the embedded NSTextField
        NSView* const imageRow = [native viewAtColumn:0 row:3 makeIfNecessary:YES];
        EXPECT_NE([imageRow viewWithTag:1004], nil); // the embedded NSImageView
    }

    TEST(table_view_apple_seam, row_height_maps_to_native)
    {
        auto table = make_populated(1, 1);
        table->set_row_height(55);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        NSScrollView* const scroll = (__bridge NSScrollView*)platform->native;
        NSTableView* const native = scroll.documentView;
        EXPECT_DOUBLE_EQ(native.rowHeight, 55.0);
    }
} // namespace

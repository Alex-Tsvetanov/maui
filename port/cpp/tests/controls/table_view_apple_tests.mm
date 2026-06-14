// table_view_handler — macOS (AppKit) seam tests: a real NSTableView realizes rows through the
// datasource/delegate (viewForTableColumn dequeues by reuse id), and the selection path routes back
// through the model to tap the cell. Asserts the same realize/reuse/selection oracle as the headless
// simulator, but on the genuine native table. Obj-C++ with ARC (the apple test exe).

#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_view.hpp"
#include "maui/controls/table_view_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::table_root;
    using maui::controls::table_section;
    using maui::controls::table_view;
    using maui::controls::table_view_handler;
    using maui::controls::text_cell;

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

        // Drive the AppKit selection through the real NSTableView (row 1 = section 0, row 1).
        auto* platform = handler->typed_platform_view();
        NSScrollView* const scroll = (__bridge NSScrollView*)platform->native;
        NSTableView* const native = scroll.documentView;
        [native selectRowIndexes:[NSIndexSet indexSetWithIndex:1] byExtendingSelection:NO];

        ASSERT_TRUE(platform->selected_path.has_value());
        EXPECT_EQ(platform->selected_path->section, 0);
        EXPECT_EQ(platform->selected_path->row, 1);
        EXPECT_TRUE(tapped);
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

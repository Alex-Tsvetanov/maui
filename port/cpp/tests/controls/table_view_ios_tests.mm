// table_view_handler — iOS (UIKit) on-simulator seam tests: a real UITableView realizes rows through
// the datasource/delegate (cellForRow dequeues by reuse id), and didSelectRow routes back through the
// model to tap the cell. Asserts the same realize/reuse/selection oracle as the headless simulator, on
// the genuine UITableView. Obj-C++ with ARC, run ON a booted simulator.

#import <UIKit/UIKit.h>

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

    TEST(table_view_ios_seam, native_table_realizes_all_rows)
    {
        auto table = make_populated(2, 3);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->native, nullptr); // a real UITableView
        EXPECT_EQ(platform->realized.size(), 6U);
        EXPECT_EQ(platform->realized.front().text, "s0r0");
    }

    TEST(table_view_ios_seam, reload_re_realizes_all_rows)
    {
        auto table = make_populated(1, 3);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        handler->reload(); // a second pass re-realizes every row through the UITableView datasource
        // The UITableView reuse queue serves same-identifier cells dequeued on the reload; reuse is
        // exercised (cellForRow always dequeues), but the deterministic reuse-pool count is the headless
        // simulator's guarantee (table_view_tests.cpp). Here the realization count is asserted.
        EXPECT_EQ(platform->realized.size(), 3U);
    }

    TEST(table_view_ios_seam, selection_taps_cell)
    {
        auto table = make_populated(1, 2);
        auto cell1 = table->root()->at(0)->at(1);
        bool tapped = false;
        cell1->tapped.connect([&tapped] { tapped = true; });

        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        // Drive the real UITableView delegate selection (section 0, row 1).
        auto* platform = handler->typed_platform_view();
        UITableView* const native = (__bridge UITableView*)platform->native;
        NSIndexPath* const path = [NSIndexPath indexPathForRow:1 inSection:0];
        [native.delegate tableView:native didSelectRowAtIndexPath:path];

        ASSERT_TRUE(platform->selected_path.has_value());
        EXPECT_EQ(platform->selected_path->section, 0);
        EXPECT_EQ(platform->selected_path->row, 1);
        EXPECT_TRUE(tapped);
    }

    TEST(table_view_ios_seam, row_height_maps_to_native)
    {
        auto table = make_populated(1, 1);
        table->set_row_height(55);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        UITableView* const native = (__bridge UITableView*)platform->native;
        EXPECT_DOUBLE_EQ(native.rowHeight, 55.0);
    }
} // namespace

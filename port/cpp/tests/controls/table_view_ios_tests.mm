// table_view_handler — iOS (UIKit) on-simulator seam tests: a real UITableView realizes rows through
// the datasource/delegate (cellForRow dequeues by reuse id), and didSelectRow routes back through the
// model to tap the cell. Asserts the same realize/reuse/selection oracle as the headless simulator, on
// the genuine UITableView. Obj-C++ with ARC, run ON a booted simulator.

#import <UIKit/UIKit.h>

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

    UITableViewCell* cell_at(const std::shared_ptr<table_view_handler>& handler, int section, int row)
    {
        UITableView* const native = (__bridge UITableView*)handler->typed_platform_view()->native;
        NSIndexPath* const path = [NSIndexPath indexPathForRow:row inSection:section];
        return [native.dataSource tableView:native cellForRowAtIndexPath:path];
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

    // ---- rich per-cell-type content + native section headers ----

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
        image->set_image_source(image_source::from_file("/tmp/a.png"));
        section->add(image);
        root->add(section);
        table->set_root(root);
        return table;
    }

    TEST(table_view_ios_seam, switch_cell_has_a_uiswitch_accessory)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        UITableViewCell* const cell = cell_at(handler, 0, 0);
        ASSERT_TRUE([cell.accessoryView isKindOfClass:[UISwitch class]]);
        EXPECT_TRUE(((UISwitch*)cell.accessoryView).on);       // bound to switch_cell.On
        EXPECT_EQ(cell.accessoryView.tag, 1002);               // tagged so reuse can rebind via viewWithTag:
        EXPECT_NE([cell.accessoryView viewWithTag:1002], nil); // the rebind hook the reuse path uses
        // The primary text is asserted through the cross-platform realized mirror (UITableViewCell.textLabel
        // is deprecated/unreliable on iOS 14+ — the C# renderer flags the same).
        EXPECT_EQ(handler->typed_platform_view()->realized[0].text, "Wi-Fi");
    }

    TEST(table_view_ios_seam, entry_cell_has_a_uitextfield_accessory)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        UITableViewCell* const cell = cell_at(handler, 0, 1);
        ASSERT_TRUE([cell.accessoryView isKindOfClass:[UITextField class]]);
        UITextField* const field = (UITextField*)cell.accessoryView;
        EXPECT_TRUE([field.text isEqualToString:@"Ada"]);
        EXPECT_TRUE([field.placeholder isEqualToString:@"enter name"]);
        EXPECT_EQ(cell.accessoryView.tag, 1003); // tagged so reuse can rebind via viewWithTag:
        EXPECT_EQ(handler->typed_platform_view()->realized[1].text, "Name"); // the label rides the primary text
    }

    TEST(table_view_ios_seam, image_cell_populates_the_image_view)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        UITableViewCell* const cell = cell_at(handler, 0, 2);
        EXPECT_NE(cell.imageView.image, nil); // a resolved ImageSource → a populated image view
        EXPECT_EQ(handler->typed_platform_view()->realized[2].text, "Avatar");
    }

    TEST(table_view_ios_seam, section_header_renders_natively)
    {
        auto table = make_rich_table();
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        auto* platform = handler->typed_platform_view();
        UITableView* const native = (__bridge UITableView*)platform->native;
        // The grouped UITableView returns the section title for its native header.
        NSString* const title = [native.dataSource tableView:native titleForHeaderInSection:0];
        EXPECT_TRUE([title isEqualToString:@"Settings"]);
        // And the mirror records it for the cross-backend oracle.
        ASSERT_EQ(platform->section_headers.size(), 1U);
        EXPECT_EQ(platform->section_headers[0].title, "Settings");
    }

    // ---- cell reuse (W8-56 #11) ----

    UIWindow* make_host_window()
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init]; // SDK-deprecated; see window_handler.mm precedent
#pragma clang diagnostic pop
        return window;
    }

    // A table of N switch cells (all the same type-keyed reuse id), each with a distinct on/off value.
    [[nodiscard]] std::shared_ptr<table_view> make_switch_table(int rows)
    {
        auto table = std::make_shared<table_view>();
        auto root = std::make_shared<table_root>();
        auto section = std::make_shared<table_section>("Toggles");
        for (int i = 0; i < rows; ++i)
        {
            auto sw = std::make_shared<switch_cell>();
            sw->set_text("row" + std::to_string(i));
            sw->set_on(i % 2 == 0); // alternating, so a misbound reuse is visible
            section->add(sw);
        }
        root->add(section);
        table->set_root(root);
        return table;
    }

    // W8-56 regression (#11): a reused switch cell must KEEP its accessory object and just REBIND the value,
    // not rebuild a fresh UISwitch every time (the bug). Mount in a small window so only a few rows fit, then
    // scroll the whole source under the recycler: the count of DISTINCT UISwitch instances ever vended stays
    // far below the row count (recycling happened), and every visible switch's `on` matches its rebound model
    // value (rebind-on-reuse is correct). Run on a booted simulator.
    TEST(table_view_ios_seam, switch_cell_reuses_accessory_instance_and_rebinds)
    {
        constexpr int rows = 40;
        auto table = make_switch_table(rows);
        auto handler = std::make_shared<table_view_handler>();
        table->set_handler(handler);

        UITableView* const native = (__bridge UITableView*)handler->typed_platform_view()->native;
        UIWindow* const window = make_host_window();
        native.frame = CGRectMake(0, 0, 320, 120); // ~2-3 rows visible → the recycler must reuse
        [window addSubview:native];
        [window makeKeyAndVisible];

        NSMutableSet<NSValue*>* const switch_instances = [NSMutableSet set];
        const auto sweep = [&] {
            for (NSUInteger r = 0; r < rows; ++r)
            {
                NSIndexPath* const path = [NSIndexPath indexPathForRow:static_cast<NSInteger>(r) inSection:0];
                [native scrollToRowAtIndexPath:path atScrollPosition:UITableViewScrollPositionTop animated:NO];
                [native layoutIfNeeded];
                for (UITableViewCell* const cell in native.visibleCells)
                {
                    NSIndexPath* const cellPath = [native indexPathForCell:cell];
                    if (cellPath == nil)
                    {
                        continue;
                    }
                    auto* sw = (UISwitch*)[cell.accessoryView viewWithTag:1002];
                    ASSERT_TRUE([sw isKindOfClass:[UISwitch class]]); // every switch row still has its accessory
                    [switch_instances addObject:[NSValue valueWithNonretainedObject:sw]];
                    EXPECT_EQ(sw.on, cellPath.row % 2 == 0); // rebound to THIS row's model value, not stale
                }
            }
        };
        sweep();
        sweep(); // a second pass guarantees recycling pressure

        // Recycling happened: far fewer distinct UISwitch objects than rows (a no-reuse build would vend a
        // fresh switch per realize). A generous bound keeps the test robust across UIKit layout differences.
        EXPECT_LT(switch_instances.count, static_cast<NSUInteger>(rows));
        (void)window;
    }
} // namespace

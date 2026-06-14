// Tests for the cell family (maui::controls::cell + text_cell / switch_cell / entry_cell / image_cell /
// view_cell). Ported from src/Controls/tests/Core.UnitTests/{CellTests,TextCellTests,SwitchCellTests,
// EntryCellTests,ViewCellTests}.cs. The cases that depend on ListView (RenderHeightINPCFromParent,
// ForceUpdateSize* via a ListView parent) are exercised through the table_view in table_view_tests.cpp
// instead (ListView is deferred → CollectionView). The ICommand-CanExecute cases are modeled through the
// port's command-as-callable + command_can_execute predicate (the button/check_box convention).

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/cells/entry_cell.hpp"
#include "maui/controls/cells/image_cell.hpp"
#include "maui/controls/cells/switch_cell.hpp"
#include "maui/controls/cells/text_cell.hpp"
#include "maui/controls/cells/view_cell.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/core/text_alignment.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::cell;
    using maui::controls::entry_cell;
    using maui::controls::image_cell;
    using maui::controls::label;
    using maui::controls::menu_item;
    using maui::controls::switch_cell;
    using maui::controls::text_cell;
    using maui::controls::view_cell;

    // A test cell mirroring CellTests.TestCell (records OnAppearing/OnDisappearing).
    class test_cell : public cell
    {
    public:
        bool on_appearing_sent = false;
        bool on_disappearing_sent = false;

    protected:
        void on_appearing() override
        {
            cell::on_appearing();
            on_appearing_sent = true;
        }
        void on_disappearing() override
        {
            cell::on_disappearing();
            on_disappearing_sent = true;
        }
    };

    // ---- Cell base (CellTests.cs) ----

    TEST(cell, tapped_event_fires_on_tapped) // CellTests.Selected
    {
        test_cell c;
        bool tapped = false;
        c.tapped.connect([&tapped] { tapped = true; });
        c.on_tapped();
        EXPECT_TRUE(tapped);
    }

    TEST(cell, appearing_event) // CellTests.AppearingEvent
    {
        test_cell c;
        bool emitted = false;
        c.appearing.connect([&emitted] { emitted = true; });
        c.send_appearing();
        EXPECT_TRUE(emitted);
        EXPECT_TRUE(c.on_appearing_sent);
        EXPECT_FALSE(c.on_disappearing_sent);
    }

    TEST(cell, disappearing_event) // CellTests.DisappearingEvent
    {
        test_cell c;
        bool emitted = false;
        c.disappearing.connect([&emitted] { emitted = true; });
        c.send_disappearing();
        EXPECT_TRUE(emitted);
        EXPECT_FALSE(c.on_appearing_sent);
        EXPECT_TRUE(c.on_disappearing_sent);
    }

    TEST(cell, default_is_enabled_true)
    {
        text_cell c;
        EXPECT_TRUE(c.is_enabled());
    }

    TEST(cell, has_context_actions) // CellTests.HasContextActions
    {
        text_cell c;
        bool changed = false;
        c.property_changed.connect([&changed](std::string_view name) {
            if (name == "has_context_actions")
            {
                changed = true;
            }
        });

        EXPECT_FALSE(c.has_context_actions());
        EXPECT_FALSE(changed);

        c.add_context_action(std::make_shared<menu_item>());
        EXPECT_TRUE(c.has_context_actions());
        EXPECT_TRUE(changed);
    }

    TEST(cell, has_context_actions_false_when_disabled)
    {
        text_cell c;
        c.add_context_action(std::make_shared<menu_item>());
        EXPECT_TRUE(c.has_context_actions());
        c.set_is_enabled(false);
        EXPECT_FALSE(c.has_context_actions()); // HasContextActions => ... && IsEnabled
    }

    TEST(cell, menu_items_get_binding_context) // CellTests.MenuItemsGetBindingContext
    {
        auto bc = std::make_shared<std::string>("ctx");
        text_cell c;
        auto item = std::make_shared<menu_item>();
        c.add_context_action(item);
        c.set_binding_context(bc);
        EXPECT_EQ(item->binding_context<std::string>(), bc);

        // Reverse order: set context first, then add the item.
        text_cell c2;
        c2.set_binding_context(bc);
        auto item2 = std::make_shared<menu_item>();
        c2.add_context_action(item2);
        EXPECT_EQ(item2->binding_context<std::string>(), bc);
    }

    TEST(cell, default_render_height_with_no_parent)
    {
        text_cell c;
        EXPECT_DOUBLE_EQ(c.render_height(), static_cast<double>(cell::default_cell_height));
    }

    // ---- TextCell (TextCellTests.cs) ----

    TEST(text_cell_, tapped_fires) // TextCellTests.TestTapped
    {
        text_cell c;
        bool tapped = false;
        c.tapped.connect([&tapped] { tapped = true; });
        c.on_tapped();
        EXPECT_TRUE(tapped);
    }

    TEST(text_cell_, command_executes_on_tapped) // TextCellTests.TestCommand
    {
        bool executed = false;
        text_cell c;
        c.set_command([&executed] { executed = true; });
        c.on_tapped();
        EXPECT_TRUE(executed);
    }

    TEST(text_cell_, tapped_honors_can_execute_true) // TextCellTests.TappedHonorsCanExecute(true)
    {
        bool executed = false;
        text_cell c;
        c.set_command([&executed] { executed = true; }, [] { return true; });
        c.on_tapped();
        EXPECT_TRUE(executed);
    }

    TEST(text_cell_, tapped_honors_can_execute_false) // TextCellTests.TappedHonorsCanExecute(false)
    {
        bool executed = false;
        text_cell c;
        c.set_command([&executed] { executed = true; }, [] { return false; });
        c.on_tapped();
        EXPECT_FALSE(executed); // CanExecute false disables the cell → OnTapped returns early
    }

    TEST(text_cell_, can_execute_false_disables_cell) // TextCellTests.TestCommandCanExecuteDisables
    {
        text_cell c;
        c.set_command([] {}, [] { return false; });
        EXPECT_FALSE(c.is_enabled());
    }

    TEST(text_cell_, change_can_execute_reenables) // TextCellTests.TestCommandCanExecuteChanged
    {
        bool first = true;
        text_cell c;
        c.set_command([] {},
                      [&first] {
                          if (first)
                          {
                              first = false;
                              return false;
                          }
                          return true;
                      });
        EXPECT_FALSE(c.is_enabled());
        c.refresh_can_execute();
        EXPECT_TRUE(c.is_enabled());
    }

    TEST(text_cell_, text_and_detail) // TextCellTests.Text / Detail
    {
        text_cell c;
        c.set_text("text");
        c.set_detail("detail");
        EXPECT_EQ(c.text(), "text");
        EXPECT_EQ(c.detail(), "detail");
    }

    // ---- SwitchCell (SwitchCellTests.cs) ----

    TEST(switch_cell_, text_and_on) // SwitchCellTemplateTests.Text / On
    {
        switch_cell c;
        c.set_text("text");
        EXPECT_EQ(c.text(), "text");
        EXPECT_FALSE(c.on());
        c.set_on(true);
        EXPECT_TRUE(c.on());
    }

    TEST(switch_cell_, on_changed_args) // SwitchCellTemplateTests.SwitchCellSwitchChangedArgs(false,true)
    {
        switch_cell c;
        c.set_on(false);

        switch_cell* sender = nullptr;
        bool new_value = false;
        // The port raises on_changed with the new value (the sender is the cell — captured here).
        c.on_changed.connect([&](bool value) {
            sender = &c;
            new_value = value;
        });

        c.set_on(true);
        EXPECT_EQ(sender, &c);
        EXPECT_TRUE(new_value);
    }

    TEST(switch_cell_, on_changed_args_true_to_false) // SwitchCellSwitchChangedArgs(true,false)
    {
        switch_cell c;
        c.set_on(true);
        bool new_value = true;
        c.on_changed.connect([&](bool value) { new_value = value; });
        c.set_on(false);
        EXPECT_FALSE(new_value);
    }

    // ---- EntryCell (EntryCellTests.cs) ----

    TEST(entry_cell_, alignment_defaults)
    {
        entry_cell c;
        EXPECT_EQ(c.horizontal_text_alignment(), maui::core::text_alignment::start);
        EXPECT_EQ(c.vertical_text_alignment(), maui::core::text_alignment::center);
    }

    TEST(entry_cell_, properties_round_trip)
    {
        entry_cell c;
        c.set_label("label");
        c.set_placeholder("ph");
        c.set_text("t");
        c.set_horizontal_text_alignment(maui::core::text_alignment::end);
        EXPECT_EQ(c.label(), "label");
        EXPECT_EQ(c.placeholder(), "ph");
        EXPECT_EQ(c.text(), "t");
        EXPECT_EQ(c.horizontal_text_alignment(), maui::core::text_alignment::end);
    }

    TEST(entry_cell_, send_completed_raises)
    {
        entry_cell c;
        bool fired = false;
        c.completed.connect([&fired] { fired = true; });
        c.send_completed();
        EXPECT_TRUE(fired);
    }

    // ---- ImageCell ----

    TEST(image_cell_, is_a_text_cell_with_image_source)
    {
        image_cell c;
        c.set_text("t"); // inherited text_cell surface
        EXPECT_EQ(c.text(), "t");
        auto source = maui::controls::image_source::from_file("a.png");
        c.set_image_source(source);
        EXPECT_EQ(c.image_source(), source);
    }

    // ---- ViewCell (ViewCellTests.cs) ----

    TEST(view_cell_, set_binding_context_before_view) // ViewCellTests.SetBindingContextBeforeView
    {
        auto context = std::make_shared<std::string>("ctx");
        auto view = std::make_shared<label>();
        view_cell c;
        c.set_binding_context(context);
        c.set_view(view);
        EXPECT_EQ(view->binding_context<std::string>(), context);
    }

    TEST(view_cell_, set_view_before_binding_context) // ViewCellTests.SetViewBeforeBindingContext
    {
        auto context = std::make_shared<std::string>("ctx");
        auto view = std::make_shared<label>();
        view_cell c;
        c.set_view(view);
        c.set_binding_context(context);
        EXPECT_EQ(view->binding_context<std::string>(), context);
    }

    TEST(view_cell_, view_is_logical_child)
    {
        auto view = std::make_shared<label>();
        view_cell c;
        EXPECT_EQ(c.view(), nullptr);
        c.set_view(view);
        ASSERT_NE(c.view(), nullptr);
        EXPECT_EQ(c.view(), view);
        EXPECT_EQ(view->logical_parent(), &c);
    }

    TEST(view_cell_, replacing_view_detaches_old)
    {
        auto first = std::make_shared<label>();
        auto second = std::make_shared<label>();
        view_cell c;
        c.set_view(first);
        c.set_view(second);
        EXPECT_EQ(first->logical_parent(), nullptr); // old detached
        EXPECT_EQ(second->logical_parent(), &c);
    }
} // namespace

// Tests for the picker control + its headless handler seam. Ported from
// src/Controls/tests/Core.UnitTests/PickerTests.cs — the string-ItemsSource subset (the port's
// ItemsSource is observable_collection<std::string>; the object-items + ItemDisplayBinding tests are
// out of the W1-06 scope, see picker.hpp). The reentrancy theories hook the port's INPC analog
// (bindable_object::property_changed) exactly where C# hooks PropertyChanged. The seam block drives
// the headless picker_platform (PickerExtensions.UpdatePicker mirrors + the on_done commit); the
// Apple/iOS .mm twins drive the real natives.
#include "maui/controls/picker.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/observable_collection.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/picker_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::observable_collection;
    using maui::controls::picker;
    using maui::core::i_element_handler;
    using maui::core::i_picker;
    using maui::core::picker_handler;

    using string_source = observable_collection<std::string>;

    std::shared_ptr<string_source> make_source(std::vector<std::string> items)
    {
        return std::make_shared<string_source>(std::move(items));
    }

    // ---- SelectedIndex coercion over Items (TestSetSelectedIndexOnNullRows / ...InRange...) ----

    TEST(picker, set_selected_index_on_empty_items_coerces_to_minus_one)
    {
        picker control;
        EXPECT_EQ(control.items().count(), 0U);
        EXPECT_EQ(control.selected_index(), -1);

        control.set_selected_index(2);
        EXPECT_EQ(control.selected_index(), -1);
    }

    TEST(picker, selected_index_clamps_into_items_range)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.items().add("George");
        control.items().add("Ringo");
        control.set_selected_index(2);
        EXPECT_EQ(control.selected_index(), 2);

        control.set_selected_index(42);
        EXPECT_EQ(control.selected_index(), 3);

        control.set_selected_index(-1);
        EXPECT_EQ(control.selected_index(), -1);

        control.set_selected_index(-42);
        EXPECT_EQ(control.selected_index(), -1);
    }

    TEST(picker, selected_index_clamps_from_the_default)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.items().add("George");
        control.items().add("Ringo");
        EXPECT_EQ(control.selected_index(), -1);

        control.set_selected_index(-5);
        EXPECT_EQ(control.selected_index(), -1);

        control.set_selected_index(2);
        EXPECT_EQ(control.selected_index(), 2);

        control.set_selected_index(42);
        EXPECT_EQ(control.selected_index(), 3);

        control.set_selected_index(-1);
        EXPECT_EQ(control.selected_index(), -1);
    }

    TEST(picker, selected_index_follows_a_shrinking_items_collection)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.items().add("George");
        control.items().add("Ringo");
        control.set_selected_index(3);
        EXPECT_EQ(control.selected_index(), 3);

        control.items().remove_at(3);
        control.items().remove_at(2);
        EXPECT_EQ(control.selected_index(), 1);

        control.items().clear();
        EXPECT_EQ(control.selected_index(), -1);
    }

    // ---- SelectedItem over a string ItemsSource ----

    TEST(picker, selected_index_out_of_range_updates_selected_item)
    {
        picker control;
        control.set_items_source(make_source({"Monkey", "Banana", "Lemon"}));
        control.set_selected_index(0);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Monkey");

        control.set_selected_index(42);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Lemon");

        control.set_selected_index(-42);
        EXPECT_FALSE(control.selected_item().has_value());
    }

    TEST(picker, setting_selected_index_updates_selected_item)
    {
        picker control;
        auto source = make_source({"Start", "Center", "End"});
        control.set_items_source(source);
        control.set_selected_item("Start");
        EXPECT_EQ(control.selected_index(), 0);

        control.set_selected_index(1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Center");
    }

    TEST(picker, setting_selected_item_updates_selected_index)
    {
        picker control;
        control.set_items_source(make_source({"John"}));
        EXPECT_EQ(control.selected_index(), -1);
        EXPECT_FALSE(control.selected_item().has_value());

        control.set_selected_item("John");
        EXPECT_EQ(control.selected_index(), 0);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "John");
    }

    // ---- ItemsSource wiring (subscribe / unsubscribe / reset) ----

    TEST(picker, swapping_items_source_unsubscribes_the_old_collection)
    {
        auto list = make_source({});
        picker control;
        control.set_items_source(list);
        EXPECT_EQ(control.items().count(), 0U);

        auto new_list = make_source({});
        control.set_items_source(new_list);
        list->add("item");
        EXPECT_EQ(control.items().count(), 0U);
    }

    TEST(picker, an_empty_items_source_resets_items)
    {
        picker control;
        control.set_items_source(make_source({"John", "George", "Ringo"}));
        EXPECT_EQ(control.items().count(), 3U);

        control.set_items_source(make_source({}));
        EXPECT_EQ(control.items().count(), 0U);
    }

    TEST(picker, clearing_items_source_unlocks_and_clears_items)
    {
        picker control;
        control.set_items_source(make_source({"John", "George", "Ringo"}));
        EXPECT_EQ(control.items().count(), 3U);

        control.set_items_source(nullptr);
        EXPECT_EQ(control.items().count(), 0U);
        control.items().add("free again"); // unlocked: direct Items mutation is legal once more
        EXPECT_EQ(control.items().count(), 1U);
    }

    TEST(picker, items_source_of_strings_populates_items)
    {
        picker control;
        control.set_items_source(make_source({"John", "Paul", "Ringo"}));
        control.set_selected_index(0);
        EXPECT_EQ(control.items().count(), 3U);
        EXPECT_EQ(control.items().at(0), "John");
    }

    TEST(picker, modifying_items_throws_while_items_source_is_set)
    {
        picker control;
        control.set_items_source(make_source({}));
        EXPECT_THROW(control.items().add("foo"), std::logic_error);
    }

    // ---- ItemsSource collection-change tracking ----

    TEST(picker, items_source_append_flows_into_items)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(0);
        EXPECT_EQ(control.items().count(), 3U);
        EXPECT_EQ(control.items().at(0), "John");

        items->add("George");
        EXPECT_EQ(control.items().count(), 4U);
        EXPECT_EQ(control.items().at(control.items().count() - 1), "George");
    }

    TEST(picker, items_source_clear_flows_into_items)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(0);
        EXPECT_EQ(control.items().count(), 3U);

        items->clear();
        EXPECT_EQ(control.items().count(), 0U);
    }

    TEST(picker, items_source_insert_flows_into_items)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(0);

        items->insert(1, "George");
        EXPECT_EQ(control.items().count(), 4U);
        EXPECT_EQ(control.items().at(1), "George");
    }

    TEST(picker, items_source_remove_flows_into_items)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(0);

        items->remove_at(1);
        EXPECT_EQ(control.items().count(), 2U);
        EXPECT_EQ(control.items().at(1), "Ringo");
    }

    // ---- selection preservation across inserts/removes (TestItemsSourceCollectionChanged*Selected
    //      + the #29235 regression pair) ----

    TEST(picker, insert_before_selection_preserves_the_selected_item)
    {
        struct case_t
        {
            std::size_t insertion_index;
            std::vector<std::string> insert_names;
        };
        const case_t cases[] = {
            {0, {"George"}},         {1, {"George"}},         {2, {"George"}},         {3, {"George"}},
            {0, {"George", "Pete"}}, {1, {"George", "Pete"}}, {2, {"George", "Pete"}}, {3, {"George", "Pete"}},
        };
        for (const auto& test_case : cases)
        {
            auto items = make_source({"John", "Paul", "Ringo"});
            picker control;
            control.set_items_source(items);
            control.set_selected_index(1);
            const auto original = control.selected_item();
            ASSERT_TRUE(original.has_value());

            items->insert_range(test_case.insertion_index, test_case.insert_names);
            EXPECT_EQ(control.items().count(), 3U + test_case.insert_names.size());
            // The selected item remains the same; the index follows it.
            EXPECT_EQ(control.selected_item(), original);
            EXPECT_EQ(control.selected_index(), items->index_of(*original));
        }
    }

    TEST(picker, remove_around_selection_preserves_or_moves_the_selected_item)
    {
        struct case_t
        {
            std::size_t remove_index;
            std::size_t remove_count;
            bool selected_item_preserved;
        };
        const case_t cases[] = {
            // removed items do NOT include the selected item ("Paul" at index 1)
            {0, 1, true},
            {2, 1, true},
            {2, 2, true},
            // removed items include the selected item
            {1, 1, false},
            {0, 2, false},
            {1, 2, false},
        };
        for (const auto& test_case : cases)
        {
            auto items = make_source({"John", "Paul", "Ringo", "George"});
            picker control;
            control.set_items_source(items);
            control.set_selected_index(1);
            const auto original = control.selected_item();
            ASSERT_TRUE(original.has_value());

            items->remove_range(test_case.remove_index, test_case.remove_count);
            EXPECT_EQ(control.items().count(), 4U - test_case.remove_count);
            if (test_case.selected_item_preserved)
            {
                EXPECT_EQ(control.selected_item(), original);
                EXPECT_EQ(control.selected_index(), items->index_of(*original));
            }
            else
            {
                EXPECT_NE(control.selected_item(), original);
            }
        }
    }

    TEST(picker, remove_at_end_with_end_selected_clamps_to_the_new_last_item)
    {
        for (const std::size_t remove_count : {std::size_t{1}, std::size_t{2}})
        {
            auto items = make_source({"John", "Paul", "Ringo", "George"});
            picker control;
            control.set_items_source(items);
            control.set_selected_index(static_cast<int>(4 - remove_count));

            items->remove_range(4 - remove_count, remove_count);
            EXPECT_EQ(control.items().count(), 4U - remove_count);
            EXPECT_EQ(control.selected_index(), static_cast<int>(items->count()) - 1);
            ASSERT_TRUE(control.selected_item().has_value());
            EXPECT_EQ(*control.selected_item(), items->at(items->count() - 1));
        }
    }

    TEST(picker, remove_before_selection_preserves_selected_item_29235)
    {
        auto items = make_source({"Item0", "Item1", "Item2"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(2);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Item2");
        EXPECT_EQ(control.selected_index(), 2);

        items->remove_at(0);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Item2");
        EXPECT_EQ(control.selected_index(), 1);
    }

    TEST(picker, insert_before_selection_preserves_selected_item_29235)
    {
        auto items = make_source({"Cat", "Dog", "Rabbit"});
        picker control;
        control.set_items_source(items);
        control.set_selected_index(1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Dog");
        EXPECT_EQ(control.selected_index(), 1);

        items->insert(0, "Goat");
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Dog");
        EXPECT_EQ(control.selected_index(), 2);
    }

    // ---- reentrancy: the collection mutates from inside the SelectedIndex change notification
    //      (C# hooks PropertyChanged; the port hooks the INPC analog bindable_object::property_changed) ----

    TEST(picker, reentrant_append_during_selected_index_change)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);

        control.property_changed.connect([&items](std::string_view name) {
            if (name == "selected_index")
            {
                items->add("George");
            }
        });

        control.set_selected_index(1);

        EXPECT_EQ(control.items().count(), 4U);
        EXPECT_EQ(control.items().at(control.items().count() - 1), "George");
        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), items->at(1));
    }

    TEST(picker, reentrant_clear_during_selected_index_change)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);

        control.property_changed.connect([&items](std::string_view name) {
            if (name == "selected_index")
            {
                items->clear();
            }
        });

        control.set_selected_index(1);

        EXPECT_EQ(control.items().count(), 0U);
        EXPECT_EQ(control.selected_index(), -1);
        EXPECT_FALSE(control.selected_item().has_value());
    }

    TEST(picker, reentrant_insert_during_selected_index_change)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);

        bool inserted = false; // C#'s handler effectively runs once (later raises see the same index)
        control.property_changed.connect([&items, &inserted](std::string_view name) {
            if (name == "selected_index" && !inserted)
            {
                inserted = true;
                items->insert(1, "George");
            }
        });

        control.set_selected_index(2);

        EXPECT_EQ(control.items().count(), 4U);
        EXPECT_EQ(control.items().at(1), "George");
        EXPECT_EQ(control.selected_index(), 2);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), items->at(2));
    }

    TEST(picker, reentrant_items_source_swap_during_selected_index_change)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);

        bool swapped = false;
        control.property_changed.connect([&control, &swapped](std::string_view name) {
            if (name == "selected_index" && !swapped)
            {
                swapped = true;
                control.set_items_source(make_source({"Peach", "Orange"}));
            }
        });

        control.set_selected_index(1);

        EXPECT_EQ(control.items().count(), 2U);
        EXPECT_EQ(control.items().at(0), "Peach");
        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Orange");
    }

    TEST(picker, reentrant_remove_during_selected_index_change)
    {
        auto items = make_source({"John", "Paul", "Ringo"});
        picker control;
        control.set_items_source(items);

        bool removed = false;
        control.property_changed.connect([&items, &removed](std::string_view name) {
            if (name == "selected_index" && !removed)
            {
                removed = true;
                items->remove_at(1);
            }
        });

        control.set_selected_index(1);

        EXPECT_EQ(control.items().count(), 2U);
        EXPECT_EQ(control.items().at(1), "Ringo");
        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), items->at(1));
    }

    // ---- the IItemDelegate face ----

    TEST(picker, item_delegate_reports_count_and_items)
    {
        picker control;
        control.items().add("a");
        control.items().add("b");
        i_picker& face = control;
        EXPECT_EQ(face.get_count(), 2);
        EXPECT_EQ(face.get_item(0), "a");
        EXPECT_EQ(face.get_item(1), "b");
        EXPECT_EQ(face.get_item(-1), ""); // negative index answers empty, never throws
        EXPECT_EQ(face.get_item(5), "");  // past the end likewise
    }

    TEST(picker, selected_index_changed_raises_on_every_real_change)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        int raised = 0;
        control.selected_index_changed.connect([&raised] { ++raised; });

        control.set_selected_index(1);
        EXPECT_EQ(raised, 1);
        control.set_selected_index(1); // unchanged -> silent
        EXPECT_EQ(raised, 1);
        control.set_selected_index(0);
        EXPECT_EQ(raised, 2);
    }

    // ---- the headless handler seam (PickerExtensions.UpdatePicker mirrors + the on_done commit) ----

    TEST(picker_handler_seam, attaching_handler_mirrors_items_selection_and_title)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.set_selected_index(1);
        control.set_title("Pick a Beatle");

        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->items, (std::vector<std::string>{"John", "Paul"}));
        EXPECT_EQ(platform->selected_index, 1);
        EXPECT_EQ(platform->text, "Paul");
        EXPECT_EQ(platform->title, "Pick a Beatle");
    }

    TEST(picker_handler_seam, items_changes_reload_the_native_list)
    {
        picker control;
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->items.empty());

        control.items().add("one");
        control.items().add("two");
        EXPECT_EQ(platform->items, (std::vector<std::string>{"one", "two"}));

        control.set_items_source(make_source({"x", "y", "z"}));
        EXPECT_EQ(platform->items, (std::vector<std::string>{"x", "y", "z"}));
    }

    TEST(picker_handler_seam, no_selection_clears_the_display_text)
    {
        picker control;
        control.items().add("one");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->text, ""); // selected_index -1 -> empty text (the Title placeholder shows)

        control.set_selected_index(0);
        EXPECT_EQ(platform->text, "one");
    }

    TEST(picker_handler_seam, native_done_commits_the_picked_row)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int raised = 0;
        control.selected_index_changed.connect([&raised] { ++raised; });

        platform->on_done(1); // the Done-accessory tap committing wheel row 1 (FinishSelectItem)
        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value()); // UpdateSelectedItem reads Items w/o a source
        EXPECT_EQ(*control.selected_item(), "Paul");
        EXPECT_EQ(platform->text, "Paul");
        EXPECT_EQ(raised, 1);
    }

    TEST(picker_handler_seam, done_with_no_pending_row_selects_the_first_item)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        platform->on_done(-1); // FinishSelectItem: an unset row with items present commits row 0
        EXPECT_EQ(control.selected_index(), 0);
        EXPECT_EQ(platform->text, "John");
    }

    TEST(picker_handler_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<picker_handler*>(handler.get()), nullptr);
    }
} // namespace

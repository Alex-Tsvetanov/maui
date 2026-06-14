// Tests for the items-source abstraction (W2-19 unit point 5): boxed_item / i_item_collection /
// items_source_factory + the empty / list / observable / observable-grouped sources. Behavior is
// derived from the C# IItemsViewSource family (src/Controls/src/Core/Handlers/Items/iOS/
// ItemsSourceFactory.cs, EmptySource.cs, ListSource.cs, ObservableItemsSource.cs,
// ObservableGroupedSource.cs) — the repo carries no unit suite for these internal classes, so these
// are characterization tests of the C# source bodies (port/CLAUDE.md: capture, then port).
// §8: collections (publishers) are declared BEFORE the sources (subscribers) throughout.

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_source_factory.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::grouping;
    using maui::controls::grouping_ptr;
    using maui::controls::i_item_collection;
    using maui::controls::i_items_view_source;
    using maui::controls::i_observable_items_view_source;
    using maui::controls::index_path;
    using maui::controls::items_source_factory;
    using maui::controls::make_grouping;
    using maui::controls::make_item_collection;
    using maui::controls::source_update;
    using maui::controls::source_update_kind;
    using maui::core::observable_collection;

    using string_collection = observable_collection<std::string>;

    std::shared_ptr<string_collection> make_strings(std::vector<std::string> items)
    {
        return std::make_shared<string_collection>(std::move(items));
    }

    // ---- boxed_item (the C# object stand-in) ----

    TEST(boxed_item, null_semantics)
    {
        const boxed_item null_item;
        EXPECT_FALSE(null_item.has_value());
        EXPECT_TRUE(null_item.equals(boxed_item{}));            // null == null
        EXPECT_FALSE(null_item.equals(boxed_item::of(int{1}))); // null != value
        EXPECT_TRUE(null_item.text().empty());
    }

    TEST(boxed_item, value_equality_for_comparable_types)
    {
        // C# object.Equals on strings/values compares by value.
        EXPECT_TRUE(boxed_item::of(std::string{"A"}).equals(boxed_item::of(std::string{"A"})));
        EXPECT_FALSE(boxed_item::of(std::string{"A"}).equals(boxed_item::of(std::string{"B"})));
        EXPECT_TRUE(boxed_item::of(int{7}) == boxed_item::of(int{7}));
        // Different boxed types are never equal.
        EXPECT_FALSE(boxed_item::of(int{7}).equals(boxed_item::of(double{7.0})));
    }

    TEST(boxed_item, reference_equality_for_shared_objects)
    {
        struct plain_object
        {
            int value = 0;
        };
        auto first = std::make_shared<plain_object>();
        auto second = std::make_shared<plain_object>();
        EXPECT_TRUE(boxed_item::of(first).equals(boxed_item::of(first)));   // same instance
        EXPECT_FALSE(boxed_item::of(first).equals(boxed_item::of(second))); // no operator== → reference
    }

    TEST(boxed_item, text_renders_strings_and_numbers)
    {
        EXPECT_EQ(boxed_item::of(std::string{"Item"}).text(), "Item");
        EXPECT_EQ(boxed_item::of(int{42}).text(), "42");
        struct opaque
        {
        };
        EXPECT_TRUE(boxed_item::of(opaque{}).text().empty()); // no reflection-free display form
    }

    TEST(boxed_item, typed_unbox_is_type_checked)
    {
        const boxed_item item = boxed_item::of(std::string{"A"});
        const std::shared_ptr<std::string> unboxed = item.as<std::string>();
        ASSERT_NE(unboxed, nullptr);
        EXPECT_EQ(*unboxed, "A");
        EXPECT_EQ(item.as<int>(), nullptr);
    }

    // ---- i_item_collection (the erased ItemsSource seam) ----

    TEST(item_collection, snapshot_flavor_has_no_change_feed)
    {
        auto collection = make_item_collection(std::vector<std::string>{"A", "B", "C"});
        EXPECT_EQ(collection->count(), 3U);
        EXPECT_EQ(collection->at(1).text(), "B");
        EXPECT_EQ(collection->index_of(boxed_item::of(std::string{"C"})), 2);
        EXPECT_EQ(collection->index_of(boxed_item::of(std::string{"missing"})), -1);
        EXPECT_EQ(collection->changed(), nullptr);
        EXPECT_EQ(collection->group_items(0), nullptr); // not a group
    }

    TEST(item_collection, observable_flavor_exposes_the_collection_event)
    {
        auto items = make_strings({"A"});
        auto collection = make_item_collection(items);
        ASSERT_NE(collection->changed(), nullptr);
        EXPECT_EQ(collection->changed(), &items->collection_changed);
    }

    TEST(item_collection, grouped_flavor_boxes_keys_and_exposes_nested_items)
    {
        auto groups = std::make_shared<observable_collection<grouping_ptr>>();
        groups->add(make_grouping(std::string{"Fruit"}, std::vector<std::string>{"Apple", "Pear"}));
        groups->add(make_grouping(std::string{"Veg"}, std::vector<std::string>{"Kale"}));
        auto collection = make_item_collection(groups);

        EXPECT_EQ(collection->count(), 2U);
        EXPECT_EQ(collection->at(0).text(), "Fruit"); // the group KEY is the boxed element
        const auto nested = collection->group_items(0);
        ASSERT_NE(nested, nullptr);
        EXPECT_EQ(nested->count(), 2U);
        EXPECT_EQ(nested->at(1).text(), "Pear");
    }

    // ---- items_source_factory (ItemsSourceFactory.Create / CreateGrouped) ----

    TEST(items_sources, factory_null_creates_the_empty_source)
    {
        const auto source = items_source_factory::create(nullptr);
        EXPECT_EQ(source->item_count(), 0);
        EXPECT_EQ(source->group_count(), 0);
        EXPECT_EQ(source->item_count_in_group(0), 0);
        EXPECT_THROW((void)source->item({0, 0}), std::out_of_range);
        EXPECT_THROW((void)source->get_index_for_item(boxed_item::of(std::string{"A"})), std::out_of_range);
    }

    TEST(items_sources, factory_snapshot_creates_the_list_source)
    {
        const auto source = items_source_factory::create(make_item_collection(std::vector<std::string>{"A", "B"}));
        EXPECT_EQ(source->item_count(), 2);
        EXPECT_EQ(source->group_count(), 1);
        EXPECT_EQ(source->item_count_in_group(0), 2);
        EXPECT_THROW((void)source->item_count_in_group(1), std::out_of_range);
        EXPECT_EQ(source->item({0, 1}).text(), "B");
        EXPECT_THROW((void)source->item({1, 0}), std::out_of_range);
        EXPECT_EQ(source->get_index_for_item(boxed_item::of(std::string{"B"})), (index_path{0, 1}));
        EXPECT_EQ(source->get_index_for_item(boxed_item::of(std::string{"missing"})), (index_path{-1, -1}));
        EXPECT_FALSE(source->group({0, 0}).has_value()); // flat source: Group() is null
        EXPECT_EQ(source->group_items_view_source({0, 0}), nullptr);
    }

    TEST(items_sources, factory_observable_creates_the_observable_source)
    {
        auto items = make_strings({"A", "B"});
        const auto source = items_source_factory::create(make_item_collection(items));
        EXPECT_NE(std::dynamic_pointer_cast<i_observable_items_view_source>(source), nullptr);
        EXPECT_EQ(source->item_count(), 2);
    }

    // ---- ObservableItemsSource: the change translation ----

    class observable_source_test : public ::testing::Test
    {
    protected:
        observable_source_test()
            : items(make_strings({"A", "B", "C", "D"})),
              source(items_source_factory::create(make_item_collection(items)))
        {
            watch = maui::core::connect_scoped(source->updated,
                                               [this](const source_update& update) { updates.push_back(update); });
        }

        std::shared_ptr<string_collection> items; // publisher first (§8)
        std::shared_ptr<i_items_view_source> source;
        std::vector<source_update> updates;
        maui::core::scoped_connection watch;
    };

    TEST_F(observable_source_test, add_translates_to_insert_items_and_adjusts_count)
    {
        items->add("E");
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::insert_items);
        EXPECT_EQ(updates[0].section, 0);
        EXPECT_EQ(updates[0].index, 4);
        EXPECT_EQ(updates[0].count, 1U);
        EXPECT_EQ(source->item_count(), 5);
    }

    TEST_F(observable_source_test, insert_range_translates_to_one_insert_items)
    {
        items->insert_range(1, {"X", "Y"});
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::insert_items);
        EXPECT_EQ(updates[0].index, 1);
        EXPECT_EQ(updates[0].count, 2U);
        EXPECT_EQ(source->item_count(), 6);
    }

    TEST_F(observable_source_test, remove_translates_to_delete_items)
    {
        items->remove_at(1);
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::delete_items);
        EXPECT_EQ(updates[0].index, 1);
        EXPECT_EQ(updates[0].count, 1U);
        EXPECT_EQ(source->item_count(), 3);
    }

    TEST_F(observable_source_test, replace_translates_to_reload_items)
    {
        items->set(2, "Z"); // the C# equal-size Replace → ReloadItems
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::reload_items);
        EXPECT_EQ(updates[0].index, 2);
        EXPECT_EQ(updates[0].count, 1U);
        EXPECT_EQ(source->item({0, 2}).text(), "Z");
    }

    TEST_F(observable_source_test, single_move_translates_to_move_item)
    {
        items->move(0, 2);
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::move_item);
        EXPECT_EQ(updates[0].index, 0);
        EXPECT_EQ(updates[0].move_to, (index_path{0, 2}));
    }

    TEST_F(observable_source_test, multi_move_translates_to_the_reload_window)
    {
        items->move(0, 2, 2); // the literal C# window: start = min, count = max + moved-count
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::reload_items);
        EXPECT_EQ(updates[0].index, 0);
        EXPECT_EQ(updates[0].count, 4U);
    }

    TEST_F(observable_source_test, clear_translates_to_reload_data)
    {
        items->clear();
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::reload_data);
        EXPECT_EQ(source->item_count(), 0);
    }

    TEST_F(observable_source_test, observe_changes_false_suppresses_updates)
    {
        const auto observable = std::dynamic_pointer_cast<i_observable_items_view_source>(source);
        ASSERT_NE(observable, nullptr);
        EXPECT_TRUE(observable->observe_changes()); // default true
        observable->set_observe_changes(false);
        items->add("E");
        EXPECT_TRUE(updates.empty());
        EXPECT_EQ(source->item_count(), 4); // C#: the count is not maintained while not observing
    }

    TEST_F(observable_source_test, item_access_and_index_lookup)
    {
        EXPECT_EQ(source->item({0, 0}).text(), "A");
        EXPECT_THROW((void)source->item({1, 0}), std::out_of_range); // wrong section
        EXPECT_EQ(source->get_index_for_item(boxed_item::of(std::string{"D"})), (index_path{0, 3}));
        EXPECT_EQ(source->get_index_for_item(boxed_item::of(std::string{"missing"})), (index_path{-1, -1}));
    }

    // ---- ObservableGroupedSource ----

    class grouped_source_test : public ::testing::Test
    {
    protected:
        grouped_source_test()
        {
            // Publishers (the nested collections + the group list) before the source (§8).
            fruit = make_strings({"Apple", "Pear"});
            veg = make_strings({"Kale"});
            groups = std::make_shared<observable_collection<grouping_ptr>>();
            groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Fruit"}), make_item_collection(fruit)));
            groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Veg"}), make_item_collection(veg)));
            source = items_source_factory::create_grouped(make_item_collection(groups));
            watch = maui::core::connect_scoped(source->updated,
                                               [this](const source_update& update) { updates.push_back(update); });
        }

        std::shared_ptr<string_collection> fruit;
        std::shared_ptr<string_collection> veg;
        std::shared_ptr<observable_collection<grouping_ptr>> groups;
        std::shared_ptr<i_items_view_source> source;
        std::vector<source_update> updates;
        maui::core::scoped_connection watch;
    };

    TEST_F(grouped_source_test, counts_and_access)
    {
        EXPECT_EQ(source->group_count(), 2);
        EXPECT_EQ(source->item_count(), 3);
        EXPECT_EQ(source->item_count_in_group(0), 2);
        EXPECT_EQ(source->item_count_in_group(1), 1);
        EXPECT_EQ(source->item({0, 1}).text(), "Pear");
        EXPECT_EQ(source->item({1, 0}).text(), "Kale");
        EXPECT_EQ(source->group({1, 0}).text(), "Veg"); // the group KEY box
        EXPECT_EQ(source->get_index_for_item(boxed_item::of(std::string{"Kale"})), (index_path{1, 0}));
    }

    TEST_F(grouped_source_test, group_items_view_source_views_one_section)
    {
        const auto section = source->group_items_view_source({1, 0});
        ASSERT_NE(section, nullptr);
        EXPECT_EQ(section->item_count(), 1);
        EXPECT_EQ(section->item({1, 0}).text(), "Kale"); // the child reports its own section index
    }

    TEST_F(grouped_source_test, adding_a_group_translates_to_insert_sections)
    {
        groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Dairy"}),
                                               make_item_collection(std::vector<std::string>{"Milk"})));
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::insert_sections);
        EXPECT_EQ(updates[0].section, 2);
        EXPECT_EQ(updates[0].count, 1U);
        EXPECT_EQ(source->group_count(), 3);
        EXPECT_EQ(source->item_count(), 4);
    }

    TEST_F(grouped_source_test, removing_a_group_translates_to_delete_sections)
    {
        groups->remove_at(0);
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::delete_sections);
        EXPECT_EQ(updates[0].section, 0);
        EXPECT_EQ(source->group_count(), 1);
        EXPECT_EQ(source->item({0, 0}).text(), "Kale"); // sections shifted
    }

    TEST_F(grouped_source_test, moving_a_group_translates_to_move_section)
    {
        groups->move(0, 1);
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::move_section);
        EXPECT_EQ(updates[0].section, 0);
        EXPECT_EQ(updates[0].move_to.section, 1);
        EXPECT_EQ(source->group({0, 0}).text(), "Veg");
    }

    TEST_F(grouped_source_test, nested_item_change_fans_out_with_the_section_index)
    {
        veg->add("Carrot"); // group 1's collection changes
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::insert_items);
        EXPECT_EQ(updates[0].section, 1);
        EXPECT_EQ(updates[0].index, 1);
        EXPECT_EQ(source->item_count_in_group(1), 2);
    }

    TEST_F(grouped_source_test, nested_fan_out_tracks_section_shifts_after_group_removal)
    {
        groups->remove_at(0); // Veg becomes section 0; tracking is rebuilt
        updates.clear();
        veg->add("Carrot");
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].section, 0);
    }

    TEST_F(grouped_source_test, reset_translates_to_reload_data)
    {
        groups->clear();
        ASSERT_EQ(updates.size(), 1U);
        EXPECT_EQ(updates[0].kind, source_update_kind::reload_data);
        EXPECT_EQ(source->group_count(), 0);
        EXPECT_EQ(source->item_count(), 0);
    }

    TEST_F(grouped_source_test, observe_changes_false_suppresses_group_and_nested_updates)
    {
        const auto observable = std::dynamic_pointer_cast<i_observable_items_view_source>(source);
        ASSERT_NE(observable, nullptr);
        observable->set_observe_changes(false);
        groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Dairy"}),
                                               make_item_collection(std::vector<std::string>{"Milk"})));
        veg->add("Carrot");
        EXPECT_TRUE(updates.empty());
    }

    TEST(items_sources, grouped_snapshot_groups_answer_without_subscriptions)
    {
        auto groups = std::make_shared<observable_collection<grouping_ptr>>();
        groups->add(make_grouping(std::string{"Fruit"}, std::vector<std::string>{"Apple"}));
        const auto source = items_source_factory::create_grouped(make_item_collection(groups));
        EXPECT_EQ(source->item_count_in_group(0), 1);
        const auto section = source->group_items_view_source({0, 0});
        ASSERT_NE(section, nullptr); // a snapshot group still answers as a list view
        EXPECT_EQ(section->item_count(), 1);
    }

    TEST(items_sources, factory_grouped_null_creates_the_empty_source)
    {
        const auto source = items_source_factory::create_grouped(nullptr);
        EXPECT_EQ(source->group_count(), 0);
        EXPECT_EQ(source->item_count(), 0);
    }
} // namespace

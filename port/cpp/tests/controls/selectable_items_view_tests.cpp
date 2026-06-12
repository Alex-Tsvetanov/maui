// Tests for selectable_items_view + selection_list — the SelectableItemsViewTests/SelectionListTests
// semantics (the historical Core.UnitTests suites; this repo snapshot no longer carries them, so the
// cases are characterization of SelectableItemsView.cs + SelectionList.cs, the unit's gold oracle).
// §8: external selection collections (publishers) are declared before the views (subscribers).

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::selectable_items_view;
    using maui::controls::selection_changed_event_args;
    using maui::controls::selection_mode;
    using maui::core::observable_collection;

    boxed_item box(const std::string& value)
    {
        return boxed_item::of(value);
    }

    // Collects every SelectionChanged raise.
    struct selection_watcher
    {
        explicit selection_watcher(selectable_items_view& view) : view_(&view)
        {
            token_ = view.selection_changed.connect(
                [this](const selection_changed_event_args& args) { changes.push_back(args); });
        }
        ~selection_watcher()
        {
            view_->selection_changed.disconnect(token_);
        }
        selection_watcher(const selection_watcher&) = delete;
        selection_watcher(selection_watcher&&) = delete;
        selection_watcher& operator=(const selection_watcher&) = delete;
        selection_watcher& operator=(selection_watcher&&) = delete;

        std::vector<selection_changed_event_args> changes;

    private:
        selectable_items_view* view_;
        maui::core::connection_token token_ = 0;
    };

    TEST(selectable_items_view, defaults)
    {
        selectable_items_view view;
        EXPECT_EQ(view.selection_mode(), selection_mode::none);
        EXPECT_FALSE(view.selected_item().has_value());
        EXPECT_EQ(view.selected_items().count(), 0U); // never null — the default empty list
    }

    // ---- SelectedItem (single selection) ----

    TEST(selectable_items_view, selected_item_change_raises_selection_changed)
    {
        selectable_items_view view;
        selection_watcher watcher{view};

        view.set_selected_item(box("A"));

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_TRUE(watcher.changes[0].previous_selection.empty()); // null maps to the empty list
        ASSERT_EQ(watcher.changes[0].current_selection.size(), 1U);
        EXPECT_EQ(watcher.changes[0].current_selection[0].text(), "A");
    }

    TEST(selectable_items_view, selected_item_same_value_does_not_signal)
    {
        selectable_items_view view;
        view.set_selected_item(box("A"));
        selection_watcher watcher{view};
        view.set_selected_item(box("A")); // SetValue equality → no change
        EXPECT_TRUE(watcher.changes.empty());
    }

    TEST(selectable_items_view, selected_item_replacement_carries_the_previous_item)
    {
        selectable_items_view view;
        view.set_selected_item(box("A"));
        selection_watcher watcher{view};

        view.set_selected_item(box("B"));

        ASSERT_EQ(watcher.changes.size(), 1U);
        ASSERT_EQ(watcher.changes[0].previous_selection.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection[0].text(), "A");
        EXPECT_EQ(watcher.changes[0].current_selection[0].text(), "B");
    }

    TEST(selectable_items_view, command_runs_before_the_event)
    {
        selectable_items_view view;
        std::vector<std::string> order;
        view.selection_changed_command = [&order] { order.emplace_back("command"); };
        const maui::core::connection_token token = view.selection_changed.connect(
            [&order](const selection_changed_event_args&) { order.emplace_back("event"); });

        view.set_selected_item(box("A"));

        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "command"); // C# SelectionPropertyChanged: Execute, then the event
        EXPECT_EQ(order[1], "event");
        view.selection_changed.disconnect(token);
    }

    // ---- SelectedItems (the SelectionList choreography) ----

    TEST(selection_list, add_notifies_with_shadow_then_live)
    {
        selectable_items_view view;
        selection_watcher watcher{view};

        view.selected_items().add(box("A"));
        view.selected_items().add(box("B"));

        ASSERT_EQ(watcher.changes.size(), 2U);
        EXPECT_TRUE(watcher.changes[0].previous_selection.empty());
        EXPECT_EQ(watcher.changes[0].current_selection.size(), 1U);
        // The second add reports the PRE-change shadow as the old selection.
        ASSERT_EQ(watcher.changes[1].previous_selection.size(), 1U);
        EXPECT_EQ(watcher.changes[1].previous_selection[0].text(), "A");
        ASSERT_EQ(watcher.changes[1].current_selection.size(), 2U);
        EXPECT_EQ(watcher.changes[1].current_selection[1].text(), "B");
    }

    TEST(selection_list, remove_of_absent_item_does_not_signal)
    {
        selectable_items_view view;
        view.selected_items().add(box("A"));
        selection_watcher watcher{view};

        EXPECT_FALSE(view.selected_items().remove(box("missing")));
        EXPECT_TRUE(watcher.changes.empty());

        EXPECT_TRUE(view.selected_items().remove(box("A")));
        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection.size(), 1U);
        EXPECT_TRUE(watcher.changes[0].current_selection.empty());
    }

    TEST(selection_list, clear_notifies_with_the_empty_list)
    {
        selectable_items_view view;
        view.selected_items().add(box("A"));
        view.selected_items().add(box("B"));
        selection_watcher watcher{view};

        view.selected_items().clear();

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection.size(), 2U);
        EXPECT_TRUE(watcher.changes[0].current_selection.empty());
        EXPECT_EQ(view.selected_items().count(), 0U);
    }

    TEST(selection_list, mutations_raise_the_selected_items_property_change)
    {
        selectable_items_view view;
        std::vector<std::string> names;
        const maui::core::connection_token token =
            view.property_changed.connect([&names](std::string_view name) { names.emplace_back(name); });

        view.selected_items().add(box("A"));

        EXPECT_EQ(std::count(names.begin(), names.end(), "selected_items"), 1);
        view.property_changed.disconnect(token);
    }

    TEST(selection_list, contains_and_index_use_value_equality)
    {
        selectable_items_view view;
        view.selected_items().add(box("A"));
        EXPECT_TRUE(view.selected_items().contains(box("A"))); // a different box, equal value
        EXPECT_EQ(view.selected_items().index_of(box("A")), 0);
        EXPECT_EQ(view.selected_items().index_of(box("B")), -1);
    }

    // ---- UpdateSelectedItems (the platform batch write-back) ----

    TEST(selectable_items_view, update_selected_items_raises_exactly_one_change)
    {
        selectable_items_view view;
        view.selected_items().add(box("A"));
        selection_watcher watcher{view};

        view.update_selected_items({box("B"), box("C")});

        ASSERT_EQ(watcher.changes.size(), 1U); // the per-mutation notifications are suppressed
        ASSERT_EQ(watcher.changes[0].previous_selection.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection[0].text(), "A");
        ASSERT_EQ(watcher.changes[0].current_selection.size(), 2U);
        EXPECT_EQ(view.selected_items().count(), 2U);
        EXPECT_EQ(view.selected_items().at(0).text(), "B");
    }

    // ---- the SelectedItems SETTER (wrap + notify) ----

    TEST(selectable_items_view, set_selected_items_notifies_old_versus_new)
    {
        selectable_items_view view;
        view.selected_items().add(box("A"));
        selection_watcher watcher{view};

        view.set_selected_items(std::vector<boxed_item>{box("X"), box("Y")});

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection.size(), 1U);
        EXPECT_EQ(watcher.changes[0].current_selection.size(), 2U);
        EXPECT_EQ(view.selected_items().at(1).text(), "Y");
    }

    TEST(selectable_items_view, bound_observable_selection_signals_external_changes)
    {
        // The viewmodel-bound SelectedItems: a direct mutation of the bound collection raises a
        // selection change (SelectionList.OnCollectionChanged), with the shadow as the old list.
        auto bound = std::make_shared<observable_collection<boxed_item>>(); // publisher first (§8)
        selectable_items_view view;
        view.set_selected_items(bound);
        selection_watcher watcher{view};

        bound->add(box("A")); // straight onto the viewmodel collection

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_TRUE(watcher.changes[0].previous_selection.empty());
        ASSERT_EQ(watcher.changes[0].current_selection.size(), 1U);
        EXPECT_EQ(view.selected_items().count(), 1U); // the face reads the live bound list

        // A mutation through the face must NOT double-signal via the collection event.
        view.selected_items().add(box("B"));
        EXPECT_EQ(watcher.changes.size(), 2U);
        EXPECT_EQ(bound->size(), 2U); // and it lands in the bound collection
    }

    // ---- SelectionMode changes (the diff logic) ----

    TEST(selectable_items_view, mode_change_with_no_selection_does_not_signal)
    {
        selectable_items_view view;
        selection_watcher watcher{view};
        view.set_selection_mode(selection_mode::single);
        view.set_selection_mode(selection_mode::multiple);
        view.set_selection_mode(selection_mode::none);
        EXPECT_TRUE(watcher.changes.empty()); // both sides empty every time
    }

    TEST(selectable_items_view, mode_change_to_none_drops_the_single_selection)
    {
        selectable_items_view view;
        view.set_selection_mode(selection_mode::single);
        view.set_selected_item(box("A"));
        selection_watcher watcher{view};

        view.set_selection_mode(selection_mode::none);

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection.size(), 1U);
        EXPECT_TRUE(watcher.changes[0].current_selection.empty());
    }

    TEST(selectable_items_view, mode_change_single_to_multiple_with_same_item_does_not_signal)
    {
        selectable_items_view view;
        view.set_selection_mode(selection_mode::single);
        view.set_selected_item(box("A"));
        view.selected_items().add(box("A")); // the same single item on both sides
        selection_watcher watcher{view};

        view.set_selection_mode(selection_mode::multiple);

        EXPECT_TRUE(watcher.changes.empty()); // same single item → suppressed
    }

    TEST(selectable_items_view, mode_change_multiple_to_single_diffs_the_selections)
    {
        selectable_items_view view;
        view.set_selection_mode(selection_mode::multiple);
        view.selected_items().add(box("A"));
        view.selected_items().add(box("B"));
        selection_watcher watcher{view};

        view.set_selection_mode(selection_mode::single); // selected_item is null → new side empty

        ASSERT_EQ(watcher.changes.size(), 1U);
        EXPECT_EQ(watcher.changes[0].previous_selection.size(), 2U);
        EXPECT_TRUE(watcher.changes[0].current_selection.empty());
    }
} // namespace

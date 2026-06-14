// Tests for the swipe_view + swipe_items + swipe_item / swipe_item_view family, and the swipe state
// machine seam — ported from SwipeViewTests.cs (src/Controls/tests/Core.UnitTests) and the state machine
// derived from the native MauiSwipeView.cs:
//   - the control: TestConstructor (empty collections + defaults), the four directional collections,
//     DefaultSwipeItems / Execute mode / SwipeBehaviorOnInvoked, ClearRemovesLogicalChildren,
//     BindingContext propagation into items + across a collection replacement
//     (BindingContextTransfersToNewSetOfSwipeItems), the SwipeItemView content, ProgrammaticallyOpen /
//     -Close (OpenRequested / CloseRequested), and the items-remain-in-logical-tree guard.
//   - the state machine (the headless swipe_view_handler twin, driven by synthetic offsets): the
//     Idle → Swiping → Open transitions, SwipeStarted/Changing/Ended fan-out, the 60% open threshold,
//     Execute-mode first-visible-item invocation + SwipeBehaviorOnInvoked close/remain-open, Reveal-mode
//     settle-open, the visibility + enabled gating of ExecuteSwipeItem, programmatic open/close, and the
//     IsOpen write-back.
//
// OWNERSHIP/§8: swipe items are NON-owning in their collection, so each item local is declared BEFORE
// the swipe_view / collection that parents it (publishers — the items — outlive their subscribers).
//
// DEVIATIONS exercised as documented (swipe_view.hpp): no ControlTemplate test (the template base is
// incompatible with the i_swipe_view contract diamond); no parent-scroll auto-close (the legacy
// ListView/CollectionView ancestors don't exist in the port).
#include "maui/controls/swipe_view.hpp"

#include <utility> // std::move

#include "maui/core/swipe_transition_mode.hpp" // swipe_transition_mode
#include "maui/core/swipe_view_requests.hpp"   // open/close requests + swipe event args

#include <memory>

#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::swipe_item;
    using maui::controls::swipe_item_view;
    using maui::controls::swipe_items;
    using maui::controls::swipe_view;
    using maui::core::i_element_handler;
    using maui::core::open_swipe_item;
    using maui::core::swipe_behavior_on_invoked;
    using maui::core::swipe_direction;
    using maui::core::swipe_machine_state;
    using maui::core::swipe_mode;
    using maui::core::swipe_view_handler;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation ----

    TEST(swipe_view, constructor_starts_with_empty_collections) // C# TestConstructor
    {
        swipe_view view;
        EXPECT_EQ(view.left_items_collection().count(), 0U);
        EXPECT_EQ(view.top_items_collection().count(), 0U);
        EXPECT_EQ(view.right_items_collection().count(), 0U);
        EXPECT_EQ(view.bottom_items_collection().count(), 0U);
        EXPECT_FALSE(view.is_open());
        EXPECT_EQ(view.threshold(), 0.0);
        EXPECT_EQ(view.transition_mode(), maui::core::swipe_transition_mode::reveal);
    }

    TEST(swipe_view, default_swipe_items_mode_and_behavior) // C# TestDefaultSwipeItems
    {
        swipe_item item;
        item.set_background_color(maui::graphics::colors::red);
        item.set_text("Text");
        swipe_view view;
        view.left_items_collection().add(item);

        EXPECT_EQ(view.left_items_collection().mode(), swipe_mode::reveal);
        EXPECT_EQ(view.left_items_collection().behavior_on_invoked(), swipe_behavior_on_invoked::automatic);
    }

    TEST(swipe_view, execute_mode_round_trips) // C# TestSwipeItemsExecuteMode
    {
        swipe_item item;
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->add(item);
        swipe_view view;
        view.set_left_items(std::move(items));
        EXPECT_EQ(view.left_items_collection().mode(), swipe_mode::execute);
    }

    TEST(swipe_view, swipe_behavior_on_invoked_round_trips) // C# TestSwipeItemsSwipeBehaviorOnInvoked
    {
        swipe_item item;
        auto items = std::make_unique<swipe_items>();
        items->set_behavior_on_invoked(swipe_behavior_on_invoked::close);
        items->add(item);
        swipe_view view;
        view.set_left_items(std::move(items));
        EXPECT_EQ(view.left_items_collection().behavior_on_invoked(), swipe_behavior_on_invoked::close);
    }

    TEST(swipe_view, the_four_directional_collections_accept_items) // C# TestLeft/Right/Top/BottomItems
    {
        swipe_item left;
        swipe_item right;
        swipe_item top;
        swipe_item bottom;
        swipe_view view;
        view.left_items_collection().add(left);
        view.right_items_collection().add(right);
        view.top_items_collection().add(top);
        view.bottom_items_collection().add(bottom);
        EXPECT_EQ(view.left_items_collection().count(), 1U);
        EXPECT_EQ(view.right_items_collection().count(), 1U);
        EXPECT_EQ(view.top_items_collection().count(), 1U);
        EXPECT_EQ(view.bottom_items_collection().count(), 1U);
    }

    TEST(swipe_view, clear_removes_logical_children) // C# ClearRemovesLogicalChildren
    {
        swipe_item a;
        swipe_item b;
        swipe_item c;
        swipe_view view;
        view.left_items_collection().add(a);
        view.left_items_collection().add(b);
        view.left_items_collection().add(c);
        EXPECT_EQ(view.left_items_collection().count(), 3U);
        EXPECT_EQ(a.logical_parent(), &view.left_items_collection());

        view.left_items_collection().clear();
        EXPECT_EQ(view.left_items_collection().count(), 0U);
        EXPECT_EQ(a.logical_parent(), nullptr); // detached from the logical tree
    }

    TEST(swipe_view,
         binding_context_propagates_to_added_items) // C# TestContentBindingContextPropagatesToAddedSwipeItems
    {
        swipe_item left;
        swipe_item right;
        swipe_item top;
        swipe_item bottom;
        swipe_view view;
        view.left_items_collection().add(left);
        view.right_items_collection().add(right);
        view.top_items_collection().add(top);
        view.bottom_items_collection().add(bottom);

        const auto context = std::make_shared<int>(42);
        view.set_binding_context(context);
        EXPECT_EQ(left.binding_context<int>(), context);
        EXPECT_EQ(right.binding_context<int>(), context);
        EXPECT_EQ(top.binding_context<int>(), context);
        EXPECT_EQ(bottom.binding_context<int>(), context);
    }

    TEST(swipe_view, binding_context_transfers_to_a_new_collection) // C# BindingContextTransfersToNewSetOfSwipeItems
    {
        swipe_item item;
        const auto bc1 = std::make_shared<int>(1);
        const auto bc2 = std::make_shared<int>(2);

        swipe_view view;
        swipe_items* const original = &view.left_items_collection();
        view.left_items_collection().add(item);
        view.set_binding_context(bc1);
        EXPECT_EQ(original->binding_context<int>(), bc1);
        EXPECT_EQ(item.binding_context<int>(), bc1);
        EXPECT_EQ(original->logical_parent(), &view);

        // Replace the collection (move `item` into the new one BEFORE swapping — the test owns it). The
        // setter destroys the old collection, so `original` dangles afterward and must not be touched.
        auto replacement = std::make_unique<swipe_items>();
        replacement->add(item);
        swipe_items* const new_collection = replacement.get();
        view.set_left_items(std::move(replacement));

        EXPECT_EQ(new_collection->logical_parent(), &view);
        EXPECT_EQ(item.logical_parent(), new_collection);

        view.set_binding_context(bc2);
        EXPECT_EQ(new_collection->binding_context<int>(), bc2);
        EXPECT_EQ(item.binding_context<int>(), bc2);
    }

    TEST(swipe_view, hosts_a_swipe_item_view_with_content) // C# TestSwipeItemView
    {
        mock_view inner;
        swipe_item_view item_view;
        item_view.set_content(inner);
        swipe_view view;
        view.left_items_collection().add(item_view);

        EXPECT_EQ(item_view.content(), &inner);
        EXPECT_EQ(view.left_items_collection().count(), 1U);
    }

    TEST(swipe_view, content_is_a_logical_child) // C# TestContentBindingContextChangedEvent
    {
        mock_view content;
        swipe_view view;
        view.set_content(content);
        EXPECT_EQ(view.content(), &content);

        const auto context = std::make_shared<int>(7);
        view.set_binding_context(context);
        EXPECT_EQ(content.binding_context<int>(), context);
    }

    TEST(swipe_view,
         items_remain_in_logical_tree_when_content_is_set) // C# SwipeItemsRemainInLogicalTreeWhenContentIsSet
    {
        swipe_item right;
        swipe_item left;
        swipe_item top;
        swipe_item bottom;
        mock_view content;
        swipe_view view;
        view.right_items_collection().add(right);
        view.left_items_collection().add(left);
        view.top_items_collection().add(top);
        view.bottom_items_collection().add(bottom);

        const auto context = std::make_shared<int>(99);
        view.set_binding_context(context);
        view.set_content(content);

        EXPECT_EQ(view.right_items_collection().logical_parent(), &view);
        EXPECT_EQ(view.left_items_collection().logical_parent(), &view);
        EXPECT_EQ(view.right_items_collection().binding_context<int>(), context);
        EXPECT_EQ(right.binding_context<int>(), context);
        EXPECT_EQ(left.binding_context<int>(), context);
    }

    TEST(swipe_view, open_and_close_raise_the_request_events) // C# TestProgrammaticallyOpen / -Close
    {
        swipe_item item;
        swipe_view view;
        view.left_items_collection().add(item);

        bool opened = false;
        view.open_requested.connect([&opened](const maui::core::swipe_view_open_request&) { opened = true; });
        bool closed = false;
        view.close_requested.connect([&closed](const maui::core::swipe_view_close_request&) { closed = true; });

        view.open(open_swipe_item::left_items);
        EXPECT_TRUE(opened);

        view.close();
        EXPECT_TRUE(closed);
    }

    // ---- the state-machine seam (the headless handler) ----

    std::shared_ptr<swipe_view_handler> attach_handler(swipe_view& view)
    {
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);
        return handler;
    }

    TEST(swipe_view_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<swipe_view>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<swipe_view_handler*>(handler.get()), nullptr);
    }

    TEST(swipe_view_seam, content_hosts_on_the_platform)
    {
        mock_view content;
        swipe_view view;
        view.set_content(content);
        auto handler = attach_handler(view);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_content, &content);
    }

    TEST(swipe_view_seam, programmatic_open_drives_state_to_open_and_writes_is_open_back)
    {
        swipe_item item; // a reveal item → the threshold is the item width (100)
        swipe_view view;
        view.left_items_collection().add(item);
        auto handler = attach_handler(view);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        view.open(open_swipe_item::left_items); // opening LEFT items is a RIGHT swipe (positive offset)
        EXPECT_EQ(platform->state.state, swipe_machine_state::open);
        EXPECT_EQ(platform->state.direction, swipe_direction::right);
        EXPECT_TRUE(platform->state.is_open);
        EXPECT_TRUE(view.is_open());

        view.close();
        EXPECT_EQ(platform->state.state, swipe_machine_state::idle);
        EXPECT_FALSE(platform->state.is_open);
        EXPECT_FALSE(view.is_open());
    }

    TEST(swipe_view_seam, synthetic_swipe_fans_out_started_changing_ended)
    {
        swipe_item item;
        swipe_view view;
        view.set_threshold(100); // an explicit threshold → 60% open = 60
        view.left_items_collection().add(item);
        auto handler = attach_handler(view);

        int started = 0;
        int changing = 0;
        int ended = 0;
        double last_offset = 0;
        view.swipe_started.connect([&started](const maui::core::swipe_view_swipe_started&) { ++started; });
        view.swipe_changing.connect([&changing, &last_offset](const maui::core::swipe_view_swipe_changing& a) {
            ++changing;
            last_offset = a.offset;
        });
        view.swipe_ended.connect([&ended](const maui::core::swipe_view_swipe_ended&) { ++ended; });

        handler->begin_swipe(swipe_direction::right);
        handler->swipe_to(20);
        handler->swipe_to(40);
        EXPECT_EQ(started, 1); // SwipeStarted fires once, on the first move
        EXPECT_EQ(changing, 2);
        EXPECT_EQ(last_offset, 40.0);
        EXPECT_TRUE(view.is_open()); // UpdateIsOpen(offset != 0)

        handler->end_swipe();
        EXPECT_EQ(ended, 1);
        // 40 < 60% of 100 (=60) → reset (not opened).
        EXPECT_FALSE(view.is_open());
    }

    TEST(swipe_view_seam, swipe_past_60_percent_settles_open_in_reveal_mode)
    {
        swipe_item item;
        swipe_view view;
        view.set_threshold(100);
        view.left_items_collection().add(item); // Reveal mode (default)
        auto handler = attach_handler(view);

        handler->begin_swipe(swipe_direction::right);
        handler->swipe_to(80); // 80 >= 60 → past the open threshold
        handler->end_swipe();

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->state.state, swipe_machine_state::open);
        EXPECT_TRUE(view.is_open());
    }

    TEST(swipe_view_seam, execute_mode_invokes_the_first_visible_item_then_closes)
    {
        swipe_item invoked_item;
        swipe_item second_item;
        int invoked = 0;
        invoked_item.invoked.connect([&invoked] { ++invoked; });
        int second_invoked = 0;
        second_item.invoked.connect([&second_invoked] { ++second_invoked; });

        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->set_behavior_on_invoked(swipe_behavior_on_invoked::automatic);
        items->add(invoked_item);
        items->add(second_item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items)); // a LEFT swipe reveals the RIGHT items
        auto handler = attach_handler(view);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80); // left swipe → negative offset, |80| >= 60
        handler->end_swipe();

        EXPECT_EQ(invoked, 1);        // only the FIRST visible item is executed (#7580)
        EXPECT_EQ(second_invoked, 0); // the second item is NOT executed
        EXPECT_FALSE(view.is_open()); // Auto behavior in Execute mode closes after invoke
    }

    TEST(swipe_view_seam, execute_mode_remain_open_keeps_it_open)
    {
        swipe_item item;
        int invoked = 0;
        item.invoked.connect([&invoked] { ++invoked; });
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->set_behavior_on_invoked(swipe_behavior_on_invoked::remain_open);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items));
        auto handler = attach_handler(view);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80);
        handler->end_swipe();

        EXPECT_EQ(invoked, 1);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->state.state, swipe_machine_state::open); // RemainOpen keeps it open
    }

    TEST(swipe_view_seam, execute_mode_skips_a_collapsed_item)
    {
        swipe_item hidden_item;
        hidden_item.set_is_visible(false);
        swipe_item visible_item;
        int hidden_invoked = 0;
        hidden_item.invoked.connect([&hidden_invoked] { ++hidden_invoked; });
        int visible_invoked = 0;
        visible_item.invoked.connect([&visible_invoked] { ++visible_invoked; });

        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->add(hidden_item);  // collapsed → skipped
        items->add(visible_item); // the first VISIBLE item is the one executed
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items));
        auto handler = attach_handler(view);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80);
        handler->end_swipe();

        EXPECT_EQ(hidden_invoked, 0);
        EXPECT_EQ(visible_invoked, 1);
    }

    TEST(swipe_view_seam, a_disabled_item_is_not_invoked) // ExecuteSwipeItem enabled gate
    {
        swipe_item item;
        item.set_is_enabled(false);
        int invoked = 0;
        item.invoked.connect([&invoked] { ++invoked; });
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::execute);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items));
        auto handler = attach_handler(view);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80);
        handler->end_swipe();

        EXPECT_EQ(invoked, 0); // a disabled item is skipped by ExecuteSwipeItem
    }
} // namespace

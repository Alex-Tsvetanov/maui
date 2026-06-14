// Tests for carousel_view — the cross-platform behavior over the W3-29 items/collection infra: the
// constructor snap defaults, the Position / CurrentItem TwoWay choreography (command → event → hook),
// Loop, and the scroll-to funnel a Position change drives through the reused collection_view_handler.
// Ported from src/Controls/tests/Core.UnitTests/CarouselViewTests.cs. Backend-agnostic (the simulator
// is the shared platform recipe on every backend). §8: the items collection (publisher) is declared
// before the view/handler (subscriber).

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/core/observable_collection.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::carousel_view;
    using maui::controls::collection_view_handler;
    using maui::controls::collection_view_platform;
    using maui::controls::current_item_changed_event_args;
    using maui::controls::items_layout_orientation;
    using maui::controls::position_changed_event_args;
    using maui::controls::scroll_to_position;
    using maui::controls::snap_points_alignment;
    using maui::controls::snap_points_type;
    using maui::core::observable_collection;

    using string_collection = observable_collection<std::string>;

    // ---- constructor + defaults (CarouselViewTests.TestConstructorAndDefaults) ----

    TEST(carousel_view, constructor_and_defaults)
    {
        const carousel_view carousel;
        EXPECT_EQ(carousel.items_source(), nullptr);       // ItemsSource null
        EXPECT_EQ(carousel.item_template(), nullptr);      // ItemTemplate null
        ASSERT_NE(carousel.items_layout(), nullptr);       // ItemsLayout NotNull
        EXPECT_EQ(carousel.position(), 0);                 // Position == 0
        EXPECT_TRUE(carousel.loop());                      // Loop default true
        EXPECT_TRUE(carousel.is_swipe_enabled());          // IsSwipeEnabled default true
        EXPECT_TRUE(carousel.is_bounce_enabled());         // IsBounceEnabled default true
        EXPECT_TRUE(carousel.is_scroll_animated());        // IsScrollAnimated default true
        EXPECT_FALSE(carousel.current_item().has_value()); // CurrentItem null
    }

    // The default ItemsLayout is the carousel snap layout: horizontal, MandatorySingle, Center.
    TEST(carousel_view, default_layout_is_horizontal_mandatory_single_center)
    {
        const carousel_view carousel;
        const auto& layout = carousel.items_layout();
        ASSERT_NE(layout, nullptr);
        EXPECT_EQ(layout->orientation(), items_layout_orientation::horizontal);
        EXPECT_EQ(layout->snap_points_type(), snap_points_type::mandatory_single);
        EXPECT_EQ(layout->snap_points_alignment(), snap_points_alignment::center);
    }

    // ---- Position changed command (CarouselViewTests.TestPositionChangedCommand) ----

    TEST(carousel_view, position_changed_command_fires_once)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"1", "2", "3"}); // publisher FIRST (§8)
        carousel_view carousel;
        carousel.set_items_source(items);

        int count_fired = 0;
        carousel.position_changed_command = [&count_fired] { ++count_fired; };
        carousel.set_position(1);
        EXPECT_EQ(count_fired, 1);
    }

    // ---- Position changed event (CarouselViewTests.TestPositionChangedEvent) ----

    TEST(carousel_view, position_changed_event_fires_once_with_old_and_new)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"1", "2", "3"});
        carousel_view carousel;
        carousel.set_items_source(items);

        int count_fired = 0;
        position_changed_event_args observed{};
        auto token = carousel.position_changed.connect([&](const position_changed_event_args& args) {
            ++count_fired;
            observed = args;
        });
        carousel.set_position(1);
        EXPECT_EQ(count_fired, 1);
        EXPECT_EQ(observed.previous_position, 0);
        EXPECT_EQ(observed.current_position, 1);
        carousel.position_changed.disconnect(token);
    }

    // ---- CurrentItem changed command (CarouselViewTests.TestCurrentItemChangedCommand) ----

    TEST(carousel_view, current_item_changed_command_fires_once)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"1", "2", "3"});
        carousel_view carousel;
        carousel.set_items_source(items);

        int count_fired = 0;
        carousel.current_item_changed_command = [&count_fired] { ++count_fired; };
        carousel.set_current_item(boxed_item::of<std::string>("2"));
        EXPECT_EQ(count_fired, 1);
    }

    // ---- CurrentItem changed event (CarouselViewTests.TestCurrentItemChangedEvent) ----

    TEST(carousel_view, current_item_changed_event_fires_once)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"1", "2", "3"});
        carousel_view carousel;
        carousel.set_items_source(items);

        int count_fired = 0;
        current_item_changed_event_args observed{};
        auto token = carousel.current_item_changed.connect([&](const current_item_changed_event_args& args) {
            ++count_fired;
            observed = args;
        });
        carousel.set_current_item(boxed_item::of<std::string>("2"));
        EXPECT_EQ(count_fired, 1);
        EXPECT_EQ(observed.current_item.text(), "2");
        carousel.current_item_changed.disconnect(token);
    }

    // ---- Loop is settable (it is OneTime in C#; defaults true) ----

    TEST(carousel_view, loop_is_settable)
    {
        carousel_view carousel;
        EXPECT_TRUE(carousel.loop());
        carousel.set_loop(false);
        EXPECT_FALSE(carousel.loop());
    }

    // ---- a Position change funnels a centered scroll-to through the handler ----

    TEST(carousel_view, position_change_drives_a_centered_scroll_to_request)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"A", "B", "C", "D", "E"}); // publisher FIRST
        carousel_view carousel;
        auto handler = std::make_shared<collection_view_handler>();
        carousel.set_items_source(items);
        carousel.set_handler(handler);

        collection_view_platform* const platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        const std::size_t before = platform->scroll_requests.size();

        carousel.set_position(2);

        ASSERT_EQ(platform->scroll_requests.size(), before + 1);
        const auto& request = platform->scroll_requests.back();
        EXPECT_EQ(request.index, 2);
        EXPECT_EQ(request.scroll_to_position, scroll_to_position::center);
    }

    // ---- without a handler, a Position change is silent on the scroll seam (dismiss_scroll) ----

    TEST(carousel_view, position_change_without_handler_is_safe)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"A", "B", "C"});
        carousel_view carousel;
        carousel.set_items_source(items);
        // No handler attached: the event/command still fire, but scroll_to is a no-op (no crash).
        int count_fired = 0;
        carousel.position_changed_command = [&count_fired] { ++count_fired; };
        carousel.set_position(1);
        EXPECT_EQ(count_fired, 1);
        EXPECT_EQ(carousel.position(), 1);
    }

    // ---- the carousel's snap layout reaches the handler's platform mirror ----

    TEST(carousel_view, snap_layout_reaches_the_handler_mirror)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"A", "B", "C"}); // publisher FIRST (§8)
        carousel_view carousel;
        auto handler = std::make_shared<collection_view_handler>();
        carousel.set_items_source(items);
        carousel.set_handler(handler);

        collection_view_platform* const platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // The carousel's default horizontal MandatorySingle/Center snap layout flows to the mirror.
        EXPECT_EQ(platform->orientation, items_layout_orientation::horizontal);
        EXPECT_EQ(platform->snap_points_type, snap_points_type::mandatory_single);
        EXPECT_EQ(platform->snap_points_alignment, snap_points_alignment::center);
    }

    // ---- AddRemove items keeps Position valid (CarouselViewTests.TestAddRemoveItems) ----

    TEST(carousel_view, add_then_remove_keeps_position_zero)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{}); // publisher FIRST (§8)
        carousel_view carousel;
        auto handler = std::make_shared<collection_view_handler>();
        carousel.set_items_source(items);
        carousel.set_handler(handler);

        items->add("1");
        items->add("2");

        carousel.scroll_to(1, -1, scroll_to_position::center, false);
        items->remove("2");

        EXPECT_EQ(carousel.position(), 0);
    }
} // namespace

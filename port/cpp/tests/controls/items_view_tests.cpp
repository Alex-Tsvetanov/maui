// Tests for the items_view hierarchy surface + the items layouts. The measurement and
// binding-context cases port src/Controls/tests/Core.UnitTests/ItemsViewTests.cs verbatim; the rest
// are characterization tests of ItemsView.cs / StructuredItemsView.cs / ItemsLayout.cs /
// LinearItemsLayout.cs / GridItemsLayout.cs / ReorderableItemsView.cs (no C# unit suite exists for
// those member bodies). §8: publishers before subscribers throughout.

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::collection_view;
    using maui::controls::collection_view_handler;
    using maui::controls::grid_items_layout;
    using maui::controls::items_layout_orientation;
    using maui::controls::items_updating_scroll_mode;
    using maui::controls::linear_items_layout;
    using maui::controls::scroll_to_mode;
    using maui::controls::scroll_to_request_event_args;
    using maui::controls::snap_points_alignment;
    using maui::controls::snap_points_type;
    using maui::controls::structured_items_view;
    using maui::devices::display_info;

    constexpr double infinity = std::numeric_limits<double>::infinity();

    // The MockDeviceDisplay the C# ItemsViewTests installs.
    class mock_device_display final : public maui::devices::i_device_display
    {
    public:
        explicit mock_device_display(display_info info) : info_(info)
        {
        }
        [[nodiscard]] bool keep_screen_on() const override
        {
            return keep_screen_on_;
        }
        void set_keep_screen_on(bool value) override
        {
            keep_screen_on_ = value;
        }
        [[nodiscard]] display_info main_display_info() const override
        {
            return info_;
        }
        maui::core::connection_token add_main_display_info_changed(
            maui::core::move_only_function<void(const display_info&)> handler) override
        {
            return changed_.connect(std::move(handler));
        }
        bool remove_main_display_info_changed(maui::core::connection_token token) override
        {
            return changed_.disconnect(token);
        }

    private:
        display_info info_;
        bool keep_screen_on_ = false;
        maui::core::event<const display_info&> changed_;
    };

    class items_view_test : public ::testing::Test
    {
    public:
        ~items_view_test() override
        {
            maui::devices::device_display::set_current(nullptr);
        }
        items_view_test(const items_view_test&) = delete;
        items_view_test(items_view_test&&) = delete;
        items_view_test& operator=(const items_view_test&) = delete;
        items_view_test& operator=(items_view_test&&) = delete;

    protected:
        items_view_test()
        {
            // 1920x1080 @ density 2 → the scaled screen size is 960x540.
            maui::devices::device_display::set_current(
                std::make_shared<mock_device_display>(display_info{.width = 1920, .height = 1080, .density = 2}));
        }
    };

    // ---- ItemsViewTests.cs ----

    TEST_F(items_view_test, vertical_list_measurement)
    {
        structured_items_view view;
        view.set_handler(std::make_shared<collection_view_handler>());

        const maui::graphics::size request = view.measure(infinity, infinity);

        EXPECT_DOUBLE_EQ(request.width, 960);
        EXPECT_DOUBLE_EQ(request.height, 540);
    }

    TEST_F(items_view_test, horizontal_list_measurement)
    {
        structured_items_view view;
        view.set_handler(std::make_shared<collection_view_handler>());
        view.set_items_layout(linear_items_layout::create_horizontal_default());

        const maui::graphics::size request = view.measure(infinity, infinity);

        EXPECT_DOUBLE_EQ(request.width, 960);
        EXPECT_DOUBLE_EQ(request.height, 540);
    }

    TEST_F(items_view_test, binding_context_propagates_layouts)
    {
        struct view_model
        {
        };
        auto context = std::make_shared<view_model>();
        structured_items_view view;
        view.set_binding_context(context);
        auto layout = linear_items_layout::create_horizontal_default();
        view.set_items_layout(layout);

        // BindingContext is set when ItemsLayout is set.
        EXPECT_EQ(layout->binding_context<view_model>(), context);

        // BindingContext is updated when it changes on the ItemsView.
        context = std::make_shared<view_model>();
        view.set_binding_context(context);
        EXPECT_EQ(layout->binding_context<view_model>(), context);
    }

    // ---- ItemsView surface (characterization of ItemsView.cs) ----

    TEST(items_view_surface, defaults)
    {
        structured_items_view view;
        EXPECT_EQ(view.items_source(), nullptr);
        EXPECT_EQ(view.item_template(), nullptr);
        EXPECT_FALSE(view.empty_view().has_value());
        EXPECT_EQ(view.empty_view_template(), nullptr);
        EXPECT_EQ(view.remaining_items_threshold(), -1);
        EXPECT_EQ(view.items_updating_scroll_mode(), items_updating_scroll_mode::keep_items_in_view);
        EXPECT_EQ(view.horizontal_scroll_bar_visibility(), maui::core::scroll_bar_visibility::default_);
        EXPECT_EQ(view.vertical_scroll_bar_visibility(), maui::core::scroll_bar_visibility::default_);
        // The default ItemsLayout is a fresh vertical linear layout.
        const auto& layout = view.items_layout();
        ASSERT_NE(layout, nullptr);
        EXPECT_EQ(layout->orientation(), items_layout_orientation::vertical);
        EXPECT_NE(std::dynamic_pointer_cast<linear_items_layout>(layout), nullptr);
    }

    TEST(items_view_surface, remaining_items_threshold_validates_at_minus_one)
    {
        structured_items_view view;
        view.set_remaining_items_threshold(3);
        EXPECT_EQ(view.remaining_items_threshold(), 3);
        view.set_remaining_items_threshold(-5); // validateValue rejects values below -1
        EXPECT_EQ(view.remaining_items_threshold(), 3);
        view.set_remaining_items_threshold(-1);
        EXPECT_EQ(view.remaining_items_threshold(), -1);
    }

    TEST(items_view_surface, send_remaining_items_threshold_runs_event_then_command)
    {
        structured_items_view view;
        std::vector<std::string> order;
        const maui::core::connection_token token =
            view.remaining_items_threshold_reached.connect([&order] { order.emplace_back("event"); });
        view.remaining_items_threshold_reached_command = [&order] { order.emplace_back("command"); };

        view.send_remaining_items_threshold_reached();

        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "event"); // C#: the event raises before the command executes
        EXPECT_EQ(order[1], "command");
        view.remaining_items_threshold_reached.disconnect(token);
    }

    TEST(items_view_surface, scroll_to_is_dismissed_without_a_handler_or_source)
    {
        structured_items_view view;
        int requests = 0;
        const maui::core::connection_token token =
            view.scroll_to_requested.connect([&requests](const scroll_to_request_event_args&) { ++requests; });

        view.scroll_to(1); // no handler AND no ItemsSource → DismissScroll
        EXPECT_EQ(requests, 0);

        view.set_items_source(std::vector<std::string>{"A", "B"});
        view.scroll_to(1); // still no handler
        EXPECT_EQ(requests, 0);

        view.set_handler(std::make_shared<collection_view_handler>());
        view.scroll_to(1);
        EXPECT_EQ(requests, 1);
        view.scroll_to_requested.disconnect(token);
    }

    TEST(items_view_surface, scroll_to_carries_the_request_shape)
    {
        structured_items_view view;
        view.set_items_source(std::vector<std::string>{"A", "B", "C"});
        view.set_handler(std::make_shared<collection_view_handler>());
        std::vector<scroll_to_request_event_args> seen;
        const maui::core::connection_token token = view.scroll_to_requested.connect(
            [&seen](const scroll_to_request_event_args& args) { seen.push_back(args); });

        view.scroll_to(2, -1, maui::controls::scroll_to_position::center, /*animate=*/false);
        view.scroll_to(boxed_item::of(std::string{"B"}), {}, maui::controls::scroll_to_position::end, true);

        ASSERT_EQ(seen.size(), 2U);
        EXPECT_EQ(seen[0].mode, scroll_to_mode::position);
        EXPECT_EQ(seen[0].index, 2);
        EXPECT_EQ(seen[0].group_index, -1);
        EXPECT_EQ(seen[0].scroll_to_position, maui::controls::scroll_to_position::center);
        EXPECT_FALSE(seen[0].is_animated);
        EXPECT_EQ(seen[1].mode, scroll_to_mode::element);
        EXPECT_EQ(seen[1].item.text(), "B");
        EXPECT_TRUE(seen[1].is_animated);
        view.scroll_to_requested.disconnect(token);
    }

    // ---- StructuredItemsView surface ----

    TEST(structured_items_view_surface, header_and_footer_round_trip)
    {
        structured_items_view view;
        EXPECT_FALSE(view.header().has_value());
        EXPECT_FALSE(view.footer().has_value());
        view.set_header(boxed_item::of(std::string{"Top"}));
        view.set_footer(boxed_item::of(std::string{"Bottom"}));
        EXPECT_EQ(view.header().text(), "Top");
        EXPECT_EQ(view.footer().text(), "Bottom");
        EXPECT_EQ(view.item_sizing_strategy(), maui::controls::item_sizing_strategy::measure_all_items);
        view.set_item_sizing_strategy(maui::controls::item_sizing_strategy::measure_first_item);
        EXPECT_EQ(view.item_sizing_strategy(), maui::controls::item_sizing_strategy::measure_first_item);
    }

    // ---- items layouts ----

    TEST(items_layouts, linear_defaults_and_validation)
    {
        linear_items_layout layout{items_layout_orientation::horizontal};
        EXPECT_EQ(layout.orientation(), items_layout_orientation::horizontal);
        EXPECT_DOUBLE_EQ(layout.item_spacing(), 0);
        EXPECT_EQ(layout.snap_points_type(), snap_points_type::none);
        EXPECT_EQ(layout.snap_points_alignment(), snap_points_alignment::start);

        layout.set_item_spacing(8);
        EXPECT_DOUBLE_EQ(layout.item_spacing(), 8);
        layout.set_item_spacing(-1); // validateValue rejects negatives
        EXPECT_DOUBLE_EQ(layout.item_spacing(), 8);
    }

    TEST(items_layouts, linear_static_instances_are_shared_and_defaults_are_fresh)
    {
        EXPECT_EQ(linear_items_layout::vertical(), linear_items_layout::vertical());
        EXPECT_EQ(linear_items_layout::vertical()->orientation(), items_layout_orientation::vertical);
        EXPECT_EQ(linear_items_layout::horizontal()->orientation(), items_layout_orientation::horizontal);
        EXPECT_NE(linear_items_layout::create_vertical_default(), linear_items_layout::create_vertical_default());
    }

    TEST(items_layouts, grid_defaults_and_validation)
    {
        grid_items_layout layout{2, items_layout_orientation::vertical};
        EXPECT_EQ(layout.span(), 2);
        EXPECT_DOUBLE_EQ(layout.vertical_item_spacing(), 0);
        EXPECT_DOUBLE_EQ(layout.horizontal_item_spacing(), 0);

        layout.set_span(0); // validateValue rejects spans below 1
        EXPECT_EQ(layout.span(), 2);
        layout.set_vertical_item_spacing(-2);
        EXPECT_DOUBLE_EQ(layout.vertical_item_spacing(), 0);
        layout.set_horizontal_item_spacing(4);
        EXPECT_DOUBLE_EQ(layout.horizontal_item_spacing(), 4);

        grid_items_layout defaulted{items_layout_orientation::horizontal};
        EXPECT_EQ(defaulted.span(), 1);
    }

    // ---- ReorderableItemsView surface ----

    TEST(reorderable_items_view_surface, defaults_and_send_reorder_completed)
    {
        collection_view view;
        EXPECT_FALSE(view.can_reorder_items());
        EXPECT_FALSE(view.can_mix_groups());

        int completed = 0;
        const maui::core::connection_token token = view.reorder_completed.connect([&completed] { ++completed; });
        view.send_reorder_completed();
        EXPECT_EQ(completed, 1);
        view.reorder_completed.disconnect(token);

        view.set_can_reorder_items(true);
        view.set_can_mix_groups(true);
        EXPECT_TRUE(view.can_reorder_items());
        EXPECT_TRUE(view.can_mix_groups());
    }
} // namespace

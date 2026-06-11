// Tests for the slider control (maui::controls::slider <= Slider) and the headless handler seam. The
// control half ports src/Controls/tests/Core.UnitTests/SliderUnitTests.cs — the constructor, the
// six set-order clamping theories, the requested-value recoercion cases, ValueChanged args and the
// drag channel; the seam half follows the headless conventions (button_tests.cpp).
#include "maui/controls/slider.hpp"

#include <memory>
#include <stdexcept>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>
#include <utility>

namespace
{
    using maui::controls::slider;
    using maui::core::i_element_handler;
    using maui::core::slider_handler;
    using maui::graphics::color;

    // ---- constructor (SliderUnitTests.TestConstructor / TestInvalidConstructor / clamping) ----

    TEST(slider, constructor_sets_min_max_value)
    {
        const slider control(20, 200, 50);
        EXPECT_EQ(control.minimum(), 20);
        EXPECT_EQ(control.maximum(), 200);
        EXPECT_EQ(control.value(), 50);
    }

    TEST(slider, invalid_constructor_throws)
    {
        EXPECT_THROW(slider(10, 5, 10), std::out_of_range);
    }

    TEST(slider, constructor_clamps_the_value)
    {
        const slider control(50, 100, 0);
        EXPECT_EQ(control.value(), 50);
    }

    TEST(slider, defaults_are_zero_to_one)
    {
        const slider control;
        EXPECT_EQ(control.minimum(), 0);
        EXPECT_EQ(control.maximum(), 1);
        EXPECT_EQ(control.value(), 0);
    }

    // ---- the six set-order theories (SetProperties_*_Order; the shared InlineData rows) ----

    struct order_case
    {
        double min;
        double max;
        double value;
    };

    class slider_set_order : public ::testing::TestWithParam<order_case>
    {
    };

    TEST_P(slider_set_order, every_set_order_converges)
    {
        const auto [min, max, value] = GetParam();
        const auto expect = [min = min, max = max, value = value](const slider& control) {
            EXPECT_EQ(control.minimum(), min);
            EXPECT_EQ(control.maximum(), max);
            EXPECT_EQ(control.value(), value);
        };

        slider a; // Min, Max, Value
        a.set_minimum(min);
        a.set_maximum(max);
        a.set_value(value);
        expect(a);

        slider b; // Min, Value, Max
        b.set_minimum(min);
        b.set_value(value);
        b.set_maximum(max);
        expect(b);

        slider c; // Max, Min, Value
        c.set_maximum(max);
        c.set_minimum(min);
        c.set_value(value);
        expect(c);

        slider d; // Max, Value, Min
        d.set_maximum(max);
        d.set_value(value);
        d.set_minimum(min);
        expect(d);

        slider e; // Value, Min, Max
        e.set_value(value);
        e.set_minimum(min);
        e.set_maximum(max);
        expect(e);

        slider f; // Value, Max, Min
        f.set_value(value);
        f.set_maximum(max);
        f.set_minimum(min);
        expect(f);
    }

    INSTANTIATE_TEST_SUITE_P(set_orders, slider_set_order,
                             ::testing::Values(order_case{10, 100, 50}, order_case{0, 1, 0.5}, order_case{-100, 100, 0},
                                               order_case{50, 150, 100}));

    // ---- requested-value recoercion (the _requestedValue oracle) ----

    TEST(slider, requested_value_preserved_across_multiple_range_changes)
    {
        slider control;
        control.set_value(50);
        control.set_minimum(-10);
        control.set_maximum(-1); // value clamped to -1
        EXPECT_EQ(control.value(), -1);

        control.set_maximum(-2); // still clamped, not corrupted
        EXPECT_EQ(control.value(), -2);

        control.set_maximum(100); // the original requested value (50) restored
        EXPECT_EQ(control.value(), 50);
    }

    TEST(slider, requested_value_preserved_when_minimum_changes_multiple_times)
    {
        slider control;
        control.set_value(5);
        control.set_maximum(100);
        control.set_minimum(10); // clamped to 10
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(20); // clamped to 20
        EXPECT_EQ(control.value(), 20);

        control.set_minimum(0); // the requested value (5) restored
        EXPECT_EQ(control.value(), 5);
    }

    TEST(slider, value_clamped_when_only_the_range_changes)
    {
        slider control; // value defaults to 0, never user-set
        control.set_minimum(10);
        control.set_maximum(100);
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(5); // 10 fits [5, 100] — stays
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(15); // clamps up to 15
        EXPECT_EQ(control.value(), 15);
    }

    TEST(slider, min_value_clamp)
    {
        slider control(0, 100, 0);
        control.set_minimum(10);
        EXPECT_EQ(control.value(), 10);
        EXPECT_EQ(control.minimum(), 10);
    }

    TEST(slider, max_value_clamp)
    {
        slider control(0, 100, 100);
        control.set_maximum(10);
        EXPECT_EQ(control.value(), 10);
        EXPECT_EQ(control.maximum(), 10);
    }

    TEST(slider, inverted_range_sets_are_legal)
    {
        // TestInvalidMaxValue / TestInvalidMinValue: no validation on Minimum/Maximum.
        slider a;
        a.set_maximum(a.minimum() - 1);
        EXPECT_EQ(a.maximum(), -1);
        slider b;
        b.set_minimum(b.maximum() + 1);
        EXPECT_EQ(b.minimum(), 2);
    }

    // ---- ValueChanged (TestValueChanged / SliderValueChangedEventArgs) ----

    TEST(slider, value_changed_fires)
    {
        slider control;
        bool changed = false;
        control.value_changed.connect([&changed](double, double) { changed = true; });
        control.set_value(control.value() + 1);
        EXPECT_TRUE(changed);
    }

    TEST(slider, value_changed_args_carry_old_and_new)
    {
        for (const auto [initial, final_value] : {std::pair{0.0, 1.0}, std::pair{1.0, 0.5}})
        {
            slider control;
            control.set_minimum(0.0);
            control.set_maximum(1.0);
            control.set_value(initial);

            double old_value = 0.0;
            double new_value = 0.0;
            control.value_changed.connect([&old_value, &new_value](double old_v, double new_v) {
                old_value = old_v;
                new_value = new_v;
            });
            control.set_value(final_value);
            EXPECT_EQ(old_value, initial);
            EXPECT_EQ(new_value, final_value);
        }
    }

    // ---- the drag channel (TestDragStarted / TestDragCompleted + the IsEnabled gate) ----

    TEST(slider, drag_started_raises_event_command_first)
    {
        slider control;
        int order = 0;
        int command_at = 0;
        int event_at = 0;
        control.drag_started_command = [&] { command_at = ++order; };
        control.drag_started.connect([&] { event_at = ++order; });

        control.send_drag_started();
        EXPECT_EQ(command_at, 1); // DragStartedCommand?.Execute before DragStarted?.Invoke
        EXPECT_EQ(event_at, 2);
    }

    TEST(slider, drag_completed_raises_event)
    {
        slider control;
        bool completed = false;
        control.drag_completed.connect([&completed] { completed = true; });
        control.send_drag_completed();
        EXPECT_TRUE(completed);
    }

    TEST(slider, disabled_slider_suppresses_drag_events)
    {
        slider control;
        control.set_is_enabled(false);
        bool any = false;
        control.drag_started.connect([&any] { any = true; });
        control.drag_completed.connect([&any] { any = true; });
        control.send_drag_started();
        control.send_drag_completed();
        EXPECT_FALSE(any);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(slider_seam, attaching_handler_maps_initial_state)
    {
        slider control(20, 200, 50);
        control.set_thumb_color(color(1.0F, 0.0F, 0.0F));
        control.set_minimum_track_color(color(0.0F, 1.0F, 0.0F));
        control.set_maximum_track_color(color(0.0F, 0.0F, 1.0F));
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->minimum, 20);
        EXPECT_EQ(platform->maximum, 200);
        EXPECT_EQ(platform->value, 50);
        EXPECT_EQ(platform->thumb_color, color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->minimum_track_color, color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(platform->maximum_track_color, color(0.0F, 0.0F, 1.0F));
    }

    TEST(slider_seam, setting_value_updates_the_platform)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(handler->typed_platform_view()->value, 42);
    }

    TEST(slider_seam, native_value_change_flows_back)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // Simulate the user dragging the native thumb (the UISlider.ValueChanged analog).
        auto* platform = handler->typed_platform_view();
        platform->value = 33;
        platform->on_value_changed();

        EXPECT_EQ(control.value(), 33);
        EXPECT_EQ(reported_new, 33);
    }

    TEST(slider_seam, native_value_clamps_through_the_control)
    {
        slider control(0, 10, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        platform->value = 50; // out of range from the native side
        platform->on_value_changed();

        EXPECT_EQ(control.value(), 10); // coerced through the Value clamp
        EXPECT_EQ(platform->value, 10); // and pushed back to the native state
    }

    TEST(slider_seam, native_drag_events_flow_back)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        bool started = false;
        bool completed = false;
        control.drag_started.connect([&started] { started = true; });
        control.drag_completed.connect([&completed] { completed = true; });

        auto* platform = handler->typed_platform_view();
        platform->on_drag_started(); // TouchDown
        EXPECT_TRUE(started);
        platform->on_drag_completed(); // TouchUpInside
        EXPECT_TRUE(completed);
    }

    TEST(slider_seam, clearing_handler_disconnects)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(slider_seam, handler_resolved_from_default_registry)
    {
        // slider -> slider_handler is self-registered in slider.cpp.
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<slider>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<slider_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        slider control(0, 10, 7);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->value, 7);
    }
} // namespace

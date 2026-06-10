// Tests for the stepper control (maui::controls::stepper <= Stepper) and the headless handler seam.
// The control half ports src/Controls/tests/Core.UnitTests/StepperUnitTests.cs — the constructor,
// min/max validation, the increment-digits rounding (SmallIncrements/InitialValue), the six set-order
// theories and the requested-value recoercion; the seam half follows the headless conventions.
#include "maui/controls/stepper.hpp"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/stepper_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::stepper;
    using maui::core::i_element_handler;
    using maui::core::stepper_handler;

    // ---- constructor (TestConstructor / TestInvalidConstructor / TestConstructorClampValue) ----

    TEST(stepper, constructor_sets_min_max_value_increment)
    {
        const stepper control(120, 200, 150, 2);
        EXPECT_EQ(control.minimum(), 120);
        EXPECT_EQ(control.maximum(), 200);
        EXPECT_EQ(control.value(), 150);
        EXPECT_EQ(control.increment(), 2);
    }

    TEST(stepper, invalid_constructor_throws)
    {
        EXPECT_THROW(stepper(100, 0, 50, 1), std::out_of_range);
    }

    TEST(stepper, constructor_clamps_the_value)
    {
        const stepper high(0, 100, 2000, 1);
        EXPECT_EQ(high.value(), 100);
        const stepper low(0, 100, -200, 1);
        EXPECT_EQ(low.value(), 0);
    }

    // ---- min/max validation (TestInvalidMaxValue / TestInvalidMinValue / the valid pair) ----

    TEST(stepper, invalid_max_below_min_is_rejected)
    {
        stepper control;
        control.set_maximum(control.minimum() - 1); // validateValue fails: ignored
        EXPECT_NE(control.minimum(), control.maximum());
        EXPECT_EQ(control.maximum(), 100);
    }

    TEST(stepper, invalid_min_above_max_is_rejected)
    {
        stepper control;
        control.set_minimum(control.maximum() + 1); // validateValue fails: ignored
        EXPECT_NE(control.minimum(), control.maximum());
        EXPECT_EQ(control.minimum(), 0);
    }

    TEST(stepper, valid_max_and_min_apply)
    {
        stepper control;
        control.set_maximum(2000);
        EXPECT_EQ(control.maximum(), 2000);
        control.set_minimum(200);
        EXPECT_EQ(control.minimum(), 200);
    }

    // ---- range-change clamping + the property-changed notifications (TestMinClampValue / Max) ----

    TEST(stepper, min_clamp_raises_minimum_and_value_changes)
    {
        stepper control;
        bool min_raised = false;
        bool value_raised = false;
        control.property_changed.connect([&](std::string_view name) {
            if (name == "minimum")
            {
                min_raised = true;
            }
            if (name == "value")
            {
                value_raised = true;
            }
        });

        control.set_minimum(10);
        EXPECT_EQ(control.minimum(), 10);
        EXPECT_EQ(control.value(), 10);
        EXPECT_TRUE(min_raised);
        EXPECT_TRUE(value_raised);
    }

    TEST(stepper, max_clamp_raises_maximum_and_value_changes)
    {
        stepper control;
        control.set_value(50);
        bool max_raised = false;
        bool value_raised = false;
        control.property_changed.connect([&](std::string_view name) {
            if (name == "maximum")
            {
                max_raised = true;
            }
            if (name == "value")
            {
                value_raised = true;
            }
        });

        control.set_maximum(25);
        EXPECT_EQ(control.maximum(), 25);
        EXPECT_EQ(control.value(), 25);
        EXPECT_TRUE(max_raised);
        EXPECT_TRUE(value_raised);
    }

    // ---- ValueChanged (TestValueChangedEvent / StepperValueChangedEventArgs) ----

    TEST(stepper, value_changed_fires)
    {
        stepper control;
        bool fired = false;
        control.value_changed.connect([&fired](double, double) { fired = true; });
        control.set_value(50);
        EXPECT_TRUE(fired);
    }

    TEST(stepper, value_changed_args_carry_old_and_new)
    {
        const struct
        {
            double initial;
            double final_value;
        } cases[] = {{100.0, 0.5}, {10.0, 25.0}, {0, 39.5}};
        for (const auto& c : cases)
        {
            stepper control;
            control.set_maximum(100);
            control.set_minimum(0);
            control.set_increment(0.5);
            control.set_value(c.initial);

            double old_value = 0.0;
            double new_value = 0.0;
            control.value_changed.connect([&](double old_v, double new_v) {
                old_value = old_v;
                new_value = new_v;
            });
            control.set_value(c.final_value);
            EXPECT_EQ(old_value, c.initial);
            EXPECT_EQ(new_value, c.final_value);
        }
    }

    // ---- SmallIncrements: the increment-digits rounding keeps long walks exact ----

    struct increment_case
    {
        int steps;
        double increment;
        double min;
        double max;
    };

    class stepper_small_increments : public ::testing::TestWithParam<increment_case>
    {
    };

    TEST_P(stepper_small_increments, walk_up_and_back_lands_on_zero)
    {
        const auto [steps, increment, min, max] = GetParam();
        stepper control(min, max, 0, increment);
        // The digits logic copied from the Stepper code (as the C# test does).
        const int digits = std::max(1, std::min(15, static_cast<int>(-std::log10(increment) + 4)));

        EXPECT_EQ(control.value(), 0.0);

        for (int i = 0; i < steps; ++i)
        {
            control.set_value(control.value() + control.increment());
        }
        // .NET Math.Round (half to even) at `digits` — std::nearbyint under the default rounding mode.
        const double power = std::pow(10.0, digits);
        EXPECT_EQ(control.value(), std::nearbyint(control.increment() * steps * power) / power);

        for (int i = 0; i < steps; ++i)
        {
            control.set_value(control.value() - control.increment());
        }
        EXPECT_EQ(control.value(), 0.0);
    }

    INSTANTIATE_TEST_SUITE_P(
        small_increments, stepper_small_increments,
        ::testing::Values(increment_case{100, .5, 0, 100}, increment_case{100, .3, 0, 100},
                          increment_case{100, .03, 0, 100}, increment_case{100, .003, 0, 100},
                          increment_case{100, .0003, 0, 100}, increment_case{100, .0000003, 0, 100},
                          increment_case{100, .0000000003, 0, 100}, increment_case{100, .0000000000003, 0, 100},
                          increment_case{100, .5, -10000, 10000}, increment_case{100, .3, -10000, 10000},
                          increment_case{100, .03, -10000, 10000}, increment_case{100, .003, -10000, 10000},
                          increment_case{100, .0003, -10000, 10000}, increment_case{100, .0000003, -10000, 10000},
                          increment_case{100, .0000000003, -10000, 10000},
                          increment_case{100, .0000000000003, -10000, 10000},
                          // 4 significant digits for the increment — no less, no more (#5168)
                          increment_case{100, .00003456, -10000, 10000}));

    TEST(stepper, initial_value_steps_cleanly) // #10032
    {
        stepper control(0, 10, 4.99, .1);
        EXPECT_EQ(control.value(), 4.99);

        control.set_value(control.value() + control.increment());
        EXPECT_EQ(control.value(), 5.09);
        control.set_value(control.value() + control.increment());
        EXPECT_EQ(control.value(), 5.19);
        control.set_value(control.value() + control.increment());
        EXPECT_EQ(control.value(), 5.29);
        control.set_value(control.value() + control.increment());
        EXPECT_EQ(control.value(), 5.39);
    }

    // ---- the six set-order theories (SetProperties_*_Order; the shared InlineData rows) ----

    struct order_case
    {
        double min;
        double max;
        double value;
    };

    class stepper_set_order : public ::testing::TestWithParam<order_case>
    {
    };

    TEST_P(stepper_set_order, every_set_order_converges)
    {
        const auto [min, max, value] = GetParam();
        const auto expect = [min = min, max = max, value = value](const stepper& control) {
            EXPECT_EQ(control.minimum(), min);
            EXPECT_EQ(control.maximum(), max);
            EXPECT_EQ(control.value(), value);
        };

        stepper a; // Min, Max, Value
        a.set_minimum(min);
        a.set_maximum(max);
        a.set_value(value);
        expect(a);

        stepper b; // Min, Value, Max
        b.set_minimum(min);
        b.set_value(value);
        b.set_maximum(max);
        expect(b);

        stepper c; // Max, Min, Value
        c.set_maximum(max);
        c.set_minimum(min);
        c.set_value(value);
        expect(c);

        stepper d; // Max, Value, Min
        d.set_maximum(max);
        d.set_value(value);
        d.set_minimum(min);
        expect(d);

        stepper e; // Value, Min, Max
        e.set_value(value);
        e.set_minimum(min);
        e.set_maximum(max);
        expect(e);

        stepper f; // Value, Max, Min
        f.set_value(value);
        f.set_maximum(max);
        f.set_minimum(min);
        expect(f);
    }

    INSTANTIATE_TEST_SUITE_P(set_orders, stepper_set_order,
                             ::testing::Values(order_case{10, 200, 50}, order_case{0, 50, 25}, order_case{-100, 100, 0},
                                               order_case{50, 150, 100}));

    // ---- requested-value recoercion (the _requestedValue oracle) ----

    TEST(stepper, requested_value_preserved_across_multiple_range_changes)
    {
        stepper control;
        control.set_value(50);
        control.set_minimum(-10);
        control.set_maximum(-1); // value clamped to -1
        EXPECT_EQ(control.value(), -1);

        control.set_maximum(-2); // still clamped, not corrupted
        EXPECT_EQ(control.value(), -2);

        control.set_maximum(100); // the original requested value (50) restored
        EXPECT_EQ(control.value(), 50);
    }

    TEST(stepper, requested_value_preserved_when_minimum_changes_multiple_times)
    {
        stepper control;
        control.set_value(5);
        control.set_maximum(100);
        control.set_minimum(10); // clamped to 10
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(20); // clamped to 20
        EXPECT_EQ(control.value(), 20);

        control.set_minimum(0); // the requested value (5) restored
        EXPECT_EQ(control.value(), 5);
    }

    TEST(stepper, value_clamped_when_only_the_range_changes)
    {
        stepper control; // value defaults to 0, never user-set
        control.set_minimum(10);
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(5); // 10 fits [5, 100] — stays
        EXPECT_EQ(control.value(), 10);

        control.set_minimum(15); // clamps up to 15
        EXPECT_EQ(control.value(), 15);
    }

    TEST(stepper, minimum_equal_to_maximum_is_legal) // #28330
    {
        stepper control;
        control.set_minimum(1);
        control.set_maximum(1);
        control.set_value(1);

        EXPECT_EQ(control.minimum(), 1);
        EXPECT_EQ(control.maximum(), 1);
        EXPECT_EQ(control.value(), 1);

        control.set_value(control.value() + control.increment()); // beyond max: unchanged
        EXPECT_EQ(control.value(), 1);
        control.set_value(control.value() - control.increment()); // below min: unchanged
        EXPECT_EQ(control.value(), 1);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(stepper_seam, attaching_handler_maps_initial_state)
    {
        stepper control(10, 90, 30, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->minimum, 10);
        EXPECT_EQ(platform->maximum, 90);
        EXPECT_EQ(platform->increment, 5);
        EXPECT_EQ(platform->value, 30);
    }

    TEST(stepper_seam, setting_value_updates_the_platform)
    {
        stepper control(0, 100, 0, 1);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(handler->typed_platform_view()->value, 42);
    }

    TEST(stepper_seam, non_positive_increment_keeps_the_native_step)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->increment, 1);

        control.set_increment(0); // UpdateIncrement only pushes a positive step
        EXPECT_EQ(handler->typed_platform_view()->increment, 1);
    }

    TEST(stepper_seam, native_step_flows_back_rounded)
    {
        stepper control(0, 10, 0, 0.1);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // Simulate the native plus-button tap (the UIStepper.ValueChanged analog) with FP noise: the
        // control's coercion rounds it at the increment digits.
        auto* platform = handler->typed_platform_view();
        platform->value = 0.30000000000000004; // 3 × 0.1 in doubles
        platform->on_value_changed();

        EXPECT_EQ(control.value(), 0.3);
        EXPECT_EQ(reported_new, 0.3);
    }

    TEST(stepper_seam, clearing_handler_disconnects)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(stepper_seam, handler_resolved_from_default_registry)
    {
        // stepper -> stepper_handler is self-registered in stepper.cpp.
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<stepper>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<stepper_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        stepper control(0, 10, 7, 1);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->value, 7);
    }
} // namespace

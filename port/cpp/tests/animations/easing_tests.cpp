// Tests for maui::animations::easing — ported from src/Core/tests/UnitTests/Animations/
// EasingTests.cs (the Linear pass-through + the AllRunFromZeroToOne endpoint sweep; the
// EasingTypeConverter cases are XAML-era and out of this unit's scope), plus value-table cases
// derived from the C# formulas in src/Core/src/Easing.cs (expected values computed with the same
// float literals the oracle uses).
#include "maui/animations/easing.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::animations::easing;

    // EasingTests.Linear: linear is the identity, even outside [0,1].
    class easing_linear_inputs : public ::testing::TestWithParam<double>
    {
    };

    TEST_P(easing_linear_inputs, linear_is_identity)
    {
        const double input = GetParam();
        EXPECT_EQ(easing::linear().ease(input), input);
    }

    INSTANTIATE_TEST_SUITE_P(easing, easing_linear_inputs, ::testing::Values(0.0, 1.0, 2.0, 5.0, 8.0, 9.0, 10.0));

    // EasingTests.AllRunFromZeroToOne: every standard easing maps 0 -> ~0 and 1 -> ~1.
    class easing_endpoints : public ::testing::TestWithParam<double>
    {
    };

    TEST_P(easing_endpoints, all_run_from_zero_to_one)
    {
        const double val = GetParam();
        constexpr double epsilon = 0.001;
        const std::vector<const easing*> all = {&easing::linear(),    &easing::bounce_in(),    &easing::bounce_out(),
                                                &easing::cubic_in(),  &easing::cubic_in_out(), &easing::cubic_out(),
                                                &easing::sin_in(),    &easing::sin_in_out(),   &easing::sin_out(),
                                                &easing::spring_in(), &easing::spring_out()};
        for (const easing* candidate : all)
        {
            EXPECT_LT(std::abs(val - candidate->ease(val)), epsilon);
        }
    }

    INSTANTIATE_TEST_SUITE_P(easing, easing_endpoints, ::testing::Values(0.0, 1.0));

    // The default easing is CubicInOut (C# Easing.Default => CubicInOut).
    TEST(easing, default_easing_is_cubic_in_out)
    {
        for (const double x : {0.1, 0.25, 0.5, 0.75, 0.9})
        {
            EXPECT_DOUBLE_EQ(easing::default_easing().ease(x), easing::cubic_in_out().ease(x));
        }
    }

    // An empty callable throws (C# ArgumentNullException).
    TEST(easing, empty_function_throws)
    {
        EXPECT_THROW(easing{easing::ease_function{}}, std::invalid_argument);
    }

    // A custom callable is applied as-is (C#'s implicit Func -> Easing conversion).
    TEST(easing, custom_function_is_applied)
    {
        const easing doubler{[](double x) { return x * 2; }};
        EXPECT_DOUBLE_EQ(doubler.ease(0.25), 0.5);
    }

    // ---- value tables from the C# formulas (x in {0.1, 0.25, 0.5, 0.75, 0.9}) ----
    struct easing_sample
    {
        const easing* curve;
        double input;
        double expected;
    };

    TEST(easing, value_tables_match_the_oracle_formulas)
    {
        const std::vector<easing_sample> samples = {
            {.curve = &easing::sin_in(), .input = 0.1, .expected = 0.01231165940486223},
            {.curve = &easing::sin_in(), .input = 0.25, .expected = 0.07612046748871326},
            {.curve = &easing::sin_in(), .input = 0.5, .expected = 0.2928932188134524},
            {.curve = &easing::sin_in(), .input = 0.75, .expected = 0.6173165676349102},
            {.curve = &easing::sin_in(), .input = 0.9, .expected = 0.843565534959769},
            {.curve = &easing::sin_out(), .input = 0.1, .expected = 0.15643446504023087},
            {.curve = &easing::sin_out(), .input = 0.25, .expected = 0.3826834323650898},
            {.curve = &easing::sin_out(), .input = 0.5, .expected = 0.7071067811865475},
            {.curve = &easing::sin_out(), .input = 0.75, .expected = 0.9238795325112867},
            {.curve = &easing::sin_out(), .input = 0.9, .expected = 0.9876883405951378},
            {.curve = &easing::sin_in_out(), .input = 0.1, .expected = 0.024471741852423234},
            {.curve = &easing::sin_in_out(), .input = 0.25, .expected = 0.1464466094067262},
            {.curve = &easing::sin_in_out(), .input = 0.5, .expected = 0.5},
            {.curve = &easing::sin_in_out(), .input = 0.75, .expected = 0.8535533905932737},
            {.curve = &easing::sin_in_out(), .input = 0.9, .expected = 0.9755282581475768},
            {.curve = &easing::cubic_in(), .input = 0.1, .expected = 0.001},
            {.curve = &easing::cubic_in(), .input = 0.25, .expected = 0.015625},
            {.curve = &easing::cubic_in(), .input = 0.5, .expected = 0.125},
            {.curve = &easing::cubic_in(), .input = 0.75, .expected = 0.421875},
            {.curve = &easing::cubic_in(), .input = 0.9, .expected = 0.729},
            {.curve = &easing::cubic_out(), .input = 0.1, .expected = 0.2709999999999999},
            {.curve = &easing::cubic_out(), .input = 0.25, .expected = 37.0 / 64.0}, // 0.578125 exactly
            {.curve = &easing::cubic_out(), .input = 0.5, .expected = 0.875},
            {.curve = &easing::cubic_out(), .input = 0.75, .expected = 0.984375},
            {.curve = &easing::cubic_out(), .input = 0.9, .expected = 0.999},
            {.curve = &easing::cubic_in_out(), .input = 0.1, .expected = 0.004},
            {.curve = &easing::cubic_in_out(), .input = 0.25, .expected = 0.0625},
            {.curve = &easing::cubic_in_out(), .input = 0.5, .expected = 0.5},
            {.curve = &easing::cubic_in_out(), .input = 0.75, .expected = 0.9375},
            {.curve = &easing::cubic_in_out(), .input = 0.9, .expected = 0.996},
            {.curve = &easing::bounce_in(), .input = 0.1, .expected = 0.011874993294477165},
            {.curve = &easing::bounce_in(), .input = 0.25, .expected = 0.027343755587935226},
            {.curve = &easing::bounce_in(), .input = 0.5, .expected = 0.2343749888241271},
            {.curve = &easing::bounce_in(), .input = 0.75, .expected = 0.52734375},
            {.curve = &easing::bounce_in(), .input = 0.9, .expected = 0.9243750000000001},
            {.curve = &easing::bounce_out(), .input = 0.1, .expected = 0.07562500000000001},
            {.curve = &easing::bounce_out(), .input = 0.25, .expected = 0.47265625},
            {.curve = &easing::bounce_out(), .input = 0.5, .expected = 0.7656250111758729},
            {.curve = &easing::bounce_out(), .input = 0.75, .expected = 0.9726562444120648},
            {.curve = &easing::bounce_out(), .input = 0.9, .expected = 0.9881250067055228},
            {.curve = &easing::spring_in(), .input = 0.1, .expected = -0.0143142204284668},
            {.curve = &easing::spring_in(), .input = 0.25, .expected = -0.0641365647315979},
            {.curve = &easing::spring_in(), .input = 0.5, .expected = -0.08769750595092773},
            {.curve = &easing::spring_in(), .input = 0.75, .expected = 0.1825903058052063},
            {.curve = &easing::spring_in(), .input = 0.9, .expected = 0.5911720161437991},
            {.curve = &easing::spring_out(), .input = 0.1, .expected = 0.4088279838562009},
            {.curve = &easing::spring_out(), .input = 0.25, .expected = 0.8174096941947937},
            {.curve = &easing::spring_out(), .input = 0.5, .expected = 1.0876975059509277},
            {.curve = &easing::spring_out(), .input = 0.75, .expected = 1.064136564731598},
            {.curve = &easing::spring_out(), .input = 0.9, .expected = 1.0143142204284668},
        };
        for (const auto& sample : samples)
        {
            EXPECT_NEAR(sample.curve->ease(sample.input), sample.expected, 1e-12) << "at x = " << sample.input;
        }
    }
} // namespace

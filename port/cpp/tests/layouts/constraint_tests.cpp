// Ported from src/Core/tests/UnitTests/Layouts/ConstraintTests.cs (LayoutManager.ResolveConstraints) plus
// the per-child ViewHandlerExtensions.ResolveConstraints rule (layout_manager::resolve_size_request).
#include "maui/layouts/layout_manager.hpp"

#include <limits>

#include "maui/core/dimension.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::layouts::layout_manager;

    constexpr double inf = std::numeric_limits<double>::infinity();
    const double unset = maui::core::dimension::unset; // NaN
    // "no minimum" / "no maximum" aliases for the per-child resolve calls: an unset min resolves to 0, an
    // unset max is +inf. Named distinctly so the (measured, exact, min, max) call sites read clearly.
    const double no_min = maui::core::dimension::unset;
    const double no_max = inf;

    // ---- LayoutManager.ResolveConstraints (the layout-level external/explicit/measured + [min,max]) ----

    TEST(constraints, external_wins_over_desired_and_measured)
    {
        EXPECT_EQ(layout_manager::resolve_constraints(100, 200, 130), 100);
        EXPECT_EQ(layout_manager::resolve_constraints(100, unset, 130), 100);
    }

    TEST(constraints, measured_wins_if_nothing_else_applies)
    {
        EXPECT_EQ(layout_manager::resolve_constraints(inf, unset, 245), 245);
    }

    TEST(constraints, requested_takes_precedence_over_measured)
    {
        EXPECT_EQ(layout_manager::resolve_constraints(inf, 90, 245), 90);
    }

    // ---- ViewHandlerExtensions.ResolveConstraints (the per-child measured/exact + [min,max]) ----

    TEST(size_request, measured_used_when_no_exact)
    {
        // Unset exact + no min (resolves to 0) + no max: the measured size passes through.
        EXPECT_EQ(layout_manager::resolve_size_request(123, unset, no_min, no_max), 123);
    }

    TEST(size_request, exact_overrides_measured)
    {
        EXPECT_EQ(layout_manager::resolve_size_request(123, 50, no_min, no_max), 50);
    }

    TEST(size_request, max_caps_measured_and_exact)
    {
        EXPECT_EQ(layout_manager::resolve_size_request(200, unset, no_min, 80), 80); // measured capped
        EXPECT_EQ(layout_manager::resolve_size_request(10, 200, no_min, 80), 80);    // exact capped (max wins)
    }

    TEST(size_request, min_floors_and_beats_max)
    {
        EXPECT_EQ(layout_manager::resolve_size_request(10, unset, 50, no_max), 50); // measured floored
        EXPECT_EQ(layout_manager::resolve_size_request(200, unset, 75, 50), 75);    // min beats max
        EXPECT_EQ(layout_manager::resolve_size_request(10, 5, 75, 50), 75);         // min beats exact and max
    }

    TEST(size_request, unset_minimum_resolves_to_zero)
    {
        // An unset (NaN) minimum is treated as 0, so it never raises the value.
        EXPECT_EQ(layout_manager::resolve_size_request(0, unset, no_min, no_max), 0);
    }
} // namespace

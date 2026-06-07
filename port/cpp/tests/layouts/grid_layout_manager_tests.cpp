// Ported from src/Core/tests/UnitTests/Layouts/GridLayoutManagerTests.cs — the behavioral oracle for
// the Grid algorithm: implied/absolute/auto/star sizing, spacing, padding, spans, and arrange offsets.
// A representative subset of the (very large) C# suite covering every sizing path.
#include "maui/layouts/grid_layout_manager.hpp"

#include <limits>

#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::grid_length;
    using maui::core::grid_unit_type;
    using maui::core::visibility;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::grid_layout_manager;
    using maui::layouts::testing::grid_fixture;
    using maui::layouts::testing::mock_view;

    constexpr double inf = std::numeric_limits<double>::infinity();

    grid_length absolute(double v)
    {
        return {v};
    }
    grid_length weighted_star(double weight)
    {
        return {weight, grid_unit_type::star};
    }

    // measure(inf, inf) then arrange at the measured size from (0,0).
    size measure_and_arrange_auto(grid_fixture& fixture)
    {
        grid_layout_manager manager(fixture.grid);
        const size measured = manager.measure(inf, inf);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));
        return measured;
    }

    // measure(w, h) then arrange at exactly (w, h) from (0,0) — for star/fixed-constraint cases.
    size measure_and_arrange_fixed(grid_fixture& fixture, double width, double height)
    {
        grid_layout_manager manager(fixture.grid);
        const size measured = manager.measure(width, height);
        manager.arrange_children(rect(0, 0, width, height));
        return measured;
    }

    // ---- implied + absolute ----

    TEST(grid, one_auto_row_one_auto_column)
    {
        grid_fixture fixture;
        auto& view = fixture.add_view({100, 100}, 0, 0);
        measure_and_arrange_auto(fixture);
        EXPECT_EQ(view.last_arrange, rect(0, 0, 100, 100)); // implied */* measured at infinity
    }

    TEST(grid, two_absolute_columns_one_absolute_row)
    {
        grid_fixture fixture;
        fixture.add_row(absolute(10));
        fixture.add_column(absolute(100));
        fixture.add_column(absolute(100));
        auto& view0 = fixture.add_view({10, 10}, 0, 0);
        auto& view1 = fixture.add_view({10, 10}, 0, 1);

        measure_and_arrange_auto(fixture);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 10));
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 100, 10));
    }

    TEST(grid, two_absolute_rows_and_columns)
    {
        grid_fixture fixture;
        fixture.add_row(absolute(10));
        fixture.add_row(absolute(30));
        fixture.add_column(absolute(100));
        fixture.add_column(absolute(100));
        auto& view0 = fixture.add_view({10, 10}, 0, 0);
        auto& view1 = fixture.add_view({10, 10}, 0, 1);
        auto& view2 = fixture.add_view({10, 10}, 1, 0);
        auto& view3 = fixture.add_view({10, 10}, 1, 1);

        measure_and_arrange_auto(fixture);

        // Each child is measured at its absolute cell size.
        EXPECT_EQ(view0.last_measure_width, 100);
        EXPECT_EQ(view0.last_measure_height, 10);
        EXPECT_EQ(view2.last_measure_height, 30);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 10));
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 100, 10));
        EXPECT_EQ(view2.last_arrange, rect(0, 10, 100, 30));
        EXPECT_EQ(view3.last_arrange, rect(100, 10, 100, 30));
    }

    TEST(grid, two_absolute_columns_one_auto_row)
    {
        grid_fixture fixture;
        fixture.add_column(absolute(100));
        fixture.add_column(absolute(100));
        auto& view0 = fixture.add_view({10, 10}, 0, 0);
        auto& view1 = fixture.add_view({10, 10}, 0, 1);

        measure_and_arrange_auto(fixture);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 10));
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 100, 10));
    }

    // ---- spacing ----

    TEST(grid, single_row_ignores_row_spacing)
    {
        grid_fixture fixture;
        fixture.grid.row_spacing_value = 10;
        auto& view = fixture.add_view({100, 100}, 0, 0);
        measure_and_arrange_auto(fixture);
        EXPECT_EQ(view.last_arrange, rect(0, 0, 100, 100));
    }

    TEST(grid, two_rows_with_spacing)
    {
        grid_fixture fixture;
        fixture.add_row(absolute(100));
        fixture.add_row(absolute(100));
        fixture.grid.row_spacing_value = 10;
        auto& view0 = fixture.add_view({100, 100}, 0, 0);
        auto& view1 = fixture.add_view({100, 100}, 1, 0);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.height, 100 + 100 + 10);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(0, 110, 100, 100));
    }

    TEST(grid, two_columns_with_spacing)
    {
        grid_fixture fixture;
        fixture.add_column(absolute(100));
        fixture.add_column(absolute(100));
        fixture.grid.column_spacing_value = 10;
        auto& view0 = fixture.add_view({100, 100}, 0, 0);
        auto& view1 = fixture.add_view({100, 100}, 0, 1);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.width, 100 + 100 + 10);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(110, 0, 100, 100));
    }

    TEST(grid, empty_auto_rows_have_no_height)
    {
        grid_fixture fixture;
        fixture.add_row(absolute(100));
        fixture.add_row(grid_length::automatic());
        fixture.add_row(absolute(100));
        auto& view0 = fixture.add_view({100, 100}, 0, 0);
        auto& view2 = fixture.add_view({100, 100}, 2, 0);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.height, 200); // the empty auto row contributes 0
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view2.last_arrange, rect(0, 100, 100, 100));
    }

    TEST(grid, empty_rows_still_count_for_spacing)
    {
        grid_fixture fixture;
        fixture.add_row(absolute(100));
        fixture.add_row(grid_length::automatic());
        fixture.add_row(absolute(100));
        fixture.grid.row_spacing_value = 10;
        fixture.add_view({100, 100}, 0, 0);
        auto& view2 = fixture.add_view({100, 100}, 2, 0);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.height, 100 + 100 + 10 + 10); // two spacing gaps, even with an empty middle row
        EXPECT_EQ(view2.last_arrange, rect(0, 120, 100, 100));
    }

    // ---- star ----

    TEST(grid, single_star_column_fills_width)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::star());
        auto& view0 = fixture.add_view({100, 100}, 0, 0);

        measure_and_arrange_fixed(fixture, 400, 600);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 400, 100)); // star col → full width; auto row → content height
    }

    TEST(grid, single_weighted_star_column_fills_width)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_column(weighted_star(3));
        auto& view0 = fixture.add_view({100, 100}, 0, 0);

        measure_and_arrange_fixed(fixture, 400, 600);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 400, 100)); // only column → gets full width regardless of weight
    }

    TEST(grid, multiple_star_columns_split_equally)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::star());
        fixture.add_column(grid_length::star());
        fixture.add_column(grid_length::star());
        auto& view0 = fixture.add_view({50, 50}, 0, 0);
        auto& view1 = fixture.add_view({50, 50}, 0, 1);
        auto& view2 = fixture.add_view({50, 50}, 0, 2);

        measure_and_arrange_fixed(fixture, 300, 600);

        constexpr double w = 100;               // 300 / 3
        EXPECT_EQ(view1.last_measure_width, w); // measured at the column width, not the full grid width
        EXPECT_EQ(view0.last_arrange, rect(0, 0, w, 50));
        EXPECT_EQ(view1.last_arrange, rect(w, 0, w, 50));
        EXPECT_EQ(view2.last_arrange, rect(w * 2, 0, w, 50));
    }

    TEST(grid, weighted_star_columns_get_proportional_space)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::star()); // 1*
        fixture.add_column(weighted_star(2));    // 2*
        auto& view0 = fixture.add_view({50, 50}, 0, 0);
        auto& view1 = fixture.add_view({50, 50}, 0, 1);

        measure_and_arrange_fixed(fixture, 300, 600);

        constexpr double w0 = 100; // 1/3 of 300
        constexpr double w1 = 200; // 2/3 of 300
        EXPECT_EQ(view0.last_arrange, rect(0, 0, w0, 50));
        EXPECT_EQ(view1.last_arrange, rect(w0, 0, w1, 50));
    }

    TEST(grid, empty_star_column_at_infinite_width_is_zero)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::star());

        grid_layout_manager manager(fixture.grid);
        const size measured = manager.measure(inf, inf);

        EXPECT_EQ(measured.width, 0); // empty star measured at infinite width collapses to 0
    }

    // ---- spans ----

    TEST(grid, view_spans_rows)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_row(grid_length::automatic());
        auto& view0 = fixture.add_view({100, 100}, 0, 0, /*row_span=*/2);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.width, 100);
        EXPECT_EQ(measured.height, 100);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
    }

    TEST(grid, view_spans_rows_with_other_views)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::automatic());
        fixture.add_column(grid_length::automatic());
        auto& view0 = fixture.add_view({100, 100}, 0, 0, /*row_span=*/2);
        auto& view1 = fixture.add_view({50, 50}, 1, 1);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.width, 150);
        EXPECT_EQ(measured.height, 100);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(100, 25, 50, 75));
    }

    TEST(grid, row_spanning_accounts_for_spacing)
    {
        grid_fixture fixture;
        fixture.add_row(grid_length::automatic());
        fixture.add_row(grid_length::automatic());
        fixture.add_column(grid_length::automatic());
        fixture.add_column(grid_length::automatic());
        fixture.grid.row_spacing_value = 5;
        auto& view0 = fixture.add_view({100, 100}, 0, 0, /*row_span=*/2);
        auto& view1 = fixture.add_view({50, 50}, 0, 1);
        auto& view2 = fixture.add_view({50, 50}, 1, 1);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.width, 150);
        EXPECT_EQ(measured.height, 50 + 50 + 5);
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 50, 50));
        EXPECT_EQ(view2.last_arrange, rect(100, 55, 50, 50));
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 105)); // spans both rows + the gap between them
    }

    // ---- padding + min/max ----

    TEST(grid, measure_and_arrange_account_for_padding)
    {
        grid_fixture fixture;
        fixture.add_column(absolute(100));
        fixture.add_row(absolute(100));
        fixture.grid.padding_value = maui::core::thickness(10, 20, 30, 40);
        auto& view0 = fixture.add_view({100, 100}, 0, 0);

        const size measured = measure_and_arrange_auto(fixture);

        EXPECT_EQ(measured.width, 100 + 10 + 30);
        EXPECT_EQ(measured.height, 100 + 20 + 40);
        EXPECT_EQ(view0.last_arrange, rect(10, 20, 100, 100)); // offset by padding's left/top
    }

    TEST(grid, measured_size_respects_max)
    {
        grid_fixture fixture;
        fixture.add_column(absolute(100));
        fixture.add_row(absolute(100));
        fixture.grid.max_width_value = 50;
        fixture.add_view({100, 100}, 0, 0);

        grid_layout_manager manager(fixture.grid);
        EXPECT_EQ(manager.measure(inf, inf).width, 50);
    }
} // namespace

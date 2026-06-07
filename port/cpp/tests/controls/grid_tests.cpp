// Tests for the grid control + its headless handler seam — the grid wraps the M3b grid_layout_manager
// and hosts children on a native panel. Three things are verified: (1) the control's measure/arrange
// delegate to the manager and reproduce the grid geometry (a representative subset of
// GridLayoutManagerTests, now driven THROUGH the control instead of a mock_grid); (2) the attached-cell
// properties (Row/Column/RowSpan/ColumnSpan) default + validate exactly as C# Grid; and (3) the headless
// layout_platform's child count tracks the control's add/insert/remove/clear so the panel stays in sync.
#include "maui/controls/grid.hpp"

#include <limits>
#include <memory>

#include "maui/controls/column_definition.hpp"
#include "maui/controls/row_definition.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_grid_layout.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::column_definition;
    using maui::controls::grid;
    using maui::controls::row_definition;
    using maui::core::grid_length;
    using maui::core::grid_unit_type;
    using maui::core::i_element_handler;
    using maui::core::i_grid_layout;
    using maui::core::i_layout;
    using maui::core::layout_handler;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
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

    // A child positioned in the grid: a mock_view added to the control with its cell coordinates set.
    mock_view& place(grid& g, std::unique_ptr<mock_view>& owner, size measured_size, int row, int column,
                     int row_span = 1, int column_span = 1)
    {
        owner = std::make_unique<mock_view>();
        owner->configure(measured_size);
        mock_view& view = *owner;
        g.add(view);
        g.set_row(view, row);
        g.set_column(view, column);
        g.set_row_span(view, row_span);
        g.set_column_span(view, column_span);
        return view;
    }

    // ---- defaults + accessors ----

    TEST(grid_control, defaults_empty_with_zero_spacing_and_padding)
    {
        grid g;
        EXPECT_EQ(g.count(), 0);
        EXPECT_EQ(g.row_definition_count(), 0);
        EXPECT_EQ(g.column_definition_count(), 0);
        EXPECT_EQ(g.row_spacing(), 0.0);
        EXPECT_EQ(g.column_spacing(), 0.0);
        EXPECT_EQ(g.padding(), thickness());
    }

    TEST(grid_control, add_definitions_grows_the_collections)
    {
        grid g;
        g.add_row_definition(absolute(50));
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::star());

        ASSERT_EQ(g.row_definition_count(), 2);
        ASSERT_EQ(g.column_definition_count(), 1);
        EXPECT_EQ(g.row_definition_at(0).height(), grid_length(50));
        EXPECT_TRUE(g.row_definition_at(1).height().is_auto());
        EXPECT_TRUE(g.column_definition_at(0).width().is_star());
    }

    TEST(grid_control, definition_default_height_and_width_are_star)
    {
        // C# RowDefinition()/ColumnDefinition() default to GridLength.Star.
        EXPECT_TRUE(row_definition{}.height().is_star());
        EXPECT_TRUE(column_definition{}.width().is_star());
    }

    TEST(grid_control, spacing_setters_round_trip_through_the_interface)
    {
        grid g;
        g.set_row_spacing(7);
        g.set_column_spacing(11);

        i_grid_layout& as_grid = g;
        EXPECT_EQ(as_grid.row_spacing(), 7.0);
        EXPECT_EQ(as_grid.column_spacing(), 11.0);
    }

    // ---- attached cell properties: defaults + validation ----

    TEST(grid_control, unpositioned_child_uses_default_cell)
    {
        grid g;
        mock_view child;
        g.add(child);

        // C# attached-property defaults: Row/Column default(int)==0, RowSpan/ColumnSpan==1.
        EXPECT_EQ(g.get_row(child), 0);
        EXPECT_EQ(g.get_column(child), 0);
        EXPECT_EQ(g.get_row_span(child), 1);
        EXPECT_EQ(g.get_column_span(child), 1);
    }

    TEST(grid_control, set_cell_properties_round_trip)
    {
        grid g;
        mock_view child;
        g.add(child);
        g.set_row(child, 2);
        g.set_column(child, 3);
        g.set_row_span(child, 4);
        g.set_column_span(child, 5);

        EXPECT_EQ(g.get_row(child), 2);
        EXPECT_EQ(g.get_column(child), 3);
        EXPECT_EQ(g.get_row_span(child), 4);
        EXPECT_EQ(g.get_column_span(child), 5);
    }

    TEST(grid_control, negative_row_or_column_is_ignored)
    {
        grid g;
        mock_view child;
        g.add(child);
        g.set_row(child, 5);
        g.set_column(child, 6);

        g.set_row(child, -1);    // invalid (< 0): C# validateValue rejects -> value unchanged
        g.set_column(child, -1); // invalid (< 0)

        EXPECT_EQ(g.get_row(child), 5);
        EXPECT_EQ(g.get_column(child), 6);
    }

    TEST(grid_control, span_below_one_is_ignored)
    {
        grid g;
        mock_view child;
        g.add(child);
        g.set_row_span(child, 3);
        g.set_column_span(child, 4);

        g.set_row_span(child, 0);     // invalid (< 1): rejected
        g.set_column_span(child, -2); // invalid (< 1): rejected

        EXPECT_EQ(g.get_row_span(child), 3);
        EXPECT_EQ(g.get_column_span(child), 4);
    }

    TEST(grid_control, removing_a_child_drops_its_cell_info)
    {
        grid g;
        mock_view child;
        g.add(child);
        g.set_row(child, 4);
        EXPECT_EQ(g.get_row(child), 4);

        g.remove_at(0);
        g.add(child); // re-added: the prior cell info must not linger -> back to the default
        EXPECT_EQ(g.get_row(child), 0);
    }

    TEST(grid_control, clear_drops_all_cell_info)
    {
        grid g;
        mock_view a;
        mock_view b;
        g.add(a);
        g.add(b);
        g.set_row(a, 1);
        g.set_row(b, 2);

        g.clear();
        g.add(a);
        EXPECT_EQ(g.get_row(a), 0); // cleared back to default
    }

    // ---- geometry through the control (ported from grid_layout_manager_tests) ----

    TEST(grid_control, two_absolute_rows_and_columns_arrange)
    {
        grid g;
        g.add_row_definition(absolute(10));
        g.add_row_definition(absolute(30));
        g.add_column_definition(absolute(100));
        g.add_column_definition(absolute(100));
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        std::unique_ptr<mock_view> o2;
        std::unique_ptr<mock_view> o3;
        auto& view0 = place(g, o0, {10, 10}, 0, 0);
        auto& view1 = place(g, o1, {10, 10}, 0, 1);
        auto& view2 = place(g, o2, {10, 10}, 1, 0);
        auto& view3 = place(g, o3, {10, 10}, 1, 1);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 10));
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 100, 10));
        EXPECT_EQ(view2.last_arrange, rect(0, 10, 100, 30));
        EXPECT_EQ(view3.last_arrange, rect(100, 10, 100, 30));
    }

    TEST(grid_control, two_rows_with_spacing)
    {
        grid g;
        g.add_row_definition(absolute(100));
        g.add_row_definition(absolute(100));
        g.set_row_spacing(10);
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        auto& view0 = place(g, o0, {100, 100}, 0, 0);
        auto& view1 = place(g, o1, {100, 100}, 1, 0);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(measured.height, 100 + 100 + 10);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(0, 110, 100, 100));
    }

    TEST(grid_control, multiple_star_columns_split_equally)
    {
        grid g;
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::star());
        g.add_column_definition(grid_length::star());
        g.add_column_definition(grid_length::star());
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        std::unique_ptr<mock_view> o2;
        auto& view0 = place(g, o0, {50, 50}, 0, 0);
        auto& view1 = place(g, o1, {50, 50}, 0, 1);
        auto& view2 = place(g, o2, {50, 50}, 0, 2);

        g.measure(300, 600);
        g.arrange(rect(0, 0, 300, 600));

        constexpr double w = 100; // 300 / 3
        EXPECT_EQ(view1.last_measure_width, w);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, w, 50));
        EXPECT_EQ(view1.last_arrange, rect(w, 0, w, 50));
        EXPECT_EQ(view2.last_arrange, rect(w * 2, 0, w, 50));
    }

    TEST(grid_control, weighted_star_columns_get_proportional_space)
    {
        grid g;
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::star()); // 1*
        g.add_column_definition(weighted_star(2));    // 2*
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        auto& view0 = place(g, o0, {50, 50}, 0, 0);
        auto& view1 = place(g, o1, {50, 50}, 0, 1);

        g.measure(300, 600);
        g.arrange(rect(0, 0, 300, 600));

        constexpr double w0 = 100; // 1/3 of 300
        constexpr double w1 = 200; // 2/3 of 300
        EXPECT_EQ(view0.last_arrange, rect(0, 0, w0, 50));
        EXPECT_EQ(view1.last_arrange, rect(w0, 0, w1, 50));
    }

    TEST(grid_control, view_spans_rows_with_other_views)
    {
        grid g;
        g.add_row_definition(grid_length::automatic());
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::automatic());
        g.add_column_definition(grid_length::automatic());
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        auto& view0 = place(g, o0, {100, 100}, 0, 0, /*row_span=*/2);
        auto& view1 = place(g, o1, {50, 50}, 1, 1);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(measured.width, 150);
        EXPECT_EQ(measured.height, 100);
        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(100, 25, 50, 75));
    }

    TEST(grid_control, measure_and_arrange_account_for_padding)
    {
        grid g;
        g.add_column_definition(absolute(100));
        g.add_row_definition(absolute(100));
        g.set_padding(thickness(10, 20, 30, 40));
        std::unique_ptr<mock_view> o0;
        auto& view0 = place(g, o0, {100, 100}, 0, 0);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(measured.width, 100 + 10 + 30);
        EXPECT_EQ(measured.height, 100 + 20 + 40);
        EXPECT_EQ(view0.last_arrange, rect(10, 20, 100, 100)); // offset by padding's left/top
    }

    TEST(grid_control, implied_single_cell_arranges_at_measured_size)
    {
        // No explicit definitions: an implied single star row/column, measured at infinity (matching
        // GridLayoutManagerTests.one_auto_row_one_auto_column).
        grid g;
        std::unique_ptr<mock_view> o0;
        auto& view = place(g, o0, {100, 100}, 0, 0);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(view.last_arrange, rect(0, 0, 100, 100));
    }

    // ---- the handler seam (control <-> handler <-> headless panel) ----

    TEST(grid_seam, attaching_handler_creates_panel)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &g);
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 0U);
    }

    TEST(grid_seam, panel_child_count_tracks_add_remove_clear)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        mock_view a;
        mock_view b;
        g.add(a); // -> handler->invoke("add", …) -> map_add -> add()
        g.add(b);
        EXPECT_EQ(platform->children.size(), 2U);
        EXPECT_EQ(platform->children[0], &a);
        EXPECT_EQ(platform->children[1], &b);

        g.remove_at(0);
        EXPECT_EQ(platform->children.size(), 1U);
        EXPECT_EQ(platform->children[0], &b);

        g.clear();
        EXPECT_EQ(platform->children.size(), 0U);
    }

    TEST(grid_seam, arrange_with_handler_still_positions_children)
    {
        // With a handler attached, arrange both sizes the host panel AND positions children via the
        // manager — the panel-sizing must not displace child arrangement.
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);
        g.add_row_definition(absolute(100));
        g.add_column_definition(absolute(100));
        g.add_column_definition(absolute(100));
        std::unique_ptr<mock_view> o0;
        std::unique_ptr<mock_view> o1;
        auto& view0 = place(g, o0, {100, 100}, 0, 0);
        auto& view1 = place(g, o1, {100, 100}, 0, 1);

        const size measured = g.measure(inf, inf);
        g.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(view1.last_arrange, rect(100, 0, 100, 100));
    }

    TEST(grid_seam, handler_resolved_from_default_registry)
    {
        // grid -> layout_handler is self-registered (MAUI_REGISTER_HANDLER), reusing the layout handler.
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<grid>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<layout_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        grid g;
        g.set_handler(handler);
        mock_view child;
        g.add(child);
        EXPECT_EQ(resolved->typed_platform_view()->children.size(), 1U);
    }

    TEST(grid_control, usable_through_layout_interface)
    {
        grid g;
        mock_view child;
        g.add(child);

        i_layout& as_layout = g;
        EXPECT_EQ(as_layout.count(), 1);
        EXPECT_EQ(&as_layout.at(0), &child);
    }
} // namespace

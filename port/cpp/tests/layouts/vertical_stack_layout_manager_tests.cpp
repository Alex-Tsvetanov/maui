// Ported from src/Core/tests/UnitTests/Layouts/VerticalStackLayoutManagerTests.cs — the behavioral
// oracle for vertical stacking (spacing, padding, min/max, collapsed/hidden visibility, fill).
#include "maui/core/dimension.hpp"
#include "maui/layouts/vertical_stack_layout_manager.hpp"

#include <array>
#include <limits>

#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::thickness;
    using maui::core::visibility;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::vertical_stack_layout_manager;
    using maui::layouts::testing::stack_fixture;

    constexpr double inf = std::numeric_limits<double>::infinity();

    TEST(vertical_stack, spacing_measurement)
    {
        struct spacing_case
        {
            int view_count;
            double view_height;
            double spacing;
            double expected_height;
        };
        for (const auto& test :
             {spacing_case{.view_count = 0, .view_height = 100, .spacing = 0, .expected_height = 0},
              spacing_case{.view_count = 1, .view_height = 100, .spacing = 0, .expected_height = 100},
              spacing_case{.view_count = 1, .view_height = 100, .spacing = 13, .expected_height = 100},
              spacing_case{.view_count = 2, .view_height = 100, .spacing = 13, .expected_height = 213},
              spacing_case{.view_count = 3, .view_height = 100, .spacing = 13, .expected_height = 326},
              spacing_case{.view_count = 3, .view_height = 100, .spacing = -13, .expected_height = 274}})
        {
            stack_fixture fixture;
            fixture.build_stack(test.view_count, 100, test.view_height);
            fixture.stack.spacing_value = test.spacing;

            vertical_stack_layout_manager manager(fixture.stack);
            EXPECT_EQ(manager.measure(100, inf).height, test.expected_height) << "view_count=" << test.view_count;
        }
    }

    TEST(vertical_stack, spacing_has_no_effect_with_one_item)
    {
        for (const double spacing : {0.0, 26.0, -54.0})
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.spacing_value = spacing;

            vertical_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(100, inf);
            manager.arrange_children(rect(0, 0, measured.width, measured.height));

            EXPECT_EQ(fixture.stack.children[0], &fixture.stack.at(0));
            EXPECT_EQ(dynamic_cast<maui::layouts::testing::mock_view&>(fixture.stack.at(0)).last_arrange,
                      rect(0, 0, 100, 100));
        }
    }

    TEST(vertical_stack, spacing_applies_between_two_items)
    {
        for (const double spacing : {26.0, -54.0})
        {
            stack_fixture fixture;
            auto& first = fixture.add_view({100, 100});
            auto& second = fixture.add_view({100, 100});
            fixture.stack.spacing_value = spacing;

            vertical_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(inf, 100);
            manager.arrange_children(rect(0, 0, measured.width, measured.height));

            EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
            EXPECT_EQ(second.last_arrange, rect(0, 100 + spacing, 100, 100));
        }
    }

    TEST(vertical_stack, stack_applies_height)
    {
        struct height_case
        {
            double view_height;
            double stack_height;
            double expected;
        };
        for (const auto& test :
             {height_case{.view_height = 150, .stack_height = 100, .expected = 100},
              height_case{.view_height = 150, .stack_height = 200, .expected = 200},
              height_case{.view_height = 1250, .stack_height = maui::core::dimension::unset, .expected = 1250}})
        {
            stack_fixture fixture;
            fixture.add_view({100, test.view_height});
            fixture.stack.height_value = test.stack_height;

            vertical_stack_layout_manager manager(fixture.stack);
            EXPECT_EQ(manager.measure(100, inf).height, test.expected);
        }
    }

    TEST(vertical_stack, ignores_collapsed_views)
    {
        stack_fixture fixture;
        auto& visible = fixture.add_view({100, 100});
        auto& collapsed = fixture.add_view({100, 100});
        collapsed.visibility_value = visibility::collapsed;

        vertical_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(100, inf);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_GE(visible.measure_count, 1);
        EXPECT_GE(visible.arrange_count, 1);
        EXPECT_EQ(collapsed.measure_count, 0);
        EXPECT_EQ(collapsed.arrange_count, 0);
    }

    TEST(vertical_stack, does_not_ignore_hidden_views)
    {
        stack_fixture fixture;
        auto& visible = fixture.add_view({100, 100});
        auto& hidden = fixture.add_view({100, 100});
        hidden.visibility_value = visibility::hidden;

        vertical_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(100, inf);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_GE(visible.measure_count, 1);
        EXPECT_GE(hidden.measure_count, 1);
        EXPECT_GE(hidden.arrange_count, 1);
    }

    TEST(vertical_stack, measure_accounts_for_padding)
    {
        for (const auto& padding : {thickness(0, 0, 0, 0), thickness(10, 10, 10, 10), thickness(10, 0, 10, 0),
                                    thickness(0, 10, 0, 10), thickness(23, 5, 3, 15)})
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.padding_value = padding;

            vertical_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(inf, inf);

            EXPECT_EQ(measured.height, padding.vertical_thickness() + 100);
            EXPECT_EQ(measured.width, padding.horizontal_thickness() + 100);
        }
    }

    TEST(vertical_stack, arrange_accounts_for_padding)
    {
        for (const auto& padding : {thickness(0, 0, 0, 0), thickness(10, 10, 10, 10), thickness(10, 0, 10, 0),
                                    thickness(0, 10, 0, 10), thickness(23, 5, 3, 15)})
        {
            stack_fixture fixture;
            auto& child = fixture.add_view({100, 100});
            fixture.stack.padding_value = padding;

            vertical_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(inf, inf);
            manager.arrange_children(rect(0, 0, measured.width, measured.height));

            EXPECT_EQ(child.last_arrange, rect(padding.left, padding.top, 100, 100));
        }
    }

    TEST(vertical_stack, arrange_respects_bounds)
    {
        stack_fixture fixture;
        auto& child = fixture.add_view({100, 100});

        vertical_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(inf, 100);
        manager.arrange_children(rect(10, 15, measured.width, measured.height));

        EXPECT_EQ(child.last_arrange, rect(10, 15, 100, 100));
    }

    TEST(vertical_stack, measure_respects_min_max)
    {
        // {set value, view size, expected} for each of max-height, max-width, min-height, min-width.
        struct clamp_case
        {
            double limit;
            double view_size;
            double expected;
        };
        const auto max_cases = std::to_array<clamp_case>({{.limit = 50, .view_size = 100, .expected = 50},
                                                          {.limit = 100, .view_size = 100, .expected = 100},
                                                          {.limit = 100, .view_size = 50, .expected = 50},
                                                          {.limit = 0, .view_size = 50, .expected = 0}});
        for (const auto& test : max_cases)
        {
            stack_fixture h;
            h.build_stack(1, 100, test.view_size);
            h.stack.max_height_value = test.limit;
            EXPECT_EQ(vertical_stack_layout_manager(h.stack).measure(inf, inf).height, test.expected);

            stack_fixture w;
            w.build_stack(1, test.view_size, 100);
            w.stack.max_width_value = test.limit;
            EXPECT_EQ(vertical_stack_layout_manager(w.stack).measure(inf, inf).width, test.expected);
        }
        const auto min_cases = std::to_array<clamp_case>({{.limit = 50, .view_size = 10, .expected = 50},
                                                          {.limit = 100, .view_size = 100, .expected = 100},
                                                          {.limit = 10, .view_size = 50, .expected = 50}});
        for (const auto& test : min_cases)
        {
            stack_fixture h;
            h.build_stack(1, 100, test.view_size);
            h.stack.min_height_value = test.limit;
            EXPECT_EQ(vertical_stack_layout_manager(h.stack).measure(inf, inf).height, test.expected);

            stack_fixture w;
            w.build_stack(1, test.view_size, 100);
            w.stack.min_width_value = test.limit;
            EXPECT_EQ(vertical_stack_layout_manager(w.stack).measure(inf, inf).width, test.expected);
        }
    }

    TEST(vertical_stack, min_and_max_dominate)
    {
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.width_value = 75;
            fixture.stack.max_width_value = 50;
            EXPECT_EQ(vertical_stack_layout_manager(fixture.stack).measure(inf, inf).width, 50); // max beats explicit
        }
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.min_width_value = 75;
            fixture.stack.max_width_value = 50;
            EXPECT_EQ(vertical_stack_layout_manager(fixture.stack).measure(inf, inf).width, 75); // min beats max
        }
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.height_value = 75;
            fixture.stack.max_height_value = 50;
            EXPECT_EQ(vertical_stack_layout_manager(fixture.stack).measure(inf, inf).height, 50);
        }
        {
            stack_fixture fixture;
            fixture.build_stack(1, 100, 100);
            fixture.stack.min_height_value = 75;
            fixture.stack.max_height_value = 50;
            EXPECT_EQ(vertical_stack_layout_manager(fixture.stack).measure(inf, inf).height, 75);
        }
    }

    TEST(vertical_stack, arrange_accounts_for_fill)
    {
        stack_fixture fixture;
        fixture.build_stack(1, 100, 100);

        vertical_stack_layout_manager manager(fixture.stack);
        (void)manager.measure(inf, inf);
        const size actual = manager.arrange_children(rect(0, 0, 1000, 1000));

        EXPECT_EQ(actual.width, 1000);
        EXPECT_EQ(actual.height, 1000);
    }

    TEST(vertical_stack, child_measure_accounts_for_padding)
    {
        struct padding_case
        {
            thickness padding;
            double expected_width_constraint{};
        };
        for (const auto& test : {padding_case{.padding = thickness(0), .expected_width_constraint = 100},
                                 padding_case{.padding = thickness(10), .expected_width_constraint = 80},
                                 padding_case{.padding = thickness(0, 10, 0, 10), .expected_width_constraint = 100},
                                 padding_case{.padding = thickness(7, 0, 14, 0), .expected_width_constraint = 79}})
        {
            stack_fixture fixture;
            auto& child = fixture.add_view({50, 50});
            fixture.stack.padding_value = test.padding;

            vertical_stack_layout_manager manager(fixture.stack);
            (void)manager.measure(100, 100);

            EXPECT_EQ(child.last_measure_width, test.expected_width_constraint);
            EXPECT_EQ(child.last_measure_height, inf);
        }
    }

    TEST(vertical_stack, collapsed_items_do_not_incur_spacing)
    {
        constexpr double view_width = 7;
        constexpr double view_height = 5;
        constexpr double spacing = 10;
        for (const int collapsed_index : {0, 1, 2})
        {
            stack_fixture fixture;
            fixture.build_stack(3, view_width, view_height);
            fixture.stack.spacing_value = spacing;
            dynamic_cast<maui::layouts::testing::mock_view&>(fixture.stack.at(collapsed_index)).visibility_value =
                visibility::collapsed;

            vertical_stack_layout_manager manager(fixture.stack);
            // Two visible items remain -> one spacing gap.
            EXPECT_EQ(manager.measure(inf, inf).height, view_height + spacing + view_height)
                << "collapsed_index=" << collapsed_index;
        }
    }

    TEST(vertical_stack, all_collapsed_items_have_no_spacing)
    {
        stack_fixture fixture;
        fixture.build_stack(3, 7, 5);
        fixture.stack.spacing_value = 10;
        for (int n = 0; n < 3; ++n)
        {
            dynamic_cast<maui::layouts::testing::mock_view&>(fixture.stack.at(n)).visibility_value =
                visibility::collapsed;
        }

        vertical_stack_layout_manager manager(fixture.stack);
        EXPECT_EQ(manager.measure(inf, inf).height, 0);
    }
} // namespace

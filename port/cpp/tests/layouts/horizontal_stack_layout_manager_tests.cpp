// Ported from src/Core/tests/UnitTests/Layouts/HorizontalStackLayoutManagerTests.cs — the behavioral
// oracle for horizontal stacking (the width/height mirror image of the vertical manager).
#include "maui/layouts/horizontal_stack_layout_manager.hpp"

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
    using maui::layouts::horizontal_stack_layout_manager;
    using maui::layouts::testing::stack_fixture;

    constexpr double inf = std::numeric_limits<double>::infinity();

    TEST(horizontal_stack, spacing_measurement)
    {
        struct spacing_case
        {
            int view_count;
            double view_width;
            double spacing;
            double expected_width;
        };
        for (const auto& test :
             {spacing_case{0, 100, 0, 0}, spacing_case{1, 100, 0, 100}, spacing_case{1, 100, 13, 100},
              spacing_case{2, 100, 13, 213}, spacing_case{3, 100, 13, 326}, spacing_case{3, 100, -13, 274}})
        {
            stack_fixture fixture;
            fixture.build_stack(test.view_count, test.view_width, 100);
            fixture.stack.spacing_value = test.spacing;

            horizontal_stack_layout_manager manager(fixture.stack);
            EXPECT_EQ(manager.measure(inf, 100).width, test.expected_width) << "view_count=" << test.view_count;
        }
    }

    TEST(horizontal_stack, spacing_applies_between_two_items)
    {
        for (const double spacing : {26.0, -54.0})
        {
            stack_fixture fixture;
            auto& first = fixture.add_view({100, 100});
            auto& second = fixture.add_view({100, 100});
            fixture.stack.spacing_value = spacing;

            horizontal_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(inf, 100);
            manager.arrange_children(rect(0, 0, measured.width, measured.height));

            EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
            EXPECT_EQ(second.last_arrange, rect(100 + spacing, 0, 100, 100));
        }
    }

    TEST(horizontal_stack, ltr_first_item_on_the_left)
    {
        stack_fixture fixture;
        auto& first = fixture.add_view({100, 100});
        auto& second = fixture.add_view({100, 100});

        horizontal_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(inf, 100);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(second.last_arrange, rect(100, 0, 100, 100));
    }

    TEST(horizontal_stack, stack_applies_width)
    {
        struct width_case
        {
            double view_width;
            double stack_width;
            double expected;
        };
        for (const auto& test : {width_case{150, 100, 100}, width_case{150, 200, 200},
                                 width_case{1250, maui::core::dimension::unset, 1250}})
        {
            stack_fixture fixture;
            fixture.add_view({test.view_width, 100});
            fixture.stack.width_value = test.stack_width;

            horizontal_stack_layout_manager manager(fixture.stack);
            EXPECT_EQ(manager.measure(inf, 100).width, test.expected);
        }
    }

    TEST(horizontal_stack, ignores_collapsed_views)
    {
        stack_fixture fixture;
        auto& visible = fixture.add_view({100, 100});
        auto& collapsed = fixture.add_view({100, 100});
        collapsed.visibility_value = visibility::collapsed;

        horizontal_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(inf, 100);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_GE(visible.measure_count, 1);
        EXPECT_GE(visible.arrange_count, 1);
        EXPECT_EQ(collapsed.measure_count, 0);
        EXPECT_EQ(collapsed.arrange_count, 0);
    }

    TEST(horizontal_stack, does_not_ignore_hidden_views)
    {
        stack_fixture fixture;
        auto& visible = fixture.add_view({100, 100});
        auto& hidden = fixture.add_view({100, 100});
        hidden.visibility_value = visibility::hidden;

        horizontal_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(inf, 100);
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_GE(visible.measure_count, 1);
        EXPECT_GE(hidden.measure_count, 1);
        EXPECT_GE(hidden.arrange_count, 1);
    }

    TEST(horizontal_stack, measure_and_arrange_account_for_padding)
    {
        for (const auto& padding : {thickness(0, 0, 0, 0), thickness(10, 10, 10, 10), thickness(10, 0, 10, 0),
                                    thickness(0, 10, 0, 10), thickness(23, 5, 3, 15)})
        {
            stack_fixture fixture;
            auto& child = fixture.add_view({100, 100});
            fixture.stack.padding_value = padding;

            horizontal_stack_layout_manager manager(fixture.stack);
            const size measured = manager.measure(inf, inf);
            EXPECT_EQ(measured.height, padding.vertical_thickness() + 100);
            EXPECT_EQ(measured.width, padding.horizontal_thickness() + 100);

            manager.arrange_children(rect(0, 0, measured.width, measured.height));
            EXPECT_EQ(child.last_arrange, rect(padding.left, padding.top, 100, 100));
        }
    }

    TEST(horizontal_stack, arrange_respects_bounds)
    {
        stack_fixture fixture;
        auto& child = fixture.add_view({100, 100});

        horizontal_stack_layout_manager manager(fixture.stack);
        const size measured = manager.measure(inf, 100);
        manager.arrange_children(rect(10, 15, measured.width, measured.height));

        EXPECT_EQ(child.last_arrange, rect(10, 15, 100, 100));
    }

    TEST(horizontal_stack, measure_respects_min_max)
    {
        struct clamp_case
        {
            double limit;
            double view_size;
            double expected;
        };
        const clamp_case max_cases[] = {{50, 100, 50}, {100, 100, 100}, {100, 50, 50}, {0, 50, 0}};
        for (const auto& test : max_cases)
        {
            stack_fixture h;
            h.build_stack(1, 100, test.view_size);
            h.stack.max_height_value = test.limit;
            EXPECT_EQ(horizontal_stack_layout_manager(h.stack).measure(inf, inf).height, test.expected);

            stack_fixture w;
            w.build_stack(1, test.view_size, 100);
            w.stack.max_width_value = test.limit;
            EXPECT_EQ(horizontal_stack_layout_manager(w.stack).measure(inf, inf).width, test.expected);
        }
        const clamp_case min_cases[] = {{50, 10, 50}, {100, 100, 100}, {10, 50, 50}};
        for (const auto& test : min_cases)
        {
            stack_fixture h;
            h.build_stack(1, 100, test.view_size);
            h.stack.min_height_value = test.limit;
            EXPECT_EQ(horizontal_stack_layout_manager(h.stack).measure(inf, inf).height, test.expected);

            stack_fixture w;
            w.build_stack(1, test.view_size, 100);
            w.stack.min_width_value = test.limit;
            EXPECT_EQ(horizontal_stack_layout_manager(w.stack).measure(inf, inf).width, test.expected);
        }
    }

    TEST(horizontal_stack, arrange_accounts_for_fill)
    {
        stack_fixture fixture;
        fixture.build_stack(1, 100, 100);

        horizontal_stack_layout_manager manager(fixture.stack);
        (void)manager.measure(inf, inf);
        const size actual = manager.arrange_children(rect(0, 0, 1000, 1000));

        EXPECT_EQ(actual.width, 1000);
        EXPECT_EQ(actual.height, 1000);
    }

    TEST(horizontal_stack, child_measure_accounts_for_padding)
    {
        struct padding_case
        {
            thickness padding;
            double expected_height_constraint;
        };
        for (const auto& test : {padding_case{thickness(0), 100}, padding_case{thickness(10), 80},
                                 padding_case{thickness(10, 0, 10, 0), 100}, padding_case{thickness(0, 7, 0, 14), 79}})
        {
            stack_fixture fixture;
            auto& child = fixture.add_view({50, 50});
            fixture.stack.padding_value = test.padding;

            horizontal_stack_layout_manager manager(fixture.stack);
            (void)manager.measure(100, 100);

            EXPECT_EQ(child.last_measure_width, inf);
            EXPECT_EQ(child.last_measure_height, test.expected_height_constraint);
        }
    }

    TEST(horizontal_stack, collapsed_items_do_not_incur_spacing)
    {
        constexpr double view_width = 5;
        constexpr double view_height = 7;
        constexpr double spacing = 10;
        for (const int collapsed_index : {0, 1, 2})
        {
            stack_fixture fixture;
            fixture.build_stack(3, view_width, view_height);
            fixture.stack.spacing_value = spacing;
            static_cast<maui::layouts::testing::mock_view&>(fixture.stack.at(collapsed_index)).visibility_value =
                visibility::collapsed;

            horizontal_stack_layout_manager manager(fixture.stack);
            EXPECT_EQ(manager.measure(inf, inf).width, view_width + spacing + view_width)
                << "collapsed_index=" << collapsed_index;
        }
    }

    TEST(horizontal_stack, all_collapsed_items_have_no_spacing)
    {
        stack_fixture fixture;
        fixture.build_stack(3, 5, 7);
        fixture.stack.spacing_value = 10;
        for (int n = 0; n < 3; ++n)
        {
            static_cast<maui::layouts::testing::mock_view&>(fixture.stack.at(n)).visibility_value =
                visibility::collapsed;
        }

        horizontal_stack_layout_manager manager(fixture.stack);
        EXPECT_EQ(manager.measure(inf, inf).width, 0);
    }
} // namespace

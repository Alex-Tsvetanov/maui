// Ported from src/Core/tests/UnitTests/Layouts/ZIndexTests.cs — the behavioral oracle for the layout
// z-ordering helpers (Microsoft.Maui.Handlers.LayoutExtensions): order_by_z_index (the stable
// OrderByZIndex) and get_layout_handler_index (the subview index a child lands at). A mock_stack stands
// in for the C# FakeLayout; mock_view children carry a settable z_index.
#include "maui/core/layout_z_order.hpp"

#include <cstddef>
#include <vector>

#include "maui/core/i_view.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::get_layout_handler_index;
    using maui::core::i_view;
    using maui::core::order_by_z_index;
    using maui::layouts::testing::mock_view;
    using maui::layouts::testing::stack_fixture;

    // The C# CreateTestView(zIndex) — a child whose ZIndex is preset.
    mock_view& add_z_view(stack_fixture& fixture, int z_index)
    {
        mock_view& view = fixture.add_view({0, 0});
        view.set_z_index(z_index);
        return view;
    }

    TEST(z_index, layout_handler_index_follows_z_order)
    {
        stack_fixture fixture;
        mock_view& view0 = add_z_view(fixture, 10);
        mock_view& view1 = add_z_view(fixture, 0);

        EXPECT_EQ(get_layout_handler_index(fixture.stack, view1), 0);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view0), 1);
    }

    TEST(z_index, layout_handler_index_follows_add_order_when_z_indexes_equal)
    {
        stack_fixture fixture;
        mock_view& view0 = add_z_view(fixture, 0);
        mock_view& view1 = add_z_view(fixture, 10);
        mock_view& view2 = add_z_view(fixture, 10);
        mock_view& view3 = add_z_view(fixture, 100);

        EXPECT_EQ(get_layout_handler_index(fixture.stack, view0), 0);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view1), 1);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view2), 2);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view3), 3);
    }

    TEST(z_index, layout_handler_index_is_negative_when_child_is_not_found)
    {
        stack_fixture fixture;
        mock_view stray; // never added to the layout

        EXPECT_EQ(get_layout_handler_index(fixture.stack, stray), -1); // count 0

        add_z_view(fixture, 0);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, stray), -1); // count 1, not the child

        add_z_view(fixture, 0);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, stray), -1); // count >1, not found
    }

    TEST(z_index, layout_handler_index_preserves_add_order_for_equal_z_indexes)
    {
        stack_fixture fixture;
        mock_view& view0 = add_z_view(fixture, 10);
        mock_view& view1 = add_z_view(fixture, 10);
        mock_view& view2 = add_z_view(fixture, 10);
        mock_view& view3 = add_z_view(fixture, 5);

        EXPECT_EQ(get_layout_handler_index(fixture.stack, view0), 1);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view1), 2);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view2), 3);
        EXPECT_EQ(get_layout_handler_index(fixture.stack, view3), 0);
    }

    TEST(z_index, items_order_by_z_index)
    {
        stack_fixture fixture;
        mock_view& view0 = add_z_view(fixture, 10);
        mock_view& view1 = add_z_view(fixture, 0);

        const std::vector<i_view*> ordered = order_by_z_index(fixture.stack);
        ASSERT_EQ(ordered.size(), 2U);
        EXPECT_EQ(ordered[0], &view1);
        EXPECT_EQ(ordered[1], &view0);
    }

    TEST(z_index, order_by_z_index_preserves_add_order_for_equal_z_indexes)
    {
        stack_fixture fixture;
        mock_view& view0 = add_z_view(fixture, 0);
        mock_view& view1 = add_z_view(fixture, 5);
        mock_view& view2 = add_z_view(fixture, 5);
        mock_view& view3 = add_z_view(fixture, 10);

        std::vector<i_view*> ordered = order_by_z_index(fixture.stack);
        ASSERT_EQ(ordered.size(), 4U);
        EXPECT_EQ(ordered[0], &view0);
        EXPECT_EQ(ordered[1], &view1);
        EXPECT_EQ(ordered[2], &view2);
        EXPECT_EQ(ordered[3], &view3);

        // Update view3 to a tied z-index; the stable order still preserves add order on the tie.
        view3.set_z_index(5);
        ordered = order_by_z_index(fixture.stack);
        EXPECT_EQ(ordered[0], &view0);
        EXPECT_EQ(ordered[1], &view1);
        EXPECT_EQ(ordered[2], &view2);
        EXPECT_EQ(ordered[3], &view3);
    }

    TEST(z_index, order_by_z_index_preserves_add_order_for_many_equal_z_indexes)
    {
        // The C# "LotsOfEqualZIndexes" case: a larger set can trip an unstable sort. 100 ties must keep
        // add order across repeated calls.
        constexpr int view_count = 100;
        stack_fixture fixture;
        std::vector<mock_view*> added;
        added.reserve(view_count);
        for (int n = 0; n < view_count; ++n)
        {
            added.push_back(&add_z_view(fixture, 0));
        }

        for (int iteration = 0; iteration < 10; ++iteration)
        {
            const std::vector<i_view*> ordered = order_by_z_index(fixture.stack);
            ASSERT_EQ(ordered.size(), static_cast<std::size_t>(view_count));
            for (int n = 0; n < view_count; ++n)
            {
                EXPECT_EQ(ordered[static_cast<std::size_t>(n)], added[static_cast<std::size_t>(n)])
                    << "iteration=" << iteration << " n=" << n;
            }
        }
    }
} // namespace

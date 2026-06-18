// Behavioral oracle for the generic StackLayout's orientation dispatch + StackOrientation.
// Ported from src/Controls/src/Core/Layout/StackLayoutManager.cs (SelectLayoutManager dispatches on
// Orientation to the Vertical/Horizontal Core manager) cross-checked against the per-axis arrangement
// already covered by {vertical,horizontal}_stack_layout_manager_tests.cpp. AndExpand is a documented
// deferral (no LayoutOptions.Expands surface on the port's views) — see stack_layout_manager.hpp.

#include "maui/controls/stack_layout.hpp"

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/stack_layout_manager.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::stack_layout;
    using maui::controls::stack_orientation;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // Owns the stack control + its (non-owning) child views, mirroring the C# StackLayout setup.
    struct fixture
    {
        stack_layout stack;

        mock_view& add_view(size sz)
        {
            auto view = std::make_unique<mock_view>();
            view->configure(sz);
            mock_view& reference = *view;
            stack.add(reference);
            owned_.push_back(std::move(view));
            return reference;
        }

    private:
        std::vector<std::unique_ptr<mock_view>> owned_;
    };

    TEST(stack_layout, orientation_defaults_to_vertical)
    {
        EXPECT_EQ(stack_layout{}.orientation(), stack_orientation::vertical);
    }

    TEST(stack_layout, orientation_property_round_trips)
    {
        stack_layout stack;
        stack.set_orientation(stack_orientation::horizontal);
        EXPECT_EQ(stack.orientation(), stack_orientation::horizontal);
        stack.set_orientation(stack_orientation::vertical);
        EXPECT_EQ(stack.orientation(), stack_orientation::vertical);
    }

    // Vertical orientation -> children stack top-to-bottom (the vertical Core manager): width is the
    // widest child, height is the sum, and arrange lays them out at running y offsets.
    TEST(stack_layout, vertical_orientation_dispatches_to_vertical_manager)
    {
        fixture f;
        f.stack.set_orientation(stack_orientation::vertical);
        auto& first = f.add_view({100, 40});
        auto& second = f.add_view({60, 30});

        maui::controls::stack_layout_manager manager(f.stack);
        const size measured = manager.measure(inf, inf);
        EXPECT_EQ(measured.width, 100); // widest child
        EXPECT_EQ(measured.height, 70); // 40 + 30, no spacing
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 40));
        EXPECT_EQ(second.last_arrange, rect(0, 40, 100, 30)); // full-width, stacked below the first
    }

    // Horizontal orientation -> children stack left-to-right (the horizontal Core manager): height is
    // the tallest child, width is the sum, and arrange lays them out at running x offsets.
    TEST(stack_layout, horizontal_orientation_dispatches_to_horizontal_manager)
    {
        fixture f;
        f.stack.set_orientation(stack_orientation::horizontal);
        auto& first = f.add_view({100, 40});
        auto& second = f.add_view({60, 30});

        maui::controls::stack_layout_manager manager(f.stack);
        const size measured = manager.measure(inf, inf);
        EXPECT_EQ(measured.width, 160); // 100 + 60, no spacing
        EXPECT_EQ(measured.height, 40); // tallest child
        manager.arrange_children(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 40));
        EXPECT_EQ(second.last_arrange, rect(100, 0, 60, 40)); // full-height, to the right of the first
    }

    // A runtime Orientation flip re-selects the algorithm on the next pass (the same manager instance):
    // the SAME two children arrange vertically, then horizontally, with no stale cached geometry.
    TEST(stack_layout, orientation_flip_redispatches)
    {
        fixture f;
        auto& first = f.add_view({100, 40});
        auto& second = f.add_view({60, 30});

        maui::controls::stack_layout_manager manager(f.stack);

        f.stack.set_orientation(stack_orientation::vertical);
        const size vertical = manager.measure(inf, inf);
        manager.arrange_children(rect(0, 0, vertical.width, vertical.height));
        EXPECT_EQ(vertical, size(100, 70));
        EXPECT_EQ(second.last_arrange, rect(0, 40, 100, 30));

        f.stack.set_orientation(stack_orientation::horizontal);
        const size horizontal = manager.measure(inf, inf);
        manager.arrange_children(rect(0, 0, horizontal.width, horizontal.height));
        EXPECT_EQ(horizontal, size(160, 40));
        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 40));
        EXPECT_EQ(second.last_arrange, rect(100, 0, 60, 40));
    }

    // Spacing flows through the selected manager in each orientation (StackBase.Spacing).
    TEST(stack_layout, spacing_applies_in_each_orientation)
    {
        {
            fixture f;
            f.stack.set_orientation(stack_orientation::vertical);
            f.stack.set_spacing(13);
            auto& first = f.add_view({100, 40});
            auto& second = f.add_view({100, 30});

            maui::controls::stack_layout_manager manager(f.stack);
            const size measured = manager.measure(inf, inf);
            EXPECT_EQ(measured.height, 40 + 13 + 30); // one spacing gap between two children
            manager.arrange_children(rect(0, 0, measured.width, measured.height));
            EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 40));
            EXPECT_EQ(second.last_arrange, rect(0, 40 + 13, 100, 30));
        }
        {
            fixture f;
            f.stack.set_orientation(stack_orientation::horizontal);
            f.stack.set_spacing(13);
            auto& first = f.add_view({100, 40});
            auto& second = f.add_view({60, 40});

            maui::controls::stack_layout_manager manager(f.stack);
            const size measured = manager.measure(inf, inf);
            EXPECT_EQ(measured.width, 100 + 13 + 60);
            manager.arrange_children(rect(0, 0, measured.width, measured.height));
            EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 40));
            EXPECT_EQ(second.last_arrange, rect(100 + 13, 0, 60, 40));
        }
    }

    // The control's own measure/arrange seam (layout<>::measure delegates to ensure_manager(), which
    // builds a stack_layout_manager via create_layout_manager) produces the same orientation-aware size.
    TEST(stack_layout, control_measure_uses_orientation_dispatching_manager)
    {
        fixture f;
        f.add_view({100, 40});
        f.add_view({60, 30});

        f.stack.set_orientation(stack_orientation::vertical);
        EXPECT_EQ(f.stack.measure(inf, inf), size(100, 70));

        // A fresh control to avoid the cached manager (the control caches its manager for its lifetime,
        // matching C# Layout.CreateLayoutManager being invoked once); the dispatch-per-pass behavior is
        // covered by orientation_flip_redispatches above.
        fixture g;
        g.add_view({100, 40});
        g.add_view({60, 30});
        g.stack.set_orientation(stack_orientation::horizontal);
        EXPECT_EQ(g.stack.measure(inf, inf), size(160, 40));
    }
} // namespace

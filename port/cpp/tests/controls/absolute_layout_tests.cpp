// Tests for the absolute_layout control + its headless handler seam. Verifies (1) the attached
// LayoutBounds/LayoutFlags default + round-trip exactly as C# AbsoluteLayout; (2) the per-child store is
// pruned on remove/clear; (3) measure/arrange through the control reproduce the AbsoluteLayout geometry
// (driven through the control, not a mock); and (4) the handler is self-registered + the panel child
// count tracks add/remove/clear. Ported from src/Controls/tests/Core.UnitTests/Layouts/AbsoluteLayoutTests.cs
// (control surface) + AbsoluteLayoutManagerTests.cs (geometry).
#include "maui/controls/absolute_layout.hpp"

#include <limits>
#include <memory>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_absolute_layout.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::absolute_layout;
    using maui::core::default_handler_registry;
    using maui::core::i_absolute_layout;
    using maui::core::i_element_handler;
    using maui::core::i_layout;
    using maui::core::layout_handler;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::absolute_layout_flags;
    using maui::layouts::testing::mock_view;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // ---- attached LayoutBounds / LayoutFlags: defaults + round-trip ----

    TEST(absolute_layout_control, unpositioned_child_uses_default_bounds_and_flags)
    {
        absolute_layout layout;
        mock_view child;
        layout.add(child);

        // C# defaults: bounds (0, 0, AutoSize, AutoSize), flags None.
        EXPECT_EQ(layout.get_layout_bounds(child), rect(0, 0, absolute_layout::auto_size, absolute_layout::auto_size));
        EXPECT_EQ(layout.get_layout_flags(child), absolute_layout_flags::none);
    }

    TEST(absolute_layout_control, set_bounds_and_flags_round_trip)
    {
        absolute_layout layout;
        mock_view child;
        layout.add(child);
        layout.set_layout_bounds(child, rect(10, 20, 30, 40));
        layout.set_layout_flags(child, absolute_layout_flags::position_proportional);

        EXPECT_EQ(layout.get_layout_bounds(child), rect(10, 20, 30, 40));
        EXPECT_EQ(layout.get_layout_flags(child), absolute_layout_flags::position_proportional);
    }

    TEST(absolute_layout_control, removing_a_child_drops_its_layout_info)
    {
        absolute_layout layout;
        mock_view child;
        layout.add(child);
        layout.set_layout_bounds(child, rect(5, 5, 5, 5));

        layout.remove_at(0);
        layout.add(child); // re-added: prior info must not linger -> back to defaults
        EXPECT_EQ(layout.get_layout_bounds(child), rect(0, 0, absolute_layout::auto_size, absolute_layout::auto_size));
    }

    TEST(absolute_layout_control, clear_drops_all_layout_info)
    {
        absolute_layout layout;
        mock_view a;
        mock_view b;
        layout.add(a);
        layout.add(b);
        layout.set_layout_flags(a, absolute_layout_flags::all);

        layout.clear();
        layout.add(a);
        EXPECT_EQ(layout.get_layout_flags(a), absolute_layout_flags::none);
    }

    // ---- geometry through the control ----

    TEST(absolute_layout_control, absolute_position_and_size_arrange)
    {
        absolute_layout layout;
        mock_view child;
        child.configure({0, 0});
        layout.add(child);
        layout.set_layout_bounds(child, rect(10, 15, 100, 100));

        const size measured = layout.measure(inf, inf);
        layout.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(measured, size(110, 115));
        EXPECT_EQ(child.last_arrange, rect(10, 15, 100, 100));
    }

    TEST(absolute_layout_control, default_bounds_uses_child_measure)
    {
        absolute_layout layout;
        mock_view child;
        child.configure({50, 75}); // DesiredSize, used for AutoSize bounds
        layout.add(child);

        const size measured = layout.measure(inf, inf);
        layout.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(measured, size(50, 75));
        EXPECT_EQ(child.last_arrange, rect(0, 0, 50, 75));
    }

    TEST(absolute_layout_control, proportional_size_and_position_arrange)
    {
        absolute_layout layout;
        mock_view child;
        child.configure({0, 0});
        layout.add(child);
        layout.set_layout_bounds(child, rect(0.5, 0.5, 0.4, 0.5));
        layout.set_layout_flags(child, absolute_layout_flags::all);

        layout.measure(100, 100);
        layout.arrange(rect(0, 0, 100, 100));

        // size 0.4*100=40, 0.5*100=50; position (100-40)*0.5=30, (100-50)*0.5=25.
        EXPECT_EQ(child.last_arrange, rect(30, 25, 40, 50));
    }

    // ---- handler seam ----

    TEST(absolute_layout_seam, panel_child_count_tracks_mutations)
    {
        absolute_layout layout;
        auto handler = std::make_shared<layout_handler>();
        layout.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        mock_view a;
        mock_view b;
        layout.add(a);
        layout.add(b);
        EXPECT_EQ(platform->children.size(), 2U);

        layout.remove_at(0);
        EXPECT_EQ(platform->children.size(), 1U);
        EXPECT_EQ(platform->children[0], &b);

        layout.clear();
        EXPECT_EQ(platform->children.size(), 0U);
    }

    TEST(absolute_layout_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler = default_handler_registry().create_handler<absolute_layout>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<layout_handler*>(handler.get()), nullptr);
    }

    TEST(absolute_layout_control, usable_through_layout_interface)
    {
        absolute_layout layout;
        mock_view child;
        layout.add(child);

        i_layout& as_layout = layout;
        EXPECT_EQ(as_layout.count(), 1);
        EXPECT_EQ(&as_layout.at(0), &child);

        i_absolute_layout& as_absolute = layout;
        EXPECT_EQ(as_absolute.get_layout_flags(child), absolute_layout_flags::none);
    }
} // namespace

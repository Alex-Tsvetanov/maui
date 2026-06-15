// Tests for the layout controls + their headless handler seam — the stack layout controls wrap the M3
// managers and host children on a native panel. Two things are verified here: (1) the control's
// measure/arrange delegate to the M3 manager and reproduce the stack geometry (the assertions mirror
// VerticalStackLayoutManagerTests), and (2) the headless layout_platform's child count tracks the
// control's add/insert/remove/clear so the native panel stays in sync.
#include "maui/controls/vertical_stack_layout.hpp"

#include <memory>

#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_cross_platform_layout.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_stack_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::horizontal_stack_layout;
    using maui::controls::vertical_stack_layout;
    using maui::core::i_element_handler;
    using maui::core::i_layout;
    using maui::core::i_stack_layout;
    using maui::core::layout_handler;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation (no handler): geometry comes from the M3 manager ----

    TEST(layout_control, defaults_empty_with_zero_spacing_and_padding)
    {
        vertical_stack_layout stack;
        EXPECT_EQ(stack.count(), 0);
        EXPECT_EQ(stack.spacing(), 0.0);
        EXPECT_EQ(stack.padding(), thickness());
    }

    TEST(layout_control, container_surface_tracks_children)
    {
        vertical_stack_layout stack;
        mock_view a;
        mock_view b;
        mock_view c;

        stack.add(a);
        stack.add(b);
        EXPECT_EQ(stack.count(), 2);
        EXPECT_EQ(&stack.at(0), &a);
        EXPECT_EQ(stack.index_of(b), 1);

        stack.insert(1, c);
        EXPECT_EQ(stack.count(), 3);
        EXPECT_EQ(&stack.at(1), &c);

        stack.remove_at(0);
        EXPECT_EQ(stack.count(), 2);
        EXPECT_EQ(&stack.at(0), &c);

        stack.clear();
        EXPECT_EQ(stack.count(), 0);
    }

    TEST(layout_control, usable_through_interface_references)
    {
        vertical_stack_layout stack;
        mock_view child;
        stack.set_spacing(7);
        stack.add(child);

        i_stack_layout& as_stack = stack;
        i_layout& as_layout = stack;
        EXPECT_EQ(as_stack.spacing(), 7.0);
        EXPECT_EQ(as_layout.count(), 1);
        EXPECT_EQ(&as_layout.at(0), &child);
    }

    TEST(layout_control, vertical_measure_stacks_heights_with_spacing)
    {
        vertical_stack_layout stack;
        stack.set_spacing(13);
        mock_view a;
        mock_view b;
        mock_view c;
        a.configure({100, 100});
        b.configure({100, 100});
        c.configure({100, 100});
        stack.add(a);
        stack.add(b);
        stack.add(c);

        // 3 * 100 + 2 * 13 spacing = 326 (cf. VerticalStackLayoutManagerTests.SpacingMeasurement).
        EXPECT_EQ(stack.measure(100, 1000).height, 326.0);
        EXPECT_EQ(stack.measure(100, 1000).width, 100.0);
    }

    TEST(layout_control, vertical_arrange_positions_children_in_a_column)
    {
        vertical_stack_layout stack;
        mock_view first;
        mock_view second;
        first.configure({100, 100});
        second.configure({100, 100});
        stack.add(first);
        stack.add(second);

        const size measured = stack.measure(100, 1000);
        stack.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(second.last_arrange, rect(0, 100, 100, 100));
    }

    TEST(layout_control, measure_accounts_for_padding)
    {
        vertical_stack_layout stack;
        stack.set_padding(thickness(10));
        mock_view child;
        child.configure({100, 100});
        stack.add(child);

        const size measured = stack.measure(1000, 1000);
        EXPECT_EQ(measured.height, 120.0); // 100 + top+bottom (10+10)
        EXPECT_EQ(measured.width, 120.0);  // 100 + left+right (10+10)
    }

    TEST(layout_control, horizontal_measure_stacks_widths_with_spacing)
    {
        horizontal_stack_layout stack;
        stack.set_spacing(13);
        mock_view a;
        mock_view b;
        mock_view c;
        a.configure({100, 100});
        b.configure({100, 100});
        c.configure({100, 100});
        stack.add(a);
        stack.add(b);
        stack.add(c);

        // Horizontal: 3 * 100 + 2 * 13 spacing = 326 wide; tallest child = 100 tall.
        EXPECT_EQ(stack.measure(1000, 100).width, 326.0);
        EXPECT_EQ(stack.measure(1000, 100).height, 100.0);
    }

    TEST(layout_control, horizontal_arrange_positions_children_in_a_row)
    {
        horizontal_stack_layout stack;
        mock_view first;
        mock_view second;
        first.configure({100, 100});
        second.configure({100, 100});
        stack.add(first);
        stack.add(second);

        const size measured = stack.measure(1000, 100);
        stack.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(second.last_arrange, rect(100, 0, 100, 100));
    }

    // ---- the handler seam (control <-> handler <-> headless panel): the panel mirrors the children ----

    TEST(layout_seam, attaching_handler_creates_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &stack);
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 0U);
    }

    TEST(layout_seam, panel_child_count_tracks_add_and_remove)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        mock_view a;
        mock_view b;
        stack.add(a); // -> handler->invoke("add", …) -> map_add -> add()
        stack.add(b);
        EXPECT_EQ(platform->children.size(), 2U);
        EXPECT_EQ(platform->children[0], &a);
        EXPECT_EQ(platform->children[1], &b);

        stack.remove_at(0);
        EXPECT_EQ(platform->children.size(), 1U);
        EXPECT_EQ(platform->children[0], &b);

        stack.clear();
        EXPECT_EQ(platform->children.size(), 0U);
    }

    TEST(layout_seam, panel_child_count_tracks_insert)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        mock_view a;
        mock_view b;
        stack.add(a);
        stack.insert(0, b);
        EXPECT_EQ(platform->children.size(), 2U);
        EXPECT_EQ(platform->children[0], &b); // inserted at the front
        EXPECT_EQ(platform->children[1], &a);
    }

    TEST(layout_seam, children_added_before_handler_are_not_double_counted)
    {
        // Children added before a handler is attached do not pre-populate the panel mirror (the panel
        // starts empty); only post-attach mutations are mirrored. This matches the headless seam, where
        // the count is driven purely by the invoke() commands.
        vertical_stack_layout stack;
        mock_view a;
        stack.add(a); // no handler yet -> no invoke

        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 0U);

        mock_view b;
        stack.add(b); // now mirrored
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 1U);
    }

    TEST(layout_seam, arrange_with_handler_still_positions_children)
    {
        // With a handler attached, arrange both sizes the host panel (handler->platform_arrange) AND
        // positions the children via the manager — the panel-sizing must not displace child arrangement.
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        mock_view first;
        mock_view second;
        first.configure({100, 100});
        second.configure({100, 100});
        stack.add(first);
        stack.add(second);

        const size measured = stack.measure(100, 1000);
        stack.arrange(rect(0, 0, measured.width, measured.height));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(second.last_arrange, rect(0, 100, 100, 100));
    }

    TEST(layout_seam, handler_resolved_from_default_registry)
    {
        // vertical_stack_layout -> layout_handler is self-registered (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<vertical_stack_layout>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<layout_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        vertical_stack_layout stack;
        stack.set_handler(handler);
        mock_view child;
        stack.add(child);
        EXPECT_EQ(resolved->typed_platform_view()->children.size(), 1U);
    }

    // ---- ClipsToBounds (Layout.IsClippedToBounds → the panel's clip flag) ----

    TEST(layout_clips_to_bounds, defaults_false_and_maps_on_connect)
    {
        vertical_stack_layout stack;
        EXPECT_FALSE(stack.clips_to_bounds());

        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        // The mapper runs on connect; the panel mirror reflects the (false) default.
        EXPECT_FALSE(handler->typed_platform_view()->clips_to_bounds);
    }

    TEST(layout_clips_to_bounds, set_pushes_to_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        stack.set_clips_to_bounds(true);
        EXPECT_TRUE(stack.clips_to_bounds());
        EXPECT_TRUE(handler->typed_platform_view()->clips_to_bounds); // map_clips_to_bounds ran

        stack.set_clips_to_bounds(false);
        EXPECT_FALSE(handler->typed_platform_view()->clips_to_bounds);
    }

    // ---- z-index: the panel's subview order follows the children's z-index ----

    TEST(layout_z_order, added_children_stack_by_z_index)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        mock_view high;
        high.set_z_index(10);
        mock_view low;
        low.set_z_index(0);

        stack.add(high); // added first, but higher z -> goes on top (last in subview order)
        stack.add(low);

        ASSERT_EQ(platform->children.size(), 2U);
        EXPECT_EQ(platform->children[0], &low);  // lower z first
        EXPECT_EQ(platform->children[1], &high); // higher z on top
    }

    TEST(layout_z_order, runtime_z_index_change_restacks_child)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        mock_view first;
        mock_view second;
        stack.add(first); // both default z 0 -> add order preserved
        stack.add(second);
        ASSERT_EQ(platform->children.size(), 2U);
        EXPECT_EQ(platform->children[0], &first);
        EXPECT_EQ(platform->children[1], &second);

        // Raise `first`'s z-index: it should re-stack above `second` (the change routes through the parent
        // layout's handler update_z_index, mirroring ViewHandler.MapZIndex).
        first.set_z_index(5);
        EXPECT_EQ(platform->children[0], &second);
        EXPECT_EQ(platform->children[1], &first);
    }

    TEST(layout_z_order, equal_z_index_keeps_add_order)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        mock_view a;
        mock_view b;
        mock_view c;
        stack.add(a);
        stack.add(b);
        stack.add(c);

        ASSERT_EQ(platform->children.size(), 3U);
        EXPECT_EQ(platform->children[0], &a);
        EXPECT_EQ(platform->children[1], &b);
        EXPECT_EQ(platform->children[2], &c);
    }

    // ---- X4: the ICrossPlatformLayout / ISafeAreaView contracts on the layout base ----

    TEST(layout_cross_platform, cross_platform_measure_matches_the_managers_measure)
    {
        vertical_stack_layout stack;
        stack.set_spacing(13);
        mock_view a;
        mock_view b;
        mock_view c;
        a.configure({100, 100});
        b.configure({100, 100});
        c.configure({100, 100});
        stack.add(a);
        stack.add(b);
        stack.add(c);

        // CrossPlatformMeasure == LayoutManager.Measure == the control's own measure (Layout.cs).
        auto& cross = static_cast<maui::core::i_cross_platform_layout&>(stack);
        const size cross_measured = cross.cross_platform_measure(100, 1000);
        EXPECT_EQ(cross_measured.height, 326.0);
        EXPECT_EQ(cross_measured.width, 100.0);
        EXPECT_EQ(cross_measured, stack.measure(100, 1000));
    }

    TEST(layout_cross_platform, cross_platform_arrange_positions_children)
    {
        vertical_stack_layout stack;
        mock_view first;
        mock_view second;
        first.configure({100, 100});
        second.configure({100, 100});
        stack.add(first);
        stack.add(second);

        auto& cross = static_cast<maui::core::i_cross_platform_layout&>(stack);
        (void)cross.cross_platform_measure(100, 1000);
        cross.cross_platform_arrange(rect(0, 0, 100, 200));

        EXPECT_EQ(first.last_arrange, rect(0, 0, 100, 100));
        EXPECT_EQ(second.last_arrange, rect(0, 100, 100, 100));
    }

    TEST(layout_safe_area, ignore_safe_area_defaults_false_and_round_trips)
    {
        vertical_stack_layout stack;
        auto& safe = static_cast<maui::core::i_safe_area_view&>(stack);
        EXPECT_FALSE(safe.ignore_safe_area()); // Layout.IgnoreSafeArea default
        stack.set_ignore_safe_area(true);
        EXPECT_TRUE(safe.ignore_safe_area());
    }
} // namespace

// iOS (UIKit) backend tests for the grid control's layout seam — placed children become real UIView
// subviews of the grid panel after the handler hosts them (and leave on remove/clear). The grid reuses
// layout_handler, so the panel is a plain UIView container, exactly as for the stack layouts; each
// child here is a button (its handler owns a real UIButton — label/entry are still headless on ios).
// Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via tools/ios-sim-run.sh). Compiled as
// Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/grid.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::grid;
    using maui::core::button_handler;
    using maui::core::grid_length;
    using maui::core::layout_handler;

    UIView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    // A button with its handler attached, so it owns a real native UIButton the panel can host. Returns
    // the child's native UIView for superview assertions.
    UIView* attach_button(button& control)
    {
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        return (__bridge UIView*)handler->native_view();
    }

    TEST(ios_grid_seam, panel_is_a_uiview)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_panel(handler) isKindOfClass:[UIView class]]);
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST(ios_grid_seam, placed_child_becomes_a_subview)
    {
        grid g;
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::automatic());
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        // A child with its own native view (a button-backed UIButton). native_view() returns the real
        // UIButton the handler's pimpl owns.
        button child;
        UIView* const child_native = attach_button(child);
        ASSERT_NE(child_native, nil);

        g.add(child); // -> handler->invoke("add", …) -> map_add -> add() -> insertSubview:atIndex:
        g.set_row(child, 0);
        g.set_column(child, 0);

        EXPECT_EQ(native_panel(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_panel(handler));
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 1U);
    }

    TEST(ios_grid_seam, arrange_sizes_the_panel)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        g.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the panel

        const CGRect frame = native_panel(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }

    TEST(ios_grid_seam, removed_child_leaves_the_panel)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        button first;
        attach_button(first);
        button second;
        attach_button(second);

        g.add(first);
        g.add(second);
        EXPECT_EQ(native_panel(handler).subviews.count, 2U);

        g.remove_at(0);
        EXPECT_EQ(native_panel(handler).subviews.count, 1U);

        g.clear();
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }
} // namespace

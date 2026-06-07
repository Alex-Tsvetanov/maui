// Apple (AppKit) backend tests for the grid control's layout seam — placed children become real NSView
// subviews of the grid panel after the handler hosts them (and leave on remove/clear). The grid reuses
// layout_handler, so the panel is a plain NSView container, exactly as for the stack layouts; each child
// here is a label (its handler owns a real NSTextField). Compiled as Objective-C++ with ARC for `apple`.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::grid;
    using maui::controls::label;
    using maui::core::grid_length;
    using maui::core::label_handler;
    using maui::core::layout_handler;

    NSView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    class apple_grid_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_grid_seam, panel_is_an_nsview)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_panel(handler) isKindOfClass:[NSView class]]);
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST_F(apple_grid_seam, placed_child_becomes_a_subview)
    {
        grid g;
        g.add_row_definition(grid_length::automatic());
        g.add_column_definition(grid_length::automatic());
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        // A child with its own native view (a label-backed NSTextField). native_view() returns the real
        // NSTextField the handler's pimpl owns.
        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge NSView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        g.add(child); // -> handler->invoke("add", …) -> map_add -> add() -> addSubview:
        g.set_row(child, 0);
        g.set_column(child, 0);

        EXPECT_EQ(native_panel(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_panel(handler));
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 1U);
    }

    TEST_F(apple_grid_seam, arrange_sizes_the_panel)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        g.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the panel

        const NSRect frame = native_panel(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }

    TEST_F(apple_grid_seam, removed_child_leaves_the_panel)
    {
        grid g;
        auto handler = std::make_shared<layout_handler>();
        g.set_handler(handler);

        label first;
        auto first_handler = std::make_shared<label_handler>();
        first.set_handler(first_handler);
        label second;
        auto second_handler = std::make_shared<label_handler>();
        second.set_handler(second_handler);

        g.add(first);
        g.add(second);
        EXPECT_EQ(native_panel(handler).subviews.count, 2U);

        g.remove_at(0);
        EXPECT_EQ(native_panel(handler).subviews.count, 1U);

        g.clear();
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }
} // namespace

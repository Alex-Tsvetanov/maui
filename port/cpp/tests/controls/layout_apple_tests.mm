// Apple (AppKit) backend tests for the layout seam — children become real NSView subviews of the
// layout panel after add (and leave on remove/clear). The panel is a plain NSView container; each child
// here is a label (its handler owns a real NSTextField). Compiled as Objective-C++ with ARC for the
// `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::label_handler;
    using maui::core::layout_handler;

    NSView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    class apple_layout_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_layout_seam, panel_is_an_nsview)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_panel(handler) isKindOfClass:[NSView class]]);
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST_F(apple_layout_seam, added_child_becomes_a_subview)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        // A child with its own native view (a label-backed NSTextField). native_view() returns the real
        // NSTextField the handler's pimpl owns (platform_view() would return the pimpl pointer itself).
        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto const child_native = (__bridge NSView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        stack.add(child); // -> handler->invoke("add", …) -> map_add -> add() -> addSubview:

        EXPECT_EQ(native_panel(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_panel(handler));
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 1U);
    }

    TEST_F(apple_layout_seam, arrange_sizes_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        stack.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the panel

        const NSRect frame = native_panel(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }

    TEST_F(apple_layout_seam, removed_child_leaves_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        label first;
        auto first_handler = std::make_shared<label_handler>();
        first.set_handler(first_handler);
        label second;
        auto second_handler = std::make_shared<label_handler>();
        second.set_handler(second_handler);

        stack.add(first);
        stack.add(second);
        EXPECT_EQ(native_panel(handler).subviews.count, 2U);

        stack.remove_at(0);
        EXPECT_EQ(native_panel(handler).subviews.count, 1U);

        stack.clear();
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST_F(apple_layout_seam, clips_to_bounds_sets_layer_masks_to_bounds)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        stack.set_clips_to_bounds(true); // -> map_clips_to_bounds -> layer.masksToBounds = YES
        EXPECT_TRUE(native_panel(handler).layer.masksToBounds);

        stack.set_clips_to_bounds(false);
        EXPECT_FALSE(native_panel(handler).layer.masksToBounds);
    }

    TEST_F(apple_layout_seam, subviews_stack_by_z_index)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        label high;
        auto high_handler = std::make_shared<label_handler>();
        high.set_handler(high_handler);
        high.set_z_index(10);
        auto const high_native = (__bridge NSView*)high_handler->native_view();

        label low;
        auto low_handler = std::make_shared<label_handler>();
        low.set_handler(low_handler);
        low.set_z_index(0);
        auto const low_native = (__bridge NSView*)low_handler->native_view();

        stack.add(high); // higher z added first, but should end up on top (last subview)
        stack.add(low);

        NSArray<NSView*>* const subviews = native_panel(handler).subviews;
        ASSERT_EQ(subviews.count, 2U);
        EXPECT_EQ(subviews[0], low_native);  // lower z at the bottom
        EXPECT_EQ(subviews[1], high_native); // higher z on top

        // A runtime z-index change re-stacks the subview (routes through the parent layout's handler).
        low.set_z_index(20);
        NSArray<NSView*>* const reordered = native_panel(handler).subviews;
        EXPECT_EQ(reordered[0], high_native);
        EXPECT_EQ(reordered[1], low_native);
    }
} // namespace

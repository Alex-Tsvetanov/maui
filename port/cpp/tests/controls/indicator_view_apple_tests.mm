// macOS (AppKit) backend tests for indicator_view — AppKit has NO UIPageControl, so the indicator is
// an NSStackView of hand-drawn dot subviews (documented deviation). These assert the real dot row:
//   - the native view is an NSStackView;
//   - Count → the arranged dot count, honoring HideSingle (a lone dot → 0);
//   - MaximumVisible caps the dots;
//   - the selected dot takes the selected color, the rest the indicator color.
//
// Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/indicator_view.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::indicator_view;
    using maui::core::indicator_view_handler;

    NSStackView* native_stack(const std::shared_ptr<indicator_view_handler>& handler)
    {
        return (__bridge NSStackView*)handler->native_view();
    }

    struct rig
    {
        indicator_view view;
        std::shared_ptr<indicator_view_handler> handler = std::make_shared<indicator_view_handler>();

        rig()
        {
            view.set_handler(handler);
        }
    };

    TEST(indicator_view_apple, native_view_is_an_nsstackview)
    {
        const rig r;
        EXPECT_TRUE([native_stack(r.handler) isKindOfClass:[NSStackView class]]);
    }

    TEST(indicator_view_apple, count_drives_dot_count)
    {
        rig r;
        r.view.set_count(5);
        EXPECT_EQ(native_stack(r.handler).arrangedSubviews.count, 5u);
    }

    // HideSingle (default true): a single dot collapses to 0 dots.
    TEST(indicator_view_apple, hide_single_collapses_a_lone_dot)
    {
        rig r;
        r.view.set_count(1);
        EXPECT_EQ(native_stack(r.handler).arrangedSubviews.count, 0u);
        r.view.set_hide_single(false);
        EXPECT_EQ(native_stack(r.handler).arrangedSubviews.count, 1u);
    }

    TEST(indicator_view_apple, maximum_visible_caps_dots)
    {
        rig r;
        r.view.set_count(10);
        r.view.set_maximum_visible(3);
        EXPECT_EQ(native_stack(r.handler).arrangedSubviews.count, 3u);
    }

    // The selected dot takes the selected color; the rest take the indicator color.
    TEST(indicator_view_apple, selected_dot_uses_the_selected_color)
    {
        rig r;
        r.view.set_count(4);
        r.view.set_position_manual(1);
        NSArray<NSView*>* const dots = native_stack(r.handler).arrangedSubviews;
        ASSERT_EQ(dots.count, 4u);
        // The dots are layer-backed; the selected (index 1) layer color differs from an unselected one.
        CGColorRef const selected = dots[1].layer.backgroundColor;
        CGColorRef const unselected = dots[0].layer.backgroundColor;
        EXPECT_FALSE(CGColorEqualToColor(selected, unselected))
            << "the selected dot should be tinted differently from the unselected dots";
    }
} // namespace

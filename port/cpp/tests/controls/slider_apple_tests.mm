// Apple (AppKit) backend tests for the slider seam — run only for MAUI_BACKEND=apple. Drives a genuine
// NSSlider: Min/Max/Value map to minValue/maxValue/doubleValue, the continuous target-action flows the
// native value back to the control, and the drag channel is exercised through the SAME notify methods
// the MauiNSSlider subclass's real mouseDown: brackets around AppKit's tracking loop (synthesizing the
// loop's blocking mouse events is not feasible in a unit test — the documented compromise, like the
// ios suite's dispatch-walk). Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/slider.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

// The drag-notify methods MauiNSSlider (slider_handler.mm) defines; declared as a category so the test
// can call them directly (the instance under test IS a MauiNSSlider — dynamic dispatch finds them).
@interface NSSlider (MauiSliderDragTesting)
- (void)notifyDragStarted;
- (void)notifyDragCompleted;
@end

namespace
{
    using maui::controls::slider;
    using maui::core::i_element_handler;
    using maui::core::slider_handler;

    NSSlider* native_slider(const std::shared_ptr<slider_handler>& handler)
    {
        return (__bridge NSSlider*)handler->typed_platform_view()->native;
    }

    // Fire the slider's registered target-action (the continuous-action delivery NSApp would perform
    // during a real drag; sendAction:to:from: needs no run loop).
    void send_value_action(NSSlider* native)
    {
        [NSApp sendAction:native.action to:native.target from:native];
    }

    // NSSlider creation needs the shared application object (no run loop required).
    class apple_slider_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_slider_seam, attaching_handler_creates_nsslider_and_maps_range)
    {
        slider control(20, 200, 50);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        NSSlider* const view = native_slider(handler);
        EXPECT_EQ(view.minValue, 20);
        EXPECT_EQ(view.maxValue, 200);
        EXPECT_EQ(view.doubleValue, 50);
        EXPECT_TRUE(view.isContinuous);
    }

    TEST_F(apple_slider_seam, setting_value_updates_the_nsslider)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(native_slider(handler).doubleValue, 42);
    }

    TEST_F(apple_slider_seam, native_value_change_flows_back)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // Simulate the user dragging the thumb: the native value moves, then the continuous action
        // fires (exactly what AppKit does during the tracking loop).
        NSSlider* const view = native_slider(handler);
        view.doubleValue = 33;
        send_value_action(view);

        EXPECT_EQ(control.value(), 33);
        EXPECT_EQ(reported_new, 33);
    }

    TEST_F(apple_slider_seam, native_drag_channel_flows_back)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        bool started = false;
        bool completed = false;
        control.drag_started.connect([&started] { started = true; });
        control.drag_completed.connect([&completed] { completed = true; });

        // Drive the same notify methods the real mouseDown: brackets around the tracking loop.
        NSSlider* const view = native_slider(handler);
        [view notifyDragStarted];
        EXPECT_TRUE(started);
        [view notifyDragCompleted];
        EXPECT_TRUE(completed);
    }

    TEST_F(apple_slider_seam, minimum_track_color_maps_to_track_fill)
    {
        slider control;
        control.set_minimum_track_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        NSColor* const fill = [native_slider(handler).trackFillColor colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
        ASSERT_NE(fill, nil);
        EXPECT_NEAR(fill.greenComponent, 1.0, 0.01);
        EXPECT_NEAR(fill.redComponent, 0.0, 0.01);

        // AppKit deviation (documented in slider_handler.mm): the unfilled side + thumb have no public
        // API — the mirrors record the push.
        control.set_maximum_track_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(handler->typed_platform_view()->maximum_track_color, maui::graphics::color(0.0F, 0.0F, 1.0F));
        EXPECT_EQ(handler->typed_platform_view()->thumb_color, maui::graphics::color(1.0F, 0.0F, 0.0F));
    }

    TEST_F(apple_slider_seam, clearing_handler_disconnects)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_slider_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<slider>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<slider_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        slider control(0, 10, 7);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSSlider*)resolved->typed_platform_view()->native).doubleValue, 7);
    }
} // namespace

// Apple (AppKit) backend tests for the stepper seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSStepper: Min/Max/Increment/Value map to minValue/maxValue/increment/doubleValue, and a
// native step ([NSStepper performClick:] advances the value and fires the target-action without a run
// loop) flows back through the handler to the control's value. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/stepper.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/stepper_handler.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::stepper;
    using maui::core::i_element_handler;
    using maui::core::stepper_handler;

    NSStepper* native_stepper(const std::shared_ptr<stepper_handler>& handler)
    {
        return (__bridge NSStepper*)handler->typed_platform_view()->native;
    }

    // NSStepper creation needs the shared application object (no run loop required).
    class apple_stepper_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_stepper_seam, attaching_handler_creates_nsstepper_and_maps_range)
    {
        stepper control(10, 90, 30, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        NSStepper* const view = native_stepper(handler);
        EXPECT_EQ(view.minValue, 10);
        EXPECT_EQ(view.maxValue, 90);
        EXPECT_EQ(view.increment, 5);
        EXPECT_EQ(view.doubleValue, 30);
        EXPECT_FALSE(view.valueWraps);
    }

    TEST_F(apple_stepper_seam, setting_value_updates_the_nsstepper)
    {
        stepper control(0, 100, 0, 1);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(native_stepper(handler).doubleValue, 42);
    }

    TEST_F(apple_stepper_seam, native_step_flows_back_to_the_control)
    {
        stepper control(0, 100, 10, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // A real native step: -[NSStepper performClick:] steps DOWN by `increment` and fires the
        // action (verified empirically — NSStepperCell's programmatic click takes the minus half; the
        // AppKit analog of tapping UIStepper's minus button).
        [native_stepper(handler) performClick:nil];

        EXPECT_EQ(control.value(), 5);
        EXPECT_EQ(reported_new, 5);
    }

    TEST_F(apple_stepper_seam, native_steps_clamp_at_the_native_minimum)
    {
        stepper control(0, 12, 3, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        // With valueWraps disabled, NSStepper clamps the overshooting (downward — see above) step at
        // minValue instead of wrapping to maxValue (the pre-26 UIStepper clamp behavior).
        [native_stepper(handler) performClick:nil];
        EXPECT_EQ(native_stepper(handler).doubleValue, 0);
        EXPECT_EQ(control.value(), 0);
    }

    TEST_F(apple_stepper_seam, non_positive_increment_keeps_the_native_step)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_stepper(handler).increment, 1);

        control.set_increment(0); // UpdateIncrement only pushes a positive step
        EXPECT_EQ(native_stepper(handler).increment, 1);
    }

    TEST_F(apple_stepper_seam, generic_iview_properties_reach_the_nsstepper)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        NSStepper* const view = native_stepper(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);
    }

    TEST_F(apple_stepper_seam, clearing_handler_disconnects)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_stepper_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<stepper>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<stepper_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        stepper control(0, 10, 7, 1);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSStepper*)resolved->typed_platform_view()->native).doubleValue, 7);
    }
} // namespace

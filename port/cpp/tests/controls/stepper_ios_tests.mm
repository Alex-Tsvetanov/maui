// iOS (UIKit) backend tests for the stepper seam — run only for MAUI_BACKEND=ios (executed ON the iOS
// simulator via tools/ios-sim-run.sh). Drives a genuine UIStepper: Min/Max/Increment/Value map to
// minimumValue/maximumValue/stepValue/value, the native ValueChanged flows back through the handler's
// proxy, and the iOS-26 boundary stepValue adjustment (AdjustStepValueForBoundaries — live behavior on
// this simulator floor) keeps the exact bounds reachable. Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION: as in button_ios_tests.mm, send_control_event replicates UIControl's
// documented dispatch walk over the REAL ValueChanged registration the handler made.
#import <UIKit/UIKit.h>

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

    UIStepper* native_stepper(const std::shared_ptr<stepper_handler>& handler)
    {
        return (__bridge UIStepper*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see
    // the header comment).
    void send_control_event(UIControl* control, UIControlEvents event)
    {
        NSArray* const targets = control.allTargets.allObjects;
        for (NSUInteger t = 0; t < targets.count; ++t)
        {
            id const target = targets[t];
            NSArray<NSString*>* const actions = [control actionsForTarget:target forControlEvent:event];
            for (NSUInteger a = 0; a < actions.count; ++a)
            {
                SEL const action = NSSelectorFromString(actions[a]);
                NSMethodSignature* const signature = [target methodSignatureForSelector:action];
                ASSERT_NE(signature, nil);
                NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
                invocation.selector = action;
                id sender = control;
                [invocation setArgument:&sender atIndex:2]; // 0 = self, 1 = _cmd, 2 = the sender
                [invocation invokeWithTarget:target];
            }
        }
    }

    TEST(ios_stepper_seam, attaching_handler_creates_uistepper_and_maps_range)
    {
        stepper control(10, 90, 30, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UIStepper* const view = native_stepper(handler);
        EXPECT_EQ(view.minimumValue, 10);
        EXPECT_EQ(view.maximumValue, 90);
        EXPECT_EQ(view.stepValue, 5);
        EXPECT_EQ(view.value, 30);
    }

    TEST(ios_stepper_seam, setting_value_updates_the_uistepper)
    {
        stepper control(0, 100, 0, 1);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(native_stepper(handler).value, 42);
    }

    TEST(ios_stepper_seam, native_step_flows_back_to_the_control)
    {
        stepper control(0, 100, 10, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // Simulate the plus-button tap: the native value advances, then ValueChanged fires.
        UIStepper* const view = native_stepper(handler);
        view.value = view.value + view.stepValue;
        send_control_event(view, UIControlEventValueChanged);

        EXPECT_EQ(control.value(), 15);
        EXPECT_EQ(reported_new, 15);
    }

    TEST(ios_stepper_seam, boundary_proximity_shrinks_the_step_value)
    {
        // The iOS-26 AdjustStepValueForBoundaries port. Attach mid-range (no adjustment needed —
        // NeedsStepValueAdjustment is false through the whole connect pass, keeping the state
        // deterministic; near a bound the C# adjustment toggles per mapper call by design).
        stepper control(0, 12, 5, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_stepper(handler).stepValue, 5);

        // Move next to the maximum: the remaining space (2) is smaller than the increment, so the
        // native stepValue shrinks to 2 — keeping the exact maximum reachable on a UIStepper that no
        // longer clamps overshooting steps.
        control.set_value(10);
        EXPECT_EQ(native_stepper(handler).stepValue, 2);

        // Taking that boundary step lands exactly on the maximum, and the proxy's pass restores the
        // full increment (the partial step is not promoted: 10 + 5 would overshoot the range).
        UIStepper* const view = native_stepper(handler);
        view.value = view.value + view.stepValue;
        send_control_event(view, UIControlEventValueChanged);
        EXPECT_EQ(control.value(), 12);
        EXPECT_EQ(native_stepper(handler).stepValue, 5);
    }

    TEST(ios_stepper_seam, step_value_restored_away_from_the_bounds)
    {
        stepper control(0, 12, 5, 5);
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);

        control.set_value(10); // near the bound: shrunk
        EXPECT_EQ(native_stepper(handler).stepValue, 2);

        control.set_value(5); // back to the middle: both spaces (5 and 7) fit the increment
        EXPECT_EQ(native_stepper(handler).stepValue, 5);
    }

    TEST(ios_stepper_seam, generic_iview_properties_reach_the_uistepper)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        UIStepper* const view = native_stepper(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);
    }

    TEST(ios_stepper_seam, clearing_handler_disconnects)
    {
        stepper control;
        auto handler = std::make_shared<stepper_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_stepper_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<stepper>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<stepper_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        stepper control(0, 10, 7, 1);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge UIStepper*)resolved->typed_platform_view()->native).value, 7);
    }
} // namespace

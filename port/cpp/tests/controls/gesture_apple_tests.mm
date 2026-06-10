// Apple (AppKit) backend tests for the gesture seam — run only for MAUI_BACKEND=apple. Asserts the
// gesture platform manager attaches/detaches REAL NSGestureRecognizers (and the pointer tracking
// area) on the handler's native NSView, and drives the bridges by firing each recognizer's
// REGISTERED target/selector pair directly (the MauiGestureTarget trampoline). NO synthetic mouse
// events are posted — real event synthesis needs a window-server session a test process does not own
// (the AppKit twin of the iOS suite's no-touch-synthesis rule) — and AppKit's NSGestureRecognizer
// resets an externally-set `state` straight back to Possible outside an active event sequence
// (verified), so the state-dependent drives (pan/pinch/press) fire the registered target/action with
// a test-subclass recognizer whose readable state/translation/location are overridden. That exercises
// the real registration + the real bridge callback against controlled native readings; only the
// window-server recognition step is skipped. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/swipe_direction.hpp"
#include <gtest/gtest.h>

// Test stand-ins for the state-dependent drives (see the header comment): the readable state — which
// AppKit zeroes outside a live event sequence — plus the pan translation are overridden so the REAL
// registered bridge callback can be fired against controlled native readings.
@interface MauiTestPanRecognizer : NSPanGestureRecognizer
@property(nonatomic) NSGestureRecognizerState testState;
@property(nonatomic) NSPoint testTranslation;
@end

@implementation MauiTestPanRecognizer
- (NSGestureRecognizerState)state
{
    return self.testState;
}

- (NSPoint)translationInView:(NSView*)view
{
    (void)view;
    return self.testTranslation;
}
@end

@interface MauiTestMagnificationRecognizer : NSMagnificationGestureRecognizer
@property(nonatomic) NSGestureRecognizerState testState;
@end

@implementation MauiTestMagnificationRecognizer
- (NSGestureRecognizerState)state
{
    return self.testState;
}
@end

@interface MauiTestPressRecognizer : NSPressGestureRecognizer
@property(nonatomic) NSGestureRecognizerState testState;
@end

@implementation MauiTestPressRecognizer
- (NSGestureRecognizerState)state
{
    return self.testState;
}
@end

namespace
{
    using maui::controls::button;
    using maui::controls::buttons_mask;
    using maui::controls::pan_gesture_recognizer;
    using maui::controls::pan_updated_event_args;
    using maui::controls::pinch_gesture_recognizer;
    using maui::controls::pinch_gesture_updated_event_args;
    using maui::controls::pointer_event_args;
    using maui::controls::pointer_gesture_recognizer;
    using maui::controls::swipe_gesture_recognizer;
    using maui::controls::swiped_event_args;
    using maui::controls::tap_gesture_recognizer;
    using maui::controls::tapped_event_args;
    using maui::core::gesture_status;
    using maui::core::swipe_direction;

    NSView* native_view(const std::shared_ptr<maui::core::button_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    // Send the registered (target, action) pair with `sender` as the one argument — NSInvocation
    // spells the dynamic send without the objc_msgSend function-pointer cast (the same approach the
    // ios button suite uses for UIControl actions).
    void invoke_action(id target, SEL action, id sender)
    {
        ASSERT_NE(target, nil);
        ASSERT_NE(action, nullptr);
        NSMethodSignature* const signature = [target methodSignatureForSelector:action];
        ASSERT_NE(signature, nil);
        NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
        invocation.selector = action;
        id argument = sender;
        [invocation setArgument:&argument atIndex:2]; // 0 = self, 1 = _cmd
        [invocation invokeWithTarget:target];
    }

    // Fire a recognizer's registered target/selector pair directly (the documented test seam — see
    // the header comment). The action signature is the standard -onGesture:(NSGestureRecognizer*).
    void fire_action(NSGestureRecognizer* recognizer)
    {
        invoke_action(recognizer.target, recognizer.action, recognizer);
    }

    // Fire the REGISTERED recognizer's target/selector pair with a test stand-in as the sender (the
    // state-dependent drives — see the header comment).
    void fire_action_as(NSGestureRecognizer* registered, NSGestureRecognizer* sender)
    {
        invoke_action(registered.target, registered.action, sender);
    }

    // NSView/NSButton creation needs the shared application object (no run loop required).
    class apple_gesture_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_gesture_seam, tap_attaches_nsclick_recognizer_with_config)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        tap->set_number_of_taps_required(2);
        tap->set_buttons(buttons_mask::primary | buttons_mask::secondary);
        control.gesture_recognizers().add(tap);

        NSView* const view = native_view(handler);
        ASSERT_EQ(view.gestureRecognizers.count, 1U);
        auto* const click = (NSClickGestureRecognizer*)view.gestureRecognizers.firstObject;
        ASSERT_TRUE([click isKindOfClass:[NSClickGestureRecognizer class]]);
        EXPECT_EQ(click.numberOfClicksRequired, 2);
        EXPECT_EQ(click.buttonMask, 0x3U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 1U);
    }

    TEST_F(apple_gesture_seam, tap_action_fires_tapped_event)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        control.gesture_recognizers().add(tap);

        std::vector<tapped_event_args> raised;
        tap->tapped.connect([&raised](const tapped_event_args& e) { raised.push_back(e); });

        fire_action(native_view(handler).gestureRecognizers.firstObject);
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0].buttons, buttons_mask::primary);
        EXPECT_TRUE(raised[0].position.has_value()); // locationInView read through the bridge
    }

    TEST_F(apple_gesture_seam, tap_buttons_change_reconfigures_native_recognizer)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        control.gesture_recognizers().add(tap);

        auto* const click = (NSClickGestureRecognizer*)native_view(handler).gestureRecognizers.firstObject;
        EXPECT_EQ(click.buttonMask, 0x1U);

        tap->set_buttons(buttons_mask::secondary); // OnTapGestureRecognizerPropertyChanged
        EXPECT_EQ(click.buttonMask, 0x2U);
        tap->set_number_of_taps_required(3);
        EXPECT_EQ(click.numberOfClicksRequired, 3);
    }

    TEST_F(apple_gesture_seam, pan_action_drives_the_state_machine)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pan = std::make_shared<pan_gesture_recognizer>();
        control.gesture_recognizers().add(pan);

        NSView* const view = native_view(handler);
        ASSERT_EQ(view.gestureRecognizers.count, 1U);
        auto* const native_pan = (NSPanGestureRecognizer*)view.gestureRecognizers.firstObject;
        ASSERT_TRUE([native_pan isKindOfClass:[NSPanGestureRecognizer class]]);

        std::vector<pan_updated_event_args> updates;
        pan->pan_updated.connect([&updates](const pan_updated_event_args& e) { updates.push_back(e); });

        MauiTestPanRecognizer* const drive = [[MauiTestPanRecognizer alloc] init];
        drive.testState = NSGestureRecognizerStateBegan;
        fire_action_as(native_pan, drive);
        drive.testTranslation = NSMakePoint(10, 20);
        drive.testState = NSGestureRecognizerStateChanged;
        fire_action_as(native_pan, drive);
        drive.testState = NSGestureRecognizerStateEnded;
        fire_action_as(native_pan, drive);

        ASSERT_EQ(updates.size(), 3U);
        EXPECT_EQ(updates[0].status_type, gesture_status::started);
        EXPECT_EQ(updates[1].status_type, gesture_status::running);
        EXPECT_EQ(updates[1].total_x, 10);
        // MAUI's TotalY grows downward; the bridge negates for non-flipped views (the unattached
        // stand-in has no view → non-flipped).
        EXPECT_EQ(updates[1].total_y, -20);
        EXPECT_EQ(updates[2].status_type, gesture_status::completed);
        EXPECT_EQ(updates[0].gesture_id, updates[2].gesture_id); // one id per gesture
    }

    TEST_F(apple_gesture_seam, pinch_magnification_drives_pinch_updated)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pinch = std::make_shared<pinch_gesture_recognizer>();
        control.gesture_recognizers().add(pinch);

        NSView* const view = native_view(handler);
        auto* const magnify = (NSMagnificationGestureRecognizer*)view.gestureRecognizers.firstObject;
        ASSERT_TRUE([magnify isKindOfClass:[NSMagnificationGestureRecognizer class]]);

        std::vector<pinch_gesture_updated_event_args> updates;
        pinch->pinch_updated.connect([&updates](const pinch_gesture_updated_event_args& e) { updates.push_back(e); });

        MauiTestMagnificationRecognizer* const drive = [[MauiTestMagnificationRecognizer alloc] init];
        drive.testState = NSGestureRecognizerStateBegan;
        fire_action_as(magnify, drive);
        drive.magnification = 0.5; // cumulative AppKit reading → scale 1.5
        drive.testState = NSGestureRecognizerStateChanged;
        fire_action_as(magnify, drive);
        drive.testState = NSGestureRecognizerStateEnded;
        fire_action_as(magnify, drive);

        ASSERT_EQ(updates.size(), 3U);
        EXPECT_EQ(updates[0].status, gesture_status::started);
        EXPECT_EQ(updates[1].status, gesture_status::running);
        // pinch_scale_delta(previous=1.0, scale=1.5, starting=1.0) = 1.5 (the iOS bridge math).
        EXPECT_DOUBLE_EQ(updates[1].scale, 1.5);
        EXPECT_EQ(updates[2].status, gesture_status::completed);
        EXPECT_FALSE(pinch->is_pinching());
    }

    TEST_F(apple_gesture_seam, swipe_is_synthesized_from_pan_deltas)
    {
        // AppKit has NO swipe recognizer: the bridge attaches a dedicated NSPanGestureRecognizer and
        // routes its deltas through SendSwipe/DetectSwipe (the documented synthesis).
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto swipe = std::make_shared<swipe_gesture_recognizer>();
        swipe->set_direction(swipe_direction::left);
        control.gesture_recognizers().add(swipe);

        NSView* const view = native_view(handler);
        ASSERT_EQ(view.gestureRecognizers.count, 1U);
        auto* const native_pan = (NSPanGestureRecognizer*)view.gestureRecognizers.firstObject;
        ASSERT_TRUE([native_pan isKindOfClass:[NSPanGestureRecognizer class]]);

        std::vector<swipe_direction> raised;
        swipe->swiped.connect([&raised](const swiped_event_args& e) { raised.push_back(e.direction); });

        // Below the threshold: accumulate then end → no detection.
        MauiTestPanRecognizer* const drive = [[MauiTestPanRecognizer alloc] init];
        drive.testTranslation = NSMakePoint(-50, 0);
        drive.testState = NSGestureRecognizerStateChanged;
        fire_action_as(native_pan, drive);
        drive.testState = NSGestureRecognizerStateEnded;
        fire_action_as(native_pan, drive);
        EXPECT_TRUE(raised.empty());

        // Beyond the 100px default threshold → swiped(left).
        drive.testTranslation = NSMakePoint(-150, 0);
        drive.testState = NSGestureRecognizerStateChanged;
        fire_action_as(native_pan, drive);
        drive.testState = NSGestureRecognizerStateEnded;
        fire_action_as(native_pan, drive);
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0], swipe_direction::left);
    }

    TEST_F(apple_gesture_seam, pointer_attaches_tracking_area_and_press_recognizer)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        control.gesture_recognizers().add(pointer);

        NSView* const view = native_view(handler);
        EXPECT_EQ(view.trackingAreas.count, 1U); // hover: entered/exited/moved
        ASSERT_EQ(view.gestureRecognizers.count, 1U);
        auto* const press = (NSPressGestureRecognizer*)view.gestureRecognizers.firstObject;
        ASSERT_TRUE([press isKindOfClass:[NSPressGestureRecognizer class]]);
        EXPECT_EQ(press.minimumPressDuration, 0);

        std::vector<std::string> raised;
        pointer->pointer_pressed.connect([&raised](const pointer_event_args&) { raised.emplace_back("pressed"); });
        pointer->pointer_released.connect([&raised](const pointer_event_args&) { raised.emplace_back("released"); });

        MauiTestPressRecognizer* const drive = [[MauiTestPressRecognizer alloc] init];
        drive.testState = NSGestureRecognizerStateBegan;
        fire_action_as(press, drive);
        drive.testState = NSGestureRecognizerStateEnded;
        fire_action_as(press, drive);

        const std::vector<std::string> expected{"pressed", "released"};
        EXPECT_EQ(raised, expected);
    }

    TEST_F(apple_gesture_seam, pointer_tracking_owner_routes_hover_messages)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        control.gesture_recognizers().add(pointer);

        NSView* const view = native_view(handler);
        ASSERT_EQ(view.trackingAreas.count, 1U);
        id const owner = view.trackingAreas.firstObject.owner;
        ASSERT_NE(owner, nil);

        std::vector<std::string> raised;
        pointer->pointer_entered.connect([&raised](const pointer_event_args&) { raised.emplace_back("entered"); });
        pointer->pointer_moved.connect([&raised](const pointer_event_args&) { raised.emplace_back("moved"); });
        pointer->pointer_exited.connect([&raised](const pointer_event_args&) { raised.emplace_back("exited"); });

        // Deliver the tracking messages straight to the owner (the same selectors AppKit sends; no
        // window-server event synthesis in a test process — documented).
        NSEvent* const enter = [NSEvent enterExitEventWithType:NSEventTypeMouseEntered
                                                      location:NSMakePoint(1, 1)
                                                 modifierFlags:0
                                                     timestamp:0
                                                  windowNumber:0
                                                       context:nil
                                                   eventNumber:0
                                                trackingNumber:0
                                                      userData:nil];
        ASSERT_NE(enter, nil); // the factory is nullable-annotated; the owner takes a non-null event
        [owner mouseEntered:enter];
        [owner mouseMoved:enter];
        [owner mouseExited:enter];

        const std::vector<std::string> expected{"entered", "moved", "exited"};
        EXPECT_EQ(raised, expected);
    }

    TEST_F(apple_gesture_seam, removing_recognizer_detaches_native)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        control.gesture_recognizers().add(tap);
        control.gesture_recognizers().add(pointer);

        NSView* const view = native_view(handler);
        EXPECT_EQ(view.gestureRecognizers.count, 2U); // click + press
        EXPECT_EQ(view.trackingAreas.count, 1U);

        EXPECT_TRUE(control.gesture_recognizers().remove(tap));
        EXPECT_EQ(view.gestureRecognizers.count, 1U);

        EXPECT_TRUE(control.gesture_recognizers().remove(pointer));
        EXPECT_EQ(view.gestureRecognizers.count, 0U);
        EXPECT_EQ(view.trackingAreas.count, 0U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 0U);
    }

    TEST_F(apple_gesture_seam, clearing_handler_detaches_native)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());

        NSView* const view = native_view(handler);
        EXPECT_EQ(view.gestureRecognizers.count, 1U);

        control.set_handler(nullptr); // GestureManager.DisconnectGestures
        EXPECT_EQ(view.gestureRecognizers.count, 0U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 0U);
    }
} // namespace

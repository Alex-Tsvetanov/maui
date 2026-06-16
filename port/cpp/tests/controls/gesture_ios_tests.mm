// iOS (UIKit) backend tests for the gesture seam — run only for MAUI_BACKEND=ios (executed ON the
// iOS simulator via tools/ios-sim-run.sh). Asserts the gesture platform manager attaches/detaches
// REAL UIGestureRecognizers on the handler's native UIView and drives the bridges by firing each
// recognizer's REGISTERED trampoline target directly (retrieved through the associated-object key —
// UIGestureRecognizer exposes no target/action query). NO touch synthesis happens on the simulator
// (documented): a spawned test process has no UIApplication event loop to deliver synthesized
// touches, so the state-dependent drives (pan/pinch/hover/press) fire the registered target with a
// test-subclass recognizer whose readable state/translation are overridden — exercising the real
// registration + the real bridge callback against controlled native readings; only UIKit's
// recognition step is skipped. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ios_gesture_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
#include "maui/controls/gestures/drop_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/swipe_direction.hpp"
#include <gtest/gtest.h>

// Test stand-ins for the state-dependent drives (see the header comment): the readable state — plus
// the pan translation — are overridden so the REAL registered bridge callback can be fired against
// controlled native readings.
@interface MauiTestPanRecognizer : UIPanGestureRecognizer
@property(nonatomic) UIGestureRecognizerState testState;
@property(nonatomic) CGPoint testTranslation;
@end

@implementation MauiTestPanRecognizer
- (UIGestureRecognizerState)state
{
    return self.testState;
}

- (CGPoint)translationInView:(UIView*)view
{
    (void)view;
    return self.testTranslation;
}
@end

@interface MauiTestPinchRecognizer : UIPinchGestureRecognizer
@property(nonatomic) UIGestureRecognizerState testState;
@end

@implementation MauiTestPinchRecognizer
- (UIGestureRecognizerState)state
{
    return self.testState;
}
@end

@interface MauiTestHoverRecognizer : UIHoverGestureRecognizer
@property(nonatomic) UIGestureRecognizerState testState;
@end

@implementation MauiTestHoverRecognizer
- (UIGestureRecognizerState)state
{
    return self.testState;
}
@end

// A NON-hover stand-in for the press half of the pointer pair (the bridge's action routes any
// non-UIHoverGestureRecognizer sender down the press path).
@interface MauiTestPressDriveRecognizer : UIGestureRecognizer
@property(nonatomic) UIGestureRecognizerState testState;
@end

@implementation MauiTestPressDriveRecognizer
- (UIGestureRecognizerState)state
{
    return self.testState;
}
@end

namespace
{
    using maui::controls::button;
    using maui::controls::buttons_mask;
    using maui::controls::drag_gesture_recognizer; // --- drag&drop (W2-22) ---
    using maui::controls::drop_gesture_recognizer; // --- drag&drop (W2-22) ---
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

    UIView* native_view(const std::shared_ptr<maui::core::button_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    // The recognizers the BRIDGE attached (identified by their associated trampoline target): a real
    // UIButton already carries UIKit-internal recognizers of its own, so the raw gestureRecognizers
    // list cannot be counted directly.
    NSArray<UIGestureRecognizer*>* maui_recognizers(UIView* view)
    {
        NSMutableArray<UIGestureRecognizer*>* const result = [NSMutableArray array];
        NSArray<UIGestureRecognizer*>* const all = view.gestureRecognizers;
        for (NSUInteger i = 0; i < all.count; ++i)
        {
            UIGestureRecognizer* const recognizer = all[i];
            if (objc_getAssociatedObject(recognizer, maui::platform::ios::gesture_target_key()) != nil)
            {
                [result addObject:recognizer];
            }
        }
        return result;
    }

    // Fire the REGISTERED recognizer's trampoline target (the associated object the bridge stored)
    // with `sender` as the action argument — the documented no-touch-synthesis test seam.
    // NSInvocation spells the dynamic send without the objc_msgSend function-pointer cast (the same
    // approach the button suite uses for UIControl actions).
    void fire_registered_target(UIGestureRecognizer* registered, UIGestureRecognizer* sender)
    {
        id const target = objc_getAssociatedObject(registered, maui::platform::ios::gesture_target_key());
        ASSERT_NE(target, nil);
        NSMethodSignature* const signature = [target methodSignatureForSelector:@selector(onGesture:)];
        ASSERT_NE(signature, nil);
        NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
        invocation.selector = @selector(onGesture:);
        id argument = sender;
        [invocation setArgument:&argument atIndex:2]; // 0 = self, 1 = _cmd
        [invocation invokeWithTarget:target];
    }

    TEST(gesture_ios_seam, tap_attaches_uitap_recognizer_with_config)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        tap->set_number_of_taps_required(2);
        tap->set_buttons(buttons_mask::primary | buttons_mask::secondary);
        control.gesture_recognizers().add(tap);

        UIView* const view = native_view(handler);
        ASSERT_EQ(maui_recognizers(view).count, 1U);
        auto* const native_tap = (UITapGestureRecognizer*)maui_recognizers(view).firstObject;
        ASSERT_TRUE([native_tap isKindOfClass:[UITapGestureRecognizer class]]);
        EXPECT_EQ(native_tap.numberOfTapsRequired, 2U);
        if (@available(iOS 13.4, *))
        {
            EXPECT_EQ(native_tap.buttonMaskRequired, UIEventButtonMaskPrimary | UIEventButtonMaskSecondary);
        }
        EXPECT_EQ(control.gesture_manager().attached_count(), 1U);
    }

    TEST(gesture_ios_seam, tap_action_fires_tapped_event)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        control.gesture_recognizers().add(tap);

        std::vector<tapped_event_args> raised;
        tap->tapped.connect([&raised](const tapped_event_args& e) { raised.push_back(e); });

        UIGestureRecognizer* const native_tap = maui_recognizers(native_view(handler)).firstObject;
        fire_registered_target(native_tap, native_tap); // a tap action is state-independent
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0].buttons, buttons_mask::primary);
        EXPECT_TRUE(raised[0].position.has_value()); // locationInView read through the bridge
    }

    TEST(gesture_ios_seam, tap_buttons_change_reconfigures_native_recognizer)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        control.gesture_recognizers().add(tap);

        auto* const native_tap = (UITapGestureRecognizer*)maui_recognizers(native_view(handler)).firstObject;
        EXPECT_EQ(native_tap.numberOfTapsRequired, 1U);

        tap->set_number_of_taps_required(3); // OnTapGestureRecognizerPropertyChanged
        EXPECT_EQ(native_tap.numberOfTapsRequired, 3U);
        if (@available(iOS 13.4, *))
        {
            tap->set_buttons(buttons_mask::secondary);
            EXPECT_EQ(native_tap.buttonMaskRequired, UIEventButtonMaskSecondary);
        }
    }

    TEST(gesture_ios_seam, pan_action_drives_the_state_machine)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pan = std::make_shared<pan_gesture_recognizer>();
        control.gesture_recognizers().add(pan);

        UIView* const view = native_view(handler);
        ASSERT_EQ(maui_recognizers(view).count, 1U);
        auto* const native_pan = (UIPanGestureRecognizer*)maui_recognizers(view).firstObject;
        ASSERT_TRUE([native_pan isKindOfClass:[UIPanGestureRecognizer class]]);
        EXPECT_EQ(native_pan.minimumNumberOfTouches, 1U); // pinned to TouchPoints
        EXPECT_EQ(native_pan.maximumNumberOfTouches, 1U);

        std::vector<pan_updated_event_args> updates;
        pan->pan_updated.connect([&updates](const pan_updated_event_args& e) { updates.push_back(e); });

        MauiTestPanRecognizer* const drive = [[MauiTestPanRecognizer alloc] init];
        drive.testState = UIGestureRecognizerStateBegan;
        fire_registered_target(native_pan, drive);
        drive.testTranslation = CGPointMake(10, 20);
        drive.testState = UIGestureRecognizerStateChanged;
        fire_registered_target(native_pan, drive);
        drive.testState = UIGestureRecognizerStateEnded;
        fire_registered_target(native_pan, drive);

        ASSERT_EQ(updates.size(), 3U);
        EXPECT_EQ(updates[0].status_type, gesture_status::started);
        EXPECT_EQ(updates[1].status_type, gesture_status::running);
        EXPECT_EQ(updates[1].total_x, 10);
        EXPECT_EQ(updates[1].total_y, 20); // UIKit's y grows down, like MAUI's TotalY
        EXPECT_EQ(updates[2].status_type, gesture_status::completed);
        EXPECT_EQ(updates[0].gesture_id, updates[2].gesture_id); // one id per gesture
    }

    TEST(gesture_ios_seam, pinch_scale_drives_pinch_updated)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pinch = std::make_shared<pinch_gesture_recognizer>();
        control.gesture_recognizers().add(pinch);

        UIView* const view = native_view(handler);
        auto* const native_pinch = (UIPinchGestureRecognizer*)maui_recognizers(view).firstObject;
        ASSERT_TRUE([native_pinch isKindOfClass:[UIPinchGestureRecognizer class]]);

        std::vector<pinch_gesture_updated_event_args> updates;
        pinch->pinch_updated.connect([&updates](const pinch_gesture_updated_event_args& e) { updates.push_back(e); });

        MauiTestPinchRecognizer* const drive = [[MauiTestPinchRecognizer alloc] init];
        drive.testState = UIGestureRecognizerStateBegan;
        fire_registered_target(native_pinch, drive);
        drive.scale = 1.5;
        drive.testState = UIGestureRecognizerStateChanged;
        fire_registered_target(native_pinch, drive);
        drive.testState = UIGestureRecognizerStateEnded;
        fire_registered_target(native_pinch, drive);

        ASSERT_EQ(updates.size(), 3U);
        EXPECT_EQ(updates[0].status, gesture_status::started);
        EXPECT_EQ(updates[1].status, gesture_status::running);
        // pinch_scale_delta(previous=1.0, scale=1.5, starting=1.0) = 1.5 (the C# Changed-case math).
        EXPECT_DOUBLE_EQ(updates[1].scale, 1.5);
        EXPECT_EQ(updates[2].status, gesture_status::completed);
        EXPECT_FALSE(pinch->is_pinching());
    }

    TEST(gesture_ios_seam, swipe_attaches_uiswipe_and_sends_swiped_on_recognition)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto swipe = std::make_shared<swipe_gesture_recognizer>();
        swipe->set_direction(swipe_direction::left);
        control.gesture_recognizers().add(swipe);

        UIView* const view = native_view(handler);
        ASSERT_EQ(maui_recognizers(view).count, 1U);
        auto* const native_swipe = (UISwipeGestureRecognizer*)maui_recognizers(view).firstObject;
        ASSERT_TRUE([native_swipe isKindOfClass:[UISwipeGestureRecognizer class]]);
        EXPECT_EQ(native_swipe.direction, UISwipeGestureRecognizerDirectionLeft); // bit-identical enums

        // A Direction change updates the native mask (OnSwipeGestureRecognizerPropertyChanged).
        swipe->set_direction(swipe_direction::right);
        EXPECT_EQ(native_swipe.direction, UISwipeGestureRecognizerDirectionRight);

        std::vector<swipe_direction> raised;
        swipe->swiped.connect([&raised](const swiped_event_args& e) { raised.push_back(e.direction); });

        // UIKit detects natively — the action sends Swiped directly (no threshold consultation).
        fire_registered_target(native_swipe, native_swipe);
        ASSERT_EQ(raised.size(), 1U);
        EXPECT_EQ(raised[0], swipe_direction::right);
    }

    TEST(gesture_ios_seam, pointer_attaches_hover_and_press_pair)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        control.gesture_recognizers().add(pointer);

        UIView* const view = native_view(handler);
        ASSERT_EQ(maui_recognizers(view).count, 2U); // UIHover + the CustomPress port
        EXPECT_TRUE([maui_recognizers(view).firstObject isKindOfClass:[UIHoverGestureRecognizer class]]);

        std::vector<std::string> raised;
        pointer->pointer_entered.connect([&raised](const pointer_event_args&) { raised.emplace_back("entered"); });
        pointer->pointer_moved.connect([&raised](const pointer_event_args&) { raised.emplace_back("moved"); });
        pointer->pointer_exited.connect([&raised](const pointer_event_args&) { raised.emplace_back("exited"); });
        pointer->pointer_pressed.connect([&raised](const pointer_event_args&) { raised.emplace_back("pressed"); });
        pointer->pointer_released.connect([&raised](const pointer_event_args&) { raised.emplace_back("released"); });

        // Hover drive: Began → entered, Changed → moved, Ended → exited (the C# hover switch arm).
        UIGestureRecognizer* const hover = maui_recognizers(view).firstObject;
        MauiTestHoverRecognizer* const hover_drive = [[MauiTestHoverRecognizer alloc] init];
        hover_drive.testState = UIGestureRecognizerStateBegan;
        fire_registered_target(hover, hover_drive);
        hover_drive.testState = UIGestureRecognizerStateChanged;
        fire_registered_target(hover, hover_drive);
        hover_drive.testState = UIGestureRecognizerStateEnded;
        fire_registered_target(hover, hover_drive);

        // Press drive: Began → pressed, Ended → released (the non-hover arm; the stand-in is a plain
        // UIGestureRecognizer subclass so the bridge routes it down the press path).
        UIGestureRecognizer* const press = maui_recognizers(view).lastObject;
        MauiTestPressDriveRecognizer* const press_drive = [[MauiTestPressDriveRecognizer alloc] init];
        press_drive.testState = UIGestureRecognizerStateBegan;
        fire_registered_target(press, press_drive);
        press_drive.testState = UIGestureRecognizerStateEnded;
        fire_registered_target(press, press_drive);

        const std::vector<std::string> expected{"entered", "moved", "exited", "pressed", "released"};
        EXPECT_EQ(raised, expected);
    }

    // The blocked half of the non-hover mask filter (a primary press on a secondary-only mask) is
    // covered headless (gesture_recognizer_tests.cpp's synthetic_pointer case); this pins the
    // Catalyst fallback half — a secondary-ONLY mask reports secondary, which passes its own mask.
    TEST(gesture_ios_seam, pointer_press_on_secondary_only_mask_reports_secondary)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        pointer->set_buttons(buttons_mask::secondary); // secondary-only mask
        control.gesture_recognizers().add(pointer);

        int pressed = 0;
        pointer->pointer_pressed.connect([&pressed](const pointer_event_args&) { ++pressed; });

        // The secondary-only mask reports secondary (the Catalyst fallback) → it passes its own mask.
        UIGestureRecognizer* const press = maui_recognizers(native_view(handler)).lastObject;
        MauiTestPressDriveRecognizer* const drive = [[MauiTestPressDriveRecognizer alloc] init];
        drive.testState = UIGestureRecognizerStateBegan;
        fire_registered_target(press, drive);
        EXPECT_EQ(pressed, 1);
    }

    TEST(gesture_ios_seam, removing_recognizer_detaches_native)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto tap = std::make_shared<tap_gesture_recognizer>();
        auto pointer = std::make_shared<pointer_gesture_recognizer>();
        control.gesture_recognizers().add(tap);
        control.gesture_recognizers().add(pointer);

        UIView* const view = native_view(handler);
        EXPECT_EQ(maui_recognizers(view).count, 3U); // tap + (hover + press)

        EXPECT_TRUE(control.gesture_recognizers().remove(pointer));
        EXPECT_EQ(maui_recognizers(view).count, 1U);

        EXPECT_TRUE(control.gesture_recognizers().remove(tap));
        EXPECT_EQ(maui_recognizers(view).count, 0U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 0U);
    }

    TEST(gesture_ios_seam, clearing_handler_detaches_native)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());

        UIView* const view = native_view(handler);
        EXPECT_EQ(maui_recognizers(view).count, 1U);

        control.set_handler(nullptr); // GestureManager.DisconnectGestures
        EXPECT_EQ(maui_recognizers(view).count, 0U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 0U);
    }

    // --- U21 gesture-delegate arbitration (ShouldReceiveTouchProxy + ShouldRecognizeSimultaneously) ---
    // Every native recognizer the bridge attaches gets the shared arbitration delegate (the C#
    // _proxy reused across all recognizers). Synthesizing a UITouch isn't supported by the spawned
    // simulator lane (no UIApplication event loop to mint events), so the touch decision is unit-tested
    // through the pure decision helper (should_receive_touch over a controlled touch-view), while the
    // delegate ASSIGNMENT and the simultaneity arms are asserted against the real native recognizers. ---

    TEST(gesture_ios_seam, attached_recognizer_gets_arbitration_delegate)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        control.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());

        UIGestureRecognizer* const native_tap = maui_recognizers(native_view(handler)).firstObject;
        ASSERT_NE(native_tap, nil);
        EXPECT_NE(native_tap.delegate, nil); // the shared ShouldReceiveTouchProxy delegate
    }

    TEST(gesture_ios_seam, should_receive_touch_blocks_input_transparent)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.set_input_transparent(true); // virtualView.InputTransparent → false

        UIView* const view = native_view(handler);
        EXPECT_FALSE(maui::platform::ios::should_receive_touch(&control, view, view));
    }

    TEST(gesture_ios_seam, should_receive_touch_blocks_disabled)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.set_is_enabled(false); // !virtualView.IsEnabled → false

        UIView* const view = native_view(handler);
        EXPECT_FALSE(maui::platform::ios::should_receive_touch(&control, view, view));
    }

    TEST(gesture_ios_seam, should_receive_touch_allows_own_view)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        UIView* const view = native_view(handler);
        // touch.View == platformView → true (the first allow arm).
        EXPECT_TRUE(maui::platform::ios::should_receive_touch(&control, view, view));
        // A touch landing outside the platformView hierarchy → false (no descendant, no own view).
        UIView* const outsider = [[UIView alloc] init];
        EXPECT_FALSE(maui::platform::ios::should_receive_touch(&control, view, outsider));
        // A null virtual view / null platform view → false (the WeakReference-dead guard).
        EXPECT_FALSE(maui::platform::ios::should_receive_touch(nullptr, view, view));
        EXPECT_FALSE(maui::platform::ios::should_receive_touch(&control, nil, view));
    }

    TEST(gesture_ios_seam, should_recognize_simultaneously_pointer_always_true)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<pointer_gesture_recognizer>());

        UIView* const view = native_view(handler);
        id<UIGestureRecognizerDelegate> const delegate =
            (id<UIGestureRecognizerDelegate>)maui_recognizers(view).firstObject.delegate;
        ASSERT_NE(delegate, nil);
        ASSERT_TRUE([delegate
            respondsToSelector:@selector(gestureRecognizer:shouldRecognizeSimultaneouslyWithGestureRecognizer:)]);
        // UIHover and the CustomPress port both report true unconditionally (the C# pointer arms).
        UIGestureRecognizer* const hover = maui_recognizers(view).firstObject;
        UIGestureRecognizer* const press = maui_recognizers(view).lastObject;
        UIGestureRecognizer* const other = [[UIPanGestureRecognizer alloc] init];
        EXPECT_TRUE([delegate gestureRecognizer:hover shouldRecognizeSimultaneouslyWithGestureRecognizer:other]);
        EXPECT_TRUE([delegate gestureRecognizer:press shouldRecognizeSimultaneouslyWithGestureRecognizer:other]);
    }

    TEST(gesture_ios_seam, should_recognize_simultaneously_tap_same_count)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<tap_gesture_recognizer>());

        UIView* const view = native_view(handler);
        auto* const native_tap = (UITapGestureRecognizer*)maui_recognizers(view).firstObject;
        id<UIGestureRecognizerDelegate> const delegate = (id<UIGestureRecognizerDelegate>)native_tap.delegate;
        ASSERT_NE(delegate, nil);

        // Same taps + touches + view → simultaneous (ShouldRecognizeTapsTogether allows it). The tap is
        // already on `view`; another tap added to the same view with matching counts is allowed together.
        auto* const other_same = [[UITapGestureRecognizer alloc] init];
        other_same.numberOfTapsRequired = native_tap.numberOfTapsRequired;
        other_same.numberOfTouchesRequired = native_tap.numberOfTouchesRequired;
        [view addGestureRecognizer:other_same];
        EXPECT_TRUE([delegate gestureRecognizer:native_tap
            shouldRecognizeSimultaneouslyWithGestureRecognizer:other_same]);

        // A different tap count → not simultaneous.
        auto* const other_diff = [[UITapGestureRecognizer alloc] init];
        other_diff.numberOfTapsRequired = native_tap.numberOfTapsRequired + 1;
        [view addGestureRecognizer:other_diff];
        EXPECT_FALSE([delegate gestureRecognizer:native_tap
            shouldRecognizeSimultaneouslyWithGestureRecognizer:other_diff]);

        // A non-tap other → not simultaneous (the cast-to-tap guard).
        auto* const non_tap = [[UIPanGestureRecognizer alloc] init];
        EXPECT_FALSE([delegate gestureRecognizer:native_tap
            shouldRecognizeSimultaneouslyWithGestureRecognizer:non_tap]);
    }

    TEST(gesture_ios_seam, should_recognize_simultaneously_swipe_scrollview_blocks)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<swipe_gesture_recognizer>());

        UIView* const view = native_view(handler);
        auto* const native_swipe = (UISwipeGestureRecognizer*)maui_recognizers(view).firstObject;
        id<UIGestureRecognizerDelegate> const delegate = (id<UIGestureRecognizerDelegate>)native_swipe.delegate;
        ASSERT_NE(delegate, nil);

        // other.View is a UIScrollView → false (let the scroll view win); otherwise → true.
        UIScrollView* const scroll = [[UIScrollView alloc] init];
        auto* const scroll_gesture = [[UIPanGestureRecognizer alloc] init];
        [scroll addGestureRecognizer:scroll_gesture];
        EXPECT_FALSE([delegate gestureRecognizer:native_swipe
            shouldRecognizeSimultaneouslyWithGestureRecognizer:scroll_gesture]);

        UIView* const plain = [[UIView alloc] init];
        auto* const plain_gesture = [[UIPanGestureRecognizer alloc] init];
        [plain addGestureRecognizer:plain_gesture];
        EXPECT_TRUE([delegate gestureRecognizer:native_swipe
            shouldRecognizeSimultaneouslyWithGestureRecognizer:plain_gesture]);
    }

    TEST(gesture_ios_seam, should_recognize_simultaneously_pan_and_pinch_false)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);
        control.gesture_recognizers().add(std::make_shared<pan_gesture_recognizer>());
        control.gesture_recognizers().add(std::make_shared<pinch_gesture_recognizer>());

        UIView* const view = native_view(handler);
        NSArray<UIGestureRecognizer*>* const attached = maui_recognizers(view);
        UIGestureRecognizer* native_pan = nil;
        UIGestureRecognizer* native_pinch = nil;
        for (NSUInteger i = 0; i < attached.count; ++i)
        {
            UIGestureRecognizer* const recognizer = attached[i];
            if ([recognizer isKindOfClass:[UIPanGestureRecognizer class]])
            {
                native_pan = recognizer;
            }
            else if ([recognizer isKindOfClass:[UIPinchGestureRecognizer class]])
            {
                native_pinch = recognizer;
            }
        }
        ASSERT_NE(native_pan, nil);
        ASSERT_NE(native_pinch, nil);
        auto* const other = [[UIPanGestureRecognizer alloc] init];
        // Pan: default false (Application config is TBD — documented). Pinch: no handler → false.
        EXPECT_FALSE([(id<UIGestureRecognizerDelegate>)native_pan.delegate gestureRecognizer:native_pan
                                          shouldRecognizeSimultaneouslyWithGestureRecognizer:other]);
        EXPECT_FALSE([(id<UIGestureRecognizerDelegate>)native_pinch.delegate gestureRecognizer:native_pinch
                                            shouldRecognizeSimultaneouslyWithGestureRecognizer:other]);
    }

    // --- drag&drop (W2-22): attachment-only native install/remove (LoadRecognizers' AddInteraction /
    // RemoveInteraction). The proof is the UIDragInteraction / UIDropInteraction on the view's
    // interactions; driving a UIDrag/UIDropSession isn't possible on the spawned simulator lane (no
    // UIApplication event loop; documented), so the session → SendDragStarting/SendDrop bridge stays
    // device-test territory (the cross-platform recognizer pins that behavior headlessly). ---
    namespace
    {
        template <class T> NSUInteger interaction_count(UIView* view)
        {
            NSUInteger count = 0;
            for (id<UIInteraction> interaction in view.interactions)
            {
                if ([interaction isKindOfClass:[T class]])
                {
                    ++count;
                }
            }
            return count;
        }
    } // namespace

    TEST(gesture_ios_seam, drag_recognizer_installs_drag_interaction)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto drag = std::make_shared<drag_gesture_recognizer>();
        control.gesture_recognizers().add(drag);

        UIView* const view = native_view(handler);
        EXPECT_EQ(interaction_count<UIDragInteraction>(view), 1U);
        EXPECT_EQ(interaction_count<UIDropInteraction>(view), 0U);
        EXPECT_EQ(maui_recognizers(view).count, 0U); // no UIGestureRecognizer for drag/drop
        EXPECT_TRUE(control.gesture_manager().native_registered_drag_source(*drag));
        EXPECT_TRUE(control.gesture_manager().is_attached(*drag));
    }

    TEST(gesture_ios_seam, drop_recognizer_installs_drop_interaction)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto drop = std::make_shared<drop_gesture_recognizer>();
        control.gesture_recognizers().add(drop);

        UIView* const view = native_view(handler);
        EXPECT_EQ(interaction_count<UIDropInteraction>(view), 1U);
        EXPECT_EQ(interaction_count<UIDragInteraction>(view), 0U);
        EXPECT_TRUE(control.gesture_manager().native_registered_drop_target(*drop));
        EXPECT_TRUE(control.gesture_manager().is_attached(*drop));
    }

    TEST(gesture_ios_seam, removing_drag_drop_recognizers_removes_interactions)
    {
        button control;
        auto handler = std::make_shared<maui::core::button_handler>();
        control.set_handler(handler);

        auto drag = std::make_shared<drag_gesture_recognizer>();
        auto drop = std::make_shared<drop_gesture_recognizer>();
        control.gesture_recognizers().add(drag);
        control.gesture_recognizers().add(drop);

        UIView* const view = native_view(handler);
        EXPECT_EQ(interaction_count<UIDragInteraction>(view), 1U);
        EXPECT_EQ(interaction_count<UIDropInteraction>(view), 1U);

        EXPECT_TRUE(control.gesture_recognizers().remove(drag));
        EXPECT_EQ(interaction_count<UIDragInteraction>(view), 0U);
        EXPECT_EQ(interaction_count<UIDropInteraction>(view), 1U);

        EXPECT_TRUE(control.gesture_recognizers().remove(drop));
        EXPECT_EQ(interaction_count<UIDropInteraction>(view), 0U);
        EXPECT_EQ(control.gesture_manager().attached_count(), 0U);
    }
} // namespace

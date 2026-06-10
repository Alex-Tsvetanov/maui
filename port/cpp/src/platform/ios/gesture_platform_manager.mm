// gesture_platform_manager — iOS (UIKit) platform partial: the native attach/detach half of the
// GestureManager/GesturePlatformManager pair, ported DIRECTLY from
// src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.iOS.cs (the same oracle the
// AppKit twin was adapted from). Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Per-recognizer recipe (each native action routes into the SAME controller-interface calls the C#
// bridge makes):
//   - tap   → UITapGestureRecognizer (CreateTapRecognizer): numberOfTapsRequired +
//             buttonMaskRequired (iOS 13.4+); a Buttons / NumberOfTapsRequired change re-configures
//             the native recognizer (OnTapGestureRecognizerPropertyChanged, narrowed to the knobs);
//   - pan   → UIPanGestureRecognizer (CreatePanRecognizer): min/max touches = TouchPoints, the
//             Began/Changed/Ended/Cancelled switch with PanGestureRecognizer.CurrentId stamping;
//   - pinch → UIPinchGestureRecognizer (CreatePinchRecognizer): the cumulative-scale →
//             per-update-delta math (shared pinch_scale_delta), the IsPinching guards, and the
//             view-relative scaled origin;
//   - swipe → UISwipeGestureRecognizer (CreateSwipeRecognizer): direction mask (bit-identical enums),
//             one touch; recognition calls SendSwiped(TransformSwipeDirectionForRotation(direction,
//             view.Rotation)) — UIKit detects natively, so the threshold seam is not consulted (as in
//             C#); a Direction change updates the native mask (OnSwipeGestureRecognizerPropertyChanged);
//   - pointer → UIHoverGestureRecognizer + MauiPressGestureRecognizer (the CustomPressGestureRecognizer
//             port: Began/Changed/Ended/Cancelled from the touches* overrides, CancelsTouchesInView
//             false), sharing one action exactly like CreatePointerRecognizer: hover Began → entered,
//             Changed → moved, end states → exited; press Began → pressed, Changed → moved while
//             inside the view's bounds (leaving sends exited and ends the recognizer), end states →
//             released. The non-hover button-mask filter is ported; the per-event DetectedButton is
//             narrowed to the event's buttonMask when available, else the Catalyst fallback.
//
// DOCUMENTED adaptations:
//   - the NumberOfTouches guards on pan Began/Changed and pinch Began only apply when the recognizer
//     reports touches (> 0): the on-simulator tests fire the registered target/selector directly (no
//     touch synthesis), where numberOfTouches is 0;
//   - the scaled pinch origin divides locationInView(view) by the view's own bounds instead of C#'s
//     window-coordinate detour (identical result for the arranged view);
//   - the trampoline target is retained via an associated object on the native recognizer
//     (UIGestureRecognizer does not retain targets; see ios_gesture_ops.hpp's gesture_target_key).

#import <UIKit/UIGestureRecognizerSubclass.h>
#import <UIKit/UIKit.h>

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "ios_gesture_ops.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_platform_manager.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_transform.hpp"
#include "maui/graphics/point.hpp"

// Obj-C trampoline: forwards a recognizer's action into a C++ callback. Retained by an associated
// object on the recognizer (UIGestureRecognizer holds its targets weakly).
@interface MauiGestureTarget : NSObject
- (instancetype)initWithCallback:(std::function<void(UIGestureRecognizer*)>)callback;
- (void)onGesture:(UIGestureRecognizer*)sender;
@end

@implementation MauiGestureTarget
{
    std::function<void(UIGestureRecognizer*)> _callback;
}

- (instancetype)initWithCallback:(std::function<void(UIGestureRecognizer*)>)callback
{
    self = [super init];
    if (self != nil)
    {
        _callback = std::move(callback);
    }
    return self;
}

- (void)onGesture:(UIGestureRecognizer*)sender
{
    if (_callback)
    {
        _callback(sender);
    }
}
@end

// The CustomPressGestureRecognizer.cs port: a raw UIGestureRecognizer whose touches* overrides drive
// the state machine (Began on touch-down — UIKit has no press recognizer that begins immediately),
// detecting the pressed button from the event's buttonMask when the platform reports one.
@interface MauiPressGestureRecognizer : UIGestureRecognizer
@property(nonatomic, readonly) maui::controls::buttons_mask detectedButton;
@end

@implementation MauiPressGestureRecognizer
{
    maui::controls::buttons_mask _detectedButton;
}

- (instancetype)initWithTarget:(id)target action:(SEL)action
{
    self = [super initWithTarget:target action:action];
    if (self != nil)
    {
        _detectedButton = maui::controls::buttons_mask::primary;
        self.cancelsTouchesInView = NO; // CustomPressGestureRecognizer ctor
    }
    return self;
}

- (maui::controls::buttons_mask)detectedButton
{
    return _detectedButton;
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    // DetermineButtonFromEvent: the event's buttonMask carries the pressed button on pointer devices.
    _detectedButton = (event.buttonMask & UIEventButtonMaskSecondary) != 0 ? maui::controls::buttons_mask::secondary
                                                                           : maui::controls::buttons_mask::primary;
    self.state = UIGestureRecognizerStateBegan;
    [super touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    self.state = UIGestureRecognizerStateChanged;
    [super touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    self.state = UIGestureRecognizerStateEnded; // keeps the button detected at TouchesBegan
    [super touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    self.state = UIGestureRecognizerStateCancelled;
    [super touchesCancelled:touches withEvent:event];
}
@end

namespace
{
    using maui::platform::ios::gesture_target_key;
    using maui::platform::ios::to_ui_button_mask;

    // One port recognizer's native attachment: the created UIGestureRecognizers (removed from the
    // view on detach — each retains its trampoline target via the associated object) and the
    // continuous per-gesture state (the pinch _previousScale memory; the pointer `exited` flag).
    struct gesture_attachment
    {
        NSMutableArray<UIGestureRecognizer*>* recognizers = nil;
        __weak UIView* view = nil;
        maui::core::scoped_connection property_token; // tap/swipe property → native knob re-sync
        double previous_scale = 1.0;                  // GesturePlatformManager._previousScale
        double starting_scale = 1.0;                  // view.Scale at pinch start
        bool pointer_exited = false;                  // CreatePointerRecognizer's `exited` capture
    };

    maui::graphics::point location_in(UIGestureRecognizer* recognizer)
    {
        const CGPoint location = [recognizer locationInView:recognizer.view];
        return {location.x, location.y};
    }
} // namespace

namespace maui::controls
{
    // The manager's backend attachment table (the forward-declared gesture_native_state): recognizer →
    // attachment (pointer-stable values). Complete only in this TU, where the manager's ctor/dtor —
    // and thus the owning unique_ptr's deleter — are defined.
    struct gesture_native_state
    {
        std::unordered_map<const gesture_recognizer*, std::unique_ptr<gesture_attachment>> attachments;
    };

    gesture_platform_manager::gesture_platform_manager() = default;

    gesture_platform_manager::~gesture_platform_manager()
    {
        native_detach_all(); // removes every native recognizer; native_state_ then frees itself
    }

    void gesture_platform_manager::native_attach(const std::shared_ptr<gesture_recognizer>& recognizer)
    {
        UIView* const view = handler_ != nullptr ? (__bridge UIView*)handler_->native_view() : nil;
        if (view == nil || sender_ == nullptr)
        {
            return; // no native view to attach to (mirrors the C# PlatformView null guards)
        }
        if (!native_state_)
        {
            native_state_ = std::make_unique<gesture_native_state>();
        }
        gesture_native_state& state = *native_state_;
        auto attachment = std::make_unique<gesture_attachment>();
        attachment->recognizers = [NSMutableArray array];
        attachment->view = view;
        gesture_attachment* const att = attachment.get();
        element* const sender = sender_;

        // Create one trampoline target, retain it via the associated object on the recognizer, and
        // add the recognizer to the view.
        const auto add_native = [att, view](UIGestureRecognizer* native,
                                            std::function<void(UIGestureRecognizer*)> callback) {
            MauiGestureTarget* const target = [[MauiGestureTarget alloc] initWithCallback:std::move(callback)];
            [native addTarget:target action:@selector(onGesture:)];
            objc_setAssociatedObject(native, gesture_target_key(), target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            [att->recognizers addObject:native];
            [view addGestureRecognizer:native];
        };

        if (auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer.get()))
        {
            UITapGestureRecognizer* const native_tap = [[UITapGestureRecognizer alloc] init];
            native_tap.numberOfTapsRequired = static_cast<NSUInteger>(tap->number_of_taps_required());
            if (@available(iOS 13.4, *))
            {
                native_tap.buttonMaskRequired = to_ui_button_mask(tap->buttons());
            }
            add_native(native_tap,
                       [tap, sender](UIGestureRecognizer* native) { tap->send_tapped(*sender, location_in(native)); });
            // OnTapGestureRecognizerPropertyChanged (Buttons → reload), narrowed to the two knobs.
            __weak UITapGestureRecognizer* const weak_tap = native_tap;
            att->property_token =
                maui::core::connect_scoped(tap->property_changed, [tap, weak_tap](std::string_view name) {
                    if (name != "buttons" && name != "number_of_taps_required")
                    {
                        return;
                    }
                    UITapGestureRecognizer* const strong_tap = weak_tap;
                    if (strong_tap != nil)
                    {
                        strong_tap.numberOfTapsRequired = static_cast<NSUInteger>(tap->number_of_taps_required());
                        if (@available(iOS 13.4, *))
                        {
                            strong_tap.buttonMaskRequired = to_ui_button_mask(tap->buttons());
                        }
                    }
                });
        }
        else if (auto* pan = dynamic_cast<pan_gesture_recognizer*>(recognizer.get()))
        {
            UIPanGestureRecognizer* const native_pan = [[UIPanGestureRecognizer alloc] init];
            // CreatePanRecognizer: the touch count is pinned to TouchPoints.
            native_pan.minimumNumberOfTouches = static_cast<NSUInteger>(pan->touch_points());
            native_pan.maximumNumberOfTouches = static_cast<NSUInteger>(pan->touch_points());
            add_native(native_pan, [pan, sender](UIGestureRecognizer* native) {
                auto* const pan_native = (UIPanGestureRecognizer*)native;
                auto& current_id = pan_gesture_recognizer::current_id();
                const auto touch_points = static_cast<NSUInteger>(pan->touch_points());
                const NSUInteger touches = native.numberOfTouches;
                const bool touches_known = touches > 0; // 0 on a direct target fire (documented)
                switch (native.state)
                {
                    case UIGestureRecognizerStateBegan:
                        if (touches_known && touches != touch_points)
                        {
                            return;
                        }
                        pan->send_pan_started(*sender, current_id.value());
                        break;
                    case UIGestureRecognizerStateChanged: {
                        if (touches_known && touches != touch_points)
                        {
                            pan_native.state = UIGestureRecognizerStateEnded;
                            pan->send_pan_completed(*sender, current_id.value());
                            current_id.increment();
                            return;
                        }
                        const CGPoint translation = [pan_native translationInView:native.view];
                        pan->send_pan(*sender, translation.x, translation.y, current_id.value());
                        break;
                    }
                    case UIGestureRecognizerStateCancelled:
                    case UIGestureRecognizerStateFailed:
                        pan->send_pan_canceled(*sender, current_id.value());
                        current_id.increment();
                        break;
                    case UIGestureRecognizerStateEnded:
                        // C#: at Ended the touches have lifted (count != TouchPoints) → complete.
                        if (touches != touch_points)
                        {
                            pan->send_pan_completed(*sender, current_id.value());
                            current_id.increment();
                        }
                        break;
                    default:
                        break;
                }
            });
        }
        else if (auto* pinch = dynamic_cast<pinch_gesture_recognizer*>(recognizer.get()))
        {
            UIPinchGestureRecognizer* const native_pinch = [[UIPinchGestureRecognizer alloc] init];
            add_native(native_pinch, [pinch, sender, att](UIGestureRecognizer* native) {
                auto* const pinch_native = (UIPinchGestureRecognizer*)native;
                UIView* const host = native.view;
                const CGPoint location = [native locationInView:host];
                const double width = host != nil ? host.bounds.size.width : 0;
                const double height = host != nil ? host.bounds.size.height : 0;
                const maui::graphics::point scaled(width > 0 ? location.x / width : 0,
                                                   height > 0 ? location.y / height : 0);
                const double scale = pinch_native.scale;
                const NSUInteger touches = native.numberOfTouches;
                const bool touches_known = touches > 0; // 0 on a direct target fire (documented)
                switch (native.state)
                {
                    case UIGestureRecognizerStateBegan: {
                        if (touches_known && touches < 2)
                        {
                            return;
                        }
                        pinch->send_pinch_started(*sender, scaled);
                        const auto* transform = dynamic_cast<const maui::core::i_transform*>(sender);
                        att->starting_scale = transform != nullptr ? transform->scale() : 1.0;
                        break;
                    }
                    case UIGestureRecognizerStateChanged: {
                        if (touches_known && touches < 2 && pinch->is_pinching())
                        {
                            pinch_native.state = UIGestureRecognizerStateEnded;
                            pinch->send_pinch_ended(*sender);
                            return;
                        }
                        pinch->send_pinch(*sender, pinch_scale_delta(att->previous_scale, scale, att->starting_scale),
                                          scaled);
                        att->previous_scale = scale;
                        break;
                    }
                    case UIGestureRecognizerStateCancelled:
                    case UIGestureRecognizerStateFailed:
                        if (pinch->is_pinching())
                        {
                            pinch->send_pinch_canceled(*sender);
                        }
                        break;
                    case UIGestureRecognizerStateEnded:
                        if (pinch->is_pinching())
                        {
                            pinch->send_pinch_ended(*sender);
                        }
                        att->previous_scale = 1.0;
                        break;
                    default:
                        break;
                }
            });
        }
        else if (auto* swipe = dynamic_cast<swipe_gesture_recognizer*>(recognizer.get()))
        {
            UISwipeGestureRecognizer* const native_swipe = [[UISwipeGestureRecognizer alloc] init];
            native_swipe.numberOfTouchesRequired = 1; // CreateSwipeRecognizer's numFingers
            // The maui and UIKit direction enums are bit-identical (the C# cast).
            native_swipe.direction = static_cast<UISwipeGestureRecognizerDirection>(swipe->direction());
            // The view's render-rotation face, resolved once at attach (the same object the lambda's
            // sender names); rotation() is still read at recognition time, like C#'s view.Rotation.
            const auto* const sender_transform = dynamic_cast<const maui::core::i_transform*>(sender);
            add_native(native_swipe, [swipe, sender, sender_transform](UIGestureRecognizer* native) {
                // UIKit detected the swipe natively; SendSwiped directly with the rotation-compensated
                // direction (the C# returnAction). The direction is read from the native recognizer so
                // a post-attach Direction change stays consistent with the native filter.
                const auto direction =
                    static_cast<maui::core::swipe_direction>(((UISwipeGestureRecognizer*)native).direction);
                const double rotation = sender_transform != nullptr ? sender_transform->rotation() : 0.0;
                swipe->send_swiped(*sender, transform_swipe_direction_for_rotation(direction, rotation));
            });
            // OnSwipeGestureRecognizerPropertyChanged: a Direction change updates the native mask.
            __weak UISwipeGestureRecognizer* const weak_swipe = native_swipe;
            att->property_token =
                maui::core::connect_scoped(swipe->property_changed, [swipe, weak_swipe](std::string_view name) {
                    if (name != "direction")
                    {
                        return;
                    }
                    UISwipeGestureRecognizer* const strong_swipe = weak_swipe;
                    if (strong_swipe != nil)
                    {
                        strong_swipe.direction = static_cast<UISwipeGestureRecognizerDirection>(swipe->direction());
                    }
                });
        }
        else if (auto* pointer = dynamic_cast<pointer_gesture_recognizer*>(recognizer.get()))
        {
            // CreatePointerRecognizer: a hover recognizer + the custom press recognizer share one action.
            const auto action = [pointer, sender, att](UIGestureRecognizer* native) {
                const bool is_hover = [native isKindOfClass:[UIHoverGestureRecognizer class]];
                buttons_mask button = effective_button(pointer->buttons());
                if (auto* const press = [native isKindOfClass:[MauiPressGestureRecognizer class]]
                                            ? (MauiPressGestureRecognizer*)native
                                            : nil)
                {
                    button = press.detectedButton;
                }
                // Non-hover events must match the recognizer's mask (the C# press filter).
                if (!is_hover && !contains(pointer->buttons(), button))
                {
                    return;
                }
                const maui::graphics::point position = location_in(native);
                switch (native.state)
                {
                    case UIGestureRecognizerStateBegan:
                        att->pointer_exited = false;
                        if (is_hover)
                        {
                            pointer->send_pointer_entered(*sender, position, button);
                        }
                        else
                        {
                            pointer->send_pointer_pressed(*sender, position, button);
                        }
                        break;
                    case UIGestureRecognizerStateChanged:
                        if (att->pointer_exited)
                        {
                            break;
                        }
                        if (is_hover)
                        {
                            pointer->send_pointer_moved(*sender, position, button);
                        }
                        else
                        {
                            UIView* const host = native.view;
                            const CGPoint raw = [native locationInView:host];
                            if (host != nil && CGRectContainsPoint(host.bounds, raw))
                            {
                                pointer->send_pointer_moved(*sender, position, button);
                            }
                            else
                            {
                                pointer->send_pointer_exited(*sender, position, button);
                                att->pointer_exited = true;
                                native.state = UIGestureRecognizerStateEnded;
                            }
                        }
                        break;
                    case UIGestureRecognizerStateCancelled:
                    case UIGestureRecognizerStateFailed:
                    case UIGestureRecognizerStateEnded:
                        if (att->pointer_exited)
                        {
                            break;
                        }
                        if (is_hover)
                        {
                            pointer->send_pointer_exited(*sender, position, button);
                        }
                        else
                        {
                            pointer->send_pointer_released(*sender, position, button);
                        }
                        break;
                    default:
                        break;
                }
            };
            UIHoverGestureRecognizer* const hover = [[UIHoverGestureRecognizer alloc] init];
            add_native(hover, action);
            MauiPressGestureRecognizer* const press = [[MauiPressGestureRecognizer alloc] init];
            add_native(press, action);
        }

        state.attachments[recognizer.get()] = std::move(attachment);
    }

    namespace
    {
        // Remove one attachment's native recognizers from its view.
        void remove_native_attachment(const gesture_attachment& attachment)
        {
            UIView* const view = attachment.view;
            NSArray<UIGestureRecognizer*>* const recognizers = attachment.recognizers;
            for (NSUInteger i = 0; i < recognizers.count; ++i)
            {
                [view removeGestureRecognizer:recognizers[i]];
            }
        }
    } // namespace

    void gesture_platform_manager::native_detach(const gesture_recognizer& recognizer)
    {
        if (!native_state_)
        {
            return;
        }
        const auto it = native_state_->attachments.find(&recognizer);
        if (it == native_state_->attachments.end())
        {
            return;
        }
        remove_native_attachment(*it->second);
        native_state_->attachments.erase(it);
    }

    void gesture_platform_manager::native_detach_all()
    {
        if (!native_state_)
        {
            return;
        }
        for (const auto& [recognizer, attachment] : native_state_->attachments)
        {
            remove_native_attachment(*attachment);
        }
        native_state_->attachments.clear();
    }
} // namespace maui::controls

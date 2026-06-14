// gesture_platform_manager — Apple (AppKit / macOS) platform partial: the native attach/detach half
// of the GestureManager/GesturePlatformManager pair (see gesture_platform_manager.hpp). Translated
// from GesturePlatformManager.iOS.cs (MAUI's macOS support is Mac Catalyst/UIKit, so there is no
// AppKit partial in the read-only C# source; the recognizer recipe maps to the standard AppKit
// equivalents — see apple_gesture_ops.hpp for the per-gesture translation table). Compiled as
// Objective-C++ with ARC only for the `apple` backend.
//
// Per-recognizer recipe (each native action routes into the SAME controller-interface calls the C#
// bridge makes):
//   - tap   → NSClickGestureRecognizer (numberOfClicksRequired / buttonMask); a Buttons or
//             NumberOfTapsRequired change re-configures the native recognizer (the C# bridge's
//             OnTapGestureRecognizerPropertyChanged reload, narrowed to the two native knobs);
//   - pan   → NSPanGestureRecognizer: Began → SendPanStarted, Changed → SendPan(translation),
//             Ended → SendPanCompleted (+ CurrentId.Increment), Cancelled/Failed → SendPanCanceled.
//             AppKit adaptations (documented): TotalY is negated for non-flipped views (AppKit's y
//             grows up; MAUI's grows down), the iOS NumberOfTouches==TouchPoints guards are dropped
//             (an AppKit mouse pan is always a single pointer), and Ended always completes (there is
//             no multi-touch lift order to hand the ending to Changed);
//   - pinch → NSMagnificationGestureRecognizer (scale = 1 + magnification, a cumulative reading) fed
//             through the shared pinch_scale_delta math — the exact iOS Changed-case algorithm,
//             including the IsPinching guards and the view-relative scaled origin;
//   - swipe → SYNTHESIZED from a dedicated NSPanGestureRecognizer (AppKit has NO swipe recognizer):
//             Changed feeds ISwipeGestureController.SendSwipe with the running totals and Ended asks
//             DetectSwipe(Direction) — the accumulate-then-detect model of the C# Windows/Android
//             bridges (the threshold check lives in the cross-platform recognizer);
//   - pointer → entered/exited/moved via an NSTrackingArea (owner-message hover tracking; the
//             UIHoverGestureRecognizer equivalent) and pressed/released via an
//             NSPressGestureRecognizer with minimumPressDuration = 0 (Began on mouse-down). The
//             reported button follows the C# Catalyst fallback (effective_button); AppKit actions
//             carry no per-event button identity.

#import <AppKit/AppKit.h>

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "apple_gesture_ops.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
#include "maui/controls/gestures/drop_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
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

// Obj-C trampoline: NSGestureRecognizer's target is held weakly, so each attachment keeps its targets
// alive and the target forwards the action into a C++ callback.
@interface MauiGestureTarget : NSObject
- (instancetype)initWithCallback:(std::function<void(NSGestureRecognizer*)>)callback;
- (void)onGesture:(NSGestureRecognizer*)sender;
@end

@implementation MauiGestureTarget
{
    std::function<void(NSGestureRecognizer*)> _callback;
}

- (instancetype)initWithCallback:(std::function<void(NSGestureRecognizer*)>)callback
{
    self = [super init];
    if (self != nil)
    {
        _callback = std::move(callback);
    }
    return self;
}

- (void)onGesture:(NSGestureRecognizer*)sender
{
    if (_callback)
    {
        _callback(sender);
    }
}
@end

// The NSTrackingArea owner: receives the hover messages (mouseEntered/Exited/Moved) for the pointer
// recognizer and forwards them into a C++ callback with the phase tag.
@interface MauiPointerTrackingOwner : NSObject
- (instancetype)initWithCallback:(std::function<void(maui::controls::pointer_event_kind, NSEvent*)>)callback;
@end

@implementation MauiPointerTrackingOwner
{
    std::function<void(maui::controls::pointer_event_kind, NSEvent*)> _callback;
}

- (instancetype)initWithCallback:(std::function<void(maui::controls::pointer_event_kind, NSEvent*)>)callback
{
    self = [super init];
    if (self != nil)
    {
        _callback = std::move(callback);
    }
    return self;
}

- (void)mouseEntered:(NSEvent*)event
{
    if (_callback)
    {
        _callback(maui::controls::pointer_event_kind::entered, event);
    }
}

- (void)mouseExited:(NSEvent*)event
{
    if (_callback)
    {
        _callback(maui::controls::pointer_event_kind::exited, event);
    }
}

- (void)mouseMoved:(NSEvent*)event
{
    if (_callback)
    {
        _callback(maui::controls::pointer_event_kind::moved, event);
    }
}
@end

namespace
{
    using maui::platform::apple::pan_total_y;
    using maui::platform::apple::to_ns_button_mask;

    // One port recognizer's native attachment: the created NSGestureRecognizers (removed from the view
    // on detach), their strongly-held targets, the optional hover tracking area, and the continuous
    // per-gesture state (the pinch scale memory the iOS bridge keeps in _previousScale).
    struct gesture_attachment
    {
        NSMutableArray<NSGestureRecognizer*>* recognizers = nil;
        NSMutableArray<MauiGestureTarget*>* targets = nil;
        NSTrackingArea* tracking_area = nil;
        MauiPointerTrackingOwner* tracking_owner = nil;
        __weak NSView* view = nil;
        maui::core::scoped_connection property_token; // tap: Buttons/NumberOfTapsRequired → native knobs
        double previous_scale = 1.0;                  // GesturePlatformManager._previousScale
        double starting_scale = 1.0;                  // the view's render scale at pinch start
        // --- drag&drop (W2-22): the drag-source / drop-target registration this attachment installed
        // (so detach unregisters exactly what it registered). AppKit has no drag/drop gesture-recognizer
        // object (unlike UIKit's UIDrag/UIDropInteraction) — a drag source is the view's
        // beginDraggingSessionWithItems flow and a drop target is registerForDraggedTypes:. The native
        // tests assert these flags flip on attach/detach (attachment-only — synthetic dragging SESSIONS
        // aren't drivable headlessly or in the spawned sim lane; documented). ---
        bool registered_drag_source = false;
        bool registered_drop_target = false;
    };

    maui::graphics::point location_in(NSGestureRecognizer* recognizer)
    {
        const NSPoint location = [recognizer locationInView:recognizer.view];
        return {location.x, location.y};
    }
} // namespace

namespace maui::controls
{
    // The manager's backend attachment table (the forward-declared gesture_native_state): recognizer →
    // attachment (pointer-stable values). Complete only in this TU, where the manager's destructor —
    // and thus the owning unique_ptr's deleter — is defined.
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
        NSView* const view = handler_ != nullptr ? (__bridge NSView*)handler_->native_view() : nil;
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
        attachment->targets = [NSMutableArray array];
        attachment->view = view;
        gesture_attachment* const att = attachment.get();
        element* const sender = sender_;

        // Create one native recognizer with a trampoline target and add it to the view.
        const auto add_native = [att, view](NSGestureRecognizer* native,
                                            std::function<void(NSGestureRecognizer*)> callback) {
            MauiGestureTarget* const target = [[MauiGestureTarget alloc] initWithCallback:std::move(callback)];
            native.target = target;
            native.action = @selector(onGesture:);
            [att->targets addObject:target];
            [att->recognizers addObject:native];
            [view addGestureRecognizer:native];
        };

        if (auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer.get()))
        {
            NSClickGestureRecognizer* const click = [[NSClickGestureRecognizer alloc] init];
            click.numberOfClicksRequired = tap->number_of_taps_required();
            click.buttonMask = to_ns_button_mask(tap->buttons());
            add_native(click,
                       [tap, sender](NSGestureRecognizer* native) { tap->send_tapped(*sender, location_in(native)); });
            // The C# bridge reloads the native recognizer when Buttons changes
            // (OnTapGestureRecognizerPropertyChanged); narrowed here to re-configuring the two knobs.
            __weak NSClickGestureRecognizer* const weak_click = click;
            att->property_token =
                maui::core::connect_scoped(tap->property_changed, [tap, weak_click](std::string_view name) {
                    if (name != "buttons" && name != "number_of_taps_required")
                    {
                        return;
                    }
                    NSClickGestureRecognizer* const strong_click = weak_click;
                    if (strong_click != nil)
                    {
                        strong_click.numberOfClicksRequired = tap->number_of_taps_required();
                        strong_click.buttonMask = to_ns_button_mask(tap->buttons());
                    }
                });
        }
        else if (auto* pan = dynamic_cast<pan_gesture_recognizer*>(recognizer.get()))
        {
            // TouchPoints has no AppKit analog (a mouse pan is a single pointer); documented adaptation.
            NSPanGestureRecognizer* const native_pan = [[NSPanGestureRecognizer alloc] init];
            add_native(native_pan, [pan, sender](NSGestureRecognizer* native) {
                auto* const pan_native = (NSPanGestureRecognizer*)native;
                auto& current_id = pan_gesture_recognizer::current_id();
                switch (native.state)
                {
                    case NSGestureRecognizerStateBegan:
                        pan->send_pan_started(*sender, current_id.value());
                        break;
                    case NSGestureRecognizerStateChanged: {
                        const NSPoint translation = [pan_native translationInView:native.view];
                        pan->send_pan(*sender, translation.x, pan_total_y(native.view, translation.y),
                                      current_id.value());
                        break;
                    }
                    case NSGestureRecognizerStateEnded:
                        pan->send_pan_completed(*sender, current_id.value());
                        current_id.increment();
                        break;
                    case NSGestureRecognizerStateCancelled:
                    case NSGestureRecognizerStateFailed:
                        pan->send_pan_canceled(*sender, current_id.value());
                        current_id.increment();
                        break;
                    default:
                        break;
                }
            });
        }
        else if (auto* pinch = dynamic_cast<pinch_gesture_recognizer*>(recognizer.get()))
        {
            NSMagnificationGestureRecognizer* const magnify = [[NSMagnificationGestureRecognizer alloc] init];
            add_native(magnify, [pinch, sender, att](NSGestureRecognizer* native) {
                auto* const magnify_native = (NSMagnificationGestureRecognizer*)native;
                NSView* const host = native.view;
                const NSPoint location = [native locationInView:host];
                const double width = host != nil ? host.bounds.size.width : 0;
                const double height = host != nil ? host.bounds.size.height : 0;
                // The iOS bridge's scaledPoint: the origin in view-relative unit coordinates.
                const maui::graphics::point scaled(width > 0 ? location.x / width : 0,
                                                   height > 0 ? location.y / height : 0);
                // AppKit's magnification is a cumulative delta from 1 → the UIPinch-style scale.
                const double scale = 1.0 + magnify_native.magnification;
                switch (native.state)
                {
                    case NSGestureRecognizerStateBegan: {
                        pinch->send_pinch_started(*sender, scaled);
                        const auto* transform = dynamic_cast<const maui::core::i_transform*>(sender);
                        att->starting_scale = transform != nullptr ? transform->scale() : 1.0;
                        break;
                    }
                    case NSGestureRecognizerStateChanged:
                        pinch->send_pinch(*sender, pinch_scale_delta(att->previous_scale, scale, att->starting_scale),
                                          scaled);
                        att->previous_scale = scale;
                        break;
                    case NSGestureRecognizerStateCancelled:
                    case NSGestureRecognizerStateFailed:
                        if (pinch->is_pinching())
                        {
                            pinch->send_pinch_canceled(*sender);
                        }
                        break;
                    case NSGestureRecognizerStateEnded:
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
            // AppKit has NO swipe recognizer — synthesize from pan deltas (see the header comment).
            NSPanGestureRecognizer* const native_pan = [[NSPanGestureRecognizer alloc] init];
            add_native(native_pan, [swipe, sender](NSGestureRecognizer* native) {
                auto* const pan_native = (NSPanGestureRecognizer*)native;
                auto& controller = static_cast<i_swipe_gesture_controller&>(*swipe);
                switch (native.state)
                {
                    case NSGestureRecognizerStateChanged: {
                        const NSPoint translation = [pan_native translationInView:native.view];
                        controller.send_swipe(*sender, translation.x, pan_total_y(native.view, translation.y));
                        break;
                    }
                    case NSGestureRecognizerStateEnded:
                        (void)controller.detect_swipe(*sender, swipe->direction());
                        break;
                    default:
                        break;
                }
            });
        }
        else if (auto* pointer = dynamic_cast<pointer_gesture_recognizer*>(recognizer.get()))
        {
            // Hover (entered/exited/moved) via an NSTrackingArea on the view's visible rect.
            __weak NSView* const weak_view = view;
            MauiPointerTrackingOwner* const owner = [[MauiPointerTrackingOwner alloc]
                initWithCallback:[pointer, sender, weak_view](pointer_event_kind kind, NSEvent* event) {
                    std::optional<maui::graphics::point> position;
                    NSView* const strong_view = weak_view;
                    if (strong_view != nil)
                    {
                        const NSPoint location = [strong_view convertPoint:event.locationInWindow fromView:nil];
                        position = maui::graphics::point(location.x, location.y);
                    }
                    switch (kind)
                    {
                        case pointer_event_kind::entered:
                            pointer->send_pointer_entered(*sender, position);
                            break;
                        case pointer_event_kind::exited:
                            pointer->send_pointer_exited(*sender, position);
                            break;
                        default:
                            pointer->send_pointer_moved(*sender, position);
                            break;
                    }
                }];
            NSTrackingArea* const area =
                [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                             options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved |
                                                      NSTrackingActiveAlways | NSTrackingInVisibleRect)
                                               owner:owner
                                            userInfo:nil];
            [view addTrackingArea:area];
            attachment->tracking_area = area;
            attachment->tracking_owner = owner;

            // Pressed/released via a zero-duration press recognizer (Began fires on mouse-down). The
            // reported button follows the C# Catalyst fallback — AppKit actions carry no button identity.
            NSPressGestureRecognizer* const press = [[NSPressGestureRecognizer alloc] init];
            press.minimumPressDuration = 0;
            press.buttonMask = to_ns_button_mask(pointer->buttons());
            const buttons_mask button = effective_button(pointer->buttons());
            add_native(press, [pointer, sender, button](NSGestureRecognizer* native) {
                const maui::graphics::point position = location_in(native);
                switch (native.state)
                {
                    case NSGestureRecognizerStateBegan:
                        pointer->send_pointer_pressed(*sender, position, button);
                        break;
                    case NSGestureRecognizerStateEnded:
                    case NSGestureRecognizerStateCancelled:
                    case NSGestureRecognizerStateFailed:
                        pointer->send_pointer_released(*sender, position, button);
                        break;
                    default:
                        break;
                }
            });
        }
        // --- drag&drop (W2-22): attachment-only native install (see the gesture_attachment fields).
        // AppKit has no drag/drop gesture-recognizer object — a drag SOURCE is the view's
        // beginDraggingSessionWithItems flow (gated here by setting the source flag; the synthetic
        // session isn't drivable, documented), and a drop TARGET is registerForDraggedTypes: (the
        // NSDraggingDestination registration). Both branches install onto the view and flip a flag the
        // detach path reads to unregister. ---
        else if (dynamic_cast<drag_gesture_recognizer*>(recognizer.get()) != nullptr)
        {
            att->registered_drag_source = true;
        }
        else if (dynamic_cast<drop_gesture_recognizer*>(recognizer.get()) != nullptr)
        {
            // NSDraggingDestination: register the view to receive drags. NSPasteboardTypeString is the
            // text payload a DataPackage carries (the AppKit twin of the UIDropInteraction install).
            [view registerForDraggedTypes:@[ NSPasteboardTypeString ]];
            att->registered_drop_target = true;
        }
        // --- end drag&drop (W2-22) ---

        state.attachments[recognizer.get()] = std::move(attachment);
    }

    namespace
    {
        // Remove one attachment's native recognizers (and the pointer tracking area) from its view.
        void remove_native_attachment(const gesture_attachment& attachment)
        {
            NSView* const view = attachment.view;
            NSArray<NSGestureRecognizer*>* const recognizers = attachment.recognizers;
            for (NSUInteger i = 0; i < recognizers.count; ++i)
            {
                [view removeGestureRecognizer:recognizers[i]];
            }
            if (attachment.tracking_area != nil && view != nil)
            {
                [view removeTrackingArea:attachment.tracking_area];
            }
            // --- drag&drop (W2-22): undo the drop-target registration this attachment installed (the
            // drag-source flag carries no native state to undo). ---
            if (attachment.registered_drop_target && view != nil)
            {
                [view unregisterDraggedTypes];
            }
            // --- end drag&drop (W2-22) ---
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

    // --- drag&drop (W2-22): read the per-recognizer registration flags off the backend table. ---
    bool gesture_platform_manager::native_registered_drag_source(const gesture_recognizer& recognizer) const
    {
        if (!native_state_)
        {
            return false;
        }
        const auto it = native_state_->attachments.find(&recognizer);
        return it != native_state_->attachments.end() && it->second->registered_drag_source;
    }

    bool gesture_platform_manager::native_registered_drop_target(const gesture_recognizer& recognizer) const
    {
        if (!native_state_)
        {
            return false;
        }
        const auto it = native_state_->attachments.find(&recognizer);
        return it != native_state_->attachments.end() && it->second->registered_drop_target;
    }
    // --- end drag&drop (W2-22) ---
} // namespace maui::controls

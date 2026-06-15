// swipe_view_handler — iOS (UIKit) platform recipe: a plain UIView host that holds the swipe Content as
// a subview, plus the swipe state machine driven by the shared cross-platform maui::core::swipe_machine.
// The native MauiSwipeView is a UIPanGestureRecognizer-driven custom UIView; this cut hosts the content
// on a plain UIView and wires a REAL UIPanGestureRecognizer (W7-U09) so the user can drag a row to reveal
// the swipe items. The pan trampoline drives the SAME shared machine the programmatic open/close +
// synthetic offsets use (begin_swipe / swipe_to / end_swipe) and applies the Reveal-mode content-frame
// translation (MauiSwipeView.Swipe), so the gesture, the notifications, and the open/close thresholds are
// the faithful machine — only the drag input is new. The generic-IView property pushes mirror the ios
// content_page_handler.mm. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// DEVIATION (documented): the pan covers the primary single-direction drag-to-reveal (open on a drag past
// the open threshold, snap closed below it) over the Reveal transition. The Drag-transition action-view
// translation, the tap-outside-to-close, and the parent UIScrollView coordination are NOT wired in this
// cut (the port's swipe host is a plain UIView with no action-view subview / scroll ancestor — see
// STATUS.md). The shared machine still runs the full open/close + Execute/Reveal behavior.

#import <UIKit/UIKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/core/swipe_view_machine.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }

    UIView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // The signed swipe offset from a pan translation (MauiSwipeView.GetSwipeOffset): x for left/right, y
    // for up/down (sign preserved — negative for left/up, positive for right/down).
    double offset_for_direction(maui::core::swipe_direction direction, CGPoint translation)
    {
        if (maui::core::is_left(direction) || maui::core::is_right(direction))
        {
            return translation.x;
        }
        return translation.y;
    }

    // Apply the Reveal-mode content-frame translation (MauiSwipeView.Swipe, Reveal branch): shift the
    // single content child along the swipe axis by `offset` from the host's bounds origin. The Drag
    // transition (action-view translation) is the documented deviation (no action-view subview here).
    void apply_reveal_frame(UIView* host, maui::core::swipe_direction direction, double offset)
    {
        if (host.subviews.count == 0)
        {
            return;
        }
        UIView* const content = host.subviews.firstObject;
        const CGRect base = host.bounds;
        const bool horizontal = maui::core::is_left(direction) || maui::core::is_right(direction);
        const CGFloat dx = horizontal ? offset : 0;
        const CGFloat dy = horizontal ? 0 : offset;
        content.frame = CGRectMake(base.origin.x + dx, base.origin.y + dy, base.size.width, base.size.height);
    }

    // The pan action (MauiSwipeView.HandlePan → HandleTouchInteractions → ProcessTouchMove/Up): drive the
    // shared swipe_machine from the real gesture and apply the reveal frame. Declared as a free function so
    // create_platform_view can hand its address to the target; defined fully below the handler members it
    // calls would need — but it only uses the handler's PUBLIC begin/swipe/end + accessors, all declared in
    // the included header, so it can live here.
    void handle_swipe_pan(void* context, UIPanGestureRecognizer* recognizer);
} // namespace

// Obj-C trampoline: forwards the UIPanGestureRecognizer's action to the C++ handler (the C#
// SwipeViewProxy.HandlePan role). A plain function pointer + opaque context keeps the target Obj-C-only.
@interface MauiSwipePanTarget : NSObject
@property(nonatomic) void (*onPan)(void*, UIPanGestureRecognizer*);
@property(nonatomic) void* context;
- (void)handlePan:(UIPanGestureRecognizer*)recognizer;
@end

@implementation MauiSwipePanTarget
- (void)handlePan:(UIPanGestureRecognizer*)recognizer
{
    if (self.onPan != nullptr)
    {
        self.onPan(self.context, recognizer);
    }
}
@end

namespace maui::core
{
    swipe_view_platform::~swipe_view_platform()
    {
        // Detach the recognizer from the host, then release the retained recognizer + target so a late
        // pan callback cannot re-enter a dead handler (the recognizer holds its target weakly via action).
        if (pan_recognizer != nullptr)
        {
            UIPanGestureRecognizer* const pan = (__bridge UIPanGestureRecognizer*)pan_recognizer;
            if (native != nullptr)
            {
                [as_host(native) removeGestureRecognizer:pan];
            }
            CFRelease(pan_recognizer);
            pan_recognizer = nullptr;
        }
        if (pan_target != nullptr)
        {
            CFRelease(pan_target);
            pan_target = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    void swipe_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void swipe_view_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
    }

    void swipe_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void swipe_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void swipe_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void swipe_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_host(native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void swipe_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void swipe_view_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<swipe_view_platform> swipe_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_view_platform>();
        UIView* const host = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)host; // the void* slot owns one reference

        // Wire the interactive drag-to-reveal pan (MauiSwipeView's _panGestureRecognizer + HandlePan). The
        // target's context is set to the handler in on_connect_handler (the handler isn't available here, a
        // static). CancelsTouchesInView=NO mirrors the C# recognizer config.
        MauiSwipePanTarget* const target = [[MauiSwipePanTarget alloc] init];
        target.onPan = &handle_swipe_pan;
        UIPanGestureRecognizer* const pan = [[UIPanGestureRecognizer alloc] initWithTarget:target
                                                                                    action:@selector(handlePan:)];
        pan.cancelsTouchesInView = NO;
        [host addGestureRecognizer:pan];
        platform->pan_target = (__bridge_retained void*)target;
        platform->pan_recognizer = (__bridge_retained void*)pan;
        return platform;
    }

    // C# ConnectHandler: bind the pan target to this handler so the gesture can drive the machine.
    void swipe_view_handler::on_connect_handler(swipe_view_platform& platform)
    {
        if (platform.pan_target != nullptr)
        {
            auto* const target = (__bridge MauiSwipePanTarget*)platform.pan_target;
            target.context = this;
        }
    }

    void swipe_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const host = as_host(platform->native);

        NSArray<UIView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (UIView* const subview = native_child(*platform->hosted_content))
        {
            [subview removeFromSuperview];
            [host addSubview:subview];
        }
    }

    void swipe_view_handler::update_transition_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->transition = virtual_view()->transition_mode();
    }

    void swipe_view_handler::update_items()
    {
        // C# MapLeftItems/... are empty; the machine reads the live collections on each swipe.
    }

    void swipe_view_handler::programmatically_open(const swipe_view_open_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::programmatically_open(platform->state, *view, request);
    }

    void swipe_view_handler::reset_swipe(bool /*animated*/)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::reset_swipe(platform->state, *view);
    }

    void swipe_view_handler::begin_swipe(swipe_direction direction)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        swipe_machine::begin_swipe(platform->state, direction);
    }

    void swipe_view_handler::swipe_to(double offset)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::swipe_to(platform->state, *view, offset);
    }

    void swipe_view_handler::end_swipe()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::end_swipe(platform->state, *view);
    }

    void swipe_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

namespace
{
    // MauiSwipeView.HandlePan → HandleTouchInteractions: on the first Changed move, pick the swipe
    // direction from the translation and begin the swipe; on every Changed move, push the signed offset
    // (swipe_to) + apply the Reveal-mode content frame; on Ended/Cancelled, end the swipe (which runs
    // ValidateSwipeThreshold) and animate the content to its settled position (open offset, or 0 closed).
    void handle_swipe_pan(void* context, UIPanGestureRecognizer* recognizer)
    {
        auto* handler = static_cast<maui::core::swipe_view_handler*>(context);
        if (handler == nullptr || recognizer == nil)
        {
            return;
        }
        auto* platform = handler->typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const host = as_host(platform->native);
        const CGPoint translation = [recognizer translationInView:host];

        switch (recognizer.state)
        {
            case UIGestureRecognizerStateBegan:
                // Direction is chosen on the first real move (translation ~0 here), mirroring C#.
                break;
            case UIGestureRecognizerStateChanged: {
                if (platform->state.state == maui::core::swipe_machine_state::idle)
                {
                    const maui::core::swipe_direction direction =
                        maui::core::swipe_machine::get_swipe_direction(0, 0, translation.x, translation.y);
                    handler->begin_swipe(direction);
                }
                const maui::core::swipe_direction direction = platform->state.direction;
                handler->swipe_to(offset_for_direction(direction, translation));
                if (platform->transition == maui::core::swipe_transition_mode::reveal)
                {
                    apply_reveal_frame(host, direction, platform->state.offset);
                }
                break;
            }
            case UIGestureRecognizerStateEnded:
            case UIGestureRecognizerStateCancelled:
            case UIGestureRecognizerStateFailed: {
                const maui::core::swipe_direction direction = platform->state.direction;
                handler->end_swipe();
                // Settle the content: end_swipe ran ValidateSwipeThreshold, so state.offset is the final
                // open offset (or 0 when it reset/executed). Animate the content to it (Swipe(animated)).
                const double settled = platform->state.is_open ? platform->state.offset : 0;
                [UIView animateWithDuration:0.2
                                 animations:^{
                                   apply_reveal_frame(host, direction, settled);
                                 }];
                break;
            }
            default:
                break;
        }
    }
} // namespace

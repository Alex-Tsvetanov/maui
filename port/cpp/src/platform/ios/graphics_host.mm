// The UIKit drawing host (graphics_host.hpp): a transparent UIView whose drawRect builds the shared
// coregraphics_canvas over UIGraphicsGetCurrentContext and replays the borrowed drawable — the
// PlatformGraphicsView (MaciOS) recipe — PLUS the PlatformTouchGraphicsView touch plumbing
// (touchesBegan/Moved/Ended/Cancelled → the connected i_graphics_view's send_*_interaction). Compiled
// as Objective-C++ with ARC for the `ios` backend.

#import <UIKit/UIKit.h>

#include <vector>

#include "coregraphics_canvas.hpp"
#include "graphics_host.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"

// The drawable-rendering UIView (C# PlatformGraphicsView / PlatformTouchGraphicsView): UIKit is already
// top-left; transparent (C# Opaque = false / BackgroundColor null); multi-touch enabled.
@interface MauiCppDrawableHostView : UIView
{
@public
    maui::graphics::i_drawable* _drawable;           // non-owning borrow (graphics_host.hpp)
    maui::core::i_graphics_view* _interactionTarget; // non-owning borrow (set on connect, cleared on disconnect)
    bool _pressedContained;                          // C# PlatformTouchGraphicsView._pressedContained
}
// Split out so the on-simulator seam test can drive the same paths a real gesture takes (UITouch sets
// cannot be synthesized in a unit test — the documented compromise shared with the other ios suites).
- (void)notifyStartInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyDragInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyEndInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyCancelInteraction;
@end

@implementation MauiCppDrawableHostView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        self.opaque = NO;
        self.backgroundColor = nil;
        self.multipleTouchEnabled = YES; // C# PlatformTouchGraphicsView ctor
    }
    return self;
}

- (void)drawRect:(CGRect)rect
{
    [super drawRect:rect];
    if (_drawable == nullptr)
    {
        return;
    }
    CGContextRef context = UIGraphicsGetCurrentContext();
    if (context == nullptr)
    {
        return;
    }
    // C# PlatformGraphicsView.Draw: a PlatformCanvas over the current context, Drawable.Draw over
    // the dirty rect.
    maui::platform::apple_shared::coregraphics_canvas canvas(context);
    _drawable->draw(canvas,
                    maui::graphics::rect_f(static_cast<float>(rect.origin.x), static_cast<float>(rect.origin.y),
                                           static_cast<float>(rect.size.width), static_cast<float>(rect.size.height)));
}

// ---- the PlatformTouchGraphicsView touch plumbing ----
// C# UIViewExtensions.GetPointsInView(evt): the multi-touch point array built from the EVENT's touches FOR
// THIS VIEW ([event touchesForView:self]) — NOT the per-callback `touches` NSSet and NOT event.allTouches
// (which is event-global, spanning every view). Each touch's location is taken in the host's coordinates.
- (std::vector<maui::graphics::point_f>)pointsFromEvent:(UIEvent*)event
{
    std::vector<maui::graphics::point_f> points;
    NSSet<UITouch*>* const touches = event != nil ? [event touchesForView:self] : nil;
    points.reserve(touches.count);
    for (UITouch* touch in touches)
    {
        const CGPoint location = [touch locationInView:self];
        points.emplace_back(static_cast<float>(location.x), static_cast<float>(location.y));
    }
    return points;
}

// Whether any of the points is inside the host bounds (C# RectF.ContainsAny).
- (bool)anyPointContained:(const std::vector<maui::graphics::point_f>&)points
{
    const CGRect bounds = self.bounds;
    for (const auto& point : points)
    {
        if (CGRectContainsPoint(bounds, CGPointMake(point.x, point.y)))
        {
            return true;
        }
    }
    return false;
}

- (void)notifyStartInteraction:(const std::vector<maui::graphics::point_f>&)points
{
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    // Latch BEFORE the raise: send_start_interaction is user code, and a handler that destroys the
    // graphics view releases the native host — writing an ivar on a freed `self` afterwards is a UAF.
    // Unobservable reorder (nothing reads _pressedContained until the next touch callback).
    _pressedContained = true;
    _interactionTarget->send_start_interaction(points);
}

- (void)notifyDragInteraction:(const std::vector<maui::graphics::point_f>&)points
{
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _pressedContained = [self anyPointContained:points];
    _interactionTarget->send_drag_interaction(points);
}

- (void)notifyEndInteraction:(const std::vector<maui::graphics::point_f>&)points
{
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _interactionTarget->send_end_interaction(points, _pressedContained);
}

- (void)notifyCancelInteraction
{
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _pressedContained = false;
    _interactionTarget->send_cancel_interaction();
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)touches; // C# uses GetPointsInView(evt) — the event's touches-for-this-view, not this NSSet
    [self notifyStartInteraction:[self pointsFromEvent:event]];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)touches;
    [self notifyDragInteraction:[self pointsFromEvent:event]];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)touches;
    [self notifyEndInteraction:[self pointsFromEvent:event]];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)touches;
    (void)event;
    [self notifyCancelInteraction];
}

@end

namespace maui::platform::ios
{
    void* create_drawable_host()
    {
        MauiCppDrawableHostView* const host = [[MauiCppDrawableHostView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        return (__bridge_retained void*)host; // the caller's void* slot owns one reference
    }

    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable)
    {
        auto* const view = (__bridge MauiCppDrawableHostView*)host;
        view->_drawable = drawable;
        [view setNeedsDisplay]; // the C# Drawable setter invalidates
    }

    void drawable_host_invalidate(void* host)
    {
        [(__bridge MauiCppDrawableHostView*)host setNeedsDisplay];
    }

    void drawable_host_set_interaction_target(void* host, maui::core::i_graphics_view* target)
    {
        if (host == nullptr) // the platform dtor detaches unconditionally; a direct ivar write would trap on nil
        {
            return;
        }
        ((__bridge MauiCppDrawableHostView*)host)->_interactionTarget = target; // non-owning borrow
    }
} // namespace maui::platform::ios

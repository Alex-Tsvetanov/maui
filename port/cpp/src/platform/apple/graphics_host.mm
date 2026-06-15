// The AppKit drawing host (graphics_host.hpp): a flipped NSView whose drawRect builds the shared
// coregraphics_canvas over the current CGContext and replays the borrowed drawable — the
// PlatformGraphicsView (MaciOS) recipe — PLUS the PlatformTouchGraphicsView mouse plumbing
// (mouseDown:/mouseDragged:/mouseUp: → the connected i_graphics_view's send_*_interaction). Compiled
// as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <vector>

#include "coregraphics_canvas.hpp"
#include "graphics_host.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"

// The drawable-rendering NSView (C# PlatformGraphicsView / PlatformTouchGraphicsView): top-left origin
// (isFlipped), transparent (no opaque background — C# sets Opaque = false / BackgroundColor null).
@interface MauiCppDrawableHostView : NSView
{
@public
    maui::graphics::i_drawable* _drawable;           // non-owning borrow (graphics_host.hpp)
    maui::core::i_graphics_view* _interactionTarget; // non-owning borrow (set on connect, cleared on disconnect)
    bool _pressedContained;                          // C# PlatformTouchGraphicsView._pressedContained
}
// Split out so the seam test can drive the same paths a real mouse gesture takes (synthesizing
// blocking AppKit mouse events in a unit test is not feasible — the slider suite's documented compromise).
- (void)notifyStartInteractionAtPoint:(NSPoint)point;
- (void)notifyDragInteractionAtPoint:(NSPoint)point;
- (void)notifyEndInteractionAtPoint:(NSPoint)point;
- (void)notifyCancelInteraction;
@end

@implementation MauiCppDrawableHostView

- (BOOL)isFlipped
{
    return YES; // canvas coordinates grow downward, like the C# PlatformGraphicsView host
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    if (_drawable == nullptr)
    {
        return;
    }
    CGContextRef context = NSGraphicsContext.currentContext.CGContext;
    if (context == nullptr)
    {
        return;
    }
    // C# PlatformGraphicsView.Draw: a PlatformCanvas over the current context, Drawable.Draw over
    // the dirty rect.
    maui::platform::apple_shared::coregraphics_canvas canvas(context);
    _drawable->draw(canvas, maui::graphics::rect_f(
                                static_cast<float>(dirtyRect.origin.x), static_cast<float>(dirtyRect.origin.y),
                                static_cast<float>(dirtyRect.size.width), static_cast<float>(dirtyRect.size.height)));
}

// ---- the PlatformTouchGraphicsView mouse plumbing ----
// The single-point vector each interaction call passes (AppKit's mouse is single-touch — the C# iOS
// view passes GetPointsInView(evt); for the mouse, that is the one cursor location in the host).
- (std::vector<maui::graphics::point_f>)pointsAt:(NSPoint)point
{
    return {maui::graphics::point_f(static_cast<float>(point.x), static_cast<float>(point.y))};
}

- (void)notifyStartInteractionAtPoint:(NSPoint)point
{
    // C# TouchesBegan: gated by VirtualView.IsEnabled, then StartInteraction(points); _pressedContained
    // begins true (the press started inside the view).
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _interactionTarget->send_start_interaction([self pointsAt:point]);
    _pressedContained = true;
}

- (void)notifyDragInteractionAtPoint:(NSPoint)point
{
    // C# TouchesMoved: _pressedContained tracks whether the point is still inside the bounds, then
    // DragInteraction(points).
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _pressedContained = NSPointInRect(point, self.bounds);
    _interactionTarget->send_drag_interaction([self pointsAt:point]);
}

- (void)notifyEndInteractionAtPoint:(NSPoint)point
{
    // C# TouchesEnded: EndInteraction(points, _pressedContained).
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _interactionTarget->send_end_interaction([self pointsAt:point], _pressedContained);
}

- (void)notifyCancelInteraction
{
    // C# TouchesCancelled: _pressedContained = false, then CancelInteraction().
    if (_interactionTarget == nullptr || !_interactionTarget->is_enabled())
    {
        return;
    }
    _pressedContained = false;
    _interactionTarget->send_cancel_interaction();
}

- (void)mouseDown:(NSEvent*)event
{
    [self notifyStartInteractionAtPoint:[self convertPoint:event.locationInWindow fromView:nil]];
}

- (void)mouseDragged:(NSEvent*)event
{
    [self notifyDragInteractionAtPoint:[self convertPoint:event.locationInWindow fromView:nil]];
}

- (void)mouseUp:(NSEvent*)event
{
    [self notifyEndInteractionAtPoint:[self convertPoint:event.locationInWindow fromView:nil]];
}

@end

namespace maui::platform::apple
{
    void* create_drawable_host()
    {
        MauiCppDrawableHostView* const host = [[MauiCppDrawableHostView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        return (__bridge_retained void*)host; // the caller's void* slot owns one reference
    }

    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable)
    {
        auto* const view = (__bridge MauiCppDrawableHostView*)host;
        view->_drawable = drawable;
        view.needsDisplay = YES; // the C# Drawable setter invalidates
    }

    void drawable_host_invalidate(void* host)
    {
        ((__bridge MauiCppDrawableHostView*)host).needsDisplay = YES;
    }

    void drawable_host_set_interaction_target(void* host, maui::core::i_graphics_view* target)
    {
        ((__bridge MauiCppDrawableHostView*)host)->_interactionTarget = target; // non-owning borrow
    }
} // namespace maui::platform::apple

// The AppKit drawing host (graphics_host.hpp): a flipped NSView whose drawRect builds the shared
// coregraphics_canvas over the current CGContext and replays the borrowed drawable — the
// PlatformGraphicsView (MaciOS) recipe. Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include "coregraphics_canvas.hpp"
#include "graphics_host.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/rect_f.hpp"

// The drawable-rendering NSView (C# PlatformGraphicsView): top-left origin (isFlipped), transparent
// (no opaque background — C# sets Opaque = false / BackgroundColor null).
@interface MauiCppDrawableHostView : NSView
{
@public
    maui::graphics::i_drawable* _drawable; // non-owning borrow (graphics_host.hpp)
}
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
} // namespace maui::platform::apple

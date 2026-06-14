// The UIKit drawing host (graphics_host.hpp): a transparent UIView whose drawRect builds the shared
// coregraphics_canvas over UIGraphicsGetCurrentContext and replays the borrowed drawable — the
// PlatformGraphicsView (MaciOS) recipe. Compiled as Objective-C++ with ARC for the `ios` backend.

#import <UIKit/UIKit.h>

#include "coregraphics_canvas.hpp"
#include "graphics_host.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/rect_f.hpp"

// The drawable-rendering UIView (C# PlatformGraphicsView): UIKit is already top-left; transparent
// (C# Opaque = false / BackgroundColor null).
@interface MauiCppDrawableHostView : UIView
{
@public
    maui::graphics::i_drawable* _drawable; // non-owning borrow (graphics_host.hpp)
}
@end

@implementation MauiCppDrawableHostView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        self.opaque = NO;
        self.backgroundColor = nil;
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
} // namespace maui::platform::ios

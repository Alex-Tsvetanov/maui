// The shared flipped child-hosting views (flipped_container.hpp): a flipped NSView reused at every
// AppKit container-creation site, plus a flipped NSClipView for the scroll viewport. Both override
// isFlipped → YES so MAUI's top-left-origin child frames render top-down (matching iOS / C# Mac
// Catalyst). The Obj-C classes live in this single TU; every other unit creates instances through the
// pure-C++ factory in the header. Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include "flipped_container.hpp"

// A plain child-hosting NSView with a top-left origin (the AppKit analog of UIKit's default coordinate
// space). The handlers position children by setting frames with MAUI's top-down coordinates directly;
// the flip is what makes that placement render top-down.
@interface MauiFlippedView : NSView
@end

@implementation MauiFlippedView
- (BOOL)isFlipped
{
    return YES;
}
@end

// A flipped scroll viewport (NSClipView): the document content scrolls with a top-left origin so the
// content reads top-down and the scroll offset grows downward (the UIScrollView contentOffset analog),
// regardless of whether the document view is itself flipped.
@interface MauiFlippedClipView : NSClipView
@end

@implementation MauiFlippedClipView
- (BOOL)isFlipped
{
    return YES;
}
@end

namespace maui::platform::apple
{
    void* create_flipped_host()
    {
        MauiFlippedView* const host = [[MauiFlippedView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        return (__bridge_retained void*)host; // the caller's void* slot owns one reference
    }

    void install_flipped_clip_view(void* scroller)
    {
        if (scroller == nullptr)
        {
            return;
        }
        auto* const view = (__bridge NSScrollView*)scroller;
        // Swap in a flipped clip view, preserving the current document view (NSScrollView re-hosts it).
        MauiFlippedClipView* const clip = [[MauiFlippedClipView alloc] initWithFrame:view.contentView.frame];
        NSView* const document = view.documentView;
        view.contentView = clip;
        if (document != nil)
        {
            view.documentView = document; // re-attach under the new clip view
        }
    }
} // namespace maui::platform::apple

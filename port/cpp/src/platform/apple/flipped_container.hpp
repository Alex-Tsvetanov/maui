#pragma once
// maui::platform::apple — the shared FLIPPED child-hosting NSView (and NSClipView) used by every
// AppKit container/host that positions MAUI children with top-down frames. AppKit's NSView default is
// a bottom-left origin; MAUI (like UIKit/iOS) lays out with a TOP-LEFT origin and the handlers set
// child frames directly with those top-down coordinates (no per-handler Y-flip compensation). So a
// hosting view must declare isFlipped=YES to give that top-left origin — otherwise multi-element pages
// render bottom-up / inverted vs iOS and vs C# MAUI (Mac Catalyst). This is the same isFlipped override
// graphics_host's drawable view already uses; here it is factored into one shared subclass reused at
// every container-creation site (layout panel, content_page / border / swipe / refresh hosts, the
// navigation container, the scroll viewport, the window's empty content host).
//
// The header is a pure C++ void* seam so the handler .mm TUs (the layout/content_page/border/...
// units compiled into maui_core) can all create flipped hosts without each redefining an Obj-C class
// (one definition, in flipped_container.mm).
//
// Ownership mirrors create_drawable_host: create_flipped_host() returns a RETAINED NSView
// (__bridge_retained) — the caller's void* slot owns one reference and CFReleases it in its dtor, exactly
// as the existing [[NSView alloc] init...] sites already do.
//
// Leaf native controls (NSButton / NSTextField / NSDatePicker / …) that manage their OWN internal
// geometry must stay UNFLIPPED — only views that host MAUI-positioned children use these.

namespace maui::platform::apple
{
    // Create a flipped (top-left origin) child-hosting NSView, retained. Drop-in replacement for the
    // container-creation sites that previously did [[NSView alloc] initWithFrame:NSMakeRect(0,0,0,0)].
    void* create_flipped_host();

    // Install a flipped clip view (top-left origin) as the scroller's content viewport, so the scroll
    // content lays out / scrolls top-down regardless of the content's own flippedness — the AppKit
    // analog of UIScrollView's top-left content offset. `scroller` is an NSScrollView void* handle.
    void install_flipped_clip_view(void* scroller);
} // namespace maui::platform::apple

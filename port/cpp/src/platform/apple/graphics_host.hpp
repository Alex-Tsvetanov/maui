#pragma once
// maui::platform::apple — the shared NSView DRAWING HOST behind graphics_view_handler and
// shape_view_handler  <=  Microsoft.Maui.Graphics.Platform.PlatformGraphicsView (the AppKit
// drawable-rendering view: isFlipped top-left origin; Draw builds a PlatformCanvas over the current
// CGContext and replays Drawable.Draw — the port draws through the W1-13 coregraphics_canvas).
// C#'s PlatformTouchGraphicsView/MauiShapeView both sit on this same view; the port's two handlers
// likewise share this one host (the touch plumbing is deferred — see graphics_view_handler.hpp).
//
// The header is pure C++ (void* seam) so both handler .mm TUs include it; the ObjC class lives in
// graphics_host.mm. Ownership: create returns a RETAINED host (__bridge_retained) — the handler's
// void* native slot owns one reference and CFReleases it in the platform dtor. The drawable borrow
// is NON-owning (the control/handler owns the drawable's lifetime).

#include "maui/graphics/i_drawable.hpp"

namespace maui::platform::apple
{
    // Create the drawing-host NSView (retained — see the header note).
    void* create_drawable_host();
    // Point the host at the drawable it renders in drawRect (null = draw nothing) + redisplay.
    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable);
    // C# InvalidateDrawable → setNeedsDisplay.
    void drawable_host_invalidate(void* host);
} // namespace maui::platform::apple

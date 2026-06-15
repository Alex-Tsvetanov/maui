#pragma once
// maui::platform::apple — the shared NSView DRAWING HOST behind graphics_view_handler and
// shape_view_handler  <=  Microsoft.Maui.Graphics.Platform.PlatformGraphicsView (the AppKit
// drawable-rendering view: isFlipped top-left origin; Draw builds a PlatformCanvas over the current
// CGContext and replays Drawable.Draw — the port draws through the W1-13 coregraphics_canvas).
// C#'s PlatformTouchGraphicsView/MauiShapeView both sit on this same view; the port's two handlers
// likewise share this one host. The host carries the AppKit touch plumbing (the PlatformTouchGraphicsView
// recipe): its mouseDown:/mouseDragged:/mouseUp: route to the connected i_graphics_view's
// send_start/drag/end_interaction, the AppKit analog of TouchesBegan/Moved/Ended → Start/Drag/End-
// Interaction. The shape host simply never connects an interaction target, so it stays draw-only.
//
// The header is pure C++ (void* seam) so both handler .mm TUs include it; the ObjC class lives in
// graphics_host.mm. Ownership: create returns a RETAINED host (__bridge_retained) — the handler's
// void* native slot owns one reference and CFReleases it in the platform dtor. The drawable borrow AND
// the interaction-target borrow are NON-owning (the control/handler owns their lifetime; the handler
// clears the target in on_disconnect_handler).

#include "maui/graphics/i_drawable.hpp"

namespace maui::core
{
    class i_graphics_view; // the interaction target (forward-declared — pure pointer seam)
} // namespace maui::core

namespace maui::platform::apple
{
    // Create the drawing-host NSView (retained — see the header note).
    void* create_drawable_host();
    // Point the host at the drawable it renders in drawRect (null = draw nothing) + redisplay.
    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable);
    // C# InvalidateDrawable → setNeedsDisplay.
    void drawable_host_invalidate(void* host);

    // ---- touch plumbing (PlatformTouchGraphicsView) ----
    // Connect the interaction target the host's mouse events route to (null disconnects). The host holds
    // a NON-OWNING pointer; the graphics_view handler sets it on connect and clears it on disconnect, so
    // the borrow never outlives the virtual view. The shape host leaves it null (draw-only).
    void drawable_host_set_interaction_target(void* host, maui::core::i_graphics_view* target);
} // namespace maui::platform::apple

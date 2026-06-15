#pragma once
// maui::platform::ios — the shared UIView DRAWING HOST behind graphics_view_handler and
// shape_view_handler  <=  Microsoft.Maui.Graphics.Platform.PlatformGraphicsView /
// PlatformTouchGraphicsView (the UIKit drawable-rendering view: drawRect builds a PlatformCanvas over
// UIGraphicsGetCurrentContext and replays Drawable.Draw — the port draws through the W1-13
// coregraphics_canvas — PLUS the touchesBegan/Moved/Ended/Cancelled plumbing → the connected
// i_graphics_view's send_*_interaction). The AppKit twin is src/platform/apple/graphics_host.hpp; same
// seam, same ownership rules (the void* slot owns one retained reference; the drawable + interaction-
// target borrows are non-owning).

#include "maui/graphics/i_drawable.hpp"

namespace maui::core
{
    class i_graphics_view; // the interaction target (forward-declared — pure pointer seam)
} // namespace maui::core

namespace maui::platform::ios
{
    // Create the drawing-host UIView (retained — see the header note).
    void* create_drawable_host();
    // Point the host at the drawable it renders in drawRect (null = draw nothing) + redisplay.
    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable);
    // C# InvalidateDrawable → setNeedsDisplay.
    void drawable_host_invalidate(void* host);

    // ---- touch plumbing (PlatformTouchGraphicsView) ----
    // Connect the interaction target the host's touch events route to (null disconnects). NON-OWNING —
    // the graphics_view handler sets it on connect and clears it on disconnect. The shape host leaves it
    // null (draw-only).
    void drawable_host_set_interaction_target(void* host, maui::core::i_graphics_view* target);
} // namespace maui::platform::ios

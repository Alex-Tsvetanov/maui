#pragma once
// maui::platform::ios — the shared UIView DRAWING HOST behind graphics_view_handler and
// shape_view_handler  <=  Microsoft.Maui.Graphics.Platform.PlatformGraphicsView (the UIKit
// drawable-rendering view: drawRect builds a PlatformCanvas over UIGraphicsGetCurrentContext and
// replays Drawable.Draw — the port draws through the W1-13 coregraphics_canvas). The AppKit twin is
// src/platform/apple/graphics_host.hpp; same seam, same ownership rules (the void* slot owns one
// retained reference; the drawable borrow is non-owning).

#include "maui/graphics/i_drawable.hpp"

namespace maui::platform::ios
{
    // Create the drawing-host UIView (retained — see the header note).
    void* create_drawable_host();
    // Point the host at the drawable it renders in drawRect (null = draw nothing) + redisplay.
    void drawable_host_set_drawable(void* host, maui::graphics::i_drawable* drawable);
    // C# InvalidateDrawable → setNeedsDisplay.
    void drawable_host_invalidate(void* host);
} // namespace maui::platform::ios

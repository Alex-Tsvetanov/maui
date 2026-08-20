#pragma once
// maui::core::i_border_stroke  <=  Microsoft.Maui.IBorderStroke
//
// How a shape outline is painted around a container: the i_stroke surface plus the shape the outline
// follows. Ported from src/Core/src/Core/IBorderStroke.cs (IBorderStroke : IStroke { IShape? Shape; }).
//
// shape() returns a non-owning borrow (the control owns the shape via a shared_ptr — the same ownership
// rule as i_view::clip()); null means no border shape.

#include "maui/core/i_stroke.hpp"
#include "maui/graphics/i_shape.hpp"

namespace maui::core
{
    class i_border_stroke : public i_stroke
    {
    public:
        // C# IBorderStroke.Shape — the geometry the border outline (and content clip) follows.
        [[nodiscard]] virtual maui::graphics::i_shape* shape() const = 0;

        // PORT-SIDE MARKER (no C# counterpart) — does this shape carry a Controls Shape's own 0.5 DIP/side
        // self-inset (maui::core::shape_self_inset)?
        //
        // In C# the answer is structural, so no flag is needed: a Border's StrokeShape IS a Controls Shape
        // (Border.StrokeShapeProperty defaults to a Rectangle, Border.cs:81) whose own StrokeThickness of
        // 1.0 deflates the bounds inside Shape.PathForBounds — and a Frame is a DIFFERENT control with no
        // StrokeShape at all, rendered by the compatibility FrameRenderer, which never calls PathForBounds.
        // The port collapses both onto maui::controls::border (frame : border SYNTHESIZES a round_rectangle
        // StrokeShape to express FrameRenderer.SetupLayer's corner radius), so the two render paths become
        // indistinguishable at the handler seam unless the facade says which one it is.
        //
        // MEASURED (the standing doctrine — the render decides): on `containers`/android the port's pre-inset Frame
        // matched MAUI's to 6 pixels in dark over the whole Frame band; insetting it moved 2894. The
        // Border on the same page needs the inset (border_stroke: MAUI leaves a 2 px gap between adjacent
        // Borders where the port abutted them). So: Border yes, Frame no.
        [[nodiscard]] virtual bool shape_self_insets() const
        {
            return true; // a real Border.StrokeShape; only the Frame facade overrides
        }
    };
} // namespace maui::core

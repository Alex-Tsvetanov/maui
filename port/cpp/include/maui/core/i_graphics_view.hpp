#pragma once
// maui::core::i_graphics_view  <=  Microsoft.Maui.IGraphicsView
//
// The virtual-view contract for a view drawn with canvas commands: the drawable + the invalidate
// request, plus the inbound touch/hover interaction channel the platform view raises. Ported from
// src/Core/src/Core/IGraphicsView.cs.
//
// drawable() returns a non-owning borrow (the control owns it via shared_ptr); null = nothing to
// draw. The interaction points arrive as a vector of point_f (C# PointF[]).
//
// Naming note (the i_button precedent): C# gives GraphicsView both an event `StartInteraction` and
// the explicit interface method `IGraphicsView.StartInteraction(points)`. C++ cannot overload a data
// member with a method, so the inbound methods take the `send_` prefix and the control's events keep
// the bare C# names.

#include <vector>

#include "maui/core/i_view.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point_f.hpp"

namespace maui::core
{
    class i_graphics_view : public i_view
    {
    public:
        // C# IGraphicsView.Drawable — the drawing content (null = nothing to draw).
        [[nodiscard]] virtual maui::graphics::i_drawable* drawable() const = 0;

        // C# IGraphicsView.Invalidate() — ask the canvas to redraw itself.
        virtual void invalidate() = 0;

        // ---- the inbound interaction channel (the platform view calls these; the control raises
        //      its public events) ----
        // C# IGraphicsView.StartHoverInteraction(PointF[]).
        virtual void send_start_hover_interaction(const std::vector<maui::graphics::point_f>& points) = 0;
        // C# IGraphicsView.MoveHoverInteraction(PointF[]).
        virtual void send_move_hover_interaction(const std::vector<maui::graphics::point_f>& points) = 0;
        // C# IGraphicsView.EndHoverInteraction().
        virtual void send_end_hover_interaction() = 0;
        // C# IGraphicsView.StartInteraction(PointF[]).
        virtual void send_start_interaction(const std::vector<maui::graphics::point_f>& points) = 0;
        // C# IGraphicsView.DragInteraction(PointF[]).
        virtual void send_drag_interaction(const std::vector<maui::graphics::point_f>& points) = 0;
        // C# IGraphicsView.EndInteraction(PointF[], bool isInsideBounds).
        virtual void send_end_interaction(const std::vector<maui::graphics::point_f>& points,
                                          bool is_inside_bounds) = 0;
        // C# IGraphicsView.CancelInteraction().
        virtual void send_cancel_interaction() = 0;
    };
} // namespace maui::core

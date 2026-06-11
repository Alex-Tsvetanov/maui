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
    };
} // namespace maui::core

#pragma once
// maui::graphics::i_pattern  <=  Microsoft.Maui.Graphics.IPattern
//
// A pattern that fills shapes on a canvas by tiling: it exposes a tile size (width/height), the repeat
// step (step_x/step_y) and a draw(canvas) that renders one tile. Ported from
// src/Graphics/src/Graphics/IPattern.cs. The CoreGraphics backend's FillWithPattern builds a CGPattern
// from these four dimensions and invokes draw() in the tile callback (re-entering a nested canvas bound
// to the tile context).
//
// An abstract class (PROFILE §11 — runtime polymorphism: the canvas tiles any pattern, the
// pattern_paint wraps any pattern).

#include "maui/graphics/i_canvas.hpp"

namespace maui::graphics
{
    class i_pattern
    {
    public:
        virtual ~i_pattern() = default;

        // C# IPattern.Width — the tile width.
        [[nodiscard]] virtual float width() const = 0;
        // C# IPattern.Height — the tile height.
        [[nodiscard]] virtual float height() const = 0;
        // C# IPattern.StepX — the horizontal repeat step.
        [[nodiscard]] virtual float step_x() const = 0;
        // C# IPattern.StepY — the vertical repeat step.
        [[nodiscard]] virtual float step_y() const = 0;

        // C# IPattern.Draw(ICanvas) — render one tile onto the canvas.
        virtual void draw(i_canvas& canvas) = 0;

    protected:
        i_pattern() = default;
        i_pattern(const i_pattern&) = default;
        i_pattern(i_pattern&&) = default;
        i_pattern& operator=(const i_pattern&) = default;
        i_pattern& operator=(i_pattern&&) = default;
    };
} // namespace maui::graphics

#pragma once
// maui::graphics::i_drawable  <=  Microsoft.Maui.Graphics.IDrawable
// An object that can draw itself onto a canvas. Ported from src/Graphics/src/Graphics/IDrawable.cs.
// An abstract class (PROFILE §11 — runtime polymorphism: a GraphicsView invokes any drawable).

#include "maui/graphics/rect_f.hpp"

namespace maui::graphics
{
    class i_canvas;

    class i_drawable
    {
    public:
        virtual ~i_drawable() = default;

        // C# IDrawable.Draw(ICanvas canvas, RectF dirtyRect).
        virtual void draw(i_canvas& canvas, const rect_f& dirty_rect) = 0;

    protected:
        i_drawable() = default;
        i_drawable(const i_drawable&) = default;
        i_drawable(i_drawable&&) = default;
        i_drawable& operator=(const i_drawable&) = default;
        i_drawable& operator=(i_drawable&&) = default;
    };
} // namespace maui::graphics

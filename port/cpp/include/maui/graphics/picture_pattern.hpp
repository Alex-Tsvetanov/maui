#pragma once
// maui::graphics::picture_pattern  <=  Microsoft.Maui.Graphics.PicturePattern
//
// A pattern (abstract_pattern) that tiles a picture: its tile size is the picture's Width/Height and
// draw() replays the picture onto the canvas. Ported from src/Graphics/src/Graphics/PicturePattern.cs —
// the two ctors (picture + explicit stepX/stepY, or picture alone using its dimensions as the step).
//
// Ownership: the picture is held NON-OWNING (a raw borrow) — the caller owns it; the pattern references
// which picture to replay (C#'s readonly IPicture reference field). The dimensions are snapshotted into
// abstract_pattern at construction (matching C#, which passes picture.Width/Height to the base ctor).

#include "maui/graphics/abstract_pattern.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_picture.hpp"

namespace maui::graphics
{
    class picture_pattern final : public abstract_pattern
    {
    public:
        // C# PicturePattern(IPicture picture, float stepX, float stepY) : base(picture.Width, picture.Height, ...).
        picture_pattern(i_picture* picture, float step_x, float step_y);
        // C# PicturePattern(IPicture picture) : base(picture.Width, picture.Height) — picture dims as the step.
        explicit picture_pattern(i_picture* picture);

        // C# PicturePattern.Draw(ICanvas) — _picture.Draw(canvas).
        void draw(i_canvas& canvas) override;

    private:
        i_picture* picture_; // non-owning (the caller owns the picture)
    };
} // namespace maui::graphics

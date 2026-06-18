#pragma once
// maui::graphics::i_picture  <=  Microsoft.Maui.Graphics.IPicture
//
// A recorded set of drawing commands that can be replayed onto a canvas, with an origin (x/y) and a size
// (width/height). Ported from src/Graphics/src/Graphics/IPicture.cs. Used here as the content source for
// picture_pattern (a pattern that tiles a picture). The full picture-recording machinery (PictureCanvas)
// is not ported — i_picture is the minimal contract picture_pattern needs; a concrete picture lands with
// a real picture-recording consumer.
//
// An abstract class (PROFILE §11 — runtime polymorphism: the pattern replays any picture).

#include "maui/graphics/i_canvas.hpp"

namespace maui::graphics
{
    class i_picture
    {
    public:
        virtual ~i_picture() = default;

        // C# IPicture.Draw(ICanvas) — replay the recorded commands onto the canvas.
        virtual void draw(i_canvas& canvas) = 0;

        // C# IPicture.X / Y — the picture origin.
        [[nodiscard]] virtual float x() const = 0;
        [[nodiscard]] virtual float y() const = 0;
        // C# IPicture.Width / Height — the picture size.
        [[nodiscard]] virtual float width() const = 0;
        [[nodiscard]] virtual float height() const = 0;

    protected:
        i_picture() = default;
        i_picture(const i_picture&) = default;
        i_picture(i_picture&&) = default;
        i_picture& operator=(const i_picture&) = default;
        i_picture& operator=(i_picture&&) = default;
    };
} // namespace maui::graphics

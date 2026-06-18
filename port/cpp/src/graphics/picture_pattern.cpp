// maui::graphics::picture_pattern  <=  Microsoft.Maui.Graphics.PicturePattern
// Out-of-line definitions (see picture_pattern.hpp). Behavior derived from
// src/Graphics/src/Graphics/PicturePattern.cs.

#include "maui/graphics/picture_pattern.hpp"

#include "maui/graphics/abstract_pattern.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_picture.hpp"

namespace maui::graphics
{
    // C# : base(picture.Width, picture.Height, stepX, stepY). picture is read for its dimensions, so the
    // base ctor runs before the member is stored — mirror C# by reading through the argument.
    picture_pattern::picture_pattern(i_picture* picture, float step_x, float step_y)
        : abstract_pattern(picture->width(), picture->height(), step_x, step_y), picture_(picture)
    {
    }

    // C# : base(picture.Width, picture.Height) — the picture's size is also the repeat step.
    picture_pattern::picture_pattern(i_picture* picture)
        : abstract_pattern(picture->width(), picture->height()), picture_(picture)
    {
    }

    void picture_pattern::draw(i_canvas& canvas)
    {
        picture_->draw(canvas);
    }
} // namespace maui::graphics

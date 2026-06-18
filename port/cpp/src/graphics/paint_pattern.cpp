// maui::graphics::paint_pattern  <=  Microsoft.Maui.Graphics.PaintPattern
// Out-of-line definitions (see paint_pattern.hpp). Behavior derived from
// src/Graphics/src/Graphics/PaintPattern.cs.

#include "maui/graphics/paint_pattern.hpp"

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_pattern.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::graphics
{
    // C# Width/Height/StepX/StepY => Wrapped?.X ?? 0 — forward to the wrapped pattern, 0 when null.
    float paint_pattern::width() const
    {
        return wrapped_ != nullptr ? wrapped_->width() : 0.0F;
    }
    float paint_pattern::height() const
    {
        return wrapped_ != nullptr ? wrapped_->height() : 0.0F;
    }
    float paint_pattern::step_x() const
    {
        return wrapped_ != nullptr ? wrapped_->step_x() : 0.0F;
    }
    float paint_pattern::step_y() const
    {
        return wrapped_ != nullptr ? wrapped_->step_y() : 0.0F;
    }

    void paint_pattern::draw(i_canvas& canvas)
    {
        if (paint_ != nullptr)
        {
            // C# guards the background fill on `BackgroundColor.Alpha > 1`, which can never be true (alpha
            // is clamped to [0, 1]) — the branch is dead in C# too. Ported faithfully; it never fires.
            if (paint_->background_color().alpha > 1.0F)
            {
                canvas.set_fill_color(paint_->background_color());
                canvas.fill_rectangle(0, 0, width(), height());
            }

            canvas.set_stroke_color(paint_->foreground_color());
            canvas.set_fill_color(paint_->foreground_color());
        }
        else
        {
            canvas.set_stroke_color(colors::black);
            canvas.set_fill_color(colors::black);
        }

        // C# Wrapped.Draw(canvas) (would NRE on a null Wrapped); guarded here to avoid UB — a null Wrapped
        // is otherwise a valid state (the dims forward to 0).
        if (wrapped_ != nullptr)
        {
            wrapped_->draw(canvas);
        }
    }
} // namespace maui::graphics

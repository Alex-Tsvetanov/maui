#pragma once
// maui::graphics::paint  <=  Microsoft.Maui.Graphics.Paint
//
// Abstract base for the brush kinds that fill a shape (the IView.Background value AND the drawing-canvas
// ICanvas fill paint). Ported from src/Graphics/src/Graphics/Paint.cs: an abstract class carrying a
// background color, a foreground color and an IsTransparent query. An abstract class (PROFILE §11 —
// runtime polymorphism via the view_mapper / the canvas SetFillPaint dynamic_cast).
//
// Subclasses modeled in the port: SolidPaint (solid_paint.hpp), the gradient paints
// (linear/radial_gradient_paint.hpp), the drawing-canvas ImagePaint (image_paint.hpp) and PatternPaint
// (pattern_paint.hpp). The view-background ImageSourcePaint lives in core/image_source_paint.hpp.
//
// C# Paint exposes BackgroundColor AND ForegroundColor as plain nullable auto-properties. The port keeps
// background_color() pure-virtual (solid/gradient compute it; pattern returns its stored color) and adds a
// virtual foreground_color() with a default (Colors.Black-shaped default color) — only PatternPaint reads
// ForegroundColor (PaintPattern.Draw sets the canvas stroke/fill to it; PatternPaint.IsTransparent reads
// its alpha), so the default keeps every other subclass unchanged.
//
// Ownership: the control owns a paint via a shared_ptr (see controls/view.hpp); i_view::background()
// returns a raw borrow and the view_platform_base mirror is a non-owning const pointer.

#include "maui/graphics/color.hpp"

namespace maui::graphics
{
    class paint
    {
    public:
        virtual ~paint() = default;

        // C# Paint.BackgroundColor — the color this paint fills with (SolidPaint returns its Color).
        [[nodiscard]] virtual maui::graphics::color background_color() const = 0;

        // C# Paint.ForegroundColor — the paint's foreground color. C#'s auto-property defaults to null;
        // here the default returns the value-type default color (opaque black). Only PatternPaint
        // overrides it with a stored, settable value (PaintPattern.Draw and PatternPaint.IsTransparent
        // are its only readers).
        [[nodiscard]] virtual maui::graphics::color foreground_color() const
        {
            return maui::graphics::color{};
        }

        // C# Paint.IsTransparent — true when the paint has transparent areas (SolidPaint: alpha < 1).
        [[nodiscard]] virtual bool is_transparent() const = 0;

    protected:
        paint() = default;
        paint(const paint&) = default;
        paint(paint&&) = default;
        paint& operator=(const paint&) = default;
        paint& operator=(paint&&) = default;
    };
} // namespace maui::graphics

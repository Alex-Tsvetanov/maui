#pragma once
// maui::graphics::paint  <=  Microsoft.Maui.Graphics.Paint
//
// Abstract base for the brush kinds that fill a shape (the IView.Background value). Ported from
// src/Graphics/src/Graphics/Paint.cs: an abstract class carrying a background color and an
// IsTransparent query. An abstract class (PROFILE §11 — runtime polymorphism via the view_mapper).
//
// FIRST CUT (this unit): only the SolidPaint subclass is modeled (see solid_paint.hpp). The gradient
// paints (LinearGradientPaint / RadialGradientPaint), the ForegroundColor, and the PatternPaint /
// ImagePaint kinds are deferred — documented here, not stubbed. Header-only (no out-of-line state).
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

#pragma once
// maui::graphics::pattern_paint  <=  Microsoft.Maui.Graphics.PatternPaint
//
// A drawing-canvas Paint that fills shapes with a repeating pattern: ICanvas.SetFillPaint detects it and
// the Fill* methods tile the pattern across the shape (the CoreGraphics backend's FillWithPattern, a
// CGPattern whose tile callback re-enters a nested canvas and calls pattern->draw). Ported from
// src/Graphics/src/Graphics/PatternPaint.cs.
//
// C# Pattern setter: stores the value, and if it is not already a PaintPattern, wraps it in
// `new PaintPattern(value) { Paint = this }`. So after a set, the effective Pattern is always a pattern
// that primes the canvas stroke/fill before drawing. The port mirrors this: a non-paint_pattern value is
// wrapped in an OWNED paint_pattern (whose Paint back-reference is this); an already-paint_pattern value
// is referenced as-is (the caller owns it), exactly as C# leaves it untouched.
//
// C# IsTransparent: true if BackgroundColor is null OR its alpha < 1; otherwise ForegroundColor.Alpha < 1.
// C#'s Paint colors are nullable; the port models pattern_paint's background/foreground as
// std::optional<color> (null = unset) so the "BackgroundColor == null -> transparent" branch is faithful.
// A freshly-constructed pattern_paint has a null background => IsTransparent is true, exactly as C#.
//
// Inherits maui::graphics::paint (PROFILE §11 — the canvas SetFillPaint dynamic_casts to it).
//
// Ownership: see the member notes — the owned wrapper (unique_ptr) for the wrap case; the effective
// pattern is a non-owning pointer into either that wrapper or the caller's paint_pattern.

#include <memory>
#include <optional>

#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::graphics
{
    class i_pattern;
    class paint_pattern;

    class pattern_paint final : public paint
    {
    public:
        pattern_paint();
        ~pattern_paint() override;
        pattern_paint(const pattern_paint&) = delete;
        pattern_paint(pattern_paint&&) = delete;
        pattern_paint& operator=(const pattern_paint&) = delete;
        pattern_paint& operator=(pattern_paint&&) = delete;

        // C# PatternPaint.Pattern getter — the effective pattern (always the PaintPattern after a set;
        // null when never set). Non-owning: see set_pattern for the ownership split.
        [[nodiscard]] i_pattern* pattern() const noexcept
        {
            return pattern_;
        }
        // C# PatternPaint.Pattern setter — wraps a non-paint_pattern value in an owned paint_pattern whose
        // Paint is this; an already-paint_pattern value is referenced as-is. A null value clears it.
        void set_pattern(i_pattern* value);

        // C# Paint.BackgroundColor — nullable; the setter stores it, the getter returns it or the default.
        void set_background_color(const color& value)
        {
            background_color_ = value;
        }
        void clear_background_color() noexcept
        {
            background_color_.reset();
        }
        // C# Paint.ForegroundColor — nullable; PaintPattern.Draw reads it (canvas stroke/fill).
        void set_foreground_color(const color& value)
        {
            foreground_color_ = value;
        }
        void clear_foreground_color() noexcept
        {
            foreground_color_.reset();
        }

        // ---- paint ----
        // The stored background color or the value-type default when unset.
        [[nodiscard]] color background_color() const override
        {
            return background_color_.value_or(color{});
        }
        // The stored foreground color or the value-type default when unset.
        [[nodiscard]] color foreground_color() const override
        {
            return foreground_color_.value_or(color{});
        }
        // C# PatternPaint.IsTransparent: BackgroundColor null || alpha < 1 => true; else ForegroundColor.Alpha < 1.
        [[nodiscard]] bool is_transparent() const override
        {
            if (!background_color_.has_value() || background_color_->alpha < 1.0F)
            {
                return true;
            }
            return foreground_color().alpha < 1.0F;
        }

    private:
        i_pattern* pattern_ = nullptr;                 // effective pattern (non-owning; points into the
                                                       // owned wrapper OR the caller's paint_pattern)
        std::unique_ptr<paint_pattern> owned_wrapper_; // set only when this paint had to wrap the value
        std::optional<color> background_color_;        // C# nullable BackgroundColor (unset = null)
        std::optional<color> foreground_color_;        // C# nullable ForegroundColor (unset = null)
    };
} // namespace maui::graphics

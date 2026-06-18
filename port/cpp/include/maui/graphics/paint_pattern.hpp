#pragma once
// maui::graphics::paint_pattern  <=  Microsoft.Maui.Graphics.PaintPattern
//
// A pattern that applies a Paint to a wrapped pattern: it forwards the tile size / repeat step to the
// wrapped pattern (0 when there is no wrapped pattern) and, in draw(), primes the canvas stroke/fill from
// the paint's foreground color (black when there is no paint) before replaying the wrapped pattern.
// Ported from src/Graphics/src/Graphics/PaintPattern.cs.
//
// Ownership: both the wrapped pattern and the paint are held NON-OWNING (raw borrows). pattern_paint
// constructs a paint_pattern wrapping the user's original pattern (the user owns it) and sets Paint to
// itself (an owner back-reference — the pattern_paint owns this paint_pattern), so neither can be owned
// here without a cycle. Matches C#'s readonly IPattern Wrapped + the Paint = this back-reference.

#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_pattern.hpp"

namespace maui::graphics
{
    class paint;

    class paint_pattern final : public i_pattern
    {
    public:
        // C# PaintPattern(IPattern pattern) — wraps the pattern.
        explicit paint_pattern(i_pattern* wrapped) : wrapped_(wrapped)
        {
        }

        // C# PaintPattern.Wrapped — the wrapped pattern (non-owning).
        [[nodiscard]] i_pattern* wrapped() const noexcept
        {
            return wrapped_;
        }

        // C# PaintPattern.Paint { get; set; } — the paint to apply (non-owning owner back-reference).
        [[nodiscard]] paint* paint() const noexcept
        {
            return paint_;
        }
        void set_paint(graphics::paint* value) noexcept
        {
            paint_ = value;
        }

        // ---- i_pattern (all forward to Wrapped, 0 when Wrapped is null, exactly as C#) ----
        [[nodiscard]] float width() const override;
        [[nodiscard]] float height() const override;
        [[nodiscard]] float step_x() const override;
        [[nodiscard]] float step_y() const override;

        // C# PaintPattern.Draw(ICanvas) — set stroke/fill from the paint foreground (black if no paint),
        // then draw the wrapped pattern.
        void draw(i_canvas& canvas) override;

    private:
        i_pattern* wrapped_;               // non-owning (the user owns the wrapped pattern)
        graphics::paint* paint_ = nullptr; // non-owning owner back-reference (the pattern_paint owns this)
    };
} // namespace maui::graphics

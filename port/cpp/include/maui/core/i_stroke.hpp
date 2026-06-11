#pragma once
// maui::core::i_stroke  <=  Microsoft.Maui.IStroke
//
// The contract describing how an element's outline is painted. Ported from src/Core/src/Core/IStroke.cs:
// the stroke brush, its thickness, the line cap/join styles, the dash pattern + offset, and the miter
// limit. The line cap/join reuse the existing maui::graphics enums (LineCap/LineJoin — the same types the
// C# interface returns); the Controls-layer Border converts its PenLineCap/PenLineJoin surface to these,
// a conversion the port collapses by exposing the graphics enums directly on the control too.
//
// stroke() returns a non-owning borrow (the control owns its paint via a shared_ptr, PROFILE §8); null
// when no stroke is set. stroke_dash_pattern() returns the dash lengths BY VALUE: C#'s StrokeDashPattern
// is a float[] materialized from the DoubleCollection on every read (Border.cs), so the port's converted
// copy is the faithful shape (empty = solid line, like C# null).

#include <vector>

#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::core
{
    class i_stroke
    {
    public:
        virtual ~i_stroke() = default;

        // C# IStroke.Stroke — the brush painting the outline (null = no stroke).
        [[nodiscard]] virtual maui::graphics::paint* stroke() const = 0;
        // C# IStroke.StrokeThickness.
        [[nodiscard]] virtual double stroke_thickness() const = 0;
        // C# IStroke.StrokeLineCap / StrokeLineJoin.
        [[nodiscard]] virtual maui::graphics::line_cap stroke_line_cap() const = 0;
        [[nodiscard]] virtual maui::graphics::line_join stroke_line_join() const = 0;
        // C# IStroke.StrokeDashPattern — dash/gap lengths (empty = solid, the C# null analog).
        [[nodiscard]] virtual std::vector<float> stroke_dash_pattern() const = 0;
        // C# IStroke.StrokeDashOffset / StrokeMiterLimit.
        [[nodiscard]] virtual float stroke_dash_offset() const = 0;
        [[nodiscard]] virtual float stroke_miter_limit() const = 0;

    protected:
        i_stroke() = default;
        i_stroke(const i_stroke&) = default;
        i_stroke(i_stroke&&) = default;
        i_stroke& operator=(const i_stroke&) = default;
        i_stroke& operator=(i_stroke&&) = default;
    };
} // namespace maui::core

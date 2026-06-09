#pragma once
// maui::graphics::linear_gradient_paint  <=  Microsoft.Maui.Graphics.LinearGradientPaint
//
// A gradient paint that transitions colors along a line from start_point to end_point (relative
// coordinates, typically (0,0)..(1,1)). Ported from src/Graphics/src/Graphics/LinearGradientPaint.cs:
// the same gradient_stop machinery as the base plus the start/end points. Defaults: start (0,0), end (1,1).
//
// Points use maui::graphics::point (double), the port of C#'s Point.
//
// Out-of-line definitions live in linear_gradient_paint.cpp.

#include <vector>

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/point.hpp"

namespace maui::graphics
{
    class linear_gradient_paint : public gradient_paint
    {
    public:
        // C# LinearGradientPaint() — start (0,0), end (1,1), default white-to-white stops.
        linear_gradient_paint();
        // C# LinearGradientPaint(GradientPaint) — copy the source's stops (start/end stay the defaults).
        explicit linear_gradient_paint(const gradient_paint& source);
        // C# LinearGradientPaint(PaintGradientStop[]) — the given stops (start/end stay the defaults).
        explicit linear_gradient_paint(std::vector<gradient_stop> gradient_stops);
        // C# LinearGradientPaint(Point startPoint, Point endPoint).
        linear_gradient_paint(maui::graphics::point start_point, maui::graphics::point end_point);
        // C# LinearGradientPaint(PaintGradientStop[], Point, Point).
        linear_gradient_paint(std::vector<gradient_stop> gradient_stops, maui::graphics::point start_point,
                              maui::graphics::point end_point);

        // C# LinearGradientPaint.StartPoint — the gradient line's start, relative coordinates.
        [[nodiscard]] maui::graphics::point start_point() const;
        void set_start_point(maui::graphics::point value);

        // C# LinearGradientPaint.EndPoint — the gradient line's end, relative coordinates.
        [[nodiscard]] maui::graphics::point end_point() const;
        void set_end_point(maui::graphics::point value);

    private:
        maui::graphics::point start_point_;
        maui::graphics::point end_point_;
    };
} // namespace maui::graphics

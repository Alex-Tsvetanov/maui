#pragma once
// maui::graphics::radial_gradient_paint  <=  Microsoft.Maui.Graphics.RadialGradientPaint
//
// A gradient paint that transitions colors outward from a center point over a radius (relative
// coordinates). Ported from src/Graphics/src/Graphics/RadialGradientPaint.cs: the base gradient_stop
// machinery plus a center and radius. Defaults: center (0.5,0.5), radius 0.5.
//
// Center uses maui::graphics::point (double); radius is a double (matching C#).
//
// Out-of-line definitions live in radial_gradient_paint.cpp.

#include <vector>

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/point.hpp"

namespace maui::graphics
{
    class radial_gradient_paint : public gradient_paint
    {
    public:
        // C# RadialGradientPaint() — center (0.5,0.5), radius 0.5, default white-to-white stops.
        radial_gradient_paint();
        // C# RadialGradientPaint(GradientPaint) — copy the source's stops (center/radius stay the defaults).
        explicit radial_gradient_paint(const gradient_paint& source);
        // C# RadialGradientPaint(PaintGradientStop[]) — the given stops (center/radius stay the defaults).
        explicit radial_gradient_paint(std::vector<gradient_stop> gradient_stops);
        // C# RadialGradientPaint(Point center, double radius).
        radial_gradient_paint(maui::graphics::point center, double radius);
        // C# RadialGradientPaint(PaintGradientStop[], Point, double).
        radial_gradient_paint(std::vector<gradient_stop> gradient_stops, maui::graphics::point center, double radius);

        // C# RadialGradientPaint.Center — the gradient's center, relative coordinates.
        [[nodiscard]] maui::graphics::point center() const;
        void set_center(maui::graphics::point value);

        // C# RadialGradientPaint.Radius — the gradient's radius, relative units.
        [[nodiscard]] double radius() const;
        void set_radius(double value);

    private:
        maui::graphics::point center_;
        double radius_ = 0.0;
    };
} // namespace maui::graphics

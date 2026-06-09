// maui::graphics::radial_gradient_paint — out-of-line definitions. See radial_gradient_paint.hpp.
// Ported from src/Graphics/src/Graphics/RadialGradientPaint.cs.

#include "maui/graphics/radial_gradient_paint.hpp"

#include <utility>
#include <vector>

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/point.hpp"

namespace maui::graphics
{
    radial_gradient_paint::radial_gradient_paint() : center_(0.5, 0.5), radius_(0.5)
    {
    }

    radial_gradient_paint::radial_gradient_paint(const gradient_paint& source) : gradient_paint(source)
    {
        // C# RadialGradientPaint(GradientPaint) chains the base copy ctor (copies the stops); Center / Radius
        // are NOT assigned, so they keep the C# defaults (Point.Zero == (0,0), radius 0.0) — NOT the
        // parameterless ctor's (0.5,0.5)/0.5. The members already default-initialize to (0,0) / 0.0.
    }

    radial_gradient_paint::radial_gradient_paint(std::vector<gradient_stop> gradient_stops)
    {
        // C# RadialGradientPaint(PaintGradientStop[]) sets only GradientStops; Center/Radius keep their
        // defaults ((0,0) / 0.0). The member initializers already default both.
        set_gradient_stops(std::move(gradient_stops));
    }

    radial_gradient_paint::radial_gradient_paint(maui::graphics::point center, double radius)
        : center_(center), radius_(radius)
    {
    }

    radial_gradient_paint::radial_gradient_paint(std::vector<gradient_stop> gradient_stops,
                                                 maui::graphics::point center, double radius)
        : center_(center), radius_(radius)
    {
        set_gradient_stops(std::move(gradient_stops));
    }

    maui::graphics::point radial_gradient_paint::center() const
    {
        return center_;
    }

    void radial_gradient_paint::set_center(maui::graphics::point value)
    {
        center_ = value;
    }

    double radial_gradient_paint::radius() const
    {
        return radius_;
    }

    void radial_gradient_paint::set_radius(double value)
    {
        radius_ = value;
    }
} // namespace maui::graphics

// maui::graphics::linear_gradient_paint — out-of-line definitions. See linear_gradient_paint.hpp.
// Ported from src/Graphics/src/Graphics/LinearGradientPaint.cs.

#include "maui/graphics/linear_gradient_paint.hpp"

#include <utility>
#include <vector>

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/point.hpp"

namespace maui::graphics
{
    linear_gradient_paint::linear_gradient_paint() : start_point_(0, 0), end_point_(1, 1)
    {
    }

    linear_gradient_paint::linear_gradient_paint(const gradient_paint& source) : gradient_paint(source)
    {
        // C# LinearGradientPaint(GradientPaint) chains the base copy ctor (copies the stops); StartPoint /
        // EndPoint are NOT assigned, so they keep the C# struct default (Point.Zero == (0,0)) — NOT the
        // parameterless ctor's (0,0)/(1,1). The point members already default-initialize to (0,0).
    }

    linear_gradient_paint::linear_gradient_paint(std::vector<gradient_stop> gradient_stops)
    {
        // C# LinearGradientPaint(PaintGradientStop[]) sets only GradientStops; StartPoint/EndPoint keep the
        // struct default (0,0). The member initializers already default both points to (0,0).
        set_gradient_stops(std::move(gradient_stops));
    }

    linear_gradient_paint::linear_gradient_paint(maui::graphics::point start_point, maui::graphics::point end_point)
        : start_point_(start_point), end_point_(end_point)
    {
    }

    linear_gradient_paint::linear_gradient_paint(std::vector<gradient_stop> gradient_stops,
                                                 maui::graphics::point start_point, maui::graphics::point end_point)
        : start_point_(start_point), end_point_(end_point)
    {
        set_gradient_stops(std::move(gradient_stops));
    }

    maui::graphics::point linear_gradient_paint::start_point() const
    {
        return start_point_;
    }

    void linear_gradient_paint::set_start_point(maui::graphics::point value)
    {
        start_point_ = value;
    }

    maui::graphics::point linear_gradient_paint::end_point() const
    {
        return end_point_;
    }

    void linear_gradient_paint::set_end_point(maui::graphics::point value)
    {
        end_point_ = value;
    }
} // namespace maui::graphics

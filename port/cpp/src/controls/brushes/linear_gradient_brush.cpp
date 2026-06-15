// maui::controls::linear_gradient_brush out-of-line definitions (header: brushes/linear_gradient_brush.hpp).

#include "maui/controls/brushes/linear_gradient_brush.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::graphics::point>& linear_gradient_brush::start_point_property()
    {
        // C# LinearGradientBrush.StartPointProperty — default (0,0).
        static const maui::core::bindable_property<maui::graphics::point> prop{"StartPoint",
                                                                               maui::graphics::point{0.0, 0.0}};
        return prop;
    }

    const maui::core::bindable_property<maui::graphics::point>& linear_gradient_brush::end_point_property()
    {
        // C# LinearGradientBrush.EndPointProperty — default (1,1).
        static const maui::core::bindable_property<maui::graphics::point> prop{"EndPoint",
                                                                               maui::graphics::point{1.0, 1.0}};
        return prop;
    }

    linear_gradient_brush::linear_gradient_brush() = default;

    linear_gradient_brush::linear_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops)
    {
        set_gradient_stops(std::move(gradient_stops));
    }

    linear_gradient_brush::linear_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops,
                                                 maui::graphics::point start_point, maui::graphics::point end_point)
    {
        set_gradient_stops(std::move(gradient_stops));
        start_point_.set(start_point);
        end_point_.set(end_point);
    }
} // namespace maui::controls

// maui::controls::radial_gradient_brush out-of-line definitions (header: brushes/radial_gradient_brush.hpp).

#include "maui/controls/brushes/radial_gradient_brush.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::graphics::point>& radial_gradient_brush::center_property()
    {
        // C# RadialGradientBrush.CenterProperty — default (0.5,0.5).
        static const maui::core::bindable_property<maui::graphics::point> prop{"Center",
                                                                               maui::graphics::point{0.5, 0.5}};
        return prop;
    }

    const maui::core::bindable_property<double>& radial_gradient_brush::radius_property()
    {
        // C# RadialGradientBrush.RadiusProperty — default 0.5.
        static const maui::core::bindable_property<double> prop{"Radius", 0.5};
        return prop;
    }

    radial_gradient_brush::radial_gradient_brush() = default;

    radial_gradient_brush::radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops)
    {
        set_gradient_stops(std::move(gradient_stops));
    }

    radial_gradient_brush::radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops,
                                                 double radius)
    {
        set_gradient_stops(std::move(gradient_stops));
        radius_.set(radius);
    }

    radial_gradient_brush::radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops,
                                                 maui::graphics::point center, double radius)
    {
        set_gradient_stops(std::move(gradient_stops));
        center_.set(center);
        radius_.set(radius);
    }
} // namespace maui::controls

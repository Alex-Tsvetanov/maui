#pragma once
// maui::controls::radial_gradient_brush  <=  Microsoft.Maui.Controls.RadialGradientBrush
//
// A gradient_brush painting outward from Center over Radius. Ported from
// src/Controls/src/Core/RadialGradientBrush.cs: the base stop machinery plus Center (default (0.5,0.5))
// and Radius (default 0.5), both bindable. The collection / (collection, radius) / (collection, center,
// radius) ctors seed the stops + the relevant fields.
//
// Center uses maui::graphics::point (double); radius is a double (matching C#).
//
// Out-of-line definitions live in radial_gradient_brush.cpp.

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    class radial_gradient_brush : public gradient_brush
    {
    public:
        // C# RadialGradientBrush() — empty stops, center (0.5,0.5), radius 0.5.
        radial_gradient_brush();
        // C# RadialGradientBrush(GradientStopCollection).
        explicit radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops);
        // C# RadialGradientBrush(GradientStopCollection, double radius).
        radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops, double radius);
        // C# RadialGradientBrush(GradientStopCollection, Point center, double radius).
        radial_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops, maui::graphics::point center,
                              double radius);

        static const maui::core::bindable_property<maui::graphics::point>& center_property();
        static const maui::core::bindable_property<double>& radius_property();

        // C# RadialGradientBrush.Center — the gradient's center (relative coords). Bindable.
        [[nodiscard]] maui::graphics::point center() const
        {
            return center_.get();
        }
        void set_center(maui::graphics::point value)
        {
            center_.set(value);
        }

        // C# RadialGradientBrush.Radius — the gradient's radius (relative units). Bindable.
        [[nodiscard]] double radius() const
        {
            return radius_.get();
        }
        void set_radius(double value)
        {
            radius_.set(value);
        }

    private:
        maui::core::property<maui::graphics::point> center_{*this, center_property()};
        maui::core::property<double> radius_{*this, radius_property()};
    };
} // namespace maui::controls

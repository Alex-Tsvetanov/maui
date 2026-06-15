#pragma once
// maui::controls::linear_gradient_brush  <=  Microsoft.Maui.Controls.LinearGradientBrush
//
// A gradient_brush painting along a line from StartPoint to EndPoint. Ported from
// src/Controls/src/Core/LinearGradientBrush.cs: the base stop machinery plus StartPoint (default (0,0))
// and EndPoint (default (1,1)), both bindable. The collection ctor seeds GradientStops; the
// (collection, start, end) ctor also sets the points.
//
// Points use maui::graphics::point (double), the port of C#'s Point.
//
// Out-of-line definitions live in linear_gradient_brush.cpp.

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
    class linear_gradient_brush : public gradient_brush
    {
    public:
        // C# LinearGradientBrush() — empty stops, start (0,0), end (1,1).
        linear_gradient_brush();
        // C# LinearGradientBrush(GradientStopCollection).
        explicit linear_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops);
        // C# LinearGradientBrush(GradientStopCollection, Point startPoint, Point endPoint).
        linear_gradient_brush(std::vector<std::shared_ptr<gradient_stop>> gradient_stops,
                              maui::graphics::point start_point, maui::graphics::point end_point);

        static const maui::core::bindable_property<maui::graphics::point>& start_point_property();
        static const maui::core::bindable_property<maui::graphics::point>& end_point_property();

        // C# LinearGradientBrush.StartPoint — the gradient line's start (relative coords). Bindable.
        [[nodiscard]] maui::graphics::point start_point() const
        {
            return start_point_.get();
        }
        void set_start_point(maui::graphics::point value)
        {
            start_point_.set(value);
        }

        // C# LinearGradientBrush.EndPoint — the gradient line's end (relative coords). Bindable.
        [[nodiscard]] maui::graphics::point end_point() const
        {
            return end_point_.get();
        }
        void set_end_point(maui::graphics::point value)
        {
            end_point_.set(value);
        }

    private:
        maui::core::property<maui::graphics::point> start_point_{*this, start_point_property()};
        maui::core::property<maui::graphics::point> end_point_{*this, end_point_property()};
    };
} // namespace maui::controls

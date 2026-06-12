#pragma once
// maui::controls::shapes::polygon  <=  Microsoft.Maui.Controls.Shapes.Polygon
//
// A shape drawing a CLOSED polygon from connected lines. Ported from Polygon.cs: Points +
// FillRule (default EvenOdd); GetPath is the polyline walk plus the Close. The "points"/"fill_rule"
// keys ride shape_view_handler's absorbed sub-handler table (PolygonHandler — the fill rule reaches
// the drawable through the i_shape_view fill_winding() port extension).

#include <utility>
#include <vector>

#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/path_segment.hpp" // point_collection
#include "maui/controls/shapes/shape.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::controls::shapes
{
    class polygon final : public shape
    {
    public:
        polygon()
        {
            this->set_style_target_type<polygon>();
        }
        // C# Polygon(PointCollection points).
        explicit polygon(point_collection points) : polygon()
        {
            set_points(std::move(points));
        }

        static const maui::core::bindable_property<point_collection>& points_property();
        static const maui::core::bindable_property<shapes::fill_rule>& fill_rule_property();

        [[nodiscard]] const point_collection& points() const
        {
            return points_.get();
        }
        void set_points(point_collection value)
        {
            points_.set(std::move(value));
        }
        [[nodiscard]] shapes::fill_rule fill_rule() const
        {
            return fill_rule_.get();
        }
        void set_fill_rule(shapes::fill_rule value)
        {
            fill_rule_.set(value);
        }

        // The C# PolygonHandler.MapFillRule push, surfaced through the contract (i_shape_view.hpp).
        [[nodiscard]] maui::graphics::winding_mode fill_winding() const override
        {
            return fill_rule() == shapes::fill_rule::even_odd ? maui::graphics::winding_mode::even_odd
                                                              : maui::graphics::winding_mode::non_zero;
        }

        [[nodiscard]] maui::graphics::path_f get_path() const override
        {
            maui::graphics::path_f path;
            const point_collection& pts = points();
            if (!pts.empty())
            {
                path.move_to(static_cast<float>(pts[0].x), static_cast<float>(pts[0].y));
                for (std::size_t index = 1; index < pts.size(); index++)
                {
                    path.line_to(static_cast<float>(pts[index].x), static_cast<float>(pts[index].y));
                }
                path.close();
            }
            return path;
        }

    private:
        maui::core::property<point_collection> points_{*this, points_property()};
        maui::core::property<shapes::fill_rule> fill_rule_{*this, fill_rule_property()};
    };
} // namespace maui::controls::shapes

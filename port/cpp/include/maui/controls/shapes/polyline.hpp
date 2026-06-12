#pragma once
// maui::controls::shapes::polyline  <=  Microsoft.Maui.Controls.Shapes.Polyline
//
// A shape drawing connected straight lines (not auto-closed, unlike polygon). Ported from
// Polyline.cs: Points (PointCollection — the plain-vector collapse) + FillRule (default EvenOdd);
// GetPath moves to the first point and lines through the rest. The "points"/"fill_rule" keys ride
// shape_view_handler's absorbed sub-handler table (PolylineHandler.MapPoints/MapFillRule — the fill
// rule reaches the drawable through the i_shape_view fill_winding() port extension). Polyline is one
// of the C# margin-adding measure types.

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
    class polyline final : public shape
    {
    public:
        polyline()
        {
            this->set_style_target_type<polyline>();
        }
        // C# Polyline(PointCollection points).
        explicit polyline(point_collection points) : polyline()
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

        // The C# PolylineHandler.MapFillRule push, surfaced through the contract (i_shape_view.hpp).
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
            }
            return path;
        }

    protected:
        [[nodiscard]] bool adds_margin_to_measure() const override
        {
            return true;
        }

    private:
        maui::core::property<point_collection> points_{*this, points_property()};
        maui::core::property<shapes::fill_rule> fill_rule_{*this, fill_rule_property()};
    };
} // namespace maui::controls::shapes

// maui::controls::shapes::path_geometry — out-of-line definitions: the AppendPath figure walk with
// the per-segment dispatch (PathGeometry.cs's AddArc/AddBezier/AddLine/AddPolyBezier/AddPolyLine/
// AddPolyQuad/AddQuad). The C# `is` type tests become dynamic_casts over the segment family.

#include "maui/controls/shapes/path_geometry.hpp"

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/controls/shapes/geometry_helper.hpp"
#include "maui/controls/shapes/path_segment.hpp"
#include "maui/controls/shapes/sweep_direction.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"

namespace maui::controls::shapes
{
    namespace
    {
        // C# PathGeometry.AddArc: flatten the elliptical arc from the path's last point (tolerance 1)
        // and emit the polyline.
        void add_arc(maui::graphics::path_f& path, const arc_segment& segment)
        {
            std::vector<maui::graphics::point> points;
            const maui::graphics::point_f last = path.last_point();
            flatten_arc(points, {last.x, last.y}, segment.point(), segment.size().width, segment.size().height,
                        segment.rotation_angle(), segment.is_large_arc(),
                        segment.sweep_direction() == sweep_direction::counter_clockwise, 1);

            for (const maui::graphics::point& pt : points)
            {
                path.line_to(static_cast<float>(pt.x), static_cast<float>(pt.y));
            }
        }

        // C# PathGeometry.AddPolyBezier: three points per cubic curve, stopping short of a partial set.
        void add_poly_bezier(maui::graphics::path_f& path, const poly_bezier_segment& segment)
        {
            const point_collection& points = segment.points();
            for (std::size_t bez = 0; bez + 2 < points.size(); bez += 3)
            {
                path.curve_to(static_cast<float>(points[bez].x), static_cast<float>(points[bez].y),
                              static_cast<float>(points[bez + 1].x), static_cast<float>(points[bez + 1].y),
                              static_cast<float>(points[bez + 2].x), static_cast<float>(points[bez + 2].y));
            }
        }

        // C# PathGeometry.AddPolyQuad: two points per quadratic curve, stopping short of a partial set.
        void add_poly_quad(maui::graphics::path_f& path, const poly_quadratic_bezier_segment& segment)
        {
            const point_collection& points = segment.points();
            if (points.size() < 2)
            {
                return;
            }
            for (std::size_t i = 0; i + 1 < points.size(); i += 2)
            {
                path.quad_to(static_cast<float>(points[i].x), static_cast<float>(points[i].y),
                             static_cast<float>(points[i + 1].x), static_cast<float>(points[i + 1].y));
            }
        }
    } // namespace

    void path_geometry::append_path(maui::graphics::path_f& path) const
    {
        for (const std::shared_ptr<path_figure>& figure : figures_)
        {
            if (figure == nullptr)
            {
                continue;
            }

            path.move_to(static_cast<float>(figure->start_point().x), static_cast<float>(figure->start_point().y));

            for (const std::shared_ptr<path_segment>& segment : figure->segments())
            {
                if (const auto* arc = dynamic_cast<const arc_segment*>(segment.get()))
                {
                    add_arc(path, *arc);
                }
                else if (const auto* bezier = dynamic_cast<const bezier_segment*>(segment.get()))
                {
                    path.curve_to(static_cast<float>(bezier->point1().x), static_cast<float>(bezier->point1().y),
                                  static_cast<float>(bezier->point2().x), static_cast<float>(bezier->point2().y),
                                  static_cast<float>(bezier->point3().x), static_cast<float>(bezier->point3().y));
                }
                else if (const auto* line = dynamic_cast<const line_segment*>(segment.get()))
                {
                    path.line_to(static_cast<float>(line->point().x), static_cast<float>(line->point().y));
                }
                else if (const auto* poly_bezier = dynamic_cast<const poly_bezier_segment*>(segment.get()))
                {
                    add_poly_bezier(path, *poly_bezier);
                }
                else if (const auto* poly_line = dynamic_cast<const poly_line_segment*>(segment.get()))
                {
                    for (const maui::graphics::point& pt : poly_line->points())
                    {
                        path.line_to(static_cast<float>(pt.x), static_cast<float>(pt.y));
                    }
                }
                else if (const auto* poly_quad = dynamic_cast<const poly_quadratic_bezier_segment*>(segment.get()))
                {
                    add_poly_quad(path, *poly_quad);
                }
                else if (const auto* quad = dynamic_cast<const quadratic_bezier_segment*>(segment.get()))
                {
                    path.quad_to(static_cast<float>(quad->point1().x), static_cast<float>(quad->point1().y),
                                 static_cast<float>(quad->point2().x), static_cast<float>(quad->point2().y));
                }
            }

            if (figure->is_closed())
            {
                path.close();
            }
        }
    }
} // namespace maui::controls::shapes

#pragma once
// maui::controls::shapes::path_segment (+ the seven concrete segments)  <=
//   Microsoft.Maui.Controls.Shapes.{PathSegment, LineSegment, PolyLineSegment, BezierSegment,
//   PolyBezierSegment, QuadraticBezierSegment, PolyQuadraticBezierSegment, ArcSegment}
//   (+ PointCollection.cs / PathSegmentCollection.cs — the vectors)
//
// The segment family a path_figure strings together. A deliberate tight cluster in one header (the
// recording_canvas op-struct precedent): the seven leaves are pure data carriers only ever consumed
// by path_geometry's append-path dispatch and the path markup parser. Ported member for member from
// the C# files; the BindableObject/IAnimatable machinery is collapsed to plain members (the
// geometry.hpp port collapse — BatchBegin/BatchCommit are empty in C# too).
//
// Ownership: a figure owns its segments via shared_ptr (path_segment_collection).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/shapes/sweep_direction.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::shapes
{
    // Microsoft.Maui.Controls.PointCollection (ObservableCollection<Point> collapsed to a vector).
    using point_collection = std::vector<maui::graphics::point>;

    class path_segment
    {
    public:
        path_segment() = default;
        virtual ~path_segment() = default;
        path_segment(const path_segment&) = default;
        path_segment(path_segment&&) = default;
        path_segment& operator=(const path_segment&) = default;
        path_segment& operator=(path_segment&&) = default;
    };

    // Microsoft.Maui.Controls.Shapes.PathSegmentCollection.
    using path_segment_collection = std::vector<std::shared_ptr<path_segment>>;

    // C# LineSegment — a straight line to `point`.
    class line_segment final : public path_segment
    {
    public:
        line_segment() = default;
        explicit line_segment(const maui::graphics::point& point) : point_(point)
        {
        }

        [[nodiscard]] const maui::graphics::point& point() const
        {
            return point_;
        }
        void set_point(const maui::graphics::point& value)
        {
            point_ = value;
        }

    private:
        maui::graphics::point point_;
    };

    // C# PolyLineSegment — connected straight lines through `points`.
    class poly_line_segment final : public path_segment
    {
    public:
        poly_line_segment() = default;
        explicit poly_line_segment(point_collection points) : points_(std::move(points))
        {
        }

        [[nodiscard]] const point_collection& points() const
        {
            return points_;
        }
        [[nodiscard]] point_collection& points()
        {
            return points_;
        }
        void set_points(point_collection value)
        {
            points_ = std::move(value);
        }

    private:
        point_collection points_;
    };

    // C# BezierSegment — a cubic Bezier (two control points + the end point).
    class bezier_segment final : public path_segment
    {
    public:
        bezier_segment() = default;
        bezier_segment(const maui::graphics::point& point1, const maui::graphics::point& point2,
                       const maui::graphics::point& point3)
            : point1_(point1), point2_(point2), point3_(point3)
        {
        }

        [[nodiscard]] const maui::graphics::point& point1() const
        {
            return point1_;
        }
        void set_point1(const maui::graphics::point& value)
        {
            point1_ = value;
        }
        [[nodiscard]] const maui::graphics::point& point2() const
        {
            return point2_;
        }
        void set_point2(const maui::graphics::point& value)
        {
            point2_ = value;
        }
        [[nodiscard]] const maui::graphics::point& point3() const
        {
            return point3_;
        }
        void set_point3(const maui::graphics::point& value)
        {
            point3_ = value;
        }

    private:
        maui::graphics::point point1_;
        maui::graphics::point point2_;
        maui::graphics::point point3_;
    };

    // C# PolyBezierSegment — connected cubic Beziers, three points per curve.
    class poly_bezier_segment final : public path_segment
    {
    public:
        poly_bezier_segment() = default;
        explicit poly_bezier_segment(point_collection points) : points_(std::move(points))
        {
        }

        [[nodiscard]] const point_collection& points() const
        {
            return points_;
        }
        [[nodiscard]] point_collection& points()
        {
            return points_;
        }
        void set_points(point_collection value)
        {
            points_ = std::move(value);
        }

    private:
        point_collection points_;
    };

    // C# QuadraticBezierSegment — a quadratic Bezier (control point + end point).
    class quadratic_bezier_segment final : public path_segment
    {
    public:
        quadratic_bezier_segment() = default;
        quadratic_bezier_segment(const maui::graphics::point& point1, const maui::graphics::point& point2)
            : point1_(point1), point2_(point2)
        {
        }

        [[nodiscard]] const maui::graphics::point& point1() const
        {
            return point1_;
        }
        void set_point1(const maui::graphics::point& value)
        {
            point1_ = value;
        }
        [[nodiscard]] const maui::graphics::point& point2() const
        {
            return point2_;
        }
        void set_point2(const maui::graphics::point& value)
        {
            point2_ = value;
        }

    private:
        maui::graphics::point point1_;
        maui::graphics::point point2_;
    };

    // C# PolyQuadraticBezierSegment — connected quadratic Beziers, two points per curve.
    class poly_quadratic_bezier_segment final : public path_segment
    {
    public:
        poly_quadratic_bezier_segment() = default;
        explicit poly_quadratic_bezier_segment(point_collection points) : points_(std::move(points))
        {
        }

        [[nodiscard]] const point_collection& points() const
        {
            return points_;
        }
        [[nodiscard]] point_collection& points()
        {
            return points_;
        }
        void set_points(point_collection value)
        {
            points_ = std::move(value);
        }

    private:
        point_collection points_;
    };

    // C# ArcSegment — an elliptical arc to `point` with the given radii/rotation/direction/size flag.
    class arc_segment final : public path_segment
    {
    public:
        arc_segment() = default;
        arc_segment(const maui::graphics::point& point, const maui::graphics::size& size, double rotation_angle,
                    shapes::sweep_direction sweep_direction, bool is_large_arc)
            : point_(point), size_(size), rotation_angle_(rotation_angle), sweep_direction_(sweep_direction),
              is_large_arc_(is_large_arc)
        {
        }

        [[nodiscard]] const maui::graphics::point& point() const
        {
            return point_;
        }
        void set_point(const maui::graphics::point& value)
        {
            point_ = value;
        }
        [[nodiscard]] const maui::graphics::size& size() const
        {
            return size_;
        }
        void set_size(const maui::graphics::size& value)
        {
            size_ = value;
        }
        [[nodiscard]] double rotation_angle() const
        {
            return rotation_angle_;
        }
        void set_rotation_angle(double value)
        {
            rotation_angle_ = value;
        }
        [[nodiscard]] shapes::sweep_direction sweep_direction() const
        {
            return sweep_direction_;
        }
        void set_sweep_direction(shapes::sweep_direction value)
        {
            sweep_direction_ = value;
        }
        [[nodiscard]] bool is_large_arc() const
        {
            return is_large_arc_;
        }
        void set_is_large_arc(bool value)
        {
            is_large_arc_ = value;
        }

    private:
        maui::graphics::point point_;
        maui::graphics::size size_;
        double rotation_angle_ = 0;
        shapes::sweep_direction sweep_direction_ = shapes::sweep_direction::counter_clockwise;
        bool is_large_arc_ = false;
    };
} // namespace maui::controls::shapes

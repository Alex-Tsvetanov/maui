// maui::controls::shapes::geometry_helper — out-of-line definitions. Ported from GeometryHelper.cs
// (the Petzold ArcSegment mathematics + the two Bezier flatteners; see
// http://www.charlespetzold.com/blog/2008/01/Mathematics-of-ArcSegment.html).

#include "maui/controls/shapes/geometry_helper.hpp"

#include <cmath>
#include <numbers>
#include <vector>

#include "maui/controls/shapes/matrix.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    namespace
    {
        // C# Point.Distance(other).
        double distance(const maui::graphics::point& a, const maui::graphics::point& b)
        {
            return std::hypot(b.x - a.x, b.y - a.y);
        }
    } // namespace

    void flatten_cubic_bezier(std::vector<maui::graphics::point>& points, const maui::graphics::point& start,
                              const maui::graphics::point& control1, const maui::graphics::point& control2,
                              const maui::graphics::point& end, double tolerance)
    {
        const int max = static_cast<int>(
            (distance(control1, start) + distance(control2, control1) + distance(end, control2)) / tolerance);

        for (int i = 0; i <= max; i++)
        {
            // C#: t = (double)i / max — for max == 0 the single iteration divides 0 by 0 (NaN), the
            // same IEEE result as C#.
            const double t = static_cast<double>(i) / max;
            const double u = 1 - t;

            const double x = (u * u * u * start.x) + (3 * t * u * u * control1.x) + (3 * t * t * u * control2.x) +
                             (t * t * t * end.x);
            const double y = (u * u * u * start.y) + (3 * t * u * u * control1.y) + (3 * t * t * u * control2.y) +
                             (t * t * t * end.y);

            points.emplace_back(x, y);
        }
    }

    void flatten_quadratic_bezier(std::vector<maui::graphics::point>& points, const maui::graphics::point& start,
                                  const maui::graphics::point& control, const maui::graphics::point& end,
                                  double tolerance)
    {
        const int max = static_cast<int>((distance(control, start) + distance(end, control)) / tolerance);

        for (int i = 0; i <= max; i++)
        {
            const double t = static_cast<double>(i) / max;
            const double u = 1 - t;

            const double x = (u * u * start.x) + (2 * t * u * control.x) + (t * t * end.x);
            const double y = (u * u * start.y) + (2 * t * u * control.y) + (t * t * end.y);

            points.emplace_back(x, y);
        }
    }

    void flatten_arc(std::vector<maui::graphics::point>& points, const maui::graphics::point& point1,
                     const maui::graphics::point& point2, double radius_x, double radius_y, double angle_rotation,
                     bool is_large_arc, bool is_counterclockwise, double tolerance)
    {
        // Adjust for different radii and rotation angle.
        matrix matx;
        matx.rotate(-angle_rotation);
        matx.scale(radius_y / radius_x, 1);
        const maui::graphics::point pt1 = matx.transform(point1);
        const maui::graphics::point pt2 = matx.transform(point2);

        // Get info about the chord that connects both points.
        const maui::graphics::point mid_point{(pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2};
        const maui::graphics::point vect{pt2.x - pt1.x, pt2.y - pt1.y};
        const double vect_length = std::sqrt((vect.x * vect.x) + (vect.y * vect.y));
        const double half_chord = vect_length / 2;

        // Get the vector from the chord to the center.
        maui::graphics::point vect_rotated;
        if (is_large_arc == is_counterclockwise)
        {
            vect_rotated = {-vect.y, vect.x};
        }
        else
        {
            vect_rotated = {vect.y, -vect.x};
        }

        // Normalize vect_rotated.
        const double vect_rotated_length =
            std::sqrt((vect_rotated.x * vect_rotated.x) + (vect_rotated.y * vect_rotated.y));
        if (vect_rotated_length != 0)
        {
            vect_rotated = {vect_rotated.x / vect_rotated_length, vect_rotated.y / vect_rotated_length};
        }
        else
        {
            vect_rotated = {};
        }

        // Distance from the chord to the center, then the center point itself.
        const double center_distance = std::sqrt(std::abs((radius_y * radius_y) - (half_chord * half_chord)));
        const maui::graphics::point center{(center_distance * vect_rotated.x) + mid_point.x,
                                           (center_distance * vect_rotated.y) + mid_point.y};

        // Angles from the center to the two points.
        double angle1 = std::atan2(pt1.y - center.y, pt1.x - center.x);
        double angle2 = std::atan2(pt2.y - center.y, pt2.x - center.x);

        const double sweep = std::abs(angle2 - angle1);
        bool reverse_arc = false;

        // C#: Math.IEEERemainder(sweep + 0.000005, Math.PI) < 0.000010 — std::remainder is the same
        // IEEE 754 remainder operation.
        if (std::remainder(sweep + 0.000005, std::numbers::pi) < 0.000010)
        {
            reverse_arc = is_counterclockwise == (angle1 < angle2);
        }
        else
        {
            const bool is_acute = sweep < std::numbers::pi;
            reverse_arc = is_large_arc == is_acute;
        }

        if (reverse_arc)
        {
            if (angle1 < angle2)
            {
                angle1 += 2 * std::numbers::pi;
            }
            else
            {
                angle2 += 2 * std::numbers::pi;
            }
        }

        // Invert the matrix for the final point calculation.
        matx.invert();

        // The number of points for the polyline approximation.
        const int max = static_cast<int>(4 * (radius_x + radius_y) * std::abs(angle2 - angle1) /
                                         (2 * std::numbers::pi) / tolerance);

        for (int i = 0; i <= max; i++)
        {
            const double angle = (((max - i) * angle1) + (i * angle2)) / max;
            const double x = center.x + (radius_y * std::cos(angle));
            const double y = center.y + (radius_y * std::sin(angle));

            points.push_back(matx.transform({x, y}));
        }
    }
} // namespace maui::controls::shapes

// maui::graphics::path_f  <=  Microsoft.Maui.Graphics.PathF (+ ArcFlattener.cs, GeometryUtil helpers)
#include "maui/graphics/path_f.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/vector2.hpp"

namespace maui::graphics
{
    namespace
    {

        constexpr float k_ratio = 0.551784777779014F; // cubic Bezier quarter-circle ratio
        constexpr float k_epsilon = 0.0000000001F;    // GeometryUtil.Epsilon
        constexpr double k_pi = std::numbers::pi;

        double degrees_to_radians(double a)
        {
            return a * (k_pi / 180.0);
        }
        double get_distance(double x1, double y1, double x2, double y2)
        {
            double const dx = x2 - x1;
            double const dy = y2 - y1;
            return std::sqrt((dx * dx) + (dy * dy));
        }

        // GeometryUtil.RotatePoint(center, point, angle)
        point_f rotate_point(const point_f& center, const point_f& p, float angle)
        {
            double const r = degrees_to_radians(angle);
            float const x =
                center.x + static_cast<float>((std::cos(r) * (p.x - center.x)) - (std::sin(r) * (p.y - center.y)));
            float const y =
                center.y + static_cast<float>((std::sin(r) * (p.x - center.x)) + (std::cos(r) * (p.y - center.y)));
            return {x, y};
        }

        // stand-in for System.Numerics.Vector2 in the Bezier-flattening math
        struct vec2
        {
            float x = 0;
            float y = 0;
            vec2() = default;
            vec2(float x_, float y_) : x(x_), y(y_)
            {
            }
            vec2 operator+(vec2 o) const
            {
                return {x + o.x, y + o.y};
            }
            vec2 operator-(vec2 o) const
            {
                return {x - o.x, y - o.y};
            }
            vec2 operator*(float s) const
            {
                return {x * s, y * s};
            }
            [[nodiscard]] float length() const
            {
                return std::sqrt((x * x) + (y * y));
            }
        };
        vec2 operator*(float s, vec2 v)
        {
            return {v.x * s, v.y * s};
        }
        vec2 as_vec(const std::vector<point_f>& pts, int i)
        {
            return {pts[i].x, pts[i].y};
        }

        vec2 de_casteljau(const std::vector<point_f>& c, int index, float t)
        {
            float const s = 1.0F - t;
            vec2 v0 = s * as_vec(c, index) + t * as_vec(c, index + 1);
            vec2 v1 = s * as_vec(c, index + 1) + t * as_vec(c, index + 2);
            vec2 const v2 = s * as_vec(c, index + 2) + t * as_vec(c, index + 3);
            v0 = s * v0 + t * v1;
            v1 = s * v1 + t * v2;
            return s * v0 + t * v1;
        }

        void flatten_cubic_segment(int index, double flatness, const std::vector<point_f>& curve,
                                   std::vector<point_f>& out)
        {
            std::array<vec2, 4> vectors{};
            double r_curve = 0;
            for (int i = index + 1; i <= index + 2; i++)
            {
                vectors[0] = (as_vec(curve, i - 1) + as_vec(curve, i + 1)) * 0.5F - as_vec(curve, i);
                double const r = vectors[0].length();
                r_curve = std::max(r, r_curve);
            }
            if (r_curve <= 0.5 * flatness)
            {
                vec2 const v = as_vec(curve, index + 3);
                out.emplace_back(v.x, v.y);
                return;
            }
            int n = static_cast<int>(std::sqrt(r_curve / flatness)) + 3;
            n = std::min(n, 1000);
            float const d = 1.0F / static_cast<float>(n);
            vectors[0] = as_vec(curve, index);
            for (int i = 1; i <= 3; i++)
            {
                vectors.at(i) = de_casteljau(curve, index, static_cast<float>(i) * d);
                out.emplace_back(vectors.at(i).x, vectors.at(i).y);
            }
            for (int i = 1; i <= 3; i++)
            {
                for (int k = 0; k <= 3 - i; k++)
                {
                    vectors.at(k) = vectors.at(k + 1) - vectors.at(k);
                }
            }
            for (int i = 4; i <= n; i++)
            {
                for (int k = 1; k <= 3; k++)
                {
                    vectors.at(k) = vectors.at(k) + vectors.at(k - 1);
                }
                out.emplace_back(vectors[3].x, vectors[3].y);
            }
        }

        // ArcFlattener.cs
        struct arc_flattener
        {
            float cx, cy, diameter, radius, fx, fy, sweep, start_angle;
            point_f start_point;

            arc_flattener(float x, float y, float width, float height, float sa, float ea, bool clockwise)
                : cx(x + (width / 2)), cy(y + (height / 2)), diameter(std::max(width, height)), radius(diameter / 2),
                  fx(width / diameter), fy(height / diameter),
                  sweep(clockwise ? -std::abs(ea - sa) : std::abs(ea - sa)), start_angle(sa),
                  start_point(point_on_arc(0))
            {
            }

            [[nodiscard]] point_f point_on_arc(float percentage) const
            {
                float angle = start_angle + (sweep * percentage);
                while (angle >= 360)
                {
                    angle -= 360;
                }
                angle *= -1;
                double const radians = degrees_to_radians(angle);
                auto const px = static_cast<float>(std::cos(radians) * radius);
                auto const py = static_cast<float>(std::sin(radians) * radius);
                return {cx + (px * fx), cy + (py * fy)};
            }
            static point_f center_of(point_f a, point_f b)
            {
                return {(a.x + b.x) / 2, (a.y + b.y) / 2};
            }
            [[nodiscard]] path_f create_flattened_path(float flatness = 0.5F) const
            {
                bool found = false;
                int n = 1;
                std::optional<point_f> end_point;
                while (!found && n < 1024)
                {
                    float const candidate = 1.0F / static_cast<float>(n);
                    point_f const mid_on_arc = point_on_arc(candidate / 2);
                    if (!end_point)
                    {
                        end_point = point_on_arc(candidate);
                    }
                    point_f const mid_on_line = center_of(start_point, *end_point);
                    if (get_distance(mid_on_arc.x, mid_on_arc.y, mid_on_line.x, mid_on_line.y) <= flatness)
                    {
                        found = true;
                        n = n << 1;
                    }
                    else
                    {
                        end_point = mid_on_arc;
                        n++;
                    }
                }
                path_f path;
                path.move_to(start_point);
                float const step = 1.0F / static_cast<float>(n);
                float percentage = 0;
                for (int i = 1; i < n; i++)
                {
                    percentage += step;
                    path.line_to(point_on_arc(percentage));
                }
                path.line_to(point_on_arc(1));
                return path;
            }
        };

    } // namespace

    // ---- ctors ----
    path_f::path_f(float x, float y)
    {
        move_to(x, y);
    }
    path_f::path_f(const point_f& pt)
    {
        move_to(pt.x, pt.y);
    }

    path_f::path_f(std::vector<point_f> points, std::vector<float> arc_angles, std::vector<bool> arc_clockwise,
                   std::vector<path_operation> operations, int sub_path_count)
        : points_(std::move(points)), operations_(std::move(operations)), arc_angles_(std::move(arc_angles)),
          arc_clockwise_(std::move(arc_clockwise)), sub_path_count_(sub_path_count)
    {
        int sub_path_index = 0;
        for (auto op : operations_)
        {
            if (op == path_operation::move)
            {
                sub_path_index++;
                sub_paths_closed_.push_back(false);
            }
            else if (op == path_operation::close)
            {
                sub_paths_closed_.erase(sub_paths_closed_.begin() + (sub_path_index - 1));
                sub_paths_closed_.push_back(true);
            }
        }
    }

    // ---- building ----
    path_f& path_f::move_to(float x, float y)
    {
        return move_to(point_f(x, y));
    }
    path_f& path_f::move_to(const point_f& point)
    {
        sub_path_count_++;
        sub_paths_closed_.push_back(false);
        points_.push_back(point);
        operations_.push_back(path_operation::move);
        invalidate();
        return *this;
    }

    bool path_f::closed() const
    {
        if (!operations_.empty())
        {
            return operations_.back() == path_operation::close;
        }
        return false;
    }

    void path_f::close()
    {
        if (!closed())
        {
            sub_paths_closed_.erase(sub_paths_closed_.begin() + (sub_path_count_ - 1));
            sub_paths_closed_.push_back(true);
            operations_.push_back(path_operation::close);
        }
        invalidate();
    }

    void path_f::open()
    {
        if (!operations_.empty() && operations_.back() == path_operation::close)
        {
            sub_paths_closed_.erase(sub_paths_closed_.begin() + (sub_path_count_ - 1));
            sub_paths_closed_.push_back(false);
            operations_.pop_back();
        }
        invalidate();
    }

    path_f& path_f::line_to(float x, float y)
    {
        return line_to(point_f(x, y));
    }
    path_f& path_f::line_to(const point_f& point)
    {
        if (points_.empty())
        {
            points_.push_back(point);
            sub_path_count_++;
            sub_paths_closed_.push_back(false);
            operations_.push_back(path_operation::move);
        }
        else
        {
            points_.push_back(point);
            operations_.push_back(path_operation::line);
        }
        invalidate();
        return *this;
    }

    path_f& path_f::insert_line_to(const point_f& point, int index)
    {
        if (index == 0)
        {
            index = 1;
        }
        if (index == operation_count())
        {
            line_to(point);
        }
        else
        {
            int const point_index = get_segment_point_index(index);
            points_.insert(points_.begin() + point_index, point);
            operations_.insert(operations_.begin() + index, path_operation::line);
            invalidate();
        }
        return *this;
    }

    path_f& path_f::add_arc(float x1, float y1, float x2, float y2, float start_angle, float end_angle, bool clockwise)
    {
        return add_arc(point_f(x1, y1), point_f(x2, y2), start_angle, end_angle, clockwise);
    }
    path_f& path_f::add_arc(const point_f& top_left, const point_f& bottom_right, float start_angle, float end_angle,
                            bool clockwise)
    {
        if (count() == 0 || operation_count() == 0 || get_segment_type(operation_count() - 1) == path_operation::close)
        {
            sub_path_count_++;
            sub_paths_closed_.push_back(false);
        }
        points_.push_back(top_left);
        points_.push_back(bottom_right);
        arc_angles_.push_back(start_angle);
        arc_angles_.push_back(end_angle);
        arc_clockwise_.push_back(clockwise);
        operations_.push_back(path_operation::arc);
        invalidate();
        return *this;
    }

    path_f& path_f::quad_to(float cx, float cy, float x, float y)
    {
        return quad_to(point_f(cx, cy), point_f(x, y));
    }
    path_f& path_f::quad_to(const point_f& control_point, const point_f& point)
    {
        points_.push_back(control_point);
        points_.push_back(point);
        operations_.push_back(path_operation::quad);
        invalidate();
        return *this;
    }
    path_f& path_f::insert_quad_to(const point_f& control_point, const point_f& point, int index)
    {
        if (index == 0)
        {
            index = 1;
        }
        if (index == operation_count())
        {
            quad_to(control_point, point);
        }
        else
        {
            int const point_index = get_segment_point_index(index);
            points_.insert(points_.begin() + point_index, point);
            points_.insert(points_.begin() + point_index, control_point);
            operations_.insert(operations_.begin() + index, path_operation::quad);
            invalidate();
        }
        return *this;
    }

    path_f& path_f::curve_to(float c1x, float c1y, float c2x, float c2y, float x, float y)
    {
        return curve_to(point_f(c1x, c1y), point_f(c2x, c2y), point_f(x, y));
    }
    path_f& path_f::curve_to(const point_f& control1, const point_f& control2, const point_f& point)
    {
        points_.push_back(control1);
        points_.push_back(control2);
        points_.push_back(point);
        operations_.push_back(path_operation::cubic);
        invalidate();
        return *this;
    }
    path_f& path_f::insert_curve_to(const point_f& control1, const point_f& control2, const point_f& point, int index)
    {
        if (index == 0)
        {
            index = 1;
        }
        if (index == operation_count())
        {
            curve_to(control1, control2, point);
        }
        else
        {
            int const point_index = get_segment_point_index(index);
            points_.insert(points_.begin() + point_index, point);
            points_.insert(points_.begin() + point_index, control2);
            points_.insert(points_.begin() + point_index, control1);
            operations_.insert(operations_.begin() + index, path_operation::cubic);
            invalidate();
        }
        return *this;
    }

    // ---- accessors ----
    int path_f::sub_path_count() const
    {
        return sub_path_count_;
    }
    int path_f::count() const
    {
        return static_cast<int>(points_.size());
    }
    int path_f::operation_count() const
    {
        return static_cast<int>(operations_.size());
    }
    const std::vector<point_f>& path_f::points() const
    {
        return points_;
    }
    const std::vector<path_operation>& path_f::segment_types() const
    {
        return operations_;
    }
    path_operation path_f::get_segment_type(int index) const
    {
        return operations_[index];
    }
    point_f path_f::first_point() const
    {
        return points_.empty() ? point_f{} : points_.front();
    }
    point_f path_f::last_point() const
    {
        return points_.empty() ? point_f{} : points_.back();
    }
    int path_f::last_point_index() const
    {
        return points_.empty() ? -1 : count() - 1;
    }
    point_f path_f::operator[](int index) const
    {
        if (index < 0 || index >= count())
        {
            return point_f{};
        }
        return points_[index];
    }
    int path_f::segment_count_excluding_open_and_close() const
    {
        int c = operation_count();
        if (c > 0)
        {
            if (operations_.front() == path_operation::move)
            {
                c--;
            }
            if (operations_.back() == path_operation::close)
            {
                c--;
            }
        }
        return c;
    }
    bool path_f::is_sub_path_closed(int sub_path_index) const
    {
        if (sub_path_index >= 0 && sub_path_index < sub_path_count())
        {
            return sub_paths_closed_[sub_path_index];
        }
        return false;
    }
    void path_f::set_point(int index, float x, float y)
    {
        points_[index] = point_f(x, y);
        invalidate();
    }
    void path_f::set_point(int index, const point_f& point)
    {
        points_[index] = point;
        invalidate();
    }
    float path_f::get_arc_angle(int index) const
    {
        return std::cmp_greater(arc_angles_.size(), index) ? arc_angles_[index] : 0;
    }
    void path_f::set_arc_angle(int index, float value)
    {
        if (std::cmp_greater(arc_angles_.size(), index))
        {
            arc_angles_[index] = value;
        }
        invalidate();
    }
    bool path_f::get_arc_clockwise(int index) const
    {
        return std::cmp_greater(arc_clockwise_.size(), index) ? arc_clockwise_[index] : false;
    }
    void path_f::set_arc_clockwise(int index, bool value)
    {
        if (std::cmp_greater(arc_clockwise_.size(), index))
        {
            arc_clockwise_[index] = value;
        }
        invalidate();
    }

    // ---- segment queries ----
    int path_f::get_segment_point_index(int index) const
    {
        if (index <= operation_count())
        {
            int point_index = 0;
            for (int oi = 0; oi < operation_count(); oi++)
            {
                switch (operations_[oi])
                {
                    case path_operation::move:
                    case path_operation::line:
                        if (oi == index)
                        {
                            return point_index;
                        }
                        point_index += 1;
                        break;
                    case path_operation::quad:
                    case path_operation::arc:
                        if (oi == index)
                        {
                            return point_index;
                        }
                        point_index += 2;
                        break;
                    case path_operation::cubic:
                        if (oi == index)
                        {
                            return point_index;
                        }
                        point_index += 3;
                        break;
                    case path_operation::close:
                        if (oi == index)
                        {
                            return point_index;
                        }
                        break;
                }
            }
        }
        return -1;
    }
    int path_f::get_segment_for_point(int point_index) const
    {
        if (point_index < count())
        {
            int index = 0;
            for (int segment = 0; segment < operation_count(); segment++)
            {
                int n = 0;
                switch (operations_[segment])
                {
                    case path_operation::move:
                    case path_operation::line:
                        n = 1;
                        break;
                    case path_operation::quad:
                    case path_operation::arc:
                        n = 2;
                        break;
                    case path_operation::cubic:
                        n = 3;
                        break;
                    case path_operation::close:
                        n = 0;
                        break;
                }
                for (int k = 0; k < n; k++)
                {
                    if (point_index == index++)
                    {
                        return segment;
                    }
                }
            }
        }
        return -1;
    }
    std::vector<point_f> path_f::get_points_for_segment(int segment_index) const
    {
        if (segment_index <= operation_count())
        {
            int pi = 0;
            for (int segment = 0; segment < operation_count(); segment++)
            {
                auto type = operations_[segment];
                int const n = (type == path_operation::move || type == path_operation::line)  ? 1
                              : (type == path_operation::quad || type == path_operation::arc) ? 2
                              : (type == path_operation::cubic)                               ? 3
                                                                                              : 0;
                if (segment == segment_index)
                {
                    std::vector<point_f> out;
                    out.reserve(n);
                    for (int k = 0; k < n; k++)
                    {
                        out.push_back(points_[pi + k]);
                    }
                    return out; // empty for close
                }
                pi += n;
            }
        }
        return {};
    }

    // ---- editing ----
    void path_f::remove_segment(int segment_index)
    {
        if (segment_index > operation_count())
        {
            return;
        }
        invalidate();
        int pi = 0;
        int ai = 0;
        int ci = 0;
        for (int segment = 0; segment < operation_count(); segment++)
        {
            auto type = operations_[segment];
            if (segment == segment_index)
            {
                int const n = (type == path_operation::move || type == path_operation::line)  ? 1
                              : (type == path_operation::quad || type == path_operation::arc) ? 2
                              : (type == path_operation::cubic)                               ? 3
                                                                                              : 0;
                points_.erase(points_.begin() + pi, points_.begin() + pi + n);
                operations_.erase(operations_.begin() + segment_index);
                if (type == path_operation::arc)
                {
                    arc_angles_.erase(arc_angles_.begin() + ai, arc_angles_.begin() + ai + 2);
                    arc_clockwise_.erase(arc_clockwise_.begin() + ci);
                }
                return;
            }
            switch (type)
            {
                case path_operation::move:
                case path_operation::line:
                    pi += 1;
                    break;
                case path_operation::quad:
                    pi += 2;
                    break;
                case path_operation::cubic:
                    pi += 3;
                    break;
                case path_operation::arc:
                    pi += 2;
                    ai += 2;
                    ci += 1;
                    break;
                case path_operation::close:
                    break;
            }
        }
    }
    void path_f::remove_all_segments_after(int segment_index)
    {
        if (segment_index > operation_count())
        {
            invalidate();
            return;
        }
        int pi = 0;
        int ai = 0;
        int ci = 0;
        for (int segment = 0; segment < operation_count(); segment++)
        {
            if (segment == segment_index)
            {
                points_.erase(points_.begin() + pi, points_.end());
                operations_.erase(operations_.begin() + segment, operations_.end());
                arc_angles_.erase(arc_angles_.begin() + ai, arc_angles_.end());
                arc_clockwise_.erase(arc_clockwise_.begin() + ci, arc_clockwise_.end());
                sub_path_count_ = 0;
                sub_paths_closed_.clear();
                for (auto op : operations_)
                {
                    if (op == path_operation::move)
                    {
                        sub_path_count_++;
                        sub_paths_closed_.push_back(false);
                    }
                    else if (op == path_operation::close)
                    {
                        sub_paths_closed_.erase(sub_paths_closed_.begin() + sub_path_count_);
                        sub_paths_closed_.push_back(true);
                    }
                }
                if (sub_path_count_ > 0)
                {
                    sub_path_count_--;
                }
                invalidate();
                return;
            }
            switch (operations_[segment])
            {
                case path_operation::move:
                case path_operation::line:
                    pi += 1;
                    break;
                case path_operation::quad:
                    pi += 2;
                    break;
                case path_operation::cubic:
                    pi += 3;
                    break;
                case path_operation::arc:
                    pi += 2;
                    ai += 2;
                    ci += 1;
                    break;
                case path_operation::close:
                    break;
            }
        }
        invalidate();
    }
    void path_f::move(float x, float y)
    {
        for (auto& p : points_)
        {
            p = p.offset(x, y);
        }
        invalidate();
    }
    void path_f::move_point(int index, float dx, float dy)
    {
        points_[index] = points_[index].offset(dx, dy);
        invalidate();
    }

    // ---- transforms ----
    point_f path_f::get_rotated_point(int point_index, const point_f& pivot, float angle) const
    {
        return rotate_point(pivot, points_[point_index], angle);
    }
    void path_f::transform(const matrix3x2& transform)
    {
        for (auto& p : points_)
        {
            p = point_f(vector2::transform(static_cast<vector2>(p), transform));
        }
        invalidate();
    }
    path_f path_f::rotate(float angle_as_degrees, const point_f& pivot) const
    {
        path_f path;
        int index = 0;
        int arc_index = 0;
        int cw_index = 0;
        for (auto type : operations_)
        {
            switch (type)
            {
                case path_operation::move:
                    path.move_to(get_rotated_point(index++, pivot, angle_as_degrees));
                    break;
                case path_operation::line: {
                    auto p = get_rotated_point(index++, pivot, angle_as_degrees);
                    path.line_to(p.x, p.y);
                    break;
                }
                case path_operation::quad: {
                    auto cp = get_rotated_point(index++, pivot, angle_as_degrees);
                    auto ep = get_rotated_point(index++, pivot, angle_as_degrees);
                    path.quad_to(cp.x, cp.y, ep.x, ep.y);
                    break;
                }
                case path_operation::cubic: {
                    auto c1 = get_rotated_point(index++, pivot, angle_as_degrees);
                    auto c2 = get_rotated_point(index++, pivot, angle_as_degrees);
                    auto ep = get_rotated_point(index++, pivot, angle_as_degrees);
                    path.curve_to(c1.x, c1.y, c2.x, c2.y, ep.x, ep.y);
                    break;
                }
                case path_operation::arc: {
                    auto tl = get_rotated_point(index++, pivot, angle_as_degrees);
                    auto br = get_rotated_point(index++, pivot, angle_as_degrees);
                    float const sa = arc_angles_[arc_index++];
                    float const ea = arc_angles_[arc_index++];
                    bool const cw = arc_clockwise_[cw_index++];
                    path.add_arc(tl, br, sa, ea, cw);
                    break;
                }
                case path_operation::close:
                    path.close();
                    break;
            }
        }
        return path;
    }

    // ---- shapes ----
    float path_f::clamp_corner_radius(float cr, float w, float h)
    {
        cr = std::min(cr, h / 2);
        cr = std::min(cr, w / 2);
        return cr;
    }
    void path_f::append_ellipse(const rect_f& rect)
    {
        append_ellipse(rect.x, rect.y, rect.width, rect.height);
    }
    void path_f::append_ellipse(float x, float y, float w, float h)
    {
        float const min_x = x;
        float const min_y = y;
        float const max_x = x + w;
        float const max_y = y + h;
        float const mid_x = x + (w / 2);
        float const mid_y = y + (h / 2);
        float const off_y = h / 2 * k_ratio;
        float const off_x = w / 2 * k_ratio;
        move_to(point_f(min_x, mid_y));
        curve_to(point_f(min_x, mid_y - off_y), point_f(mid_x - off_x, min_y), point_f(mid_x, min_y));
        curve_to(point_f(mid_x + off_x, min_y), point_f(max_x, mid_y - off_y), point_f(max_x, mid_y));
        curve_to(point_f(max_x, mid_y + off_y), point_f(mid_x + off_x, max_y), point_f(mid_x, max_y));
        curve_to(point_f(mid_x - off_x, max_y), point_f(min_x, mid_y + off_y), point_f(min_x, mid_y));
        close();
    }
    void path_f::append_circle(const point_f& center, float r)
    {
        append_circle(center.x, center.y, r);
    }
    void path_f::append_circle(float cx, float cy, float r)
    {
        float const min_x = cx - r;
        float const min_y = cy - r;
        float const max_x = cx + r;
        float const max_y = cy + r;
        float const mid_x = cx;
        float const mid_y = cy;
        float const off = r * k_ratio;
        move_to(point_f(min_x, mid_y));
        curve_to(point_f(min_x, mid_y - off), point_f(mid_x - off, min_y), point_f(mid_x, min_y));
        curve_to(point_f(mid_x + off, min_y), point_f(max_x, mid_y - off), point_f(max_x, mid_y));
        curve_to(point_f(max_x, mid_y + off), point_f(mid_x + off, max_y), point_f(mid_x, max_y));
        curve_to(point_f(mid_x - off, max_y), point_f(min_x, mid_y + off), point_f(min_x, mid_y));
        close();
    }
    void path_f::append_rectangle(const rect_f& rect, bool include_last)
    {
        append_rectangle(rect.x, rect.y, rect.width, rect.height, include_last);
    }
    void path_f::append_rectangle(float x, float y, float w, float h, bool include_last)
    {
        float const min_x = x;
        float const min_y = y;
        float const max_x = x + w;
        float const max_y = y + h;
        move_to(point_f(min_x, min_y));
        line_to(point_f(max_x, min_y));
        line_to(point_f(max_x, max_y));
        line_to(point_f(min_x, max_y));
        if (include_last)
        {
            line_to(point_f(min_x, min_y));
        }
        close();
    }
    void path_f::append_rounded_rectangle(const rect_f& rect, float corner_radius, bool include_last)
    {
        append_rounded_rectangle(rect.x, rect.y, rect.width, rect.height, corner_radius, include_last);
    }
    void path_f::append_rounded_rectangle(float x, float y, float w, float h, float corner_radius, bool include_last)
    {
        corner_radius = clamp_corner_radius(corner_radius, w, h);
        float const min_x = x;
        float const min_y = y;
        float const max_x = x + w;
        float const max_y = y + h;
        float const handle = corner_radius * k_ratio;
        float const co = corner_radius - handle;
        move_to(point_f(min_x, min_y + corner_radius));
        curve_to(point_f(min_x, min_y + co), point_f(min_x + co, min_y), point_f(min_x + corner_radius, min_y));
        line_to(point_f(max_x - corner_radius, min_y));
        curve_to(point_f(max_x - co, min_y), point_f(max_x, min_y + co), point_f(max_x, min_y + corner_radius));
        line_to(point_f(max_x, max_y - corner_radius));
        curve_to(point_f(max_x, max_y - co), point_f(max_x - co, max_y), point_f(max_x - corner_radius, max_y));
        line_to(point_f(min_x + corner_radius, max_y));
        curve_to(point_f(min_x + co, max_y), point_f(min_x, max_y - co), point_f(min_x, max_y - corner_radius));
        if (include_last)
        {
            line_to(point_f(min_x, min_y + corner_radius));
        }
        close();
    }
    void path_f::append_rounded_rectangle(const rect_f& rect, float tl, float tr, float bl, float br, bool include_last)
    {
        append_rounded_rectangle(rect.x, rect.y, rect.width, rect.height, tl, tr, bl, br, include_last);
    }
    void path_f::append_rounded_rectangle(float x, float y, float w, float h, float tl, float tr, float bl, float br,
                                          bool include_last)
    {
        tl = clamp_corner_radius(tl, w, h);
        tr = clamp_corner_radius(tr, w, h);
        bl = clamp_corner_radius(bl, w, h);
        br = clamp_corner_radius(br, w, h);
        float const min_x = x;
        float const min_y = y;
        float const max_x = x + w;
        float const max_y = y + h;
        float const tlo = tl - (tl * k_ratio);
        float const tro = tr - (tr * k_ratio);
        float const blo = bl - (bl * k_ratio);
        float const bro = br - (br * k_ratio);
        move_to(point_f(min_x, min_y + tl));
        curve_to(point_f(min_x, min_y + tlo), point_f(min_x + tlo, min_y), point_f(min_x + tl, min_y));
        line_to(point_f(max_x - tr, min_y));
        curve_to(point_f(max_x - tro, min_y), point_f(max_x, min_y + tro), point_f(max_x, min_y + tr));
        line_to(point_f(max_x, max_y - br));
        curve_to(point_f(max_x, max_y - bro), point_f(max_x - bro, max_y), point_f(max_x - br, max_y));
        line_to(point_f(min_x + bl, max_y));
        curve_to(point_f(min_x + blo, max_y), point_f(min_x, max_y - blo), point_f(min_x, max_y - bl));
        if (include_last)
        {
            line_to(point_f(min_x, min_y + tl));
        }
        close();
    }
    void path_f::append_rounded_rectangle(const rect_f& rect, float x_corner_radius, float y_corner_radius)
    {
        float const xr = std::min(x_corner_radius, rect.width / 2);
        float const yr = std::min(y_corner_radius, rect.height / 2);
        float const min_x = std::min(rect.x, rect.x + rect.width);
        float const min_y = std::min(rect.y, rect.y + rect.height);
        float const max_x = std::max(rect.x, rect.x + rect.width);
        float const max_y = std::max(rect.y, rect.y + rect.height);
        float const xo = xr - (xr * k_ratio);
        float const yo = yr - (yr * k_ratio);
        move_to(point_f(min_x, min_y + yr));
        curve_to(point_f(min_x, min_y + yo), point_f(min_x + xo, min_y), point_f(min_x + xr, min_y));
        line_to(point_f(max_x - xr, min_y));
        curve_to(point_f(max_x - xo, min_y), point_f(max_x, min_y + yo), point_f(max_x, min_y + yr));
        line_to(point_f(max_x, max_y - yr));
        curve_to(point_f(max_x, max_y - yo), point_f(max_x - xo, max_y), point_f(max_x - xr, max_y));
        line_to(point_f(min_x + xr, max_y));
        curve_to(point_f(min_x + xo, max_y), point_f(min_x, max_y - yo), point_f(min_x, max_y - yr));
        line_to(point_f(min_x, min_y + yr));
    }

    // ---- compose / flatten ----
    std::vector<path_f> path_f::separate() const
    {
        std::vector<path_f> paths;
        path_f* current = nullptr;
        int i = 0;
        int a = 0;
        int c = 0;
        for (auto op : operations_)
        {
            if (op == path_operation::move)
            {
                paths.emplace_back();
                current = &paths.back();
                current->move_to(points_[i++]);
                continue;
            }
            // Every non-move op extends the current sub-path; a well-formed path always opens one
            // with a move first (the C# original would throw NullReferenceException here otherwise).
            if (current == nullptr)
            {
                continue;
            }
            switch (op)
            {
                case path_operation::move:
                    break;
                case path_operation::line:
                    current->line_to(points_[i++]);
                    break;
                case path_operation::quad: {
                    auto cp = points_[i++];
                    current->quad_to(cp, points_[i++]);
                    break;
                }
                case path_operation::cubic: {
                    auto c1 = points_[i++];
                    auto c2 = points_[i++];
                    current->curve_to(c1, c2, points_[i++]);
                    break;
                }
                case path_operation::arc: {
                    auto tl = points_[i++];
                    auto brp = points_[i++];
                    float const sa = arc_angles_[a++];
                    float const ea = arc_angles_[a++];
                    current->add_arc(tl, brp, sa, ea, arc_clockwise_[c++]);
                    break;
                }
                case path_operation::close:
                    current->close();
                    current = nullptr;
                    break;
            }
        }
        return paths;
    }

    path_f path_f::reverse() const
    {
        std::vector<point_f> points(points_.rbegin(), points_.rend());
        std::vector<float> arc_sizes(arc_angles_.rbegin(), arc_angles_.rend());
        std::vector<bool> arc_clockwise(arc_clockwise_.rbegin(), arc_clockwise_.rend());
        std::vector<path_operation> operations(operations_.rbegin(), operations_.rend());

        bool segment_closed = false;
        int segment_start = -1;
        for (int i = 0; std::cmp_less(i, operations.size()); i++)
        {
            if (operations[i] == path_operation::move)
            {
                if (segment_start == -1)
                {
                    operations.erase(operations.begin() + i);
                    operations.insert(operations.begin(), path_operation::move);
                }
                else if (segment_closed)
                {
                    operations[segment_start] = path_operation::move;
                    operations[i] = path_operation::close;
                }
                segment_start = i + 1;
            }
            else if (operations[i] == path_operation::close)
            {
                segment_start = i;
                segment_closed = true;
            }
        }
        return {std::move(points), std::move(arc_sizes), std::move(arc_clockwise), std::move(operations),
                sub_path_count_};
    }

    path_f path_f::get_flattened_path(float flatness, bool include_sub_paths) const
    {
        path_f flattened;
        std::vector<point_f> flattened_points;
        std::vector<point_f> curve_points;
        bool found_closed = false;
        int point_index = 0;
        int arc_angle_index = 0;
        int arc_cw_index = 0;

        for (int i = 0; i < operation_count() && !found_closed; i++)
        {
            switch (operations_[i])
            {
                case path_operation::move:
                    flattened.move_to(points_[point_index++]);
                    break;
                case path_operation::line:
                    flattened.line_to(points_[point_index++]);
                    break;
                case path_operation::quad: {
                    flattened_points.clear();
                    curve_points.clear();
                    // QuadToCubic(point_index)
                    point_f const start = points_[point_index - 1];
                    point_f const qcp = points_[point_index];
                    point_f const end = points_[point_index + 1];
                    point_f const cp1(start.x + (2.0F * (qcp.x - start.x) / 3.0F),
                                      start.y + (2.0F * (qcp.y - start.y) / 3.0F));
                    point_f const cp2(end.x + (2.0F * (qcp.x - end.x) / 3.0F), end.y + (2.0F * (qcp.y - end.y) / 3.0F));
                    curve_points = {start, cp1, cp2, end};
                    flatten_cubic_segment(0, flatness, curve_points, flattened_points);
                    for (auto& p : flattened_points)
                    {
                        flattened.line_to(p);
                    }
                    point_index += 2;
                    break;
                }
                case path_operation::cubic: {
                    flattened_points.clear();
                    flatten_cubic_segment(point_index - 1, flatness, points_, flattened_points);
                    for (auto& p : flattened_points)
                    {
                        flattened.line_to(p);
                    }
                    point_index += 3;
                    break;
                }
                case path_operation::arc: {
                    point_f const tl = points_[point_index++];
                    point_f const br = points_[point_index++];
                    float const sa = get_arc_angle(arc_angle_index++);
                    float const ea = get_arc_angle(arc_angle_index++);
                    bool const cw = get_arc_clockwise(arc_cw_index++);
                    arc_flattener const af(tl.x, tl.y, br.x - tl.x, br.y - tl.y, sa, ea, cw);
                    path_f const arc_path = af.create_flattened_path(flatness).get_flattened_path();
                    for (const auto& p : arc_path.points())
                    {
                        flattened.line_to(p);
                    }
                    break;
                }
                case path_operation::close:
                    flattened.close();
                    if (!include_sub_paths)
                    {
                        found_closed = true;
                    }
                    break;
            }
        }
        return flattened;
    }

    rect_f path_f::get_bounds_by_flattening(float flatness) const
    {
        if (cached_bounds_)
        {
            return *cached_bounds_;
        }
        path_f const path = get_flattened_path(flatness, true);
        float l = 0;
        float t = 0;
        float r = 0;
        float b = 0;
        if (path.count() > 0)
        {
            l = r = path[0].x;
            t = b = path[0].y;
            for (int i = 1; i < path.count(); i++)
            {
                auto p = path[i];
                l = std::min(p.x, l);
                t = std::min(p.y, t);
                r = std::max(p.x, r);
                b = std::max(p.y, b);
            }
        }
        cached_bounds_ = rect_f(l, t, r - l, b - t);
        return *cached_bounds_;
    }
    rect_f path_f::bounds() const
    {
        if (cached_bounds_)
        {
            return *cached_bounds_;
        }
        cached_bounds_ = get_bounds_by_flattening();
        return *cached_bounds_;
    }

    // ---- equality ----
    bool path_f::equals(const path_f& other, float epsilon) const
    {
        if (operation_count() != other.operation_count())
        {
            return false;
        }
        for (int i = 0; i < operation_count(); i++)
        {
            if (operations_[i] != other.get_segment_type(i))
            {
                return false;
            }
        }
        for (int i = 0; i < count(); i++)
        {
            if (!points_[i].equals(other[i], epsilon))
            {
                return false;
            }
        }
        for (int i = 0; std::cmp_less(i, arc_angles_.size()); i++)
        {
            if (std::abs(arc_angles_[i] - other.get_arc_angle(i)) > epsilon)
            {
                return false;
            }
        }
        for (int i = 0; std::cmp_less(i, arc_clockwise_.size()); i++)
        {
            if (arc_clockwise_[i] != other.get_arc_clockwise(i))
            {
                return false;
            }
        }
        return true;
    }
    bool path_f::equals(const path_f& other) const
    {
        return equals(other, k_epsilon);
    }

    void path_f::invalidate()
    {
        cached_bounds_.reset();
    }
    void path_f::dispose()
    {
    }
    void* path_f::platform_path() const
    {
        return platform_path_;
    }
    void path_f::set_platform_path(void* p)
    {
        platform_path_ = p;
    }

    bool operator==(const path_f& a, const path_f& b)
    {
        return a.equals(b);
    }
    bool operator!=(const path_f& a, const path_f& b)
    {
        return !a.equals(b);
    }

    // ---- SVG elliptical arc (PathArcExtensions.cs) ----
    namespace
    {
        // ComputeSvgArc -> {cx, cy, angleStart, angleExtent, rx, ry, xAxisRotation}
        std::array<float, 7> compute_svg_arc(float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag,
                                             float x, float y, float last_x, float last_y)
        {
            float const x_axis_rotation = angle;
            float const dx2 = (last_x - x) / 2.0F;
            float const dy2 = (last_y - y) / 2.0F;
            auto const a = static_cast<float>(degrees_to_radians(angle));
            float const cos_a = std::cos(a);
            float const sin_a = std::sin(a);
            float const x1 = (cos_a * dx2) + (sin_a * dy2);
            float const y1 = (-sin_a * dx2) + (cos_a * dy2);
            rx = std::abs(rx);
            ry = std::abs(ry);
            float prx = rx * rx;
            float pry = ry * ry;
            float const px1 = x1 * x1;
            float const py1 = y1 * y1;
            float const radii_check = (px1 / prx) + (py1 / pry);
            if (radii_check > 1)
            {
                rx = std::sqrt(radii_check) * rx;
                ry = std::sqrt(radii_check) * ry;
                prx = rx * rx;
                pry = ry * ry;
            }
            float sign = (large_arc_flag == sweep_flag) ? -1.0F : 1.0F;
            float sq = ((prx * pry) - (prx * py1) - (pry * px1)) / ((prx * py1) + (pry * px1));
            sq = sq < 0 ? 0 : sq;
            float const coef = sign * std::sqrt(sq);
            float const cx1 = coef * (rx * y1 / ry);
            float const cy1 = coef * -(ry * x1 / rx);
            float const sx2 = (last_x + x) / 2.0F;
            float const sy2 = (last_y + y) / 2.0F;
            float const cx = sx2 + ((cos_a * cx1) - (sin_a * cy1));
            float const cy = sy2 + ((sin_a * cx1) + (cos_a * cy1));
            float const ux = (x1 - cx1) / rx;
            float const uy = (y1 - cy1) / ry;
            float const vx = (-x1 - cx1) / rx;
            float const vy = (-y1 - cy1) / ry;
            float n = std::sqrt((ux * ux) + (uy * uy));
            float p = ux;
            sign = uy < 0 ? -1.0F : 1.0F;
            auto angle_start = static_cast<float>((sign * std::acos(p / n)) * 180.0 / k_pi);
            n = std::sqrt(((ux * ux) + (uy * uy)) * ((vx * vx) + (vy * vy)));
            p = (ux * vx) + (uy * vy);
            sign = ((ux * vy) - (uy * vx)) < 0 ? -1.0F : 1.0F;
            auto angle_extent = static_cast<float>((sign * std::acos(p / n)) * 180.0 / k_pi);
            if (!sweep_flag && angle_extent > 0)
            {
                angle_extent -= 360;
            }
            else if (sweep_flag && angle_extent < 0)
            {
                angle_extent += 360;
            }
            angle_extent = std::fmod(angle_extent, 360.0F);
            angle_start = std::fmod(angle_start, 360.0F);
            return {cx, cy, angle_start, angle_extent, rx, ry, x_axis_rotation};
        }
    } // namespace

    void path_f::draw_arc(float x, float y, float start_angle, float arc, float radius, float y_radius,
                          float x_axis_rotation)
    {
        if (std::abs(arc) > 360)
        {
            arc = 360;
        }
        float const segs = std::ceil(std::abs(arc) / 45);
        float const seg_angle = arc / segs;
        auto const theta = static_cast<float>(degrees_to_radians(seg_angle));
        auto angle = static_cast<float>(degrees_to_radians(start_angle));
        if (segs > 0)
        {
            auto const beta = static_cast<float>(degrees_to_radians(x_axis_rotation));
            float const sinbeta = std::sin(beta);
            float const cosbeta = std::cos(beta);
            for (int i = 0; i < static_cast<int>(segs); i++)
            {
                angle += theta;
                float sinangle = std::sin(angle - (theta / 2));
                float cosangle = std::cos(angle - (theta / 2));
                float const div = std::cos(theta / 2);
                float const cx = x + (((radius * cosangle * cosbeta) - (y_radius * sinangle * sinbeta)) / div);
                float const cy = y + (((radius * cosangle * sinbeta) + (y_radius * sinangle * cosbeta)) / div);
                sinangle = std::sin(angle);
                cosangle = std::cos(angle);
                float const x1 = x + ((radius * cosangle * cosbeta) - (y_radius * sinangle * sinbeta));
                float const y1 = y + ((radius * cosangle * sinbeta) + (y_radius * sinangle * cosbeta));
                quad_to(cx, cy, x1, y1);
            }
        }
    }

    void path_f::svg_arc_to(float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag, float x, float y,
                            float last_x, float last_y)
    {
        auto v = compute_svg_arc(rx, ry, angle, large_arc_flag, sweep_flag, x, y, last_x, last_y);
        draw_arc(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
    }

} // namespace maui::graphics

#pragma once
// maui::graphics::path_f  <=  Microsoft.Maui.Graphics.PathF  (+ PathOperation.cs, ArcFlattener.cs)
//
// A geometric path: parallel arrays of points/operations (+ arc angles/flags), grouped into
// sub-paths that each begin with a Move. Ported from src/Graphics/src/Graphics/PathF.cs.
// The C# original is a reference type implementing IDisposable; here it is a copyable value type
// (deterministic teardown, no native handle on the headless backend).
//
// Deliberate M0 deviations (recorded in port/STATUS.md):
//  - System.Numerics omitted: no Transform(Matrix3x2). TODO: revisit with a maui linalg type.
//  - PlatformPath kept as an opaque void* (no native path on headless).

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::graphics
{

    class path_f
    {
    public:
        path_f() = default;
        path_f(float x, float y);           // initial MoveTo(x, y)
        explicit path_f(const point_f &pt); // initial MoveTo(pt)
        // copy/move/dtor are the defaults (value semantics over std::vector members)

        // ---- sub-path / segment building (fluent: return *this) ----
        path_f &move_to(float x, float y);
        path_f &move_to(const point_f &point);
        void close();
        void open();
        path_f &line_to(float x, float y);
        path_f &line_to(const point_f &point);
        path_f &insert_line_to(const point_f &point, int index);
        path_f &add_arc(float x1, float y1, float x2, float y2, float start_angle, float end_angle, bool clockwise);
        path_f &add_arc(const point_f &top_left, const point_f &bottom_right, float start_angle, float end_angle,
                        bool clockwise);
        path_f &quad_to(float cx, float cy, float x, float y);
        path_f &quad_to(const point_f &control_point, const point_f &point);
        path_f &insert_quad_to(const point_f &control_point, const point_f &point, int index);
        path_f &curve_to(float c1x, float c1y, float c2x, float c2y, float x, float y);
        path_f &curve_to(const point_f &control1, const point_f &control2, const point_f &point);
        path_f &insert_curve_to(const point_f &control1, const point_f &control2, const point_f &point, int index);

        // ---- accessors ----
        int sub_path_count() const
        {
            return sub_path_count_;
        }
        bool closed() const;
        point_f first_point() const;
        point_f last_point() const;
        int last_point_index() const;
        int count() const
        {
            return static_cast<int>(points_.size());
        }
        int operation_count() const
        {
            return static_cast<int>(operations_.size());
        }
        int segment_count_excluding_open_and_close() const;
        point_f operator[](int index) const; // default-constructed point_f if out of range
        const std::vector<point_f> &points() const
        {
            return points_;
        }
        const std::vector<path_operation> &segment_types() const
        {
            return operations_;
        }
        path_operation get_segment_type(int index) const
        {
            return operations_[index];
        }
        bool is_sub_path_closed(int sub_path_index) const;

        void set_point(int index, float x, float y);
        void set_point(int index, const point_f &point);
        float get_arc_angle(int index) const;
        void set_arc_angle(int index, float value);
        bool get_arc_clockwise(int index) const;
        void set_arc_clockwise(int index, bool value);

        // ---- segment queries ----
        int get_segment_point_index(int index) const;
        int get_segment_for_point(int point_index) const;
        std::vector<point_f> get_points_for_segment(int segment_index) const;

        // ---- editing ----
        void remove_segment(int segment_index);
        void remove_all_segments_after(int segment_index);
        void move(float x, float y); // offset every point
        void move_point(int index, float dx, float dy);

        // ---- transforms ----
        path_f rotate(float angle_as_degrees, const point_f &pivot) const;
        point_f get_rotated_point(int point_index, const point_f &pivot, float angle) const;

        // ---- SVG elliptical arc (PathArcExtensions.cs) ----
        void svg_arc_to(float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag, float x, float y,
                        float last_x, float last_y);
        void draw_arc(float x, float y, float start_angle, float arc, float radius, float y_radius,
                      float x_axis_rotation);

        // ---- shape append helpers ----
        void append_ellipse(const rect_f &rect);
        void append_ellipse(float x, float y, float w, float h);
        void append_circle(const point_f &center, float r);
        void append_circle(float cx, float cy, float r);
        void append_rectangle(const rect_f &rect, bool include_last = false);
        void append_rectangle(float x, float y, float w, float h, bool include_last = false);
        void append_rounded_rectangle(const rect_f &rect, float corner_radius, bool include_last = false);
        void append_rounded_rectangle(float x, float y, float w, float h, float corner_radius,
                                      bool include_last = false);
        void append_rounded_rectangle(const rect_f &rect, float tl, float tr, float bl, float br,
                                      bool include_last = false);
        void append_rounded_rectangle(float x, float y, float w, float h, float tl, float tr, float bl, float br,
                                      bool include_last = false);
        void append_rounded_rectangle(const rect_f &rect, float x_corner_radius, float y_corner_radius);

        // ---- compose / flatten ----
        std::vector<path_f> separate() const;
        path_f reverse() const;
        path_f get_flattened_path(float flatness = 0.001F, bool include_sub_paths = false) const;
        rect_f get_bounds_by_flattening(float flatness = 0.001F) const;
        rect_f bounds() const;

        // ---- equality ----
        bool equals(const path_f &other) const; // uses GeometryUtil.Epsilon
        bool equals(const path_f &other, float epsilon) const;

        // ---- lifetime (headless: no native resource) ----
        void invalidate();
        void dispose()
        {
        }
        void *platform_path() const
        {
            return platform_path_;
        }
        void set_platform_path(void *p)
        {
            platform_path_ = p;
        }

    private:
        std::vector<point_f> points_;
        std::vector<path_operation> operations_;
        std::vector<float> arc_angles_;
        std::vector<bool> arc_clockwise_;
        int sub_path_count_ = 0;
        std::vector<bool> sub_paths_closed_;
        mutable std::optional<rect_f> cached_bounds_;
        void *platform_path_ = nullptr;

        // private ctor used by reverse(): rebuilds sub-path-closed state from operations
        path_f(std::vector<point_f> points, std::vector<float> arc_angles, std::vector<bool> arc_clockwise,
               std::vector<path_operation> operations, int sub_path_count);

        static float clamp_corner_radius(float cr, float w, float h);
    };

    inline bool operator==(const path_f &a, const path_f &b)
    {
        return a.equals(b);
    }
    inline bool operator!=(const path_f &a, const path_f &b)
    {
        return !a.equals(b);
    }

} // namespace maui::graphics

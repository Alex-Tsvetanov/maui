#pragma once
// maui::controls::shapes::line_geometry  <=  Microsoft.Maui.Controls.Shapes.LineGeometry
//
// The geometry of a straight line between two points. Ported from LineGeometry.cs; append_path is
// its AppendPath (a Move to the start, then a LineTo — C# calls path.Move(x, y) which OFFSETS every
// existing point; that is a long-standing C# quirk for a fresh path where it is equivalent to MoveTo,
// and the port uses move_to, the WPF-intended behavior).

#include "maui/controls/shapes/geometry.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    class line_geometry : public geometry
    {
    public:
        line_geometry() = default;
        // C# LineGeometry(Point startPoint, Point endPoint).
        line_geometry(const maui::graphics::point& start_point, const maui::graphics::point& end_point)
            : start_point_(start_point), end_point_(end_point)
        {
        }

        [[nodiscard]] const maui::graphics::point& start_point() const
        {
            return start_point_;
        }
        void set_start_point(const maui::graphics::point& value)
        {
            start_point_ = value;
        }
        [[nodiscard]] const maui::graphics::point& end_point() const
        {
            return end_point_;
        }
        void set_end_point(const maui::graphics::point& value)
        {
            end_point_ = value;
        }

        void append_path(maui::graphics::path_f& path) const override
        {
            path.move_to(static_cast<float>(start_point_.x), static_cast<float>(start_point_.y));
            path.line_to(static_cast<float>(end_point_.x), static_cast<float>(end_point_.y));
        }

    private:
        maui::graphics::point start_point_;
        maui::graphics::point end_point_;
    };
} // namespace maui::controls::shapes

#pragma once
// maui::controls::shapes::ellipse_geometry  <=  Microsoft.Maui.Controls.Shapes.EllipseGeometry
//
// The geometry of an ellipse/circle given a center and the two radii. Ported from EllipseGeometry.cs;
// append_path is its AppendPath (AppendEllipse over the center-radius box).

#include "maui/controls/shapes/geometry.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    class ellipse_geometry : public geometry
    {
    public:
        ellipse_geometry() = default;
        // C# EllipseGeometry(Point center, double radiusX, double radiusY).
        ellipse_geometry(const maui::graphics::point& center, double radius_x, double radius_y)
            : center_(center), radius_x_(radius_x), radius_y_(radius_y)
        {
        }

        [[nodiscard]] const maui::graphics::point& center() const
        {
            return center_;
        }
        void set_center(const maui::graphics::point& value)
        {
            center_ = value;
        }
        [[nodiscard]] double radius_x() const
        {
            return radius_x_;
        }
        void set_radius_x(double value)
        {
            radius_x_ = value;
        }
        [[nodiscard]] double radius_y() const
        {
            return radius_y_;
        }
        void set_radius_y(double value)
        {
            radius_y_ = value;
        }

        void append_path(maui::graphics::path_f& path) const override
        {
            const auto center_x = static_cast<float>(center_.x);
            const auto center_y = static_cast<float>(center_.y);
            const auto radius_x = static_cast<float>(radius_x_);
            const auto radius_y = static_cast<float>(radius_y_);
            path.append_ellipse(center_x - radius_x, center_y - radius_y, radius_x * 2.0F, radius_y * 2.0F);
        }

    private:
        maui::graphics::point center_;
        double radius_x_ = 0;
        double radius_y_ = 0;
    };
} // namespace maui::controls::shapes

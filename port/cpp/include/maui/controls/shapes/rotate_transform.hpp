#pragma once
// maui::controls::shapes::rotate_transform  <=  Microsoft.Maui.Controls.Shapes.RotateTransform
//
// Rotates around (center_x, center_y) by `angle` degrees. Ported from RotateTransform.cs; value() is
// the exact matrix its OnTransformPropertyChanged builds:
//   (cos, sin, -sin, cos, cx(1-cos) + cy*sin, cy(1-cos) - cx*sin).
// Scalars are plain members computed on read (the transform.hpp port collapse).

#include <cmath>
#include <numbers>

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    class rotate_transform : public transform
    {
    public:
        rotate_transform() = default;
        // C# RotateTransform(double angle) / (angle, centerX, centerY).
        explicit rotate_transform(double angle) : angle_(angle)
        {
        }
        rotate_transform(double angle, double center_x, double center_y)
            : angle_(angle), center_x_(center_x), center_y_(center_y)
        {
        }

        [[nodiscard]] double angle() const
        {
            return angle_;
        }
        void set_angle(double value)
        {
            angle_ = value;
        }
        [[nodiscard]] double center_x() const
        {
            return center_x_;
        }
        void set_center_x(double value)
        {
            center_x_ = value;
        }
        [[nodiscard]] double center_y() const
        {
            return center_y_;
        }
        void set_center_y(double value)
        {
            center_y_ = value;
        }

        [[nodiscard]] matrix value() const override
        {
            const double radians = std::numbers::pi * angle_ / 180.0;
            const double sin = std::sin(radians);
            const double cos = std::cos(radians);
            return {cos,
                    sin,
                    -sin,
                    cos,
                    (center_x_ * (1 - cos)) + (center_y_ * sin),
                    (center_y_ * (1 - cos)) - (center_x_ * sin)};
        }

    private:
        double angle_ = 0;
        double center_x_ = 0;
        double center_y_ = 0;
    };
} // namespace maui::controls::shapes

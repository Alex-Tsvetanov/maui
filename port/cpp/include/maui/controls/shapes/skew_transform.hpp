#pragma once
// maui::controls::shapes::skew_transform  <=  Microsoft.Maui.Controls.Shapes.SkewTransform
//
// Skews (shears) by angle_x/angle_y degrees around (center_x, center_y). Ported from
// SkewTransform.cs; value() is its OnTransformPropertyChanged matrix:
//   (1, tanY, tanX, 1, -cy*tanX, -cx*tanY). Plain scalars computed on read (the transform.hpp
// port collapse).

#include <cmath>
#include <numbers>

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    class skew_transform : public transform
    {
    public:
        skew_transform() = default;
        // C# SkewTransform(angleX, angleY) / (angleX, angleY, centerX, centerY).
        skew_transform(double angle_x, double angle_y) : angle_x_(angle_x), angle_y_(angle_y)
        {
        }
        skew_transform(double angle_x, double angle_y, double center_x, double center_y)
            : angle_x_(angle_x), angle_y_(angle_y), center_x_(center_x), center_y_(center_y)
        {
        }

        [[nodiscard]] double angle_x() const
        {
            return angle_x_;
        }
        void set_angle_x(double value)
        {
            angle_x_ = value;
        }
        [[nodiscard]] double angle_y() const
        {
            return angle_y_;
        }
        void set_angle_y(double value)
        {
            angle_y_ = value;
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
            const double tan_x = std::tan(std::numbers::pi * angle_x_ / 180.0);
            const double tan_y = std::tan(std::numbers::pi * angle_y_ / 180.0);
            return {1, tan_y, tan_x, 1, -center_y_ * tan_x, -center_x_ * tan_y};
        }

    private:
        double angle_x_ = 0;
        double angle_y_ = 0;
        double center_x_ = 0;
        double center_y_ = 0;
    };
} // namespace maui::controls::shapes

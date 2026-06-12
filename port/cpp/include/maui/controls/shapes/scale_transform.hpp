#pragma once
// maui::controls::shapes::scale_transform  <=  Microsoft.Maui.Controls.Shapes.ScaleTransform
//
// Scales by (scale_x, scale_y) around (center_x, center_y). Ported from ScaleTransform.cs; value() is
// its OnTransformPropertyChanged matrix: (sx, 0, 0, sy, cx(1-sx), cy(1-sy)). Plain scalars computed
// on read (the transform.hpp port collapse).

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    class scale_transform : public transform
    {
    public:
        scale_transform() = default;
        // C# ScaleTransform(scaleX, scaleY) / (scaleX, scaleY, centerX, centerY).
        scale_transform(double scale_x, double scale_y) : scale_x_(scale_x), scale_y_(scale_y)
        {
        }
        scale_transform(double scale_x, double scale_y, double center_x, double center_y)
            : scale_x_(scale_x), scale_y_(scale_y), center_x_(center_x), center_y_(center_y)
        {
        }

        [[nodiscard]] double scale_x() const
        {
            return scale_x_;
        }
        void set_scale_x(double value)
        {
            scale_x_ = value;
        }
        [[nodiscard]] double scale_y() const
        {
            return scale_y_;
        }
        void set_scale_y(double value)
        {
            scale_y_ = value;
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
            return {scale_x_, 0, 0, scale_y_, center_x_ * (1 - scale_x_), center_y_ * (1 - scale_y_)};
        }

    private:
        double scale_x_ = 1;
        double scale_y_ = 1;
        double center_x_ = 0;
        double center_y_ = 0;
    };
} // namespace maui::controls::shapes

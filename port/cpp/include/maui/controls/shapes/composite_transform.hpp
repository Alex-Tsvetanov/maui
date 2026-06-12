#pragma once
// maui::controls::shapes::composite_transform  <=  Microsoft.Maui.Controls.Shapes.CompositeTransform
//
// Combines scale, skew, rotation and translation around a common center in the fixed WPF order:
// translate(-center) * scale * skew * rotate * translate(center) * translate(translate_x/y).
// Ported from CompositeTransform.cs — value() builds the exact TransformGroup its
// OnTransformPropertyChanged composes. Plain scalars computed on read (the transform.hpp port
// collapse).

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/rotate_transform.hpp"
#include "maui/controls/shapes/scale_transform.hpp"
#include "maui/controls/shapes/skew_transform.hpp"
#include "maui/controls/shapes/transform.hpp"
#include "maui/controls/shapes/translate_transform.hpp"

namespace maui::controls::shapes
{
    class composite_transform : public transform
    {
    public:
        composite_transform() = default;

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
        [[nodiscard]] double skew_x() const
        {
            return skew_x_;
        }
        void set_skew_x(double value)
        {
            skew_x_ = value;
        }
        [[nodiscard]] double skew_y() const
        {
            return skew_y_;
        }
        void set_skew_y(double value)
        {
            skew_y_ = value;
        }
        [[nodiscard]] double rotation() const
        {
            return rotation_;
        }
        void set_rotation(double value)
        {
            rotation_ = value;
        }
        [[nodiscard]] double translate_x() const
        {
            return translate_x_;
        }
        void set_translate_x(double value)
        {
            translate_x_ = value;
        }
        [[nodiscard]] double translate_y() const
        {
            return translate_y_;
        }
        void set_translate_y(double value)
        {
            translate_y_ = value;
        }

        // The exact six-child TransformGroup C#'s OnTransformPropertyChanged builds, folded.
        [[nodiscard]] matrix value() const override
        {
            matrix result;
            result = matrix::multiply(result, translate_transform(-center_x_, -center_y_).value());
            result = matrix::multiply(result, scale_transform(scale_x_, scale_y_).value());
            result = matrix::multiply(result, skew_transform(skew_x_, skew_y_).value());
            result = matrix::multiply(result, rotate_transform(rotation_).value());
            result = matrix::multiply(result, translate_transform(center_x_, center_y_).value());
            result = matrix::multiply(result, translate_transform(translate_x_, translate_y_).value());
            return result;
        }

    private:
        double center_x_ = 0;
        double center_y_ = 0;
        double scale_x_ = 1;
        double scale_y_ = 1;
        double skew_x_ = 0;
        double skew_y_ = 0;
        double rotation_ = 0;
        double translate_x_ = 0;
        double translate_y_ = 0;
    };
} // namespace maui::controls::shapes

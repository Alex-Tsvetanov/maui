#pragma once
// maui::controls::shapes::translate_transform  <=  Microsoft.Maui.Controls.Shapes.TranslateTransform
//
// Moves by the (x, y) offset. Ported from TranslateTransform.cs; value() is its
// OnTransformPropertyChanged matrix (1, 0, 0, 1, x, y). Plain scalars computed on read (the
// transform.hpp port collapse).

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    class translate_transform : public transform
    {
    public:
        translate_transform() = default;
        // C# TranslateTransform(double x, double y).
        translate_transform(double x, double y) : x_(x), y_(y)
        {
        }

        [[nodiscard]] double x() const
        {
            return x_;
        }
        void set_x(double value)
        {
            x_ = value;
        }
        [[nodiscard]] double y() const
        {
            return y_;
        }
        void set_y(double value)
        {
            y_ = value;
        }

        [[nodiscard]] matrix value() const override
        {
            return {1, 0, 0, 1, x_, y_};
        }

    private:
        double x_ = 0;
        double y_ = 0;
    };
} // namespace maui::controls::shapes

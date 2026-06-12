#pragma once
// maui::controls::shapes::matrix_transform  <=  Microsoft.Maui.Controls.Shapes.MatrixTransform
//
// An arbitrary linear transform given directly as a matrix. Ported from MatrixTransform.cs; value()
// returns the stored matrix (the C# OnTransformPropertyChanged copies Matrix into Value). Plain
// member computed on read (the transform.hpp port collapse).

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    class matrix_transform : public transform
    {
    public:
        matrix_transform() = default;

        [[nodiscard]] const matrix& get_matrix() const
        {
            return matrix_;
        }
        void set_matrix(const matrix& value)
        {
            matrix_ = value;
        }

        [[nodiscard]] matrix value() const override
        {
            return matrix_;
        }

    private:
        matrix matrix_;
    };
} // namespace maui::controls::shapes

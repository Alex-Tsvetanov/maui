// maui::controls::shapes::matrix — out-of-line definitions. Ported from Matrix.cs + MatrixUtil.cs +
// MatrixExtensions.cs (the general-math branches; the MatrixTypes fast paths are collapsed — see the
// header note).

#include "maui/controls/shapes/matrix.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>

#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    namespace
    {
        constexpr double degrees_to_radians(double degrees)
        {
            return degrees * (std::numbers::pi / 180.0);
        }
    } // namespace

    void matrix::set_identity()
    {
        *this = identity();
    }

    bool matrix::is_identity() const
    {
        return m11 == 1 && m12 == 0 && m21 == 0 && m22 == 1 && offset_x == 0 && offset_y == 0;
    }

    // MatrixUtil.MultiplyMatrix's general (Unknown * Unknown) branch — every flagged fast path of the
    // C# switch computes exactly this product.
    matrix matrix::multiply(const matrix& trans1, const matrix& trans2)
    {
        return {(trans1.m11 * trans2.m11) + (trans1.m12 * trans2.m21),
                (trans1.m11 * trans2.m12) + (trans1.m12 * trans2.m22),
                (trans1.m21 * trans2.m11) + (trans1.m22 * trans2.m21),
                (trans1.m21 * trans2.m12) + (trans1.m22 * trans2.m22),
                (trans1.offset_x * trans2.m11) + (trans1.offset_y * trans2.m21) + trans2.offset_x,
                (trans1.offset_x * trans2.m12) + (trans1.offset_y * trans2.m22) + trans2.offset_y};
    }

    void matrix::append(const matrix& other)
    {
        *this = multiply(*this, other);
    }

    void matrix::prepend(const matrix& other)
    {
        *this = multiply(other, *this);
    }

    void matrix::rotate(double angle)
    {
        angle = std::fmod(angle, 360.0);
        append(create_rotation_radians(degrees_to_radians(angle)));
    }

    void matrix::rotate_prepend(double angle)
    {
        angle = std::fmod(angle, 360.0);
        prepend(create_rotation_radians(degrees_to_radians(angle)));
    }

    void matrix::rotate_at(double angle, double center_x, double center_y)
    {
        angle = std::fmod(angle, 360.0);
        append(create_rotation_radians(degrees_to_radians(angle), center_x, center_y));
    }

    void matrix::rotate_at_prepend(double angle, double center_x, double center_y)
    {
        angle = std::fmod(angle, 360.0);
        prepend(create_rotation_radians(degrees_to_radians(angle), center_x, center_y));
    }

    void matrix::scale(double scale_x, double scale_y)
    {
        append(create_scaling(scale_x, scale_y));
    }

    void matrix::scale_prepend(double scale_x, double scale_y)
    {
        prepend(create_scaling(scale_x, scale_y));
    }

    void matrix::scale_at(double scale_x, double scale_y, double center_x, double center_y)
    {
        append(create_scaling(scale_x, scale_y, center_x, center_y));
    }

    void matrix::scale_at_prepend(double scale_x, double scale_y, double center_x, double center_y)
    {
        prepend(create_scaling(scale_x, scale_y, center_x, center_y));
    }

    void matrix::skew(double skew_x, double skew_y)
    {
        skew_x = std::fmod(skew_x, 360.0);
        skew_y = std::fmod(skew_y, 360.0);
        append(create_skew_radians(degrees_to_radians(skew_x), degrees_to_radians(skew_y)));
    }

    void matrix::skew_prepend(double skew_x, double skew_y)
    {
        skew_x = std::fmod(skew_x, 360.0);
        skew_y = std::fmod(skew_y, 360.0);
        prepend(create_skew_radians(degrees_to_radians(skew_x), degrees_to_radians(skew_y)));
    }

    void matrix::translate(double offset_x_, double offset_y_)
    {
        // C# Matrix.Translate appends the offsets directly (the translation row is the only change).
        offset_x += offset_x_;
        offset_y += offset_y_;
    }

    void matrix::translate_prepend(double offset_x_, double offset_y_)
    {
        prepend(create_translation(offset_x_, offset_y_));
    }

    maui::graphics::point matrix::transform(const maui::graphics::point& point) const
    {
        // C# MultiplyPoint (general branch): v' = v * M with the translation row added.
        const double x = (point.x * m11) + (point.y * m21) + offset_x;
        const double y = (point.x * m12) + (point.y * m22) + offset_y;
        return {x, y};
    }

    double matrix::determinant() const
    {
        return (m11 * m22) - (m12 * m21);
    }

    bool matrix::has_inverse() const
    {
        return determinant() != 0;
    }

    void matrix::invert()
    {
        const double det = determinant();
        if (det == 0)
        {
            // C# throws InvalidOperationException for a singular matrix.
            throw std::logic_error("matrix is not invertible");
        }
        const double invdet = 1.0 / det;
        *this = {m22 * invdet,
                 -m12 * invdet,
                 -m21 * invdet,
                 m11 * invdet,
                 ((m21 * offset_y) - (offset_x * m22)) * invdet,
                 ((offset_x * m12) - (m11 * offset_y)) * invdet};
    }

    matrix matrix::create_rotation_radians(double angle_radians, double center_x, double center_y)
    {
        const double sin = std::sin(angle_radians);
        const double cos = std::cos(angle_radians);
        const double dx = (center_x * (1.0 - cos)) + (center_y * sin);
        const double dy = (center_y * (1.0 - cos)) - (center_x * sin);
        return {cos, sin, -sin, cos, dx, dy};
    }

    matrix matrix::create_scaling(double scale_x, double scale_y, double center_x, double center_y)
    {
        return {scale_x, 0, 0, scale_y, center_x - (scale_x * center_x), center_y - (scale_y * center_y)};
    }

    matrix matrix::create_skew_radians(double skew_x_radians, double skew_y_radians)
    {
        return {1.0, std::tan(skew_y_radians), std::tan(skew_x_radians), 1.0, 0.0, 0.0};
    }

    matrix matrix::create_translation(double offset_x_, double offset_y_)
    {
        return {1, 0, 0, 1, offset_x_, offset_y_};
    }

    maui::graphics::matrix3x2 to_matrix3x2(const matrix& value)
    {
        return {static_cast<float>(value.m11), static_cast<float>(value.m12),      static_cast<float>(value.m21),
                static_cast<float>(value.m22), static_cast<float>(value.offset_x), static_cast<float>(value.offset_y)};
    }
} // namespace maui::controls::shapes

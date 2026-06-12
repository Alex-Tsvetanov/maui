#pragma once
// maui::controls::shapes::matrix  <=  Microsoft.Maui.Controls.Shapes.Matrix
//
// The WPF-style 3x3 affine transformation matrix (double precision) the Shapes transform model
// computes with — distinct from maui::graphics::matrix3x2 (the System.Numerics float interop type
// the canvas/path layer uses). Ported from src/Controls/src/Core/Shapes/Matrix.cs:
//
//     | m11      m12      0 |
//     | m21      m22      0 |
//     | offset_x offset_y 1 |    (a row vector (x, y, 1) is transformed as v * M)
//
// PORT COLLAPSE (documented, not stubbed): the C# struct carries a MatrixTypes flag enum
// (Identity/Translation/Scaling/Unknown) purely as a fast-path optimization for MultiplyPoint /
// MultiplyMatrix / Determinant; every flagged branch computes the same value as the general one. The
// port stores the six cells directly and always runs the general math — the observable results
// (Transform, Multiply, Invert, Determinant) are identical. The only behavioral nuance dropped is
// C# Matrix.Equals comparing the *flag* too (two value-identical matrices built differently could
// compare unequal in C#); the port compares the six cells, which is the documented intent.
//
// Out-of-line definitions: src/controls/shapes/matrix.cpp. to_matrix3x2() ports
// MatrixExtensions.ToMatrix3X2 (the bridge into the graphics/path layer).

#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    struct matrix
    {
        double m11 = 1;
        double m12 = 0;
        double m21 = 0;
        double m22 = 1;
        double offset_x = 0;
        double offset_y = 0;

        constexpr matrix() = default;
        // C# Matrix(double m11, m12, m21, m22, offsetX, offsetY).
        constexpr matrix(double m11_, double m12_, double m21_, double m22_, double offset_x_, double offset_y_)
            : m11(m11_), m12(m12_), m21(m21_), m22(m22_), offset_x(offset_x_), offset_y(offset_y_)
        {
        }

        // C# Matrix.Identity (the default-constructed value).
        [[nodiscard]] static constexpr matrix identity()
        {
            return {};
        }
        // C# Matrix.SetIdentity().
        void set_identity();
        // C# Matrix.IsIdentity.
        [[nodiscard]] bool is_identity() const;

        // C# Matrix.Multiply(trans1, trans2) / operator* — trans1 * trans2 (row-vector composition).
        [[nodiscard]] static matrix multiply(const matrix& trans1, const matrix& trans2);

        // C# Matrix.Append / Prepend.
        void append(const matrix& other);
        void prepend(const matrix& other);

        // C# Matrix.Rotate/RotatePrepend/RotateAt/RotateAtPrepend (degrees, % 360 first).
        void rotate(double angle);
        void rotate_prepend(double angle);
        void rotate_at(double angle, double center_x, double center_y);
        void rotate_at_prepend(double angle, double center_x, double center_y);

        // C# Matrix.Scale/ScalePrepend/ScaleAt/ScaleAtPrepend.
        void scale(double scale_x, double scale_y);
        void scale_prepend(double scale_x, double scale_y);
        void scale_at(double scale_x, double scale_y, double center_x, double center_y);
        void scale_at_prepend(double scale_x, double scale_y, double center_x, double center_y);

        // C# Matrix.Skew/SkewPrepend (degrees, % 360 first).
        void skew(double skew_x, double skew_y);
        void skew_prepend(double skew_x, double skew_y);

        // C# Matrix.Translate/TranslatePrepend.
        void translate(double offset_x_, double offset_y_);
        void translate_prepend(double offset_x_, double offset_y_);

        // C# Matrix.Transform(Point).
        [[nodiscard]] maui::graphics::point transform(const maui::graphics::point& point) const;

        // C# Matrix.Determinant / HasInverse / Invert (Invert throws std::logic_error when singular,
        // the InvalidOperationException analog).
        [[nodiscard]] double determinant() const;
        [[nodiscard]] bool has_inverse() const;
        void invert();

        // ---- the factory internals C#'s transforms call (Matrix.CreateRotationRadians & co.) ----
        [[nodiscard]] static matrix create_rotation_radians(double angle_radians, double center_x = 0,
                                                            double center_y = 0);
        [[nodiscard]] static matrix create_scaling(double scale_x, double scale_y, double center_x = 0,
                                                   double center_y = 0);
        [[nodiscard]] static matrix create_skew_radians(double skew_x_radians, double skew_y_radians);
        [[nodiscard]] static matrix create_translation(double offset_x_, double offset_y_);

        friend constexpr bool operator==(const matrix& a, const matrix& b) = default;
    };

    // MatrixExtensions.ToMatrix3X2 — the bridge into the float graphics layer (PathF.Transform).
    [[nodiscard]] maui::graphics::matrix3x2 to_matrix3x2(const matrix& value);
} // namespace maui::controls::shapes

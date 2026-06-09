#pragma once
// maui::graphics::matrix3x2  <=  System.Numerics.Matrix3x2
//
// A 3x2 row-major affine transform (the 2D matrix .NET's System.Numerics uses), standing in for
// System.Numerics.Matrix3x2 at the maui::graphics interop boundary (point_f::transform_by,
// path_f::transform). The six stored cells mirror .NET's fields exactly:
//
//     | m11  m12 |
//     | m21  m22 |
//     | m31  m32 |   (m31/m32 = translation)
//
// A row vector (x, y, 1) is transformed as (x, y, 1) * M (see vector2::transform). The port supplies
// only the surface the ported call sites use: the field ctor, identity(), and multiply (matching
// Matrix3x2.operator*). CreateTranslation/Scale/Rotation factories are intentionally omitted — no
// transform path in the graphics layer needs them (callers build matrices via the field ctor); add
// them only when a real call site does. Not a general linear-algebra library.

namespace maui::graphics
{
    struct matrix3x2
    {
        float m11 = 0;
        float m12 = 0;
        float m21 = 0;
        float m22 = 0;
        float m31 = 0;
        float m32 = 0;

        constexpr matrix3x2() = default;
        constexpr matrix3x2(float m11_, float m12_, float m21_, float m22_, float m31_, float m32_)
            : m11(m11_), m12(m12_), m21(m21_), m22(m22_), m31(m31_), m32(m32_)
        {
        }

        // System.Numerics.Matrix3x2.Identity.
        [[nodiscard]] static constexpr matrix3x2 identity()
        {
            return {1, 0, 0, 1, 0, 0};
        }
    };

    // System.Numerics.Matrix3x2.operator* (left * right), row-major affine composition.
    [[nodiscard]] constexpr matrix3x2 operator*(const matrix3x2& a, const matrix3x2& b)
    {
        return {
            (a.m11 * b.m11) + (a.m12 * b.m21),         (a.m11 * b.m12) + (a.m12 * b.m22),
            (a.m21 * b.m11) + (a.m22 * b.m21),         (a.m21 * b.m12) + (a.m22 * b.m22),
            (a.m31 * b.m11) + (a.m32 * b.m21) + b.m31, (a.m31 * b.m12) + (a.m32 * b.m22) + b.m32,
        };
    }

    constexpr bool operator==(const matrix3x2& a, const matrix3x2& b)
    {
        return a.m11 == b.m11 && a.m12 == b.m12 && a.m21 == b.m21 && a.m22 == b.m22 && a.m31 == b.m31 && a.m32 == b.m32;
    }
    constexpr bool operator!=(const matrix3x2& a, const matrix3x2& b)
    {
        return !(a == b);
    }
} // namespace maui::graphics

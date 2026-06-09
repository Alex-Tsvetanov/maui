#pragma once
// maui::graphics::vector2  <=  System.Numerics.Vector2
//
// A minimal 2D float vector standing in for System.Numerics.Vector2 at the maui::graphics interop
// boundary (point_f(vector2), (vector2)point_f, path_f::transform). .NET MAUI uses System.Numerics
// directly; C++ has no such type, so the port supplies just the surface the graphics primitives use:
// the {x, y} fields, equality, and transform(vector2, matrix3x2) (matching Vector2.Transform with a
// Matrix3x2 — the row-vector affine convention). This is intentionally NOT a general linear-algebra
// library; add members only when a ported call site needs them.

namespace maui::graphics
{
    struct matrix3x2;

    struct vector2
    {
        float x = 0;
        float y = 0;

        constexpr vector2() = default;
        constexpr vector2(float x_, float y_) : x(x_), y(y_)
        {
        }

        // System.Numerics.Vector2.Transform(position, matrix): row-vector * affine matrix.
        // (x*m11 + y*m21 + m31, x*m12 + y*m22 + m32). Body in vector2.cpp (needs matrix3x2's layout).
        [[nodiscard]] static vector2 transform(const vector2& position, const matrix3x2& matrix);
    };

    constexpr bool operator==(const vector2& a, const vector2& b)
    {
        return a.x == b.x && a.y == b.y;
    }
    constexpr bool operator!=(const vector2& a, const vector2& b)
    {
        return !(a == b);
    }
} // namespace maui::graphics

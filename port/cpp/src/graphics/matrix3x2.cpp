// maui::graphics::matrix3x2 — out-of-line definitions. See matrix3x2.hpp. The factories port
// System.Numerics.Matrix3x2.CreateTranslation/CreateScale/CreateRotation (minus CreateRotation's
// near-axis snapping micro-optimization — an epsilon snap of sin/cos to 0/±1 that is observationally
// irrelevant at float precision for the canvas transform tracking); the free functions port
// Microsoft.Maui.Graphics.Matrix3x2Extensions (src/Graphics/src/Graphics/Matrix3x2Extensions.cs).

#include "maui/graphics/matrix3x2.hpp"

#include <cmath>

namespace maui::graphics
{
    matrix3x2 matrix3x2::create_translation(float x, float y)
    {
        return {1, 0, 0, 1, x, y};
    }

    matrix3x2 matrix3x2::create_scale(float sx, float sy)
    {
        return {sx, 0, 0, sy, 0, 0};
    }

    matrix3x2 matrix3x2::create_rotation(float radians)
    {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        // [  cos  sin ]
        // [ -sin  cos ]
        // [   0    0  ]
        return {c, s, -s, c, 0, 0};
    }

    float matrix3x2::get_determinant() const
    {
        // System.Numerics.Matrix3x2.GetDeterminant: (M11 * M22) - (M21 * M12).
        return (m11 * m22) - (m21 * m12);
    }

    void deconstruct_scales(const matrix3x2& value, float& scale, float& scale_x, float& scale_y)
    {
        // C# DeconstructScales: scale = sqrt|det|; per-axis scales are the row lengths
        // (|m11| / |m22| when the off-diagonal cell is 0); scaley negated when the determinant is
        // negative (a mirrored matrix).
        const float det = value.get_determinant();
        scale = std::sqrt(std::abs(det));
        scale_x = value.m12 == 0 ? std::abs(value.m11) : std::hypot(value.m11, value.m12);
        scale_y = value.m21 == 0 ? std::abs(value.m22) : std::hypot(value.m21, value.m22);
        if (det < 0)
        {
            scale_y = -scale_y;
        }
    }

    float get_length_scale(const matrix3x2& matrix)
    {
        // C# GetLengthScale: sqrt(abs(determinant)) — the area scale's square root.
        return std::sqrt(std::abs(matrix.get_determinant()));
    }
} // namespace maui::graphics

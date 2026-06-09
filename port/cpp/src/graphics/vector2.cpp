// maui::graphics::vector2  <=  System.Numerics.Vector2 (the transform body; see vector2.hpp).
#include "maui/graphics/vector2.hpp"

#include "maui/graphics/matrix3x2.hpp"

namespace maui::graphics
{
    vector2 vector2::transform(const vector2& position, const matrix3x2& matrix)
    {
        // System.Numerics.Vector2.Transform(Vector2, Matrix3x2): a row vector (x, y, 1) times the
        // affine matrix. Mirrors the reference decomposition in System.Numerics.
        return {
            (position.x * matrix.m11) + (position.y * matrix.m21) + matrix.m31,
            (position.x * matrix.m12) + (position.y * matrix.m22) + matrix.m32,
        };
    }
} // namespace maui::graphics

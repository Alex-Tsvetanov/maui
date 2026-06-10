#pragma once
// maui::graphics::quaternion  <=  System.Numerics.Quaternion
//
// A minimal quaternion standing in for System.Numerics.Quaternion at the interop boundary (the
// essentials OrientationSensorData.Orientation reading). The port supplies only the surface the
// ported call sites use: the {x, y, z, w} fields, equality, Quaternion.Multiply (the iOS
// OrientationSensor partial composes the MAUI->iOS reference-frame rotation with the CoreMotion
// attitude), and CreateFromAxisAngle (used to build that fixed 90-degree Z rotation). Formulas are
// ported 1:1 from System.Numerics. Not a general math library; extend only when a call site needs it.

namespace maui::graphics
{
    struct quaternion
    {
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 1; // System.Numerics.Quaternion.Identity is (0, 0, 0, 1)

        constexpr quaternion() = default;
        constexpr quaternion(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_)
        {
        }

        // System.Numerics.Quaternion.Multiply(value1, value2): the concatenated rotation.
        [[nodiscard]] static constexpr quaternion multiply(const quaternion& value1, const quaternion& value2)
        {
            // Cross product (q1.xyz x q2.xyz) + dot product, exactly as System.Numerics spells it.
            const float cx = (value1.y * value2.z) - (value1.z * value2.y);
            const float cy = (value1.z * value2.x) - (value1.x * value2.z);
            const float cz = (value1.x * value2.y) - (value1.y * value2.x);
            const float dot = (value1.x * value2.x) + (value1.y * value2.y) + (value1.z * value2.z);
            return {(value1.x * value2.w) + (value2.x * value1.w) + cx,
                    (value1.y * value2.w) + (value2.y * value1.w) + cy,
                    (value1.z * value2.w) + (value2.z * value1.w) + cz, (value1.w * value2.w) - dot};
        }
    };

    constexpr bool operator==(const quaternion& a, const quaternion& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
    constexpr bool operator!=(const quaternion& a, const quaternion& b)
    {
        return !(a == b);
    }
} // namespace maui::graphics

#pragma once
// maui::graphics::vector3  <=  System.Numerics.Vector3
//
// A minimal 3-component float vector standing in for System.Numerics.Vector3 at the interop
// boundary (the essentials sensor readings: AccelerometerData.Acceleration,
// GyroscopeData.AngularVelocity, MagnetometerData.MagneticField). .NET MAUI uses System.Numerics
// directly; C++ has no such type, so the port supplies just the {x, y, z} fields and equality the
// ported call sites use. Not a general linear-algebra library; add members only when a ported call
// site needs them (sibling of vector2.hpp / vector4.hpp).

namespace maui::graphics
{
    struct vector3
    {
        float x = 0;
        float y = 0;
        float z = 0;

        constexpr vector3() = default;
        constexpr vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_)
        {
        }
    };

    constexpr bool operator==(const vector3& a, const vector3& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    constexpr bool operator!=(const vector3& a, const vector3& b)
    {
        return !(a == b);
    }
} // namespace maui::graphics

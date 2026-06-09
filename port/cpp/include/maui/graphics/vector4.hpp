#pragma once
// maui::graphics::vector4  <=  System.Numerics.Vector4
//
// A minimal 4-component float vector standing in for System.Numerics.Vector4 at the maui::graphics
// interop boundary (color <-> vector4, RGBA). .NET MAUI uses System.Numerics directly; C++ has no
// such type, so the port supplies just the {x, y, z, w} fields and equality the ported call sites
// use. Not a general linear-algebra library; add members only when a call site needs them.

namespace maui::graphics
{
    struct vector4
    {
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 0;

        constexpr vector4() = default;
        constexpr vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_)
        {
        }
    };

    constexpr bool operator==(const vector4& a, const vector4& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
    constexpr bool operator!=(const vector4& a, const vector4& b)
    {
        return !(a == b);
    }
} // namespace maui::graphics

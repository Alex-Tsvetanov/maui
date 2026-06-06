#pragma once
// maui::graphics::point_f  <=  Microsoft.Maui.Graphics.PointF
//
// 2D point with float components. Ported from src/Graphics/src/Graphics/PointF.cs. Interconverts
// with its double sibling `point` and with `size_f` (cross-precision casts + mixed operators), so
// those are forward-declared here and the cross-type bodies live in point_f.cpp.
//
// Deliberate M0 deviation (port/STATUS.md): System.Numerics interop omitted (no Vector2
// ctors/casts, no TransformBy/TransformNormalBy). TODO: revisit with a maui linalg type.

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct size_f;
    struct point;

    struct point_f
    {
        float x = 0;
        float y = 0;

        static const point_f zero;

        point_f() = default;
        point_f(float x_, float y_);
        explicit point_f(const size_f &sz);

        point_f offset(float dx, float dy) const;
        point_f round() const;
        bool is_empty() const;
        float distance(const point_f &o) const;
        bool equals(const point_f &o, float epsilon) const;
        std::string to_string() const;

        static bool try_parse(std::string_view value, point_f &out);

        explicit operator size_f() const;
        operator point() const; // implicit widening to double (matches C#)
    };

    bool operator==(const point_f &a, const point_f &b);
    bool operator!=(const point_f &a, const point_f &b);
    point_f operator+(const point_f &pt, const size_f &sz);
    size_f operator-(const point_f &a, const point_f &b);
    point_f operator-(const point_f &pt, const size_f &sz);
}

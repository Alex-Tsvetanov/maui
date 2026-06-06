#pragma once
// maui::graphics::size_f  <=  Microsoft.Maui.Graphics.SizeF
//
// 2D size with float components. Ported from src/Graphics/src/Graphics/SizeF.cs. Converts to
// `point_f` and to its double sibling `size` (forward-declared here; cross-type bodies in size_f.cpp).
//
// Deliberate M0 deviation (port/STATUS.md): System.Numerics interop omitted. TODO: maui linalg type.

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct point_f;
    struct size;

    struct size_f
    {
        float width = 0;
        float height = 0;

        static const size_f zero;

        size_f() = default;
        explicit size_f(float size_);
        size_f(float width_, float height_);

        bool is_zero() const;
        bool equals(const size_f &o) const;
        std::string to_string() const;

        static bool try_parse(std::string_view value, size_f &out);

        explicit operator point_f() const;
        operator size() const; // implicit widening to double
    };

    bool operator==(const size_f &a, const size_f &b);
    bool operator!=(const size_f &a, const size_f &b);
    size_f operator+(const size_f &a, const size_f &b);
    size_f operator-(const size_f &a, const size_f &b);
    size_f operator*(const size_f &s, float v);
    size_f operator/(const size_f &s, float v);
}

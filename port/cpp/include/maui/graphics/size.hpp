#pragma once
// maui::graphics::size  <=  Microsoft.Maui.Graphics.Size
//
// 2D size with double components. Ported from src/Graphics/src/Graphics/Size.cs. Converts to
// `point` and to its float sibling `size_f` (forward-declared here; cross-type bodies in size.cpp).
//
// Deliberate M0 deviation (port/STATUS.md): System.Numerics interop omitted. TODO: maui linalg type.

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct point;
    struct size_f;

    struct size
    {
        double width = 0;
        double height = 0;

        static const size zero;

        size() = default;
        explicit size(double size_);
        size(double width_, double height_);

        bool is_zero() const;
        bool equals(const size &o) const;
        std::string to_string() const;

        static bool try_parse(std::string_view value, size &out);

        explicit operator point() const;
        operator size_f() const; // implicit narrowing to float
    };

    bool operator==(const size &a, const size &b);
    bool operator!=(const size &a, const size &b);
    size operator+(const size &a, const size &b);
    size operator-(const size &a, const size &b);
    size operator*(const size &s, double v);
    size operator/(const size &s, double v);
}

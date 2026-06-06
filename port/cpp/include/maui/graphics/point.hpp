#pragma once
// maui::graphics::point  <=  Microsoft.Maui.Graphics.Point
//
// 2D point with double components. Ported from src/Graphics/src/Graphics/Point.cs. Interconverts
// with its float sibling `point_f` and with `size`/`size_f`; those are forward-declared here and
// the cross-type bodies live in point.cpp.
//
// Deliberate M0 deviation (port/STATUS.md): System.Numerics interop omitted. TODO: maui linalg type.

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct size;
    struct size_f;
    struct point_f;

    struct point
    {
        double x = 0;
        double y = 0;

        static const point zero;

        point() = default;
        point(double x_, double y_);
        explicit point(const size &sz);
        explicit point(const size_f &sz);

        point offset(double dx, double dy) const;
        point round() const;
        bool is_empty() const;
        double distance(const point &o) const;
        bool equals(const point &o, double epsilon) const;
        std::string to_string() const;

        static bool try_parse(std::string_view value, point &out);

        explicit operator size() const;
        operator point_f() const; // implicit narrowing to float (matches C#)
    };

    bool operator==(const point &a, const point &b);
    bool operator!=(const point &a, const point &b);
    point operator+(const point &pt, const size_f &sz);
    size operator-(const point &a, const point &b);
    point operator-(const point &pt, const size_f &sz);
}

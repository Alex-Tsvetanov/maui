#pragma once
// maui::graphics::rect  <=  Microsoft.Maui.Graphics.Rect
//
// Axis-aligned rectangle with double components. Ported from src/Graphics/src/Graphics/Rect.cs.
// Builds on point/size and interconverts with its float sibling `rect_f` (forward-declared here;
// cross-type bodies in rect.cpp). Naming: C# `Union` -> `union_with` (`union` is a C++ keyword).
// The `size()` accessor shadows the `size` type within the struct, so `size`-typed members are
// written `::maui::graphics::size`.

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct point;
    struct size;
    struct rect_f;

    struct rect
    {
        double x = 0;
        double y = 0;
        double width = 0;
        double height = 0;

        static const rect zero;

        rect() = default;
        rect(double x_, double y_, double w, double h);
        rect(const point &loc, const ::maui::graphics::size &sz);

        static rect from_ltrb(double left_, double top_, double right_, double bottom_);

        bool equals(const rect &o) const;

        double left() const;
        double top() const;
        double right() const;
        double bottom() const;
        void set_left(double v);
        void set_top(double v);
        void set_right(double v);
        void set_bottom(double v);

        bool is_empty() const;

        ::maui::graphics::size size() const;
        void set_size(const ::maui::graphics::size &v);
        point location() const;
        void set_location(const point &v);
        point center() const;

        bool contains(const rect &r) const;
        bool contains(const point &pt) const;
        bool contains(double px, double py) const;
        bool intersects_with(const rect &r) const;

        rect union_with(const rect &r) const;
        static rect union_with(const rect &r1, const rect &r2);
        rect intersect(const rect &r) const;
        static rect intersect(const rect &r1, const rect &r2);

        rect inflate(const ::maui::graphics::size &sz) const;
        rect inflate(double w, double h) const;
        rect offset(double dx, double dy) const;
        rect offset(const point &dr) const;
        rect round() const;

        std::string to_string() const;

        static bool try_parse(std::string_view value, rect &out);

        operator rect_f() const; // implicit narrowing to float
    };

    bool operator==(const rect &a, const rect &b);
    bool operator!=(const rect &a, const rect &b);
}

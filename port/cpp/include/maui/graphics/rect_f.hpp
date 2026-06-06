#pragma once
// maui::graphics::rect_f  <=  Microsoft.Maui.Graphics.RectF
//
// Axis-aligned rectangle with float components. Ported from src/Graphics/src/Graphics/RectF.cs.
// Builds on point_f/size_f and interconverts with its double sibling `rect` (forward-declared here;
// cross-type bodies in rect_f.cpp). Naming: C# `Union` -> `union_with` (`union` is a C++ keyword).

#include <string>
#include <string_view>

namespace maui::graphics
{
    struct point_f;
    struct size_f;
    struct rect;

    struct rect_f
    {
        float x = 0;
        float y = 0;
        float width = 0;
        float height = 0;

        static const rect_f zero;

        rect_f() = default;
        rect_f(float x_, float y_, float w, float h);
        rect_f(const point_f &loc, const size_f &sz);

        static rect_f from_ltrb(float left_, float top_, float right_, float bottom_);

        bool equals(const rect_f &o) const;

        // edges (getters + setters mirror the C# get/set properties)
        float left() const;
        float top() const;
        float right() const;
        float bottom() const;
        void set_left(float v);
        void set_top(float v);
        void set_right(float v);
        void set_bottom(float v);

        bool is_empty() const;

        size_f size() const;
        void set_size(const size_f &v);
        point_f location() const;
        void set_location(const point_f &v);
        point_f center() const;

        bool contains(const rect_f &r) const;
        bool contains(const point_f &pt) const;
        bool contains(float px, float py) const;
        bool intersects_with(const rect_f &r) const;

        rect_f union_with(const rect_f &r) const;
        static rect_f union_with(const rect_f &r1, const rect_f &r2);
        rect_f intersect(const rect_f &r) const;
        static rect_f intersect(const rect_f &r1, const rect_f &r2);

        rect_f inflate(const size_f &sz) const;
        rect_f inflate(float w, float h) const;
        rect_f offset(float dx, float dy) const;
        rect_f offset(const point_f &dr) const;
        rect_f round() const;

        std::string to_string() const;

        static bool try_parse(std::string_view value, rect_f &out);

        operator rect() const; // implicit widening to double
    };

    bool operator==(const rect_f &a, const rect_f &b);
    bool operator!=(const rect_f &a, const rect_f &b);
}

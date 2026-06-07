// Parsing cases ported from src/Graphics/tests/Graphics.Tests/RectTypeConverterTests.cs
// (converters delegate to T::TryParse). Plus characterization of the rect API from Rect.cs/RectF.cs.

#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include <array>
#include <gtest/gtest.h>

using maui::graphics::point_f;
using maui::graphics::rect;
using maui::graphics::rect_f;
using maui::graphics::size_f;

namespace
{
    struct rcase
    {
        const char* from;
        bool ok;
        double x, y, w, h;
    };
    constexpr auto k_cases = std::to_array<rcase>({
        {.from = "1,2,3,4", .ok = true, .x = 1, .y = 2, .w = 3, .h = 4},
        {.from = "1.1,2.0,3,4.4", .ok = true, .x = 1.1, .y = 2.0, .w = 3, .h = 4.4},
        {.from = "0,0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "0, 0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "0,  0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "1,2, 0, 0", .ok = true, .x = 1, .y = 2, .w = 0, .h = 0},
        {.from = "1, 2, 0, 0", .ok = true, .x = 1, .y = 2, .w = 0, .h = 0},
        {.from = "1,  2, 0, 0", .ok = true, .x = 1, .y = 2, .w = 0, .h = 0},
        {.from = "0.0,0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "0.0, 0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "0.0,  0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "1.1,2.1, 0, 0", .ok = true, .x = 1.1, .y = 2.1, .w = 0, .h = 0},
        {.from = "1.1, 2.1, 0, 0", .ok = true, .x = 1.1, .y = 2.1, .w = 0, .h = 0},
        {.from = "1.1,  2.1, 0, 0", .ok = true, .x = 1.1, .y = 2.1, .w = 0, .h = 0},
        {.from = "0,-0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "-0, 0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "-0,  -0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "-1,2, 0, 0", .ok = true, .x = -1, .y = 2, .w = 0, .h = 0},
        {.from = "1, -2, 0, 0", .ok = true, .x = 1, .y = -2, .w = 0, .h = 0},
        {.from = "-1,  -2, 0, 0", .ok = true, .x = -1, .y = -2, .w = 0, .h = 0},
        {.from = "-0.0,0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "0.0, -0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "-0.0,  -0.0, 0, 0", .ok = true, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "-1.1,2.1, 0, 0", .ok = true, .x = -1.1, .y = 2.1, .w = 0, .h = 0},
        {.from = "1.1, -2.1, 0, 0", .ok = true, .x = 1.1, .y = -2.1, .w = 0, .h = 0},
        {.from = "-1.1,  -2.1, 0, 0", .ok = true, .x = -1.1, .y = -2.1, .w = 0, .h = 0},
        {.from = ".1,0, 0, 0", .ok = true, .x = 0.1, .y = 0, .w = 0, .h = 0},
        {.from = "-.1, 1, 0, 0", .ok = true, .x = -0.1, .y = 1, .w = 0, .h = 0},
        {.from = "1, \t 1, 0, 0", .ok = true, .x = 1, .y = 1, .w = 0, .h = 0},
        {.from = "0", .ok = false, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = ",1, 0, 0", .ok = false, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "1,, 0, 0", .ok = false, .x = 0, .y = 0, .w = 0, .h = 0},
        {.from = "", .ok = false, .x = 0, .y = 0, .w = 0, .h = 0},
    });
} // namespace

TEST(rect_converter_tests, rect_from_string)
{
    for (const auto& c : k_cases)
    {
        rect out;
        EXPECT_EQ(c.ok, rect::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(rect(c.x, c.y, c.w, c.h), out) << "'" << c.from << "'";
        }
    }
}

TEST(rect_converter_tests, rect_f_from_string)
{
    for (const auto& c : k_cases)
    {
        rect_f out;
        EXPECT_EQ(c.ok, rect_f::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(rect_f((float)c.x, (float)c.y, (float)c.w, (float)c.h), out) << "'" << c.from << "'";
        }
    }
}

// ---- characterization of the rect API (derived from Rect.cs / RectF.cs) ----

TEST(rect_api, edges_and_geometry)
{
    rect_f r(1, 2, 3, 4);
    EXPECT_FLOAT_EQ(1, r.left());
    EXPECT_FLOAT_EQ(2, r.top());
    EXPECT_FLOAT_EQ(4, r.right());  // x + width
    EXPECT_FLOAT_EQ(6, r.bottom()); // y + height
    EXPECT_EQ(size_f(3, 4), r.size());
    EXPECT_EQ(point_f(1, 2), r.location());
    EXPECT_EQ(point_f(2.5F, 4), r.center()); // (x+w/2, y+h/2)
    EXPECT_FALSE(r.is_empty());
    EXPECT_TRUE(rect_f(0, 0, 0, 5).is_empty()); // width <= 0

    rect_f s = r;
    s.set_right(10);
    EXPECT_FLOAT_EQ(9, s.width); // width = value - x
}

TEST(rect_api, from_ltrb_and_construct)
{
    EXPECT_EQ(rect_f(0, 0, 10, 20), rect_f::from_ltrb(0, 0, 10, 20));
    EXPECT_EQ(rect_f(5, 5, 2, 3), rect_f(point_f(5, 5), size_f(2, 3)));
}

TEST(rect_api, contains)
{
    rect_f r(0, 0, 10, 10);
    EXPECT_TRUE(r.contains(point_f(5, 5)));
    EXPECT_TRUE(r.contains(0, 0));    // inclusive at left/top
    EXPECT_FALSE(r.contains(10, 10)); // exclusive at right/bottom
    EXPECT_TRUE(r.contains(rect_f(2, 2, 3, 3)));
    EXPECT_FALSE(r.contains(rect_f(5, 5, 10, 10)));
}

TEST(rect_api, intersects_union_intersect)
{
    rect_f a(0, 0, 10, 10);
    rect_f b(5, 5, 10, 10);
    rect_f disjoint(20, 20, 5, 5);

    EXPECT_TRUE(a.intersects_with(b));
    EXPECT_FALSE(a.intersects_with(disjoint));

    EXPECT_EQ(rect_f(0, 0, 15, 15), a.union_with(b)); // FromLTRB(0,0,15,15)
    EXPECT_EQ(rect_f(5, 5, 5, 5), a.intersect(b));    // overlap
    EXPECT_EQ(rect_f::zero, a.intersect(disjoint));   // no overlap -> Zero
}

TEST(rect_api, inflate_offset_round)
{
    EXPECT_EQ(rect_f(3, 2, 14, 16), rect_f(5, 5, 10, 10).inflate(2, 3)); // x-=w, y-=h, w+=2w, h+=2h
    EXPECT_EQ(rect_f(7, 8, 10, 10), rect_f(5, 5, 10, 10).offset(2, 3));
    EXPECT_EQ(rect_f(0, 2, 2, 4), rect_f(0.5F, 1.5F, 2.5F, 3.5F).round()); // ties-to-even: .5->even
}

TEST(rect_api, cross_precision_casts)
{
    rect_f rf(1.5F, 2.5F, 3.5F, 4.5F);
    rect rd = rf; // implicit widening
    EXPECT_DOUBLE_EQ(1.5, rd.x);
    EXPECT_DOUBLE_EQ(4.5, rd.height);

    rect back_src(1.5, 2.5, 3.5, 4.5);
    rect_f back = back_src; // implicit narrowing
    EXPECT_FLOAT_EQ(1.5F, back.x);
    EXPECT_FLOAT_EQ(4.5F, back.height);
}

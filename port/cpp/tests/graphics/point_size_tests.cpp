// Parsing cases ported from src/Graphics/tests/Graphics.Tests/{Point,Size}TypeConverterTests.cs.
// (The converters delegate to T::TryParse, so we exercise try_parse directly.)
// Plus a small characterization block for the cross-type API the C# oracle doesn't test.
#include "graphics_test_support.hpp"

#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"

using maui::graphics::point;
using maui::graphics::point_f;
using maui::graphics::size;
using maui::graphics::size_f;

namespace
{
    // One table drives all four types: {input, expectedSuccess, x/width, y/height}.
    struct parse_case
    {
        const char *from;
        bool ok;
        double a;
        double b;
    };
    constexpr parse_case k_cases[] = {
        {"0,0", true, 0, 0},
        {"0, 0", true, 0, 0},
        {"0,  0", true, 0, 0},
        {"1,2", true, 1, 2},
        {"1, 2", true, 1, 2},
        {"1,  2", true, 1, 2},
        {"0.0,0.0", true, 0, 0},
        {"0.0, 0.0", true, 0, 0},
        {"0.0,  0.0", true, 0, 0},
        {"1.1,2.1", true, 1.1, 2.1},
        {"1.1, 2.1", true, 1.1, 2.1},
        {"1.1,  2.1", true, 1.1, 2.1},
        {"0,-0", true, 0, 0},
        {"-0, 0", true, 0, 0},
        {"-0,  -0", true, 0, 0},
        {"-1,2", true, -1, 2},
        {"1, -2", true, 1, -2},
        {"-1,  -2", true, -1, -2},
        {"-0.0,0.0", true, 0, 0},
        {"0.0, -0.0", true, 0, 0},
        {"-0.0,  -0.0", true, 0, 0},
        {"-1.1,2.1", true, -1.1, 2.1},
        {"1.1, -2.1", true, 1.1, -2.1},
        {"-1.1,  -2.1", true, -1.1, -2.1},
        {".1,0", true, 0.1, 0},
        {"-.1, 1", true, -0.1, 1},
        {"1, \t 1", true, 1, 1},
        {"0", false, 0, 0},
        {",1", false, 0, 0},
        {"1,", false, 0, 0},
        {"", false, 0, 0},
    };
} // namespace

TEST(point_converter_tests, point_from_string)
{
    for (const auto &c : k_cases)
    {
        point out;
        EXPECT_EQ(c.ok, point::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(point(c.a, c.b), out) << "'" << c.from << "'";
        }
    }
}

TEST(point_converter_tests, point_f_from_string)
{
    for (const auto &c : k_cases)
    {
        point_f out;
        EXPECT_EQ(c.ok, point_f::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(point_f(static_cast<float>(c.a), static_cast<float>(c.b)), out) << "'" << c.from << "'";
        }
    }
}

TEST(size_converter_tests, size_from_string)
{
    for (const auto &c : k_cases)
    {
        size out;
        EXPECT_EQ(c.ok, size::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(size(c.a, c.b), out) << "'" << c.from << "'";
        }
    }
}

TEST(size_converter_tests, size_f_from_string)
{
    for (const auto &c : k_cases)
    {
        size_f out;
        EXPECT_EQ(c.ok, size_f::try_parse(c.from, out)) << "'" << c.from << "'";
        if (c.ok)
        {
            EXPECT_EQ(size_f(static_cast<float>(c.a), static_cast<float>(c.b)), out) << "'" << c.from << "'";
        }
    }
}

// ---- characterization of the cross-type API (derived from the C# source) ----

TEST(point_size_api, point_f_basics)
{
    EXPECT_EQ(point_f(0, 0), point_f::zero);
    EXPECT_TRUE(point_f(0, 0).is_empty());
    EXPECT_FALSE(point_f(1, 0).is_empty());
    EXPECT_EQ(point_f(3, 5), point_f(1, 2).offset(2, 3));
    EXPECT_FLOAT_EQ(5.0f, point_f(0, 0).distance(point_f(3, 4)));
    EXPECT_TRUE(point_f(1.0f, 2.0f).equals(point_f(1.001f, 2.0f), 0.01f));
    EXPECT_FALSE(point_f(1.0f, 2.0f).equals(point_f(1.1f, 2.0f), 0.01f));
    EXPECT_EQ(point_f(2, 4), point_f(2.5f, 3.5f).round()); // MathF.Round ties-to-even: 2.5->2, 3.5->4
}

TEST(point_size_api, point_basics)
{
    EXPECT_TRUE(point::zero.is_empty());
    EXPECT_EQ(point(3, 5), point(1, 2).offset(2, 3));
    EXPECT_DOUBLE_EQ(5.0, point(0, 0).distance(point(3, 4)));
}

TEST(point_size_api, cross_precision_casts)
{
    point_f pf(1.5f, 2.5f);
    point p = pf; // implicit widening
    EXPECT_DOUBLE_EQ(1.5, p.x);
    EXPECT_DOUBLE_EQ(2.5, p.y);

    point pd(1.5, 2.5);
    point_f back = pd; // implicit narrowing (matches C#)
    EXPECT_FLOAT_EQ(1.5f, back.x);
    EXPECT_FLOAT_EQ(2.5f, back.y);
}

TEST(point_size_api, point_size_conversions_and_operators)
{
    // point_f <-> size_f (explicit) and mixed operators
    EXPECT_EQ(size_f(3, 4), static_cast<size_f>(point_f(3, 4)));
    EXPECT_EQ(point_f(4, 6), point_f(1, 2) + size_f(3, 4));
    EXPECT_EQ(size_f(2, 2), point_f(3, 4) - point_f(1, 2));
    EXPECT_EQ(point_f(1, 2), point_f(4, 6) - size_f(3, 4));

    // size_f arithmetic + size_f <-> size
    EXPECT_EQ(size_f(4, 6), size_f(1, 2) + size_f(3, 4));
    EXPECT_EQ(size_f(2, 4), size_f(1, 2) * 2.0f);
    EXPECT_TRUE(size_f(0, 0).is_zero());
    size widened = size_f(1.5f, 2.5f); // implicit
    EXPECT_DOUBLE_EQ(1.5, widened.width);

    // double family
    EXPECT_EQ(size(2, 2), point(3, 4) - point(1, 2));
    EXPECT_EQ(point(4, 6), point(1, 2) + size_f(3, 4));
}

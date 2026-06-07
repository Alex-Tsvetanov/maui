// Ported from src/Graphics/tests/Graphics.Tests/ColorUnitTests.cs (xUnit -> GoogleTest).
// Behavioral oracle for maui::graphics::color. C# assertion notes inline.
#include "graphics_test_support.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <set>
#include <stdexcept>

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"

using maui::graphics::color;
namespace colors = maui::graphics::colors;

TEST(color_tests, hsl_post_set_equality)
{
    color c(1, 0.5F, 0.2F);
    color c2 = c.with_luminosity(.2f);
    EXPECT_FALSE(c == c2);
}

TEST(color_tests, hsl_post_set_inequality)
{
    color c(1, 0.5F, 0.2F);
    color c2 = c.with_luminosity(.2f);
    EXPECT_TRUE(c != c2);
}

TEST(color_tests, hsl_set_to_default_value)
{
    color c(0.2F, 0.5F, 0.8F);
    c = c.with_saturation(0); // saturation 0 -> gray
    EXPECT_FLOAT_EQ(c.red, c.green);
    EXPECT_FLOAT_EQ(c.red, c.blue);
}

TEST(color_tests, hsl_modifiers)
{
    color c = color::from_hsla(.8f, .6f, .2f);
    EXPECT_EQ(color::from_hsla(.1f, .6f, .2f), c.with_hue(.1f));
    EXPECT_EQ(color::from_hsla(.8f, .1f, .2f), c.with_saturation(.1f));
    EXPECT_EQ(color::from_hsla(.8f, .6f, .1f), c.with_luminosity(.1f));
}

TEST(color_tests, multiply_alpha)
{
    color c(1.F, 1.F, 1.F, 1.F);
    c = c.multiply_alpha(0.25F);
    EXPECT_FLOAT_EQ(0.25F, c.alpha);

    c = color::from_hsla(1.F, 1.F, 1.F, 1.F);
    c = c.multiply_alpha(0.25F);
    EXPECT_FLOAT_EQ(0.25F, c.alpha);
}

TEST(color_tests, clamping)
{
    color hi(2.F, 2.F, 2.F, 2.F);
    EXPECT_FLOAT_EQ(1, hi.red);
    EXPECT_FLOAT_EQ(1, hi.green);
    EXPECT_FLOAT_EQ(1, hi.blue);
    EXPECT_FLOAT_EQ(1, hi.alpha);

    color lo(-1.F, -1.F, -1.F, -1.F);
    EXPECT_FLOAT_EQ(0, lo.red);
    EXPECT_FLOAT_EQ(0, lo.green);
    EXPECT_FLOAT_EQ(0, lo.blue);
    EXPECT_FLOAT_EQ(0, lo.alpha);
}

TEST(color_tests, rgb_to_hsl)
{
    color c(.5f, .1f, .1f);
    MAUI_EXPECT_TOL(1.0F, c.get_hue(), 3.0F);       // C#: Assert.Equal((float)1, hue, (float)3) — loose
    MAUI_EXPECT_PREC(0.662, c.get_saturation(), 1); // C#: Assert.Equal(0.662, .., 1)
    MAUI_EXPECT_PREC(0.302, c.get_luminosity(), 1);
}

TEST(color_tests, hsl_to_rgb)
{
    color c = color::from_hsla(0, .662, .302); // double overload
    MAUI_EXPECT_PREC(0.5, c.red, 2);
    MAUI_EXPECT_PREC(0.1, c.green, 2);
    MAUI_EXPECT_PREC(0.1, c.blue, 2);
}

TEST(color_tests, color_from_value)
{
    color c(0.2F);
    EXPECT_EQ(color(0.2F, 0.2F, 0.2F, 1), c);
}

TEST(color_tests, add_luminosity)
{
    color c(0.2F);
    color brighter = c.add_luminosity(0.2F);
    MAUI_EXPECT_PREC(brighter.get_luminosity(), c.get_luminosity() + 0.2, 3);
}

TEST(color_tests, zero_luminosity)
{
    color c(0.1F, 0.2F, 0.3F);
    c = c.add_luminosity(-1);
    EXPECT_FLOAT_EQ(0, c.get_luminosity());
    EXPECT_FLOAT_EQ(0, c.red);
    EXPECT_FLOAT_EQ(0, c.green);
    EXPECT_FLOAT_EQ(0, c.blue);
}

TEST(color_tests, hash_code)
{
    color c1(0.1F);
    color c2(0.1F);
    EXPECT_EQ(std::hash<color>{}(c1), std::hash<color>{}(c2));
}

TEST(color_tests, hash_code_named_colors)
{
    EXPECT_NE(std::hash<color>{}(colors::red), std::hash<color>{}(colors::blue));
}

TEST(color_tests, hash_code_all)
{
    // C# adds each hash as a Dictionary key (throws on collision). Here: assert all distinct.
    std::set<std::size_t> seen;
    auto add = [&](const color& c) { EXPECT_TRUE(seen.insert(std::hash<color>{}(c)).second) << "hash collision"; };
    add(colors::transparent);
    add(colors::aqua);
    add(colors::black);
    add(colors::blue);
    add(colors::fuchsia);
    add(colors::gray);
    add(colors::green);
    add(colors::lime);
    add(colors::maroon);
    add(colors::navy);
    add(colors::olive);
    add(colors::purple);
    add(colors::pink);
    add(colors::red);
    add(colors::silver);
    add(colors::teal);
    add(colors::yellow);
}

TEST(color_tests, set_hue)
{
    color c(0.2F, 0.5F, 0.7F);
    c = color::from_hsla(.2f, c.get_saturation(), c.get_luminosity());
    MAUI_EXPECT_TOL(0.6F, c.red, 3.0F); // C#: tolerance 3f (loose)
    MAUI_EXPECT_TOL(0.7F, c.green, 3.0F);
    MAUI_EXPECT_TOL(0.2F, c.blue, 3.0F);
}

TEST(color_tests, zero_lumin_to_rgb)
{
    color c(0);
    EXPECT_FLOAT_EQ(0, c.get_luminosity());
    EXPECT_FLOAT_EQ(0, c.get_hue());
    EXPECT_FLOAT_EQ(0, c.get_saturation());
}

TEST(color_tests, to_string)
{
    color c(1, 1, 1, 0.5F);
    EXPECT_EQ("[Color: Red=1, Green=1, Blue=1, Alpha=0.5]", c.to_string());
}

TEST(color_tests, from_hex)
{
    color c = color::from_rgb(138, 43, 226);
    EXPECT_EQ(c, color::from_argb("8a2be2"));
    EXPECT_EQ(color::from_rgba(138, 43, 226, 128), color::from_argb("#808a2be2"));
    EXPECT_EQ(color::from_argb("#aabbcc"), color::from_argb("#abc"));
    EXPECT_EQ(color::from_argb("#aabbccdd"), color::from_argb("#abcd"));
}

TEST(color_tests, to_hex)
{
    color color_rgb = color::from_rgb(138, 43, 226);
    EXPECT_EQ(color::from_argb(color_rgb.to_argb_hex()), color_rgb);

    color color_rgba = color::from_rgba(138, 43, 226, .2); // double alpha -> floating overload
    EXPECT_EQ(color::from_argb(color_rgba.to_argb_hex()), color_rgba);

    color color_hsl = color::from_hsla(240.0F, 1.0F, 1.0F); // explicit floats (C# picks float overload)
    EXPECT_EQ(color::from_argb(color_hsl.to_argb_hex()), color_hsl);

    color color_hsla = color::from_hsla(240.0F, 1.0F, 1.0F, .1f);
    color hex_from_hsla = color::from_argb(color_hsla.to_argb_hex());
    MAUI_EXPECT_TOL(hex_from_hsla.alpha, color_hsla.alpha, 2.0F); // C#: tolerance 2f/3f (loose)
    MAUI_EXPECT_TOL(hex_from_hsla.red, color_hsla.red, 3.0F);
    MAUI_EXPECT_TOL(hex_from_hsla.green, color_hsla.green, 3.0F);
    MAUI_EXPECT_TOL(hex_from_hsla.blue, color_hsla.blue, 3.0F);
}

TEST(color_tests, from_hsv)
{
    color c = color::from_rgb(1, .29f, .752f);   // floating overload (direct)
    color c_hsv = color::from_hsv(321, 71, 100); // integer overload (degrees/percent)
    MAUI_EXPECT_TOL(c.red, c_hsv.red, 3.0F);
    MAUI_EXPECT_TOL(c.green, c_hsv.green, 3.0F);
    MAUI_EXPECT_TOL(c.blue, c_hsv.blue, 3.0F);
}

TEST(color_tests, from_hsva)
{
    color c = color::from_rgba(1, .29, .752, .5);
    color c_hsv = color::from_hsva(321, 71, 100, 50);
    MAUI_EXPECT_TOL(c.red, c_hsv.red, 3.0F);
    MAUI_EXPECT_TOL(c.green, c_hsv.green, 3.0F);
    MAUI_EXPECT_TOL(c.blue, c_hsv.blue, 3.0F);
    MAUI_EXPECT_TOL(c.alpha, c_hsv.alpha, 3.0F);
}

TEST(color_tests, from_hsv_double)
{
    color c = color::from_rgb(1, .29f, .758f);
    color c_hsv = color::from_hsv(.89f, .71f, 1); // floating overload
    MAUI_EXPECT_TOL(c.red, c_hsv.red, 2.0F);
    MAUI_EXPECT_TOL(c.green, c_hsv.green, 2.0F);
    MAUI_EXPECT_TOL(c.blue, c_hsv.blue, 2.0F);
}

TEST(color_tests, from_hsva_double)
{
    color c = color::from_rgba(1, .29, .758, .5);
    color c_hsv = color::from_hsva(.89f, .71f, 1.F, .5f);
    MAUI_EXPECT_TOL(c.red, c_hsv.red, 2.0F);
    MAUI_EXPECT_TOL(c.green, c_hsv.green, 2.0F);
    MAUI_EXPECT_TOL(c.blue, c_hsv.blue, 2.0F);
    MAUI_EXPECT_TOL(c.alpha, c_hsv.alpha, 2.0F);
}

TEST(color_tests, from_rgb_double)
{
    color c = color::from_rgb(0.2, 0.3, 0.4);
    EXPECT_EQ(color(0.2F, 0.3F, 0.4F), c);
}

TEST(color_tests, from_rgba_double)
{
    color c = color::from_rgba(0.2, 0.3, 0.4, 0.5);
    EXPECT_EQ(color(0.2F, 0.3F, 0.4F, 0.5F), c);
}

TEST(color_tests, default_colors_match)
{
    EXPECT_EQ(colors::cornflower_blue, color::from_rgb(100, 149, 237));
    EXPECT_EQ(colors::dark_salmon, color::from_rgb(233, 150, 122));
    EXPECT_EQ(colors::transparent, color::from_rgba(0, 0, 0, 0));
    EXPECT_EQ(colors::wheat, color::from_rgb(245, 222, 179));
    EXPECT_EQ(colors::white, color::from_rgb(255, 255, 255));
}

TEST(color_tests, from_uint)
{
    color expected(1, 0.65F, 0, 1);
    int const blue = static_cast<int>(expected.blue * 255);
    int const red = static_cast<int>(expected.red * 255);
    int const green = static_cast<int>(expected.green * 255);
    int const alpha = static_cast<int>(expected.alpha * 255);
    auto argb = static_cast<std::uint32_t>(blue | (green << 8) | (red << 16) | (alpha << 24));
    color from_uint = color::from_uint(argb);
    MAUI_EXPECT_TOL(expected.alpha, from_uint.alpha, 2.0F);
    MAUI_EXPECT_TOL(expected.red, from_uint.red, 2.0F);
    MAUI_EXPECT_TOL(expected.green, from_uint.green, 2.0F);
    MAUI_EXPECT_TOL(expected.blue, from_uint.blue, 2.0F);
}

TEST(color_tests, to_uint)
{
    color c = color::from_rgba(255, 122, 15, 255);
    EXPECT_EQ(4294933007U, c.to_uint());
}

TEST(color_tests, get_complementary)
{
    struct test_case
    {
        const char* original{};
        const char* expected{};
    };
    const std::array cases = {
        test_case{.original = "#FF0000", .expected = "#00FFFF"}, // red & cyan
        test_case{.original = "#00FF00", .expected = "#FF00FF"}, // green & fuchsia
        test_case{.original = "#0000FF", .expected = "#FFFF00"}, // blue & yellow
        test_case{.original = "#0AF56C", .expected = "#F50A93"}, // lime green & bright purple
    };
    for (const auto& tc : cases)
    {
        color orig = color::from_argb(tc.original);
        color expected_comp = color::from_argb(tc.expected);
        color comp = orig.get_complementary();
        MAUI_EXPECT_TOL(expected_comp.alpha, comp.alpha, 3.0F); // C#: tolerance 3f (loose)
        MAUI_EXPECT_TOL(expected_comp.red, comp.red, 3.0F);
        MAUI_EXPECT_TOL(expected_comp.green, comp.green, 3.0F);
        MAUI_EXPECT_TOL(expected_comp.blue, comp.blue, 3.0F);
    }
}

TEST(color_tests, from_rgba_string)
{
    struct test_case
    {
        const char* value{};
        color expected;
    };
    const std::array cases = {
        test_case{.value = "#111", .expected = color::from_rgb(0x11, 0x11, 0x11)},
        test_case{.value = "#a222", .expected = color::from_rgba(0xaa, 0x22, 0x22, 0x22)},
        test_case{.value = "#F2E2D2", .expected = color::from_rgb(0xF2, 0xE2, 0xD2)},
        test_case{.value = "#C2F2E2D2", .expected = color::from_rgba(0xC2, 0xF2, 0xE2, 0xD2)},
        test_case{.value = "111", .expected = color::from_rgb(0x11, 0x11, 0x11)},
        test_case{.value = "a222", .expected = color::from_rgba(0xaa, 0x22, 0x22, 0x22)},
        test_case{.value = "F2E2D2", .expected = color::from_rgb(0xF2, 0xE2, 0xD2)},
        test_case{.value = "C2F2E2D2", .expected = color::from_rgba(0xC2, 0xF2, 0xE2, 0xD2)},
    };
    for (const auto& tc : cases)
    {
        EXPECT_EQ(tc.expected, color::from_rgba(tc.value)) << tc.value;
    }
}

TEST(color_tests, from_argb_string)
{
    struct test_case
    {
        const char* value{};
        color expected;
    };
    const std::array cases = {
        test_case{.value = "#111", .expected = color::from_rgb(0x11, 0x11, 0x11)},
        test_case{.value = "#a222", .expected = color::from_rgba(0x22, 0x22, 0x22, 0xaa)},
        test_case{.value = "#F2E2D2", .expected = color::from_rgb(0xF2, 0xE2, 0xD2)},
        test_case{.value = "#C2F2E2D2", .expected = color::from_rgba(0xF2, 0xE2, 0xD2, 0xC2)},
        test_case{.value = "#000000", .expected = color::from_rgba(0x00, 0x00, 0x00, 0xFF)},
        test_case{.value = "#000", .expected = color::from_rgba(0x00, 0x00, 0x00, 0xFF)},
        test_case{.value = "#00FFff 40%",
                  .expected = color::from_rgba(0.F, 0.F, 0.F, 1.F)}, // unsupported -> black, no throw
        test_case{.value = "111", .expected = color::from_rgb(0x11, 0x11, 0x11)},
        test_case{.value = "a222", .expected = color::from_rgba(0x22, 0x22, 0x22, 0xaa)},
        test_case{.value = "F2E2D2", .expected = color::from_rgb(0xF2, 0xE2, 0xD2)},
        test_case{.value = "C2F2E2D2", .expected = color::from_rgba(0xF2, 0xE2, 0xD2, 0xC2)},
    };
    for (const auto& tc : cases)
    {
        EXPECT_EQ(tc.expected, color::from_argb(tc.value)) << tc.value;
    }
}

TEST(color_tests, parse_valid)
{
    struct test_case
    {
        const char* value{};
        color expected;
    };
    const std::array cases = {
        // from TestFromArgbValuesHash
        test_case{.value = "#111", .expected = color::from_rgb(0x11, 0x11, 0x11)},
        test_case{.value = "#a222", .expected = color::from_rgba(0x22, 0x22, 0x22, 0xaa)},
        test_case{.value = "#F2E2D2", .expected = color::from_rgb(0xF2, 0xE2, 0xD2)},
        test_case{.value = "#C2F2E2D2", .expected = color::from_rgba(0xF2, 0xE2, 0xD2, 0xC2)},
        test_case{.value = "#000000", .expected = color::from_rgba(0x00, 0x00, 0x00, 0xFF)},
        test_case{.value = "#000", .expected = color::from_rgba(0x00, 0x00, 0x00, 0xFF)},
        test_case{.value = "#00FFff 40%", .expected = color::from_rgba(0.F, 0.F, 0.F, 1.F)},
        // rgb / rgba
        test_case{.value = "rgb(255,0,0)", .expected = color::from_rgb(255, 0, 0)},
        test_case{.value = "rgb(100%, 0%, 0%)", .expected = color::from_rgb(255, 0, 0)},
        test_case{.value = "rgba(0, 255, 0, 0.7)", .expected = color::from_rgba(0, 255, 0, 0.7F)},
        test_case{.value = "rgba(0%, 100%, 0%, 0.7)", .expected = color::from_rgba(0, 255, 0, 0.7F)},
        // hsl / hsla
        test_case{.value = "hsl(120, 100%, 50%)", .expected = color::from_hsla(120.F / 360.F, 1.0F, .5F)},
        test_case{.value = "hsl(120, 75, 20%)", .expected = color::from_hsla(120.F / 360.F, .75F, .2F)},
        test_case{.value = "hsla(160, 100%, 50%, .4)", .expected = color::from_hsla(160.F / 360.F, 1.0F, .5F, .4F)},
        test_case{.value = "hsla(160,100%,50%,.6)", .expected = color::from_hsla(160.F / 360.F, 1.0F, .5F, .6F)},
        // hsv / hsva
        test_case{.value = "hsv(120, 85%, 35%)", .expected = color::from_hsv(120.F / 360.F, .85F, .35F)},
        test_case{.value = "hsv(120, 85, 35)", .expected = color::from_hsv(120.F / 360.F, .85F, .35F)},
        test_case{.value = "hsva(120, 100%, 50%, .8)", .expected = color::from_hsva(120.F / 360.F, 1.0F, .5F, .8F)},
        test_case{.value = "hsva(120, 100, 50, .8)", .expected = color::from_hsva(120.F / 360.F, 1.0F, .5F, .8F)},
    };
    for (const auto& tc : cases)
    {
        color actual;
        EXPECT_TRUE(color::try_parse(tc.value, actual)) << tc.value;
        EXPECT_EQ(tc.expected, actual) << tc.value;
        EXPECT_EQ(tc.expected, color::parse(tc.value)) << tc.value;
    }
}

TEST(color_tests, parse_bad)
{
    // C#'s null InlineData behaves like empty here.
    const auto bad = std::to_array<const char*>(
        {"", "default", "notAColor", "#ZZZ", "#12g", "#1g3", "#zyxv", "222", "rgb)255,0,0(", "rgb255,0,0",
         "rgba(255, 0, 0, 0.8", "hsv(120, 100#, 50#)", "hsv(120%, 100%, 50%)", "hsva(120, 120%, 50%, a)"});
    for (const char* v : bad)
    {
        color c;
        EXPECT_FALSE(color::try_parse(v, c)) << v;
        EXPECT_THROW((void)color::parse(v), std::invalid_argument) << v;
    }
}

// Combines C#'s TestParseAllBuiltInColors + ConvertStandardValuesAreComplete: every named
// color parses (case-insensitively) back to its value. The parse table and the constants are
// generated from the same macro, so this guards them against drift.
TEST(color_tests, parse_all_built_in_colors)
{
    int count = 0;
#define MAUI_GRAPHICS_CHECK(name, str, argb)                                                                           \
    {                                                                                                                  \
        color c;                                                                                                       \
        EXPECT_TRUE(color::try_parse(str, c)) << (str);                                                                \
        EXPECT_EQ(color::from_uint(argb), c) << (str);                                                                 \
        ++count;                                                                                                       \
    }
    MAUI_GRAPHICS_NAMED_COLORS(MAUI_GRAPHICS_CHECK)
#undef MAUI_GRAPHICS__CHECK
    EXPECT_GT(count, 100);

    color c;
    EXPECT_TRUE(color::try_parse("CornflowerBlue", c)); // PascalCase field name (case-insensitive)
    EXPECT_EQ(colors::cornflower_blue, c);
    EXPECT_TRUE(color::try_parse("BLACK", c));
    EXPECT_EQ(colors::black, c);
}

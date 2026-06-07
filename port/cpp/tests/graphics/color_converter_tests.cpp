// Ported from src/Graphics/tests/Graphics.Tests/ColorTypeConverterTests.cs.
// The C# ColorTypeConverter just delegates to Color.Parse, so we exercise color::try_parse.

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include <array>
#include <gtest/gtest.h>

using maui::graphics::color;
namespace colors = maui::graphics::colors;

TEST(color_converter_tests, converts_from_string)
{
    struct test_case
    {
        const char* from{};
        color expected;
    };
    const std::array cases = {
        test_case{.from = "#000000", .expected = color()},
        test_case{.from = "#ff000000", .expected = color()},
        test_case{.from = "Black", .expected = color()},
        test_case{.from = "black", .expected = color()},
        test_case{.from = "rgb(0,0,0)", .expected = color()},
        test_case{.from = "rgba(0,0,0,255)", .expected = color()},
        test_case{.from = "rgba(0,0,0,0)", .expected = colors::transparent},
        test_case{.from = "hsl(0,0,0)", .expected = color()},
        test_case{.from = "hsla(0,0,0,1)", .expected = color()},
        test_case{.from = "hsla(0,0,0,0)", .expected = colors::transparent},
        test_case{.from = "hsv(0,0,0)", .expected = color()},
        test_case{.from = "hsva(0,0,0,1)", .expected = color()},
        test_case{.from = "hsva(0,0,0,0)", .expected = colors::transparent},
        test_case{.from = "hsl(253,66,50)", .expected = color::from_argb("#4F2BD3")},
        test_case{.from = "hsv(253,80,83)", .expected = color::from_argb("#4F2AD3")},
        test_case{.from = "rgb(81,43,212)", .expected = color::from_argb("#512BD4")},
    };
    for (const auto& tc : cases)
    {
        color c;
        bool const ok = color::try_parse(tc.from, c); // ColorTypeConverter.ConvertFrom -> Color.Parse
        EXPECT_TRUE(ok) << tc.from;
        if (ok)
        {
            EXPECT_EQ(tc.expected, c) << tc.from;
        }
    }
}

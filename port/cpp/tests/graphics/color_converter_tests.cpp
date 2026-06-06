// Ported from src/Graphics/tests/Graphics.Tests/ColorTypeConverterTests.cs.
// The C# ColorTypeConverter just delegates to Color.Parse, so we exercise color::try_parse.
#include "graphics_test_support.hpp"

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"

using maui::graphics::color;
namespace colors = maui::graphics::colors;

TEST(color_converter_tests, converts_from_string)
{
    struct
    {
        const char *from;
        color expected;
    } cases[] = {
        {"#000000", color()},
        {"#ff000000", color()},
        {"Black", color()},
        {"black", color()},
        {"rgb(0,0,0)", color()},
        {"rgba(0,0,0,255)", color()},
        {"rgba(0,0,0,0)", colors::transparent},
        {"hsl(0,0,0)", color()},
        {"hsla(0,0,0,1)", color()},
        {"hsla(0,0,0,0)", colors::transparent},
        {"hsv(0,0,0)", color()},
        {"hsva(0,0,0,1)", color()},
        {"hsva(0,0,0,0)", colors::transparent},
        {"hsl(253,66,50)", color::from_argb("#4F2BD3")},
        {"hsv(253,80,83)", color::from_argb("#4F2AD3")},
        {"rgb(81,43,212)", color::from_argb("#512BD4")},
    };
    for (const auto &tc : cases)
    {
        color c;
        bool ok = color::try_parse(tc.from, c); // ColorTypeConverter.ConvertFrom -> Color.Parse
        EXPECT_TRUE(ok) << tc.from;
        if (ok)
        {
            EXPECT_EQ(tc.expected, c) << tc.from;
        }
    }
}

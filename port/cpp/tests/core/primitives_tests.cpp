// Tests for the core primitive value types: thickness + font.
// Derived from the C# behavior in src/Core/src/Primitives/{Thickness,Font}.cs.
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"

#include <gtest/gtest.h>

namespace
{
    using maui::core::font;
    using maui::core::font_slant;
    using maui::core::font_weight;
    using maui::core::thickness;

    TEST(thickness, default_is_zero_and_empty)
    {
        thickness const t;
        EXPECT_EQ(t.left, 0.0);
        EXPECT_TRUE(t.is_empty());
        EXPECT_EQ(t, thickness::zero);
    }

    TEST(thickness, uniform_ctor)
    {
        thickness const t(5);
        EXPECT_EQ(t.left, 5.0);
        EXPECT_EQ(t.top, 5.0);
        EXPECT_EQ(t.right, 5.0);
        EXPECT_EQ(t.bottom, 5.0);
        EXPECT_EQ(t.horizontal_thickness(), 10.0);
        EXPECT_EQ(t.vertical_thickness(), 10.0);
        EXPECT_FALSE(t.is_empty());
    }

    TEST(thickness, horizontal_vertical_ctor)
    {
        thickness const t(2, 3);
        EXPECT_EQ(t.left, 2.0);
        EXPECT_EQ(t.top, 3.0);
        EXPECT_EQ(t.right, 2.0);
        EXPECT_EQ(t.bottom, 3.0);
    }

    TEST(thickness, full_ctor_and_structured_bindings)
    {
        thickness const t(1, 2, 3, 4);
        auto const [l, top, r, bottom] = t; // public members => deconstruct
        EXPECT_EQ(l, 1.0);
        EXPECT_EQ(top, 2.0);
        EXPECT_EQ(r, 3.0);
        EXPECT_EQ(bottom, 4.0);
    }

    TEST(thickness, implicit_from_double)
    {
        thickness const t = 7; // C#-style implicit uniform conversion
        EXPECT_EQ(t, thickness(7, 7, 7, 7));
    }

    TEST(thickness, operators)
    {
        EXPECT_EQ(thickness(1, 2, 3, 4) + 1.0, thickness(2, 3, 4, 5));
        EXPECT_EQ(thickness(1, 2, 3, 4) + thickness(10, 20, 30, 40), thickness(11, 22, 33, 44));
        EXPECT_EQ(thickness(5, 5, 5, 5) - 1.0, thickness(4, 4, 4, 4));
        EXPECT_NE(thickness(1), thickness(2));
    }

    TEST(font, default_font_is_default_and_regular)
    {
        font const f;
        EXPECT_TRUE(f.is_default());
        EXPECT_TRUE(f.family().empty());
        EXPECT_EQ(f.size(), 0.0);
        EXPECT_EQ(f.slant(), font_slant::normal);
        EXPECT_EQ(f.weight(), font_weight::regular); // unset weight reports as regular
        EXPECT_TRUE(f.auto_scaling_enabled());
    }

    TEST(font, default_font_factory)
    {
        EXPECT_TRUE(font::default_font().is_default());
        EXPECT_EQ(font::default_font().weight(), font_weight::regular);
    }

    TEST(font, of_size)
    {
        font const f = font::of_size("Arial", 12, font_weight::bold, font_slant::italic);
        EXPECT_EQ(f.family(), "Arial");
        EXPECT_EQ(f.size(), 12.0);
        EXPECT_EQ(f.weight(), font_weight::bold);
        EXPECT_EQ(f.slant(), font_slant::italic);
        EXPECT_FALSE(f.is_default());
    }

    TEST(font, system_fonts_have_no_family)
    {
        EXPECT_TRUE(font::system_font_of_size(14).family().empty());
        EXPECT_EQ(font::system_font_of_size(14).size(), 14.0);
        EXPECT_EQ(font::system_font_of_weight(font_weight::heavy).weight(), font_weight::heavy);
    }

    TEST(font, builders_return_modified_copies)
    {
        font const base = font::of_size("Arial", 12);
        EXPECT_EQ(base.with_size(20).size(), 20.0);
        EXPECT_EQ(base.with_weight(font_weight::bold).weight(), font_weight::bold);
        EXPECT_EQ(base.with_slant(font_slant::oblique).slant(), font_slant::oblique);
        EXPECT_FALSE(base.with_auto_scaling(false).auto_scaling_enabled());
        EXPECT_EQ(base.size(), 12.0); // base unchanged
    }

    TEST(font, equality)
    {
        EXPECT_EQ(font::of_size("Arial", 12), font::of_size("Arial", 12));
        EXPECT_NE(font::of_size("Arial", 12), font::of_size("Arial", 14));
        EXPECT_NE(font::of_size("Arial", 12), font::of_size("Times", 12));
        EXPECT_NE(font::of_size("Arial", 12), font::of_size("Arial", 12, font_weight::bold));
        // unset weight (default) equals an explicitly-regular font, since weight() maps 0 -> regular
        EXPECT_EQ(font(), font::default_font());
    }
} // namespace

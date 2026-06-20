// Tests for the shapes_page demo (src/samples/pages/shapes_page.hpp) — a faithful reproduction of the
// maui-compare Shapes() reference (Ellipse / RoundRectangle / EvenOdd Polygon / Line). Backend-agnostic:
// the page is pure cross-platform control wiring, so this suite compiles in every preset and proves the
// demo's structure + per-shape decoration without a hosting main.
#include "src/samples/pages/shapes_page.hpp"

#include "maui/controls/shapes/fill_rule.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::shapes_page;

    TEST(shapes_page, builds_the_shape_tree)
    {
        shapes_page demo;
        // The stack hosts four bold captions interleaved with the four shapes.
        EXPECT_EQ(demo.stack().count(), 8);

        // Ellipse: red fill + dark-blue stroke (StrokeThickness 4).
        ASSERT_NE(demo.ellipse().fill(), nullptr);
        ASSERT_NE(demo.ellipse().stroke(), nullptr);
        EXPECT_EQ(demo.ellipse().stroke_thickness(), 4.0);

        // RoundRectangle (a Rectangle): navy fill, RadiusX 12 / RadiusY 24.
        ASSERT_NE(demo.rect().fill(), nullptr);
        EXPECT_EQ(demo.rect().radius_x(), 12.0);
        EXPECT_EQ(demo.rect().radius_y(), 24.0);

        // EvenOdd Polygon (pentagram): five points, blue fill + red stroke, EvenOdd fill rule.
        EXPECT_EQ(demo.polygon().points().size(), 5U);
        EXPECT_EQ(demo.polygon().fill_rule(), maui::controls::shapes::fill_rule::even_odd);
        ASSERT_NE(demo.polygon().fill(), nullptr);
        ASSERT_NE(demo.polygon().stroke(), nullptr);

        // Line: (40,0) -> (0,80), purple stroke.
        EXPECT_EQ(demo.line().x1(), 40.0);
        EXPECT_EQ(demo.line().y1(), 0.0);
        EXPECT_EQ(demo.line().x2(), 0.0);
        EXPECT_EQ(demo.line().y2(), 80.0);
        ASSERT_NE(demo.line().stroke(), nullptr);
    }
} // namespace

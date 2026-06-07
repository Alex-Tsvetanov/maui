// Characterization tests for the SVG path parser, derived from PathBuilder.cs behavior.
// (No C# unit tests exist for PathBuilder.)

#include "maui/graphics/path_builder.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include <gtest/gtest.h>

using maui::graphics::path_builder;
using maui::graphics::path_f;
using maui::graphics::path_operation;
using maui::graphics::point_f;

TEST(path_builder_tests, empty)
{
    EXPECT_EQ(0, path_builder::build("").count());
    EXPECT_EQ(0, path_builder::build("").operation_count());
}

TEST(path_builder_tests, absolute_polyline_closed)
{
    path_f p = path_builder::build("M0,0 L10,0 L10,10 Z");
    EXPECT_EQ(3, p.count());
    EXPECT_EQ(4, p.operation_count()); // move, line, line, close
    EXPECT_TRUE(p.closed());
    EXPECT_EQ(point_f(0, 0), p[0]);
    EXPECT_EQ(point_f(10, 0), p[1]);
    EXPECT_EQ(point_f(10, 10), p[2]);
    EXPECT_EQ(path_operation::move, p.get_segment_type(0));
    EXPECT_EQ(path_operation::line, p.get_segment_type(1));
}

TEST(path_builder_tests, space_separated)
{
    path_f p = path_builder::build("M0 0 L10 0");
    EXPECT_EQ(2, p.count());
    EXPECT_EQ(point_f(0, 0), p[0]);
    EXPECT_EQ(point_f(10, 0), p[1]);
}

TEST(path_builder_tests, relative_move_line)
{
    path_f p = path_builder::build("m10,10 l5,5");
    EXPECT_EQ(2, p.count());
    EXPECT_EQ(point_f(10, 10), p[0]);
    EXPECT_EQ(point_f(15, 15), p[1]); // relative line
}

TEST(path_builder_tests, multiple_line_coords)
{
    // Implicit coordinates after an explicit L repeat as line-to (last_command stays 'L').
    path_f p = path_builder::build("M0,0 L1,1 2,2");
    EXPECT_EQ(3, p.count());
    EXPECT_EQ(point_f(1, 1), p[1]);
    EXPECT_EQ(point_f(2, 2), p[2]);
    EXPECT_EQ(path_operation::line, p.get_segment_type(1));
    EXPECT_EQ(path_operation::line, p.get_segment_type(2));
}

TEST(path_builder_tests, horizontal_vertical)
{
    path_f p = path_builder::build("M0,0 H10 V10");
    EXPECT_EQ(3, p.count());
    EXPECT_EQ(point_f(0, 0), p[0]);
    EXPECT_EQ(point_f(10, 0), p[1]);  // H -> (10, y)
    EXPECT_EQ(point_f(10, 10), p[2]); // V -> (x, 10)
}

TEST(path_builder_tests, cubic)
{
    path_f p = path_builder::build("M0,0 C1,1 2,2 3,3");
    EXPECT_EQ(4, p.count()); // move + 3 cubic points
    EXPECT_EQ(2, p.operation_count());
    EXPECT_EQ(path_operation::cubic, p.get_segment_type(1));
    EXPECT_EQ(point_f(3, 3), p.last_point());
}

TEST(path_builder_tests, quad)
{
    path_f p = path_builder::build("M0,0 Q1,1 2,2");
    EXPECT_EQ(3, p.count()); // move + 2 quad points
    EXPECT_EQ(path_operation::quad, p.get_segment_type(1));
    EXPECT_EQ(point_f(2, 2), p.last_point());
}

TEST(path_builder_tests, arc)
{
    // Semicircle from (0,0) to (10,0); DrawArc emits quad segments.
    path_f p = path_builder::build("M0,0 A5,5 0 0 1 10,0");
    EXPECT_GT(p.operation_count(), 1);
    EXPECT_EQ(point_f(0, 0), p.first_point());
    bool has_quad = false;
    for (auto op : p.segment_types())
    {
        if (op == path_operation::quad)
        {
            has_quad = true;
        }
    }
    EXPECT_TRUE(has_quad);
    EXPECT_NEAR(10.0F, p.last_point().x, 0.5F);
    EXPECT_NEAR(0.0F, p.last_point().y, 0.5F);
}

TEST(path_builder_tests, parse_float)
{
    EXPECT_FLOAT_EQ(5.96F, path_builder::parse_float("5.96"));
    EXPECT_FLOAT_EQ(5.96F, path_builder::parse_float("5.96.88")); // Illustrator malformed
    EXPECT_FLOAT_EQ(-3.5F, path_builder::parse_float("-3.5"));
    EXPECT_FLOAT_EQ(100.0F, path_builder::parse_float("1e2"));
    EXPECT_FLOAT_EQ(12.0F, path_builder::parse_float("12"));
}

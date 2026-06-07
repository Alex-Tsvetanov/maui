// path_f has NO C# unit tests (only a benchmark), so these are CHARACTERIZATION tests: expected
// values derived by reading src/Graphics/src/Graphics/PathF.cs. They pin the ported behavior.

#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

using maui::graphics::path_f;
using maui::graphics::path_operation;
using maui::graphics::point_f;
using maui::graphics::rect_f;

TEST(path_tests, empty_path)
{
    path_f p;
    EXPECT_EQ(0, p.count());
    EXPECT_EQ(0, p.operation_count());
    EXPECT_EQ(0, p.sub_path_count());
    EXPECT_FALSE(p.closed());
    EXPECT_EQ(point_f(0, 0), p.first_point());
    EXPECT_EQ(point_f(0, 0), p.last_point());
    EXPECT_EQ(-1, p.last_point_index());
    EXPECT_EQ(point_f(0, 0), p[0]); // out of range -> default
}

TEST(path_tests, move_line_building)
{
    path_f p;
    p.move_to(1, 2).line_to(3, 4).line_to(5, 6);
    EXPECT_EQ(3, p.count());
    EXPECT_EQ(3, p.operation_count());
    EXPECT_EQ(1, p.sub_path_count());
    EXPECT_EQ(point_f(1, 2), p.first_point());
    EXPECT_EQ(point_f(5, 6), p.last_point());
    EXPECT_EQ(2, p.last_point_index());
    EXPECT_EQ(path_operation::move, p.get_segment_type(0));
    EXPECT_EQ(path_operation::line, p.get_segment_type(1));
    EXPECT_EQ(2, p.segment_count_excluding_open_and_close()); // 3 - leading move
}

TEST(path_tests, line_to_on_empty_starts_subpath)
{
    path_f p;
    p.line_to(1, 2); // auto MoveTo
    EXPECT_EQ(1, p.count());
    EXPECT_EQ(1, p.sub_path_count());
    EXPECT_EQ(path_operation::move, p.get_segment_type(0));
}

TEST(path_tests, close_and_open)
{
    path_f p;
    p.move_to(0, 0).line_to(1, 1);
    p.close();
    EXPECT_TRUE(p.closed());
    EXPECT_TRUE(p.is_sub_path_closed(0));
    EXPECT_EQ(3, p.operation_count()); // move, line, close
    EXPECT_EQ(2, p.count());           // close adds no point
    EXPECT_EQ(1, p.segment_count_excluding_open_and_close());
    p.open();
    EXPECT_FALSE(p.closed());
    EXPECT_EQ(2, p.operation_count());
}

TEST(path_tests, ctors)
{
    path_f a(1, 2);
    EXPECT_EQ(1, a.sub_path_count());
    EXPECT_EQ(point_f(1, 2), a.first_point());
    path_f b(point_f(3, 4));
    EXPECT_EQ(point_f(3, 4), b.first_point());
}

TEST(path_tests, multiple_subpaths)
{
    path_f p;
    p.move_to(0, 0).line_to(1, 1).move_to(5, 5).line_to(6, 6);
    EXPECT_EQ(2, p.sub_path_count());
    EXPECT_EQ(4, p.count());
}

TEST(path_tests, quad_and_cubic)
{
    path_f p;
    p.move_to(0, 0).quad_to(point_f(1, 1), point_f(2, 2)).curve_to(point_f(3, 3), point_f(4, 4), point_f(5, 5));
    EXPECT_EQ(1 + 2 + 3, p.count());
    EXPECT_EQ(3, p.operation_count());
    EXPECT_EQ(path_operation::quad, p.get_segment_type(1));
    EXPECT_EQ(path_operation::cubic, p.get_segment_type(2));
}

TEST(path_tests, add_arc)
{
    path_f p;
    p.add_arc(0, 0, 10, 10, 0, 90, true);
    EXPECT_EQ(1, p.sub_path_count()); // arc on empty path starts a sub-path
    EXPECT_EQ(2, p.count());          // top-left + bottom-right
    EXPECT_EQ(path_operation::arc, p.get_segment_type(0));
    EXPECT_FLOAT_EQ(0, p.get_arc_angle(0));
    EXPECT_FLOAT_EQ(90, p.get_arc_angle(1));
    EXPECT_TRUE(p.get_arc_clockwise(0));
}

TEST(path_tests, append_rectangle)
{
    path_f p;
    p.append_rectangle(rect_f(0, 0, 10, 20));
    EXPECT_EQ(4, p.count());
    EXPECT_EQ(5, p.operation_count()); // move, line, line, line, close
    EXPECT_TRUE(p.closed());
    EXPECT_EQ(point_f(0, 0), p[0]);
    EXPECT_EQ(point_f(10, 0), p[1]);
    EXPECT_EQ(point_f(10, 20), p[2]);
    EXPECT_EQ(point_f(0, 20), p[3]);

    path_f q;
    q.append_rectangle(0, 0, 10, 20, /*include_last*/ true);
    EXPECT_EQ(5, q.count());
    EXPECT_EQ(6, q.operation_count());
}

TEST(path_tests, append_circle_and_ellipse)
{
    path_f c;
    c.append_circle(point_f(0, 0), 5);
    EXPECT_EQ(6, c.operation_count()); // move + 4 cubic + close
    EXPECT_EQ(1 + 12, c.count());
    EXPECT_TRUE(c.closed());
    EXPECT_EQ(point_f(-5, 0), c.first_point()); // (cx-r, cy)

    path_f e;
    e.append_ellipse(rect_f(0, 0, 10, 20));
    EXPECT_EQ(6, e.operation_count());
    EXPECT_EQ(13, e.count());
}

TEST(path_tests, append_rounded_rectangle)
{
    path_f p;
    p.append_rounded_rectangle(rect_f(0, 0, 10, 10), 2.0F);
    // move, curve, line, curve, line, curve, line, curve, close
    EXPECT_EQ(9, p.operation_count());
    EXPECT_EQ(1 + (4 * 3) + 3, p.count()); // move + 4 cubics + 3 lines
    EXPECT_TRUE(p.closed());
}

TEST(path_tests, bounds_rectangle_and_line)
{
    path_f rectp;
    rectp.append_rectangle(rect_f(0, 0, 10, 20));
    EXPECT_EQ(rect_f(0, 0, 10, 20), rectp.bounds());

    path_f line;
    line.move_to(0, 0).line_to(10, 20);
    EXPECT_EQ(rect_f(0, 0, 10, 20), line.get_bounds_by_flattening());
}

TEST(path_tests, bounds_circle_approx)
{
    path_f c;
    c.append_circle(point_f(0, 0), 5);
    rect_f b = c.bounds();
    EXPECT_NEAR(-5.0F, b.x, 0.02F);
    EXPECT_NEAR(-5.0F, b.y, 0.02F);
    EXPECT_NEAR(10.0F, b.width, 0.02F);
    EXPECT_NEAR(10.0F, b.height, 0.02F);
}

TEST(path_tests, equals)
{
    path_f a;
    a.move_to(0, 0).line_to(10, 10);
    path_f b;
    b.move_to(0, 0).line_to(10, 10);
    EXPECT_TRUE(a.equals(b));
    EXPECT_TRUE(a == b);

    path_f c;
    c.move_to(0, 0).line_to(10, 11);
    EXPECT_FALSE(a.equals(c));

    path_f d;
    d.move_to(0, 0); // different op count
    EXPECT_FALSE(a.equals(d));

    path_f e;
    e.move_to(0, 0).line_to(10.0001F, 10);
    EXPECT_TRUE(a.equals(e, 0.001F)); // within epsilon
    EXPECT_FALSE(a.equals(e, 0.00001F));
}

TEST(path_tests, reverse)
{
    path_f p;
    p.move_to(0, 0).line_to(1, 1).line_to(2, 2);
    path_f r = p.reverse();
    EXPECT_EQ(3, r.count());
    EXPECT_EQ(3, r.operation_count());
    EXPECT_EQ(point_f(2, 2), r.first_point());
    EXPECT_EQ(point_f(0, 0), r.last_point());
    EXPECT_EQ(path_operation::move, r.get_segment_type(0));
    EXPECT_EQ(path_operation::line, r.get_segment_type(1));
    EXPECT_EQ(path_operation::line, r.get_segment_type(2));
}

TEST(path_tests, separate)
{
    path_f p;
    p.move_to(0, 0).line_to(1, 1).move_to(5, 5).line_to(6, 6);
    auto parts = p.separate();
    ASSERT_EQ(2U, parts.size());
    EXPECT_EQ(point_f(0, 0), parts[0].first_point());
    EXPECT_EQ(2, parts[0].count());
    EXPECT_EQ(point_f(5, 5), parts[1].first_point());
    EXPECT_EQ(2, parts[1].count());
}

TEST(path_tests, rotate)
{
    path_f p;
    p.move_to(1, 0);
    path_f r = p.rotate(90, point_f(0, 0)); // (1,0) rotated 90deg about origin -> (0,1)
    EXPECT_NEAR(0.0F, r.first_point().x, 1e-4F);
    EXPECT_NEAR(1.0F, r.first_point().y, 1e-4F);
}

TEST(path_tests, get_flattened_path)
{
    path_f p;
    p.move_to(0, 0).curve_to(point_f(0, 10), point_f(10, 10), point_f(10, 0));
    path_f flat = p.get_flattened_path();
    EXPECT_GT(flat.operation_count(), 2); // move + many lines
    for (auto op : flat.segment_types())
    {
        EXPECT_TRUE(op == path_operation::move || op == path_operation::line);
    }
    // Lands ~near the curve end point (10,0). The forward-difference flattening accumulates
    // rounding and doesn't return exactly to the endpoint — this matches the C# algorithm.
    EXPECT_NEAR(10.0F, flat.last_point().x, 0.1F);
    EXPECT_NEAR(0.0F, flat.last_point().y, 0.1F);
}

TEST(path_tests, set_point_and_segment_points)
{
    path_f p;
    p.move_to(0, 0).line_to(1, 1).quad_to(point_f(2, 2), point_f(3, 3));
    p.set_point(1, point_f(5, 5));
    EXPECT_EQ(point_f(5, 5), p[1]);

    auto seg0 = p.get_points_for_segment(0);
    ASSERT_EQ(1U, seg0.size());
    EXPECT_EQ(point_f(0, 0), seg0[0]);
    auto seg2 = p.get_points_for_segment(2); // quad -> 2 points
    ASSERT_EQ(2U, seg2.size());
    EXPECT_EQ(point_f(2, 2), seg2[0]);
    EXPECT_EQ(point_f(3, 3), seg2[1]);
}

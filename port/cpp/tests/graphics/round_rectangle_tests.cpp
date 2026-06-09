// maui::graphics::shapes::round_rectangle — per-corner clip radii.
//
// Characterization derived from src/Controls/src/Core/Shapes/RoundRectangle.cs (GetPath/PathForBounds):
// path_for_bounds builds a rounded-rectangle path over the bounds from the four per-corner radii (order:
// top-left, top-right, bottom-left, bottom-right). The simplified port omits Stretch/StrokeThickness.

#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"

#include <gtest/gtest.h>

using maui::graphics::corner_radius;
using maui::graphics::path_f;
using maui::graphics::point_f;
using maui::graphics::rect;
using maui::graphics::rect_f;
using maui::graphics::shapes::round_rectangle;

TEST(round_rectangle_tests, default_corner_radius_is_zero)
{
    const round_rectangle rr;
    EXPECT_EQ(corner_radius(), rr.corner_radius());
}

TEST(round_rectangle_tests, uniform_ctor_sets_all_corners)
{
    const round_rectangle rr(8.0);
    EXPECT_EQ(corner_radius(8, 8, 8, 8), rr.corner_radius());
}

TEST(round_rectangle_tests, per_corner_ctor_keeps_each_radius)
{
    const round_rectangle rr(corner_radius(12, 0, 0, 24));
    EXPECT_EQ(corner_radius(12, 0, 0, 24), rr.corner_radius());
}

TEST(round_rectangle_tests, set_corner_radius)
{
    round_rectangle rr;
    rr.set_corner_radius(corner_radius(1, 2, 3, 4));
    EXPECT_EQ(corner_radius(1, 2, 3, 4), rr.corner_radius());
}

TEST(round_rectangle_tests, path_for_bounds_structure)
{
    // RoundRectangle.GetPath -> AppendRoundedRectangle: move + 4 cubics + 3 lines + close = 9 ops.
    const round_rectangle rr(corner_radius(8, 4, 2, 6));
    const path_f path = rr.path_for_bounds(rect(0, 0, 60, 40));
    EXPECT_EQ(9, path.operation_count());
    EXPECT_TRUE(path.closed());
    // Path starts at (minX, minY + topLeftRadius).
    EXPECT_EQ(point_f(0.0F, 8.0F), path.first_point());

    const rect_f bounds = path.get_bounds_by_flattening();
    EXPECT_NEAR(60.0F, bounds.width, 0.5F);
    EXPECT_NEAR(40.0F, bounds.height, 0.5F);
}

TEST(round_rectangle_tests, uniform_radius_path_matches_per_corner)
{
    // A uniform-constructed shape and a 4-equal-radii shape produce the same path.
    const round_rectangle uniform(5.0);
    const round_rectangle per_corner(corner_radius(5, 5, 5, 5));
    EXPECT_EQ(uniform.path_for_bounds(rect(0, 0, 50, 30)), per_corner.path_for_bounds(rect(0, 0, 50, 30)));
}

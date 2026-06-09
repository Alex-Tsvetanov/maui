// Tests for the gradient paints — gradient_stop / gradient_paint (via linear_gradient_paint as the
// concrete vehicle) / linear_gradient_paint / radial_gradient_paint. Characterization tests: there is no
// C# unit-test oracle for these types, so behavior is derived directly from the C# source
// (src/Graphics/src/Graphics/{GradientPaint,LinearGradientPaint,RadialGradientPaint,PaintGradientStop}.cs
// + GeometryUtil.cs's GetColorAt interpolation math).

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"

#include <vector>

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::graphics::color;
    using maui::graphics::gradient_paint;
    using maui::graphics::gradient_stop;
    using maui::graphics::linear_gradient_paint;
    using maui::graphics::point;
    using maui::graphics::radial_gradient_paint;
    namespace colors = maui::graphics::colors;

    // ---- gradient_stop ----

    TEST(gradient_stop_test, offset_and_color)
    {
        const gradient_stop stop(0.25F, colors::red);
        EXPECT_FLOAT_EQ(stop.offset(), 0.25F);
        EXPECT_EQ(stop.color(), colors::red);
    }

    TEST(gradient_stop_test, default_is_zero_offset_black)
    {
        const gradient_stop stop;
        EXPECT_FLOAT_EQ(stop.offset(), 0.0F);
        EXPECT_EQ(stop.color(), color()); // value-type default color
    }

    TEST(gradient_stop_test, compares_by_offset)
    {
        const gradient_stop a(0.1F, colors::red);
        const gradient_stop b(0.9F, colors::blue);
        EXPECT_TRUE(a < b);
        EXPECT_FALSE(b < a);
    }

    // ---- gradient_paint defaults (via linear_gradient_paint) ----

    TEST(gradient_paint_test, default_stops_are_white_to_white)
    {
        const linear_gradient_paint paint;
        ASSERT_EQ(paint.gradient_stops().size(), 2U);
        EXPECT_FLOAT_EQ(paint.gradient_stops()[0].offset(), 0.0F);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::white);
        EXPECT_FLOAT_EQ(paint.gradient_stops()[1].offset(), 1.0F);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::white);
    }

    TEST(gradient_paint_test, setting_empty_stops_restores_default)
    {
        linear_gradient_paint paint;
        paint.set_gradient_stops(std::vector<gradient_stop>{gradient_stop(0.5F, colors::red)});
        ASSERT_EQ(paint.gradient_stops().size(), 1U);

        paint.set_gradient_stops(std::vector<gradient_stop>{}); // empty -> default white/white
        ASSERT_EQ(paint.gradient_stops().size(), 2U);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::white);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::white);
    }

    // ---- start/end color + index ----

    TEST(gradient_paint_test, start_and_end_colors_for_default_stops)
    {
        const linear_gradient_paint paint;
        EXPECT_EQ(paint.start_color(), colors::white);
        EXPECT_EQ(paint.end_color(), colors::white);
        EXPECT_EQ(paint.start_color_index(), 0);
        EXPECT_EQ(paint.end_color_index(), 1);
    }

    TEST(gradient_paint_test, start_is_lowest_offset_end_is_highest)
    {
        // Stops out of order: lowest offset (0.2) is start, highest (0.8) is end — regardless of position.
        const linear_gradient_paint paint{std::vector<gradient_stop>{
            gradient_stop(0.8F, colors::blue), gradient_stop(0.2F, colors::red), gradient_stop(0.5F, colors::green)}};
        EXPECT_EQ(paint.start_color(), colors::red); // offset 0.2 (lowest)
        EXPECT_EQ(paint.end_color(), colors::blue);  // offset 0.8 (highest)
        EXPECT_EQ(paint.start_color_index(), 1);     // index of the 0.2 stop
        EXPECT_EQ(paint.end_color_index(), 0);       // index of the 0.8 stop
    }

    TEST(gradient_paint_test, set_start_color_writes_lowest_offset_stop)
    {
        linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::white), gradient_stop(1.0F, colors::white)}};
        paint.set_start_color(colors::red);
        paint.set_end_color(colors::blue);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::red);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::blue);
    }

    // ---- get_color_at (the interpolation core) ----

    TEST(gradient_paint_test, get_color_at_exact_stop_returns_stop_color)
    {
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)}};
        EXPECT_EQ(paint.get_color_at(0.0F), colors::red);
        EXPECT_EQ(paint.get_color_at(1.0F), colors::blue);
    }

    TEST(gradient_paint_test, get_color_at_midpoint_blends_halfway)
    {
        // red (1,0,0) -> blue (0,0,1); midpoint is (0.5, 0, 0.5).
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)}};
        const color mid = paint.get_color_at(0.5F);
        EXPECT_NEAR(mid.red, 0.5F, 1e-5F);
        EXPECT_NEAR(mid.green, 0.0F, 1e-5F);
        EXPECT_NEAR(mid.blue, 0.5F, 1e-5F);
        EXPECT_NEAR(mid.alpha, 1.0F, 1e-5F);
    }

    TEST(gradient_paint_test, get_color_at_quarter_blends_proportionally)
    {
        // black (0,0,0) -> white (1,1,1); at 0.25 -> (0.25, 0.25, 0.25).
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::black), gradient_stop(1.0F, colors::white)}};
        const color q = paint.get_color_at(0.25F);
        EXPECT_NEAR(q.red, 0.25F, 1e-5F);
        EXPECT_NEAR(q.green, 0.25F, 1e-5F);
        EXPECT_NEAR(q.blue, 0.25F, 1e-5F);
    }

    TEST(gradient_paint_test, get_color_at_before_first_stop_returns_start)
    {
        // offset below all stops: no "before" bracket -> StartColor.
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.3F, colors::red), gradient_stop(0.8F, colors::blue)}};
        EXPECT_EQ(paint.get_color_at(0.0F), colors::red); // start (lowest-offset stop)
    }

    TEST(gradient_paint_test, get_color_at_after_last_stop_returns_end)
    {
        // offset above all stops: no "after" bracket -> EndColor.
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.3F, colors::red), gradient_stop(0.8F, colors::blue)}};
        EXPECT_EQ(paint.get_color_at(1.0F), colors::blue); // end (highest-offset stop)
    }

    TEST(gradient_paint_test, get_color_at_single_stop_returns_its_color)
    {
        const linear_gradient_paint paint{std::vector<gradient_stop>{gradient_stop(0.5F, colors::green)}};
        EXPECT_EQ(paint.get_color_at(0.0F), colors::green);
        EXPECT_EQ(paint.get_color_at(0.9F), colors::green);
    }

    // ---- is_transparent ----

    TEST(gradient_paint_test, is_transparent_false_for_opaque_stops)
    {
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)}};
        EXPECT_FALSE(paint.is_transparent());
    }

    TEST(gradient_paint_test, is_transparent_true_when_any_stop_has_alpha_below_one)
    {
        const linear_gradient_paint paint{std::vector<gradient_stop>{
            gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue.with_alpha(0.5F))}};
        EXPECT_TRUE(paint.is_transparent());
    }

    // ---- get_sorted_stops ----

    TEST(gradient_paint_test, get_sorted_stops_orders_by_offset)
    {
        const linear_gradient_paint paint{std::vector<gradient_stop>{
            gradient_stop(0.8F, colors::blue), gradient_stop(0.2F, colors::red), gradient_stop(0.5F, colors::green)}};
        const std::vector<gradient_stop> sorted = paint.get_sorted_stops();
        ASSERT_EQ(sorted.size(), 3U);
        EXPECT_FLOAT_EQ(sorted[0].offset(), 0.2F);
        EXPECT_FLOAT_EQ(sorted[1].offset(), 0.5F);
        EXPECT_FLOAT_EQ(sorted[2].offset(), 0.8F);
        // the original is unchanged
        EXPECT_FLOAT_EQ(paint.gradient_stops()[0].offset(), 0.8F);
    }

    // ---- add_offset / remove_offset / set_gradient_stops(offsets, colors) ----

    TEST(gradient_paint_test, add_offset_with_color_appends_stop)
    {
        linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)}};
        paint.add_offset(0.5F, colors::green);
        ASSERT_EQ(paint.gradient_stops().size(), 3U);
        EXPECT_FLOAT_EQ(paint.gradient_stops()[2].offset(), 0.5F);
        EXPECT_EQ(paint.gradient_stops()[2].color(), colors::green);
    }

    TEST(gradient_paint_test, add_offset_interpolates_color)
    {
        // black->white; add at 0.5 without a color -> interpolated grey (0.5,0.5,0.5).
        linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::black), gradient_stop(1.0F, colors::white)}};
        paint.add_offset(0.5F);
        ASSERT_EQ(paint.gradient_stops().size(), 3U);
        const color added = paint.gradient_stops()[2].color();
        EXPECT_NEAR(added.red, 0.5F, 1e-5F);
        EXPECT_NEAR(added.green, 0.5F, 1e-5F);
        EXPECT_NEAR(added.blue, 0.5F, 1e-5F);
    }

    TEST(gradient_paint_test, remove_offset_removes_stop)
    {
        linear_gradient_paint paint{std::vector<gradient_stop>{
            gradient_stop(0.0F, colors::red), gradient_stop(0.5F, colors::green), gradient_stop(1.0F, colors::blue)}};
        paint.remove_offset(1);
        ASSERT_EQ(paint.gradient_stops().size(), 2U);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::red);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::blue);
    }

    TEST(gradient_paint_test, remove_offset_out_of_range_is_noop)
    {
        linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)}};
        paint.remove_offset(-1);
        paint.remove_offset(5);
        EXPECT_EQ(paint.gradient_stops().size(), 2U);
    }

    TEST(gradient_paint_test, set_gradient_stops_from_parallel_arrays_uses_min_length)
    {
        linear_gradient_paint paint;
        const std::vector<float> offsets{0.0F, 0.5F, 1.0F};
        const std::vector<color> cols{colors::red, colors::green}; // shorter -> only 2 stops
        paint.set_gradient_stops(offsets, cols);
        ASSERT_EQ(paint.gradient_stops().size(), 2U);
        EXPECT_FLOAT_EQ(paint.gradient_stops()[0].offset(), 0.0F);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::red);
        EXPECT_FLOAT_EQ(paint.gradient_stops()[1].offset(), 0.5F);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::green);
    }

    // ---- blend_start_and_end_colors ----

    TEST(gradient_paint_test, blend_midpoint_of_start_and_end)
    {
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::black), gradient_stop(1.0F, colors::white)}};
        const color blended = paint.blend_start_and_end_colors();
        EXPECT_NEAR(blended.red, 0.5F, 1e-5F);
        EXPECT_NEAR(blended.green, 0.5F, 1e-5F);
        EXPECT_NEAR(blended.blue, 0.5F, 1e-5F);
    }

    TEST(gradient_paint_test, blend_with_fewer_than_two_stops_is_white)
    {
        const linear_gradient_paint paint{std::vector<gradient_stop>{gradient_stop(0.5F, colors::red)}};
        EXPECT_EQ(paint.blend_start_and_end_colors(), colors::white);
    }

    // background_color (paint contract): a gradient reports its start/end blend as a representative color.
    TEST(gradient_paint_test, background_color_is_start_end_blend)
    {
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::black), gradient_stop(1.0F, colors::white)}};
        const color bg = paint.background_color();
        EXPECT_NEAR(bg.red, 0.5F, 1e-5F);
    }

    // It is usable polymorphically through the abstract paint base.
    TEST(gradient_paint_test, usable_as_paint_base)
    {
        const linear_gradient_paint paint{std::vector<gradient_stop>{
            gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue.with_alpha(0.5F))}};
        const maui::graphics::paint& base = paint;
        EXPECT_TRUE(base.is_transparent());
    }

    // ---- linear_gradient_paint ----

    TEST(linear_gradient_paint_test, default_points)
    {
        const linear_gradient_paint paint;
        EXPECT_EQ(paint.start_point(), point(0, 0));
        EXPECT_EQ(paint.end_point(), point(1, 1));
    }

    TEST(linear_gradient_paint_test, point_ctor_sets_points)
    {
        const linear_gradient_paint paint{point(0.1, 0.2), point(0.9, 0.8)};
        EXPECT_EQ(paint.start_point(), point(0.1, 0.2));
        EXPECT_EQ(paint.end_point(), point(0.9, 0.8));
    }

    TEST(linear_gradient_paint_test, stops_and_points_ctor)
    {
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)},
            point(0, 0), point(1, 0)};
        EXPECT_EQ(paint.start_point(), point(0, 0));
        EXPECT_EQ(paint.end_point(), point(1, 0));
        EXPECT_EQ(paint.start_color(), colors::red);
        EXPECT_EQ(paint.end_color(), colors::blue);
    }

    TEST(linear_gradient_paint_test, set_points)
    {
        linear_gradient_paint paint;
        paint.set_start_point(point(0.2, 0.3));
        paint.set_end_point(point(0.7, 0.6));
        EXPECT_EQ(paint.start_point(), point(0.2, 0.3));
        EXPECT_EQ(paint.end_point(), point(0.7, 0.6));
    }

    TEST(linear_gradient_paint_test, copy_from_gradient_paint_copies_stops)
    {
        // The source gradient with custom stops; the copy ctor deep-copies them.
        linear_gradient_paint source{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)},
            point(0.1, 0.1), point(0.9, 0.9)};
        const linear_gradient_paint copy{static_cast<const gradient_paint&>(source)};
        ASSERT_EQ(copy.gradient_stops().size(), 2U);
        EXPECT_EQ(copy.gradient_stops()[0].color(), colors::red);
        EXPECT_EQ(copy.gradient_stops()[1].color(), colors::blue);
        // The GradientPaint-copy ctor does NOT carry start/end points (C# leaves them at the struct default).
        EXPECT_EQ(copy.start_point(), point(0, 0));
        EXPECT_EQ(copy.end_point(), point(0, 0));
    }

    // ---- radial_gradient_paint ----

    TEST(radial_gradient_paint_test, default_center_and_radius)
    {
        const radial_gradient_paint paint;
        EXPECT_EQ(paint.center(), point(0.5, 0.5));
        EXPECT_DOUBLE_EQ(paint.radius(), 0.5);
    }

    TEST(radial_gradient_paint_test, center_radius_ctor)
    {
        const radial_gradient_paint paint{point(0.3, 0.4), 0.25};
        EXPECT_EQ(paint.center(), point(0.3, 0.4));
        EXPECT_DOUBLE_EQ(paint.radius(), 0.25);
    }

    TEST(radial_gradient_paint_test, stops_center_radius_ctor)
    {
        const radial_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, colors::red), gradient_stop(1.0F, colors::blue)},
            point(0.5, 0.5), 0.75};
        EXPECT_EQ(paint.center(), point(0.5, 0.5));
        EXPECT_DOUBLE_EQ(paint.radius(), 0.75);
        EXPECT_EQ(paint.start_color(), colors::red);
        EXPECT_EQ(paint.end_color(), colors::blue);
    }

    TEST(radial_gradient_paint_test, set_center_and_radius)
    {
        radial_gradient_paint paint;
        paint.set_center(point(0.1, 0.2));
        paint.set_radius(0.9);
        EXPECT_EQ(paint.center(), point(0.1, 0.2));
        EXPECT_DOUBLE_EQ(paint.radius(), 0.9);
    }

    TEST(radial_gradient_paint_test, default_stops_white_to_white)
    {
        const radial_gradient_paint paint;
        ASSERT_EQ(paint.gradient_stops().size(), 2U);
        EXPECT_EQ(paint.gradient_stops()[0].color(), colors::white);
        EXPECT_EQ(paint.gradient_stops()[1].color(), colors::white);
    }
} // namespace

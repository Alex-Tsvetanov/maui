// System.Numerics interop + CornerRadius for the maui::graphics port.
//
// corner_radius cases are ported from the C# oracle: src/Controls/tests/Core.UnitTests/GeometryTests.cs
// (CornerRadius(12, 0, 0, 12) field order) and src/Core/tests/UnitTests/Views/BorderTests.cs
// (CornerRadius(12) / CornerRadius(12, 0, 0, 24)); the Equals/ctor semantics are from
// src/Core/src/Primitives/CornerRadius.cs.
//
// The vector2 / vector4 / matrix3x2 stand-ins reproduce System.Numerics semantics
// (Vector2.Transform(Vector2, Matrix3x2), Matrix3x2.Identity / operator*) verified against
// hand-computed values; the point_f / color / path_f interop is derived from PointF.cs / Color.cs /
// PathF.cs (no dedicated C# unit test — characterization, like the other graphics ports).

#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/vector2.hpp"
#include "maui/graphics/vector4.hpp"

#include <gtest/gtest.h>

using maui::graphics::color;
using maui::graphics::corner_radius;
using maui::graphics::matrix3x2;
using maui::graphics::path_f;
using maui::graphics::path_operation;
using maui::graphics::point_f;
using maui::graphics::rect_f;
using maui::graphics::vector2;
using maui::graphics::vector4;

// ============================ corner_radius ============================

TEST(corner_radius_tests, four_arg_ctor_assigns_each_corner)
{
    // GeometryTests.cs: new CornerRadius(12, 0, 0, 12).
    const corner_radius cr(12, 0, 0, 12);
    EXPECT_DOUBLE_EQ(12.0, cr.top_left);
    EXPECT_DOUBLE_EQ(0.0, cr.top_right);
    EXPECT_DOUBLE_EQ(0.0, cr.bottom_left);
    EXPECT_DOUBLE_EQ(12.0, cr.bottom_right);
}

TEST(corner_radius_tests, four_arg_ctor_distinct_corners)
{
    // BorderTests.cs: new CornerRadius(12, 0, 0, 24).
    const corner_radius cr(12, 0, 0, 24);
    EXPECT_DOUBLE_EQ(12.0, cr.top_left);
    EXPECT_DOUBLE_EQ(0.0, cr.top_right);
    EXPECT_DOUBLE_EQ(0.0, cr.bottom_left);
    EXPECT_DOUBLE_EQ(24.0, cr.bottom_right);
}

TEST(corner_radius_tests, uniform_ctor_sets_all_four)
{
    const corner_radius cr(15);
    EXPECT_DOUBLE_EQ(15.0, cr.top_left);
    EXPECT_DOUBLE_EQ(15.0, cr.top_right);
    EXPECT_DOUBLE_EQ(15.0, cr.bottom_left);
    EXPECT_DOUBLE_EQ(15.0, cr.bottom_right);
}

TEST(corner_radius_tests, implicit_double_conversion_is_uniform)
{
    // C#'s implicit operator CornerRadius(double): a bare double becomes a uniform corner_radius.
    const corner_radius cr = 7.0;
    EXPECT_EQ(corner_radius(7, 7, 7, 7), cr);
}

TEST(corner_radius_tests, default_is_all_zero)
{
    const corner_radius cr;
    EXPECT_DOUBLE_EQ(0.0, cr.top_left);
    EXPECT_DOUBLE_EQ(0.0, cr.top_right);
    EXPECT_DOUBLE_EQ(0.0, cr.bottom_left);
    EXPECT_DOUBLE_EQ(0.0, cr.bottom_right);
}

TEST(corner_radius_tests, equality)
{
    EXPECT_EQ(corner_radius(1, 2, 3, 4), corner_radius(1, 2, 3, 4));
    EXPECT_NE(corner_radius(1, 2, 3, 4), corner_radius(1, 2, 3, 5));
    EXPECT_EQ(corner_radius(5), corner_radius(5, 5, 5, 5));
    EXPECT_NE(corner_radius(5), corner_radius(5, 5, 5, 6));

    // CornerRadius.Equals: two default (non-parameterized) instances are equal; a default vs a
    // parameterized all-zero compares fields and is also equal (all corners zero).
    EXPECT_EQ(corner_radius(), corner_radius());
    EXPECT_EQ(corner_radius(), corner_radius(0));
    EXPECT_NE(corner_radius(), corner_radius(1));
}

// ============================ vector2 / vector4 ============================

TEST(vector2_tests, fields_and_equality)
{
    const vector2 v(3.0F, 4.0F);
    EXPECT_FLOAT_EQ(3.0F, v.x);
    EXPECT_FLOAT_EQ(4.0F, v.y);
    EXPECT_EQ(vector2(3, 4), v);
    EXPECT_NE(vector2(3, 4), vector2(3, 5));
    EXPECT_EQ(vector2(0, 0), vector2());
}

TEST(vector4_tests, fields_and_equality)
{
    const vector4 v(1.0F, 2.0F, 3.0F, 4.0F);
    EXPECT_FLOAT_EQ(1.0F, v.x);
    EXPECT_FLOAT_EQ(2.0F, v.y);
    EXPECT_FLOAT_EQ(3.0F, v.z);
    EXPECT_FLOAT_EQ(4.0F, v.w);
    EXPECT_EQ(vector4(1, 2, 3, 4), v);
    EXPECT_NE(vector4(1, 2, 3, 4), vector4(1, 2, 3, 5));
    EXPECT_EQ(vector4(0, 0, 0, 0), vector4());
}

// ============================ matrix3x2 ============================

TEST(matrix3x2_tests, identity)
{
    const matrix3x2 m = matrix3x2::identity();
    EXPECT_EQ(matrix3x2(1, 0, 0, 1, 0, 0), m);
}

TEST(matrix3x2_tests, multiply_by_identity_is_unchanged)
{
    const matrix3x2 m(2, 0, 0, 3, 5, 7);
    EXPECT_EQ(m, m * matrix3x2::identity());
    EXPECT_EQ(m, matrix3x2::identity() * m);
}

TEST(matrix3x2_tests, multiply_composes_translation_then_scale)
{
    // System.Numerics row-major: (a * b) applies a's transform first when transforming a row vector.
    // a = translate(10, 20); b = scale(2, 3).
    const matrix3x2 translate(1, 0, 0, 1, 10, 20);
    const matrix3x2 scale(2, 0, 0, 3, 0, 0);
    const matrix3x2 m = translate * scale;
    // Expected via the Matrix3x2.operator* formula:
    //   m11 = 1*2 + 0*0 = 2; m12 = 1*0 + 0*3 = 0;
    //   m21 = 0*2 + 1*0 = 0; m22 = 0*0 + 1*3 = 3;
    //   m31 = 10*2 + 20*0 + 0 = 20; m32 = 10*0 + 20*3 + 0 = 60.
    EXPECT_EQ(matrix3x2(2, 0, 0, 3, 20, 60), m);
}

TEST(matrix3x2_tests, transform_translation)
{
    const matrix3x2 translate(1, 0, 0, 1, 10, -5);
    EXPECT_EQ(vector2(13, 2), vector2::transform(vector2(3, 7), translate));
}

TEST(matrix3x2_tests, transform_scale)
{
    const matrix3x2 scale(2, 0, 0, 3, 0, 0);
    EXPECT_EQ(vector2(6, 21), vector2::transform(vector2(3, 7), scale));
}

TEST(matrix3x2_tests, transform_full_affine)
{
    // x*m11 + y*m21 + m31, x*m12 + y*m22 + m32.
    const matrix3x2 m(2, 1, 3, 4, 5, 6);
    // (1, 2): x = 1*2 + 2*3 + 5 = 13; y = 1*1 + 2*4 + 6 = 15.
    EXPECT_EQ(vector2(13, 15), vector2::transform(vector2(1, 2), m));
}

// ============================ point_f interop ============================

TEST(point_f_numerics, ctor_from_vector2)
{
    const point_f p(vector2(3.5F, -2.0F));
    EXPECT_EQ(point_f(3.5F, -2.0F), p);
}

TEST(point_f_numerics, implicit_from_vector2)
{
    // C#'s implicit operator PointF(Vector2): a vector2 converts to point_f without a cast.
    const point_f p = vector2(1.0F, 2.0F);
    EXPECT_EQ(point_f(1.0F, 2.0F), p);
}

TEST(point_f_numerics, explicit_to_vector2)
{
    const auto v = static_cast<vector2>(point_f(4.0F, 5.0F));
    EXPECT_EQ(vector2(4.0F, 5.0F), v);
}

TEST(point_f_numerics, transform_by_translation)
{
    // PointF.TransformBy: Vector2.Transform((Vector2)this, transform).
    const matrix3x2 translate(1, 0, 0, 1, 10, 20);
    EXPECT_EQ(point_f(11.0F, 22.0F), point_f(1.0F, 2.0F).transform_by(translate));
}

TEST(point_f_numerics, transform_by_identity_is_unchanged)
{
    EXPECT_EQ(point_f(3.0F, 4.0F), point_f(3.0F, 4.0F).transform_by(matrix3x2::identity()));
}

// ============================ color <-> vector4 ============================

TEST(color_numerics, ctor_from_vector4_rgba)
{
    // Color(Vector4): Red=X, Green=Y, Blue=Z, Alpha=W.
    const color c(vector4(0.1F, 0.2F, 0.3F, 0.4F));
    EXPECT_FLOAT_EQ(0.1F, c.red);
    EXPECT_FLOAT_EQ(0.2F, c.green);
    EXPECT_FLOAT_EQ(0.3F, c.blue);
    EXPECT_FLOAT_EQ(0.4F, c.alpha);
}

TEST(color_numerics, ctor_from_vector4_clamps)
{
    // Color(Vector4) clamps each component to [0, 1] (X.Clamp(0,1), ...).
    const color c(vector4(-0.5F, 1.5F, 0.5F, 2.0F));
    EXPECT_FLOAT_EQ(0.0F, c.red);
    EXPECT_FLOAT_EQ(1.0F, c.green);
    EXPECT_FLOAT_EQ(0.5F, c.blue);
    EXPECT_FLOAT_EQ(1.0F, c.alpha);
}

TEST(color_numerics, implicit_from_vector4)
{
    // C#'s implicit operator Color(Vector4).
    const color c = vector4(0.0F, 0.0F, 1.0F, 1.0F);
    EXPECT_EQ(color(0.0F, 0.0F, 1.0F, 1.0F), c);
}

TEST(color_numerics, to_vector4_roundtrip)
{
    const color c(0.25F, 0.5F, 0.75F, 0.9F);
    const vector4 v = c.to_vector4();
    EXPECT_EQ(vector4(0.25F, 0.5F, 0.75F, 0.9F), v);
    // Round-trips back to an equal color.
    EXPECT_EQ(c, color(v));
}

// ============================ path_f::transform ============================

TEST(path_f_numerics, transform_translates_all_points)
{
    // PathF.Transform(Matrix3x2): every point goes through Vector2.Transform.
    path_f p;
    p.move_to(0, 0).line_to(10, 0).line_to(10, 10);
    p.transform(matrix3x2(1, 0, 0, 1, 5, 7)); // translate (5, 7)
    EXPECT_EQ(point_f(5, 7), p[0]);
    EXPECT_EQ(point_f(15, 7), p[1]);
    EXPECT_EQ(point_f(15, 17), p[2]);
}

TEST(path_f_numerics, transform_scales_all_points)
{
    path_f p;
    p.move_to(1, 2).line_to(3, 4);
    p.transform(matrix3x2(2, 0, 0, 3, 0, 0)); // scale (2, 3)
    EXPECT_EQ(point_f(2, 6), p[0]);
    EXPECT_EQ(point_f(6, 12), p[1]);
}

TEST(path_f_numerics, transform_identity_is_unchanged)
{
    path_f p;
    p.move_to(1, 2).line_to(3, 4).close();
    path_f before = p;
    p.transform(matrix3x2::identity());
    EXPECT_EQ(before, p);
}

// ===================== path_f four-radius rounded rectangle =====================

TEST(path_f_rounded_rect, four_radius_overload_structure)
{
    // PathF.AppendRoundedRectangle(x, y, w, h, tl, tr, bl, br): move + 4 cubics + 3 lines + close.
    path_f p;
    p.append_rounded_rectangle(0.0F, 0.0F, 60.0F, 40.0F, 8.0F, 4.0F, 2.0F, 6.0F);
    EXPECT_EQ(9, p.operation_count());
    EXPECT_TRUE(p.closed());
    EXPECT_EQ(path_operation::move, p.get_segment_type(0));

    // The flattened bounds span the rectangle (with the small corner-Bézier overshoot the curve tests
    // also tolerate).
    const rect_f bounds = p.get_bounds_by_flattening();
    EXPECT_NEAR(0.0F, bounds.x, 0.5F);
    EXPECT_NEAR(0.0F, bounds.y, 0.5F);
    EXPECT_NEAR(60.0F, bounds.width, 0.5F);
    EXPECT_NEAR(40.0F, bounds.height, 0.5F);
}

TEST(path_f_rounded_rect, four_equal_radii_match_uniform_overload)
{
    // All four corners equal => identical path to the uniform overload (same op count, same points).
    path_f four;
    four.append_rounded_rectangle(0.0F, 0.0F, 50.0F, 30.0F, 5.0F, 5.0F, 5.0F, 5.0F);

    path_f uniform;
    uniform.append_rounded_rectangle(0.0F, 0.0F, 50.0F, 30.0F, 5.0F);

    EXPECT_EQ(uniform, four);
}

TEST(path_f_rounded_rect, four_radius_first_point_uses_top_left)
{
    // RoundRectangle.GetPath starts the path at (minX, minY + topLeftRadius).
    path_f p;
    p.append_rounded_rectangle(0.0F, 0.0F, 60.0F, 40.0F, 8.0F, 4.0F, 2.0F, 6.0F);
    EXPECT_EQ(point_f(0.0F, 8.0F), p.first_point());
}

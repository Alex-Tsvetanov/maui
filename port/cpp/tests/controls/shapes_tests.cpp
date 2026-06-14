// Tests for the maui::controls::shapes GEOMETRY + TRANSFORM model and the path markup parser —
// ported from src/Controls/tests/Core.UnitTests/GeometryTests.cs (the FlattenArc theory + the four
// geometry construction tests), PolylineTests.cs (the PointCollectionConverter parse),
// Shapes/PathGeometryConverterTests.cs (the empty-input parse), plus source-derived suites pinning
// the transform Value matrices (RotateTransform/ScaleTransform/SkewTransform/TranslateTransform/
// MatrixTransform/TransformGroup/CompositeTransform.OnTransformPropertyChanged — untested in C#) and
// the PathFigureCollectionConverter grammar (M/L/H/V/C/S/Q/T/A/Z, relative forms, the F prefix,
// error cases).
#include "maui/controls/shapes/path_geometry.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

#include "maui/controls/shapes/composite_transform.hpp"
#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry_group.hpp"
#include "maui/controls/shapes/geometry_helper.hpp"
#include "maui/controls/shapes/line_geometry.hpp"
#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/matrix_transform.hpp"
#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/path_segment.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/shapes/rotate_transform.hpp"
#include "maui/controls/shapes/round_rectangle_geometry.hpp"
#include "maui/controls/shapes/scale_transform.hpp"
#include "maui/controls/shapes/skew_transform.hpp"
#include "maui/controls/shapes/sweep_direction.hpp"
#include "maui/controls/shapes/transform.hpp"
#include "maui/controls/shapes/transform_group.hpp"
#include "maui/controls/shapes/translate_transform.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::graphics::corner_radius;
    using maui::graphics::path_f;
    using maui::graphics::path_operation;
    using maui::graphics::point;
    using maui::graphics::rect;
    namespace shapes = maui::controls::shapes;

    void expect_matrix_near(const shapes::matrix& actual, const shapes::matrix& expected, double tolerance = 1e-12)
    {
        EXPECT_NEAR(actual.m11, expected.m11, tolerance);
        EXPECT_NEAR(actual.m12, expected.m12, tolerance);
        EXPECT_NEAR(actual.m21, expected.m21, tolerance);
        EXPECT_NEAR(actual.m22, expected.m22, tolerance);
        EXPECT_NEAR(actual.offset_x, expected.offset_x, tolerance);
        EXPECT_NEAR(actual.offset_y, expected.offset_y, tolerance);
    }

    // ---- GeometryTests.cs ----

    struct flatten_arc_case
    {
        double angle;
        bool is_large_arc;
    };

    class geometry_flatten_arc : public ::testing::TestWithParam<flatten_arc_case>
    {
    };

    TEST_P(geometry_flatten_arc, flatten_arc_produces_points) // C# GeometryTests.FlattenArcTest
    {
        const flatten_arc_case& param = GetParam();

        shapes::arc_segment arc_segment;
        arc_segment.set_point(point(10, 100));
        arc_segment.set_size({100, 50});
        arc_segment.set_rotation_angle(param.angle);
        arc_segment.set_is_large_arc(param.is_large_arc);

        std::vector<point> points;
        shapes::flatten_arc(points, point(0, 0), arc_segment.point(), arc_segment.size().width,
                            arc_segment.size().height, arc_segment.rotation_angle(), arc_segment.is_large_arc(),
                            arc_segment.sweep_direction() == shapes::sweep_direction::counter_clockwise, 1);

        EXPECT_FALSE(points.empty());
    }

    INSTANTIATE_TEST_SUITE_P(angles, geometry_flatten_arc,
                             ::testing::Values(flatten_arc_case{0, true}, flatten_arc_case{0, false},
                                               flatten_arc_case{45, true}, flatten_arc_case{45, false},
                                               flatten_arc_case{180, true}, flatten_arc_case{180, false},
                                               flatten_arc_case{270, true}, flatten_arc_case{270, false}));

    TEST(geometry, line_geometry_construction) // C# TestRoundLineGeometryConstruction
    {
        const shapes::line_geometry line_geometry(point(0, 0), point(100, 100));
        EXPECT_EQ(line_geometry.start_point().x, 0);
        EXPECT_EQ(line_geometry.start_point().y, 0);
        EXPECT_EQ(line_geometry.end_point().x, 100);
        EXPECT_EQ(line_geometry.end_point().y, 100);
    }

    TEST(geometry, ellipse_geometry_construction) // C# TestEllipseGeometryConstruction
    {
        const shapes::ellipse_geometry ellipse_geometry(point(50, 50), 10, 20);
        EXPECT_EQ(ellipse_geometry.center().x, 50);
        EXPECT_EQ(ellipse_geometry.center().y, 50);
        EXPECT_EQ(ellipse_geometry.radius_x(), 10);
        EXPECT_EQ(ellipse_geometry.radius_y(), 20);
    }

    TEST(geometry, rectangle_geometry_construction) // C# TestRectangleGeometryConstruction
    {
        const shapes::rectangle_geometry rectangle_geometry{rect(0, 0, 150, 150)};
        EXPECT_EQ(rectangle_geometry.rect().height, 150);
        EXPECT_EQ(rectangle_geometry.rect().width, 150);
    }

    TEST(geometry, round_rectangle_geometry_construction) // C# TestRoundRectangleGeometryConstruction
    {
        const shapes::round_rectangle_geometry round_rect(corner_radius(12, 0, 0, 12), rect(0, 0, 150, 150));
        EXPECT_EQ(round_rect.corner_radius().top_left, 12);
        EXPECT_EQ(round_rect.corner_radius().top_right, 0);
        EXPECT_EQ(round_rect.corner_radius().bottom_left, 0);
        EXPECT_EQ(round_rect.corner_radius().bottom_right, 12);
        EXPECT_EQ(round_rect.rect().height, 150);
        EXPECT_EQ(round_rect.rect().width, 150);
    }

    // ---- geometry → path_f appends (the AppendPath bodies, source-derived) ----

    TEST(geometry, line_geometry_appends_move_and_line)
    {
        const shapes::line_geometry line_geometry(point(1, 2), point(3, 4));
        path_f path;
        line_geometry.append_path(path);
        ASSERT_EQ(path.operation_count(), 2);
        EXPECT_EQ(path.get_segment_type(0), path_operation::move);
        EXPECT_EQ(path.get_segment_type(1), path_operation::line);
        EXPECT_EQ(path[0], maui::graphics::point_f(1, 2));
        EXPECT_EQ(path[1], maui::graphics::point_f(3, 4));
    }

    TEST(geometry, ellipse_geometry_appends_the_center_radius_box)
    {
        const shapes::ellipse_geometry ellipse_geometry(point(50, 50), 10, 20);
        path_f path;
        ellipse_geometry.append_path(path);

        path_f expected;
        expected.append_ellipse(40, 30, 20, 40); // (cx - rx, cy - ry, 2rx, 2ry)
        EXPECT_TRUE(path == expected);
    }

    TEST(geometry, geometry_group_appends_every_child)
    {
        shapes::geometry_group group;
        EXPECT_EQ(group.fill_rule(), shapes::fill_rule::even_odd); // C# default
        group.children().push_back(std::make_shared<shapes::line_geometry>(point(0, 0), point(5, 5)));
        group.children().push_back(std::make_shared<shapes::rectangle_geometry>(rect(0, 0, 4, 4)));

        path_f path;
        group.append_path(path);

        path_f expected;
        expected.move_to(0, 0).line_to(5, 5);
        expected.append_rectangle(0, 0, 4, 4);
        EXPECT_TRUE(path == expected);
    }

    TEST(geometry, path_geometry_walks_figures_segments_and_close)
    {
        auto figure = std::make_shared<shapes::path_figure>();
        figure->set_start_point(point(10, 10));
        figure->set_is_closed(true);
        figure->segments().push_back(std::make_shared<shapes::line_segment>(point(20, 10)));
        figure->segments().push_back(
            std::make_shared<shapes::bezier_segment>(point(25, 10), point(30, 15), point(30, 20)));
        figure->segments().push_back(std::make_shared<shapes::quadratic_bezier_segment>(point(25, 25), point(20, 20)));
        figure->segments().push_back(
            std::make_shared<shapes::poly_line_segment>(shapes::point_collection{{15, 20}, {12, 18}}));

        const shapes::path_geometry geometry(shapes::path_figure_collection{figure});
        EXPECT_EQ(geometry.fill_rule(), shapes::fill_rule::even_odd); // C# default

        path_f path;
        geometry.append_path(path);

        path_f expected;
        expected.move_to(10, 10);
        expected.line_to(20, 10);
        expected.curve_to(25, 10, 30, 15, 30, 20);
        expected.quad_to(25, 25, 20, 20);
        expected.line_to(15, 20).line_to(12, 18);
        expected.close();
        EXPECT_TRUE(path == expected);
    }

    TEST(geometry, path_geometry_arc_segment_flattens_into_lines)
    {
        auto figure = std::make_shared<shapes::path_figure>();
        figure->set_start_point(point(0, 0));
        figure->segments().push_back(std::make_shared<shapes::arc_segment>(
            point(10, 100), maui::graphics::size(100, 50), 0.0, shapes::sweep_direction::counter_clockwise, false));

        const shapes::path_geometry geometry(shapes::path_figure_collection{figure});
        path_f path;
        geometry.append_path(path);

        // a Move followed only by the flattened polyline.
        ASSERT_GE(path.operation_count(), 2);
        EXPECT_EQ(path.get_segment_type(0), path_operation::move);
        for (int i = 1; i < path.operation_count(); i++)
        {
            EXPECT_EQ(path.get_segment_type(i), path_operation::line);
        }
    }

    TEST(geometry, poly_bezier_ignores_a_partial_triple) // C# AddPolyBezier's bez + 2 bound
    {
        auto figure = std::make_shared<shapes::path_figure>();
        figure->set_start_point(point(0, 0));
        figure->segments().push_back(std::make_shared<shapes::poly_bezier_segment>(
            shapes::point_collection{{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}})); // 5 points: one curve + leftovers

        const shapes::path_geometry geometry(shapes::path_figure_collection{figure});
        path_f path;
        geometry.append_path(path);

        path_f expected;
        expected.move_to(0, 0);
        expected.curve_to(1, 1, 2, 2, 3, 3);
        EXPECT_TRUE(path == expected);
    }

    // ---- the matrix (Matrix.cs, source-derived) ----

    TEST(shapes_matrix, defaults_to_identity_and_multiplies)
    {
        const shapes::matrix identity;
        EXPECT_TRUE(identity.is_identity());

        const shapes::matrix a(1, 2, 3, 4, 5, 6);
        const shapes::matrix product = shapes::matrix::multiply(a, shapes::matrix::identity());
        EXPECT_EQ(product, a);

        const shapes::matrix b(7, 8, 9, 10, 11, 12);
        // row-vector composition: (a*b).offset = a.offset * b + b.offset
        const shapes::matrix ab = shapes::matrix::multiply(a, b);
        expect_matrix_near(ab, shapes::matrix(25, 28, 57, 64, 100, 112));
    }

    TEST(shapes_matrix, transform_applies_the_affine_map)
    {
        shapes::matrix m;
        m.translate(10, 20);
        const point moved = m.transform(point(1, 2));
        EXPECT_EQ(moved.x, 11);
        EXPECT_EQ(moved.y, 22);

        shapes::matrix r;
        r.rotate(90);
        const point rotated = r.transform(point(1, 0));
        EXPECT_NEAR(rotated.x, 0, 1e-12);
        EXPECT_NEAR(rotated.y, 1, 1e-12);
    }

    TEST(shapes_matrix, rotate_at_pivots_around_the_center)
    {
        shapes::matrix m;
        m.rotate_at(180, 5, 5);
        const point rotated = m.transform(point(0, 0));
        EXPECT_NEAR(rotated.x, 10, 1e-12);
        EXPECT_NEAR(rotated.y, 10, 1e-12);
    }

    TEST(shapes_matrix, invert_round_trips_and_rejects_singular)
    {
        shapes::matrix m(2, 0, 0, 4, 10, 20);
        EXPECT_EQ(m.determinant(), 8);
        ASSERT_TRUE(m.has_inverse());
        m.invert();
        const point back = m.transform(point(12, 24)); // (1,1) through the original
        EXPECT_NEAR(back.x, 1, 1e-12);
        EXPECT_NEAR(back.y, 1, 1e-12);

        shapes::matrix singular(0, 0, 0, 0, 0, 0);
        EXPECT_FALSE(singular.has_inverse());
        EXPECT_THROW(singular.invert(), std::logic_error);
    }

    TEST(shapes_matrix, to_matrix3x2_carries_the_six_cells) // MatrixExtensions.ToMatrix3X2
    {
        const maui::graphics::matrix3x2 m = shapes::to_matrix3x2(shapes::matrix(1, 2, 3, 4, 5, 6));
        EXPECT_EQ(m, maui::graphics::matrix3x2(1, 2, 3, 4, 5, 6));
    }

    // ---- the transforms (the C# OnTransformPropertyChanged matrices, source-derived) ----

    TEST(transforms, translate_transform_value)
    {
        const shapes::translate_transform t(7, -3);
        expect_matrix_near(t.value(), shapes::matrix(1, 0, 0, 1, 7, -3));
    }

    TEST(transforms, scale_transform_value_centers)
    {
        const shapes::scale_transform s(2, 3, 10, 20);
        // (sx, 0, 0, sy, cx(1-sx), cy(1-sy))
        expect_matrix_near(s.value(), shapes::matrix(2, 0, 0, 3, -10, -40));
    }

    TEST(transforms, rotate_transform_value_centers)
    {
        const shapes::rotate_transform r(90, 1, 0);
        // (cos, sin, -sin, cos, cx(1-cos)+cy*sin, cy(1-cos)-cx*sin)
        expect_matrix_near(r.value(), shapes::matrix(0, 1, -1, 0, 1, -1));
    }

    TEST(transforms, skew_transform_value)
    {
        const shapes::skew_transform k(45, 0, 0, 10);
        // (1, tanY, tanX, 1, -cy*tanX, -cx*tanY)
        expect_matrix_near(k.value(), shapes::matrix(1, 0, 1, 1, -10, 0));
    }

    TEST(transforms, matrix_transform_value_is_the_stored_matrix)
    {
        shapes::matrix_transform t;
        t.set_matrix(shapes::matrix(1, 2, 3, 4, 5, 6));
        EXPECT_EQ(t.value(), shapes::matrix(1, 2, 3, 4, 5, 6));
    }

    TEST(transforms, transform_group_folds_children_in_order)
    {
        auto group = std::make_shared<shapes::transform_group>();
        group->children().push_back(std::make_shared<shapes::scale_transform>(2, 2));
        group->children().push_back(std::make_shared<shapes::translate_transform>(5, 5));

        // Multiply(scale, translate): scale then move.
        const point mapped = group->value().transform(point(1, 1));
        EXPECT_NEAR(mapped.x, 7, 1e-12);
        EXPECT_NEAR(mapped.y, 7, 1e-12);
    }

    TEST(transforms, composite_transform_matches_the_equivalent_group)
    {
        shapes::composite_transform composite;
        composite.set_center_x(10);
        composite.set_center_y(20);
        composite.set_scale_x(2);
        composite.set_scale_y(3);
        composite.set_skew_x(15);
        composite.set_skew_y(30);
        composite.set_rotation(45);
        composite.set_translate_x(7);
        composite.set_translate_y(-7);

        // The exact six-child group CompositeTransform.OnTransformPropertyChanged composes.
        shapes::transform_group group;
        group.children().push_back(std::make_shared<shapes::translate_transform>(-10, -20));
        group.children().push_back(std::make_shared<shapes::scale_transform>(2, 3));
        group.children().push_back(std::make_shared<shapes::skew_transform>(15, 30));
        group.children().push_back(std::make_shared<shapes::rotate_transform>(45));
        group.children().push_back(std::make_shared<shapes::translate_transform>(10, 20));
        group.children().push_back(std::make_shared<shapes::translate_transform>(7, -7));

        expect_matrix_near(composite.value(), group.value());
    }

    TEST(transforms, base_transform_holds_a_settable_value)
    {
        shapes::transform t;
        EXPECT_TRUE(t.value().is_identity());
        t.set_value(shapes::matrix(2, 0, 0, 2, 0, 0));
        EXPECT_EQ(t.value(), shapes::matrix(2, 0, 0, 2, 0, 0));
    }

    // ---- PointCollectionConverter (PolylineTests.cs) ----

    TEST(point_collection_parser, parses_the_polyline_test_string) // C# CreatePolylineFromStringPointCollectionTest
    {
        const shapes::point_collection points =
            shapes::parse_point_collection("0 48, 0 144, 96 150, 100 0, 192 0, 192 96, 50 96, 48 192, 150 200 144 48");
        ASSERT_EQ(points.size(), 10U);
        EXPECT_EQ(points[0], point(0, 48));
        EXPECT_EQ(points[9], point(144, 48));
    }

    TEST(point_collection_parser, rejects_an_odd_count_and_bad_tokens)
    {
        EXPECT_THROW((void)shapes::parse_point_collection("1 2 3"), std::invalid_argument);
        EXPECT_THROW((void)shapes::parse_point_collection("1 x"), std::invalid_argument);
    }

    // ---- PathGeometryConverter / PathFigureCollectionConverter ----

    TEST(path_markup_parser, empty_input_yields_empty_figures) // C# PathGeometryConverterTests.ConvertNullTest
    {
        const shapes::path_geometry geometry = shapes::parse_path_geometry("");
        EXPECT_TRUE(geometry.figures().empty());
    }

    TEST(path_markup_parser, parses_move_and_lines)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 L10,10 20,20");
        ASSERT_EQ(figures.size(), 1U);
        EXPECT_EQ(figures[0]->start_point(), point(0, 0));
        ASSERT_EQ(figures[0]->segments().size(), 2U);
        const auto* first = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(first, nullptr);
        EXPECT_EQ(first->point(), point(10, 10));
        const auto* second = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[1].get());
        ASSERT_NE(second, nullptr);
        EXPECT_EQ(second->point(), point(20, 20));
        EXPECT_FALSE(figures[0]->is_closed());
    }

    TEST(path_markup_parser, relative_h_v_and_close)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "m5,5 l5,0 v10 h-5 Z");
        ASSERT_EQ(figures.size(), 1U);
        EXPECT_EQ(figures[0]->start_point(), point(5, 5));
        ASSERT_EQ(figures[0]->segments().size(), 3U);
        const auto* l = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(l, nullptr);
        EXPECT_EQ(l->point(), point(10, 5));
        const auto* v = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[1].get());
        ASSERT_NE(v, nullptr);
        EXPECT_EQ(v->point(), point(10, 15));
        const auto* h = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[2].get());
        ASSERT_NE(h, nullptr);
        EXPECT_EQ(h->point(), point(5, 15));
        EXPECT_TRUE(figures[0]->is_closed());
    }

    TEST(path_markup_parser, cubic_and_smooth_cubic_reflection)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 C1,1 2,2 3,3 S5,5 6,6");
        ASSERT_EQ(figures.size(), 1U);
        ASSERT_EQ(figures[0]->segments().size(), 2U);

        const auto* curve = dynamic_cast<const shapes::bezier_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(curve, nullptr);
        EXPECT_EQ(curve->point1(), point(1, 1));
        EXPECT_EQ(curve->point2(), point(2, 2));
        EXPECT_EQ(curve->point3(), point(3, 3));

        // S after C reflects the previous control point: 2*(3,3) - (2,2) = (4,4).
        const auto* smooth = dynamic_cast<const shapes::bezier_segment*>(figures[0]->segments()[1].get());
        ASSERT_NE(smooth, nullptr);
        EXPECT_EQ(smooth->point1(), point(4, 4));
        EXPECT_EQ(smooth->point2(), point(5, 5));
        EXPECT_EQ(smooth->point3(), point(6, 6));
    }

    TEST(path_markup_parser, quadratic_and_smooth_quadratic_reflection)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 Q1,2 3,4 T7,8");
        ASSERT_EQ(figures.size(), 1U);
        ASSERT_EQ(figures[0]->segments().size(), 2U);

        const auto* quad = dynamic_cast<const shapes::quadratic_bezier_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(quad, nullptr);
        EXPECT_EQ(quad->point1(), point(1, 2));
        EXPECT_EQ(quad->point2(), point(3, 4));

        // T after Q reflects the previous control point: 2*(3,4) - (1,2) = (5,6).
        const auto* smooth = dynamic_cast<const shapes::quadratic_bezier_segment*>(figures[0]->segments()[1].get());
        ASSERT_NE(smooth, nullptr);
        EXPECT_EQ(smooth->point1(), point(5, 6));
        EXPECT_EQ(smooth->point2(), point(7, 8));
    }

    TEST(path_markup_parser, arc_command_carries_all_flags)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 A 3,4 5, 1, 0, 6,7");
        ASSERT_EQ(figures.size(), 1U);
        ASSERT_EQ(figures[0]->segments().size(), 1U);
        const auto* arc = dynamic_cast<const shapes::arc_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(arc, nullptr);
        EXPECT_EQ(arc->size().width, 3);
        EXPECT_EQ(arc->size().height, 4);
        EXPECT_EQ(arc->rotation_angle(), 5);
        EXPECT_TRUE(arc->is_large_arc());
        EXPECT_EQ(arc->sweep_direction(), shapes::sweep_direction::counter_clockwise);
        EXPECT_EQ(arc->point(), point(6, 7));
    }

    TEST(path_markup_parser, multiple_points_after_move_become_lines)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 10,10 20,20");
        ASSERT_EQ(figures.size(), 1U);
        EXPECT_EQ(figures[0]->segments().size(), 2U);
    }

    TEST(path_markup_parser, fill_rule_prefix_is_accepted)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, " F1 M0,0 L1,1");
        ASSERT_EQ(figures.size(), 1U);
        EXPECT_EQ(figures[0]->segments().size(), 1U);
    }

    TEST(path_markup_parser, decimals_exponents_and_negative_numbers)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M-1.5,2.25 L1e2,-3.5E-1");
        ASSERT_EQ(figures.size(), 1U);
        EXPECT_EQ(figures[0]->start_point(), point(-1.5, 2.25));
        const auto* line = dynamic_cast<const shapes::line_segment*>(figures[0]->segments()[0].get());
        ASSERT_NE(line, nullptr);
        EXPECT_EQ(line->point(), point(100, -0.35));
    }

    TEST(path_markup_parser, rejects_bad_input)
    {
        shapes::path_figure_collection figures;
        // a path must start with M|m
        EXPECT_THROW(shapes::parse_path_figure_collection(figures, "L0,0"), std::invalid_argument);
        // 'F' must be followed by 0|1
        EXPECT_THROW(shapes::parse_path_figure_collection(figures, "F2 M0,0"), std::invalid_argument);
        // an unknown command letter
        EXPECT_THROW(shapes::parse_path_figure_collection(figures, "M0,0 X1,1"), std::invalid_argument);
        // a dangling comma is only allowed between numbers
        EXPECT_THROW(shapes::parse_path_figure_collection(figures, "M0,0,"), std::invalid_argument);
    }

    TEST(path_markup_parser, second_move_opens_a_second_figure)
    {
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 L1,1 M10,10 L11,11");
        ASSERT_EQ(figures.size(), 2U);
        EXPECT_EQ(figures[1]->start_point(), point(10, 10));
    }
} // namespace

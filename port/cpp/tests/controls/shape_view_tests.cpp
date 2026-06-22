// Tests for the shape CONTROL family + the shape_view_handler seam — source-derived from Shape.cs
// (the property defaults, GetPath bodies, PathForBounds/TransformPathForBounds aspect fitting,
// MeasureOverride) and the ShapeViewHandler/ShapeDrawable recipe (the golden-op replay over the
// headless drawing seat standing in for MauiShapeView's drawRect). The C# unit suite covers the
// geometry model (ported in shapes_tests.cpp); the control surface is DeviceTest-only there, so
// these pin the source behavior per the porting doctrine.
//
// §8 teardown order: each control is declared before its handler (the handler borrows the control).
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_segment.hpp"

#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/shapes/shape.hpp"
#include "maui/controls/shapes/translate_transform.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/recording_canvas.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/winding_mode.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::path_aspect;
    using maui::core::shape_view_handler;
    using maui::graphics::canvas_op;
    using maui::graphics::color;
    using maui::graphics::line_cap;
    using maui::graphics::line_join;
    using maui::graphics::path_f;
    using maui::graphics::point_f;
    using maui::graphics::recording_canvas;
    using maui::graphics::rect_f;
    using maui::graphics::winding_mode;
    namespace shapes = maui::controls::shapes;
    namespace ops = maui::graphics::canvas_ops;

    // ---- the shape base surface (Shape.cs defaults) ----

    TEST(shape_controls, defaults_match_the_csharp_property_defaults)
    {
        const shapes::line view; // line keeps the base Aspect default (None)
        EXPECT_EQ(view.fill(), nullptr);
        EXPECT_EQ(view.stroke(), nullptr);
        EXPECT_EQ(view.stroke_thickness(), 1.0);
        EXPECT_TRUE(view.stroke_dash_array().empty());
        EXPECT_TRUE(view.stroke_dash_pattern().empty());
        EXPECT_EQ(view.stroke_dash_offset(), 0.0F);
        EXPECT_EQ(view.stroke_line_cap(), line_cap::butt);    // PenLineCap.Flat
        EXPECT_EQ(view.stroke_line_join(), line_join::miter); // PenLineJoin.Miter
        EXPECT_EQ(view.stroke_miter_limit(), 10.0F);
        EXPECT_EQ(view.aspect(), path_aspect::none); // Stretch.None
        // IShapeView.Shape => this (through the contract — inside a shape-derived class the bare
        // `shape` name is the injected class name, so callers go through i_shape_view).
        const maui::core::i_shape_view& contract = view;
        EXPECT_EQ(contract.shape(), static_cast<const maui::graphics::i_shape*>(&view));
    }

    TEST(shape_controls, rectangle_and_ellipse_default_to_fill_aspect) // the C# ctor overrides
    {
        EXPECT_EQ(shapes::rectangle().aspect(), path_aspect::stretch); // Stretch.Fill
        EXPECT_EQ(shapes::ellipse().aspect(), path_aspect::stretch);
    }

    // ---- GetPath (the C# bodies) ----

    TEST(shape_controls, rectangle_path_insets_by_half_the_stroke) // Rectangle.GetPath
    {
        shapes::rectangle view;
        view.set_stroke_thickness(2);
        const path_f path = view.path_for_bounds(maui::graphics::rect(0, 0, 100, 50));

        path_f expected;
        expected.append_rectangle(1, 1, 98, 48);
        EXPECT_TRUE(path == expected); // aspect Fill over the already-fitting box is the identity
    }

    TEST(shape_controls, rectangle_radius_switches_to_the_rounded_append) // Rectangle.GetPath (radius)
    {
        shapes::rectangle view;
        view.set_radius_x(6);
        view.set_radius_y(4);
        view.set_stroke_thickness(0);
        const path_f path = view.path_for_bounds(maui::graphics::rect(0, 0, 100, 50));

        path_f expected;
        expected.append_rounded_rectangle(0, 0, 100, 50, 6); // max(RadiusX, RadiusY)
        EXPECT_TRUE(path == expected);
    }

    TEST(shape_controls, ellipse_path_is_the_inset_ellipse) // Ellipse.GetPath
    {
        shapes::ellipse view;
        view.set_stroke_thickness(4);
        view.set_frame(maui::graphics::rect(0, 0, 40, 20)); // arranged → WidthForPathComputation

        const path_f path = view.get_path();
        path_f expected;
        expected.append_ellipse(2, 2, 36, 16);
        EXPECT_TRUE(path == expected);
        // (path_for_bounds additionally runs the Fill fitting over the flattened curve bounds,
        // which is not an exact identity for an ellipse — exactly as in C#.)
    }

    TEST(shape_controls, line_path_is_move_then_line) // Line.GetPath
    {
        const shapes::line view(1, 2, 30, 40);
        const path_f path = view.get_path();
        ASSERT_EQ(path.operation_count(), 2);
        EXPECT_EQ(path[0], point_f(1, 2));
        EXPECT_EQ(path[1], point_f(30, 40));
    }

    TEST(shape_controls, polygon_closes_and_polyline_does_not) // Polygon/Polyline.GetPath
    {
        const shapes::point_collection points{{0, 0}, {10, 0}, {10, 10}};

        const shapes::polygon closed(points);
        EXPECT_TRUE(closed.get_path().closed());

        const shapes::polyline open(points);
        EXPECT_FALSE(open.get_path().closed());
        EXPECT_EQ(open.get_path().count(), 3);
    }

    TEST(shape_controls, polyline_from_the_parsed_point_collection) // C# PolylineTests, the control half
    {
        const shapes::point_collection points =
            shapes::parse_point_collection("0 48, 0 144, 96 150, 100 0, 192 0, 192 96, 50 96, 48 192, 150 200 144 48");
        const shapes::polyline polyline(points);
        EXPECT_EQ(polyline.points().size(), 10U);
    }

    TEST(shape_controls, fill_rule_maps_to_the_winding_mode) // PolygonHandler.MapFillRule's mapping
    {
        shapes::polygon view;
        EXPECT_EQ(view.fill_rule(), shapes::fill_rule::even_odd); // the C# default
        EXPECT_EQ(view.fill_winding(), winding_mode::even_odd);
        view.set_fill_rule(shapes::fill_rule::nonzero);
        EXPECT_EQ(view.fill_winding(), winding_mode::non_zero);
    }

    // ---- TransformPathForBounds (Shape.cs aspect fitting) ----

    TEST(shape_controls, aspect_fill_stretches_the_path_into_the_bounds)
    {
        // a path twice as large as the (stroke-inset) bounds must scale down by 0.5 under Fill.
        shapes::path view;
        view.set_stroke_thickness(0);
        view.set_aspect(path_aspect::stretch);
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 L200,0 L200,100 L0,100 Z");
        view.set_data(std::make_shared<shapes::path_geometry>(std::move(figures)));

        const path_f fitted = view.path_for_bounds(maui::graphics::rect(0, 0, 100, 50));
        const rect_f bounds = fitted.get_bounds_by_flattening(1);
        EXPECT_NEAR(bounds.width, 100, 1e-3);
        EXPECT_NEAR(bounds.height, 50, 1e-3);
    }

    TEST(shape_controls, aspect_uniform_keeps_the_ratio_and_centers)
    {
        shapes::path view;
        view.set_stroke_thickness(0);
        view.set_aspect(path_aspect::aspect_fit); // Stretch.Uniform
        shapes::path_figure_collection figures;
        shapes::parse_path_figure_collection(figures, "M0,0 L200,0 L200,200 L0,200 Z"); // a square
        view.set_data(std::make_shared<shapes::path_geometry>(std::move(figures)));

        const path_f fitted = view.path_for_bounds(maui::graphics::rect(0, 0, 100, 50));
        const rect_f bounds = fitted.get_bounds_by_flattening(1);
        EXPECT_NEAR(bounds.width, 50, 1e-3); // min scale 0.25 → 50x50, centered horizontally
        EXPECT_NEAR(bounds.height, 50, 1e-3);
        EXPECT_NEAR(bounds.x, 25, 1e-3);
        EXPECT_NEAR(bounds.y, 0, 1e-3);
    }

    TEST(shape_controls, aspect_none_translates_into_the_bounds)
    {
        // a path left of the bounds shifts right to the (stroke-inset) left edge.
        shapes::line view(-10, 0, -5, 5);
        view.set_stroke_thickness(0);
        const path_f fitted = view.path_for_bounds(maui::graphics::rect(0, 0, 100, 50));
        const rect_f bounds = fitted.get_bounds_by_flattening(1);
        EXPECT_NEAR(bounds.x, 0, 1e-3);
    }

    // ---- MeasureOverride (Shape.cs) ----

    TEST(shape_controls, measure_fill_uses_the_requests_and_adds_the_stroke)
    {
        shapes::rectangle view; // Aspect = Fill
        view.set_width_request(100);
        view.set_height_request(50);
        const maui::graphics::size measured =
            view.measure(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        // the Fill branch takes the requests; the outer resolve clamps to them exactly.
        EXPECT_EQ(measured.width, 100.0);
        EXPECT_EQ(measured.height, 50.0);
    }

    TEST(shape_controls, measure_none_reports_the_path_extent_plus_stroke)
    {
        shapes::line view(0, 0, 30, 40); // Aspect = None
        const maui::graphics::size measured = view.measure(1000, 1000);
        // path bounds 30x40 + offset 0 + thickness 1.
        EXPECT_NEAR(measured.width, 31, 1e-6);
        EXPECT_NEAR(measured.height, 41, 1e-6);
    }

    // ---- the handler seam: golden-op replay (ShapeDrawable.Draw) ----

    TEST(shape_view_seam, replay_fills_then_strokes) // the C# fill-before-stroke order
    {
        shapes::rectangle view;
        view.set_fill(std::make_shared<maui::graphics::solid_paint>(color(1.0F, 0.0F, 0.0F)));
        view.set_stroke(std::make_shared<maui::graphics::solid_paint>(color(0.0F, 0.0F, 1.0F)));
        view.set_stroke_thickness(2);
        view.set_stroke_line_cap(line_cap::round);
        view.set_stroke_line_join(line_join::bevel);
        view.set_stroke_miter_limit(4);

        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->drawable.shape_view(), &view);

        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 100, 50));
        const std::vector<canvas_op>& recorded = canvas.ops();

        // the fill pass: save, transparent fill staged, the path clip, the fill paint, the fill.
        std::size_t i = 0;
        auto next_is = [&recorded, &i]<class Op>(std::in_place_type_t<Op> /*tag*/) -> const Op* {
            for (; i < recorded.size(); i++)
            {
                if (const Op* found = std::get_if<Op>(&recorded[i]))
                {
                    i++;
                    return found;
                }
            }
            return nullptr;
        };

        const auto* clip = next_is(std::in_place_type<ops::clip_path>);
        ASSERT_NE(clip, nullptr);
        EXPECT_EQ(clip->winding, winding_mode::non_zero); // the ShapeDrawable default

        const auto* fill_paint = next_is(std::in_place_type<ops::set_fill_paint>);
        ASSERT_NE(fill_paint, nullptr);
        EXPECT_EQ(fill_paint->background_color, color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(fill_paint->rectangle, rect_f(0, 0, 100, 50)); // the fill maps the dirty rect

        const auto* filled = next_is(std::in_place_type<ops::fill_path>);
        ASSERT_NE(filled, nullptr);
        EXPECT_EQ(filled->winding, winding_mode::non_zero); // a rectangle's fill rule is NonZero

        // the stroke pass, after the fill: the staged stroke state then the path draw.
        const auto* stroke_color = next_is(std::in_place_type<ops::set_stroke_color>);
        ASSERT_NE(stroke_color, nullptr);
        EXPECT_EQ(stroke_color->value, color(0.0F, 0.0F, 1.0F));

        const auto* cap = next_is(std::in_place_type<ops::set_stroke_line_cap>);
        ASSERT_NE(cap, nullptr);
        EXPECT_EQ(cap->value, line_cap::round);

        const auto* join = next_is(std::in_place_type<ops::set_stroke_line_join>);
        ASSERT_NE(join, nullptr);
        EXPECT_EQ(join->value, line_join::bevel);

        const auto* drawn = next_is(std::in_place_type<ops::draw_path>);
        ASSERT_NE(drawn, nullptr);
        path_f expected;
        expected.append_rectangle(1, 1, 98, 48); // inset by half the 2px stroke
        EXPECT_TRUE(drawn->path == expected);
    }

    TEST(shape_view_seam, no_stroke_paint_or_zero_thickness_skips_the_stroke_pass)
    {
        shapes::ellipse view;
        view.set_fill(std::make_shared<maui::graphics::solid_paint>(color(0.0F, 1.0F, 0.0F)));
        view.set_stroke_thickness(0); // stroke paint unset AND thickness 0 — both skip in C#

        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 10, 10));
        for (const canvas_op& op : canvas.ops())
        {
            EXPECT_EQ(std::get_if<ops::draw_path>(&op), nullptr);
        }
    }

    TEST(shape_view_seam, polygon_fill_rule_reaches_the_drawable_clip) // PolygonHandler.MapFillRule
    {
        shapes::polygon view(shapes::point_collection{{0, 0}, {10, 0}, {5, 10}});
        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->drawable.winding_mode(), winding_mode::even_odd); // the EvenOdd default

        view.set_fill_rule(shapes::fill_rule::nonzero); // → "fill_rule" → InvalidateShape + re-read
        EXPECT_EQ(platform->drawable.winding_mode(), winding_mode::non_zero);

        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 10, 10));
        bool saw_clip = false;
        for (const canvas_op& op : canvas.ops())
        {
            if (const auto* clip = std::get_if<ops::clip_path>(&op))
            {
                saw_clip = true;
                EXPECT_EQ(clip->winding, winding_mode::non_zero);
            }
        }
        EXPECT_TRUE(saw_clip);
    }

    // R7a: BOTH the clip and the FILL op must carry the polygon's fill rule. A Nonzero, self-
    // intersecting star must select the winding (NonZero) fill so its arms + center fill solid; the
    // CoreGraphics backend maps NonZero → CGContextFillPath and EvenOdd → CGContextEOFillPath, so an
    // even-odd fill op would hollow the star center. (PolygonGalleryPage's EvenOdd vs Nonzero stars.)
    TEST(shape_view_seam, polygon_fill_rule_selects_the_fill_op_winding)
    {
        const auto fill_winding_for = [](shapes::fill_rule rule) {
            shapes::polygon view(shapes::point_collection{{10, 100}, {50, 0}, {90, 100}, {0, 35}, {100, 35}});
            view.set_fill(std::make_shared<maui::graphics::solid_paint>(color(0.0F, 0.0F, 0.0F)));
            view.set_stroke_thickness(0); // fill pass only
            view.set_fill_rule(rule);

            auto handler = std::make_shared<shape_view_handler>();
            view.set_handler(handler);
            auto* platform = handler->typed_platform_view();
            EXPECT_NE(platform, nullptr);

            recording_canvas canvas;
            platform->replay(canvas, rect_f(0, 0, 100, 100));
            std::optional<winding_mode> fill_winding;
            for (const canvas_op& op : canvas.ops())
            {
                if (const auto* filled = std::get_if<ops::fill_path>(&op))
                {
                    fill_winding = filled->winding;
                }
            }
            return fill_winding;
        };

        const std::optional<winding_mode> even_odd = fill_winding_for(shapes::fill_rule::even_odd);
        ASSERT_TRUE(even_odd.has_value());
        EXPECT_EQ(*even_odd, winding_mode::even_odd); // EvenOdd star → EO fill (hollow center)

        const std::optional<winding_mode> nonzero = fill_winding_for(shapes::fill_rule::nonzero);
        ASSERT_TRUE(nonzero.has_value());
        EXPECT_EQ(*nonzero, winding_mode::non_zero); // Nonzero star → winding fill (solid)
    }

    TEST(shape_view_seam, path_render_transform_moves_the_drawn_path) // PathHandler.MapRenderTransform
    {
        shapes::path view(std::make_shared<shapes::path_geometry>([] {
            shapes::path_figure_collection figures;
            shapes::parse_path_figure_collection(figures, "M0,0 L10,0 L10,10 Z");
            return figures;
        }()));
        view.set_aspect(path_aspect::none);
        view.set_stroke_thickness(0);
        view.set_render_transform(std::make_shared<shapes::translate_transform>(5, 7));

        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(platform->drawable.render_transform().has_value());

        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 100, 100));
        bool saw_fill = false;
        for (const canvas_op& op : canvas.ops())
        {
            if (const auto* filled = std::get_if<ops::fill_path>(&op))
            {
                saw_fill = true;
                EXPECT_EQ(filled->path.first_point(), point_f(5, 7)); // (0,0) translated by (5,7)
            }
        }
        EXPECT_TRUE(saw_fill);
    }

    TEST(shape_view_seam, every_stroke_property_change_requests_a_redraw)
    {
        shapes::rectangle view;
        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int before = platform->invalidations;
        view.set_stroke_thickness(3);
        EXPECT_GT(platform->invalidations, before);

        before = platform->invalidations;
        view.set_radius_x(4); // Rectangle.OnPropertyChanged → UpdateValue(Shape)
        EXPECT_GT(platform->invalidations, before);

        before = platform->invalidations;
        view.set_stroke_dash_array({2.0, 1.0}); // the StrokeDashArray remap (Shape.Mapper.cs)
        EXPECT_GT(platform->invalidations, before);
    }

    TEST(shape_view_seam, handlers_resolve_from_the_default_registry)
    {
        const std::shared_ptr<maui::core::i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<shapes::rectangle>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<shape_view_handler*>(handler.get()), nullptr);

        const std::shared_ptr<maui::core::i_element_handler> path_handler =
            maui::core::default_handler_registry().create_handler<shapes::path>();
        ASSERT_NE(path_handler, nullptr);
        EXPECT_NE(dynamic_cast<shape_view_handler*>(path_handler.get()), nullptr);
    }

    // ---- alignment fidelity: a shape inside a vertical stack honors HorizontalOptions via the
    // inherited view<>::arrange -> compute_frame (LayoutExtensions.ComputeFrame), exactly like a plain
    // view. These pin the path_aspect_gallery / auto_size_shapes visual-parity behavior: a shape does
    // NOT bypass ComputeFrame, so its arranged FRAME (which drives PathForBounds/TransformPathForBounds)
    // is alignment-reduced. The stack passes the FULL band width to child.arrange (VerticalStackLayout-
    // Manager.ArrangeChildren); the child reduces it. ----

    // Drive one shape through a single-child vertical stack arranged into `band`, return the shape frame.
    maui::graphics::rect arrange_shape_in_stack(shapes::shape& s, const maui::graphics::rect& band)
    {
        maui::controls::vertical_stack_layout stack;
        stack.add(s);
        stack.measure(band.width, band.height);
        stack.arrange(band);
        return s.frame();
    }

    TEST(shape_alignment, start_shape_in_vertical_stack_is_left_at_desired_width)
    {
        // A 100-wide Start rectangle in a 300-wide band: frame at x=0, width=100 (NOT the full band).
        // This is the path_aspect_gallery contract (the C# <Style TargetType="Path"> sets Start).
        shapes::rectangle r;
        r.set_width_request(100);
        r.set_height_request(100);
        r.set_horizontal_layout_alignment(maui::core::layout_alignment::start);
        const maui::graphics::rect frame = arrange_shape_in_stack(r, maui::graphics::rect(0, 0, 300, 100));
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.width, 100);
    }

    TEST(shape_alignment, center_shape_in_vertical_stack_is_centered_at_desired_width)
    {
        shapes::rectangle r;
        r.set_width_request(100);
        r.set_height_request(100);
        r.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
        const maui::graphics::rect frame = arrange_shape_in_stack(r, maui::graphics::rect(0, 0, 300, 100));
        EXPECT_EQ(frame.x, 100); // (300 - 100) / 2
        EXPECT_EQ(frame.width, 100);
    }

    TEST(shape_alignment, end_shape_in_vertical_stack_is_right_aligned_at_desired_width)
    {
        shapes::rectangle r;
        r.set_width_request(100);
        r.set_height_request(100);
        r.set_horizontal_layout_alignment(maui::core::layout_alignment::end);
        const maui::graphics::rect frame = arrange_shape_in_stack(r, maui::graphics::rect(0, 0, 300, 100));
        EXPECT_EQ(frame.x, 200); // 300 - 100
        EXPECT_EQ(frame.width, 100);
    }

    TEST(shape_alignment, fill_shape_with_no_explicit_width_fills_the_band)
    {
        // The common default + the auto_size_shapes contract: a Fill ellipse with NO width request
        // consumes the whole band (LayoutExtensions: Fill && !IsExplicitSet -> min(bounds, MaxWidth)).
        shapes::ellipse e;
        const maui::graphics::rect frame = arrange_shape_in_stack(e, maui::graphics::rect(0, 0, 300, 100));
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.width, 300);
    }

    TEST(shape_alignment, fill_shape_with_explicit_width_centers_at_that_width)
    {
        // C# AlignHorizontal: Fill + an explicit width is treated as Center over the space it "fills".
        shapes::rectangle r;
        r.set_width_request(100);
        r.set_height_request(100);
        // HorizontalOptions left at the Fill default.
        const maui::graphics::rect frame = arrange_shape_in_stack(r, maui::graphics::rect(0, 0, 300, 100));
        EXPECT_EQ(frame.x, 100); // centered, NOT stretched to 300
        EXPECT_EQ(frame.width, 100);
    }

    // The arranged FRAME drives TransformPathForBounds: a Start rectangle's stretched geometry is
    // confined to the 100-wide frame (NOT the 300-wide band) when rendered through PathForBounds(frame).
    TEST(shape_alignment, frame_drives_transform_path_for_bounds)
    {
        shapes::rectangle r; // aspect = Stretch.Fill default
        r.set_width_request(100);
        r.set_height_request(80);
        r.set_stroke_thickness(0); // no inset, so the path exactly matches the frame
        r.set_horizontal_layout_alignment(maui::core::layout_alignment::start);
        const maui::graphics::rect frame = arrange_shape_in_stack(r, maui::graphics::rect(0, 0, 300, 80));
        ASSERT_EQ(frame.width, 100);

        // Render through the production contract: PathForBounds(frame) — the dirty rect IS the frame
        // (the native host is sized to the frame by arrange_native, drawRect's bounds == frame size).
        const path_f rendered = r.path_for_bounds(frame);
        const rect_f flat = rendered.get_bounds_by_flattening(1);
        EXPECT_NEAR(static_cast<double>(flat.width), 100.0, 0.5); // confined to the 100-wide frame
        EXPECT_NEAR(static_cast<double>(flat.height), 80.0, 0.5);
    }

    // The Fill render twin (auto_size_shapes — a Fill shape must STILL fill). A Fill ellipse with no
    // explicit size, given a DEFINITE cell (the way a Grid star cell arranges its child — measure with a
    // finite height, arrange into the cell), fills the cell and its rendered geometry spans the whole
    // cell. This is the auto_size_shapes pipeline (Ellipse in a star-sized Grid cell), NOT a vertical
    // stack (which would stack by the child's degenerate desired height under an infinite constraint).
    TEST(shape_alignment, fill_shape_in_a_definite_cell_renders_across_the_full_frame)
    {
        shapes::ellipse e;
        e.set_stroke_thickness(0);
        // A star Grid cell measures the child against the cell extent and arranges it into the cell.
        e.measure(300, 100);
        e.arrange(maui::graphics::rect(0, 0, 300, 100));
        const maui::graphics::rect frame = e.frame();
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.width, 300); // Fill consumes the cell width (NOT the intrinsic circle width)
        EXPECT_EQ(frame.height, 100);

        const path_f rendered = e.path_for_bounds(frame);
        const rect_f flat = rendered.get_bounds_by_flattening(1);
        EXPECT_NEAR(static_cast<double>(flat.width), 300.0, 0.5);
        EXPECT_NEAR(static_cast<double>(flat.height), 100.0, 0.5);
    }
} // namespace

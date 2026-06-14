// Tests for the graphics_view + box_view controls and their headless handler seams — ported from
// src/Controls/tests/Core.UnitTests/BoxViewUnitTests.cs (TestConstructor / DefaultSize) plus
// source-derived suites pinning GraphicsView.cs (the Drawable property, Invalidate →
// Handler.Invoke, the seven interaction events) and the GraphicsViewHandler recipe (MapDrawable /
// MapBackground / MapFlowDirection / MapInvalidate over the op-recording headless seat — the
// golden-op replay standing in for the native drawRect).
//
// §8 teardown order: controls (publishers via their handlers) are declared before handlers where a
// seam test owns both; the handler borrows the control, so the control outlives it.
#include "maui/controls/graphics_view.hpp"

#include <limits>
#include <memory>
#include <variant>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/recording_canvas.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::box_view;
    using maui::controls::graphics_view;
    using maui::core::graphics_view_handler;
    using maui::core::shape_view_handler;
    using maui::graphics::canvas_op;
    using maui::graphics::color;
    using maui::graphics::point_f;
    using maui::graphics::recording_canvas;
    using maui::graphics::rect_f;
    using maui::graphics::canvas_ops::fill_rectangle;

    // A deterministic drawable: paints one rectangle so the replay has a recognizable golden op.
    class rect_drawable final : public maui::graphics::i_drawable
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const rect_f& dirty_rect) override
        {
            canvas.set_fill_color(color(1.0F, 0.0F, 0.0F));
            canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
            draw_count++;
        }
        int draw_count = 0;
    };

    // ---- the control in isolation (GraphicsView.cs) ----

    TEST(graphics_view, drawable_defaults_null_and_is_owned)
    {
        graphics_view view;
        EXPECT_EQ(view.drawable(), nullptr);

        auto drawable = std::make_shared<rect_drawable>();
        view.set_drawable(drawable);
        EXPECT_EQ(view.drawable(), drawable.get());
    }

    TEST(graphics_view, interaction_channel_raises_the_events)
    {
        graphics_view view;
        std::vector<point_f> seen;
        int hover_ends = 0;
        int cancels = 0;
        bool last_inside = false;

        view.start_interaction.connect([&seen](const std::vector<point_f>& points) { seen = points; });
        view.end_interaction.connect(
            [&last_inside](const std::vector<point_f>& /*points*/, bool inside) { last_inside = inside; });
        view.end_hover_interaction.connect([&hover_ends] { hover_ends++; });
        view.cancel_interaction.connect([&cancels] { cancels++; });

        maui::core::i_graphics_view& contract = view;
        contract.send_start_interaction({point_f(3, 4)});
        ASSERT_EQ(seen.size(), 1U);
        EXPECT_EQ(seen[0], point_f(3, 4));

        contract.send_end_interaction({point_f(3, 4)}, true);
        EXPECT_TRUE(last_inside);
        contract.send_end_hover_interaction();
        EXPECT_EQ(hover_ends, 1);
        contract.send_cancel_interaction();
        EXPECT_EQ(cancels, 1);
    }

    // ---- the handler seam (GraphicsViewHandler + the headless drawing seat) ----

    TEST(graphics_view_seam, attaching_handler_points_the_host_at_the_drawable)
    {
        graphics_view view;
        auto drawable = std::make_shared<rect_drawable>();
        view.set_drawable(drawable);

        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->drawable, drawable.get());
        EXPECT_GE(platform->invalidations, 1); // the Drawable push redraws (PlatformGraphicsView)
    }

    TEST(graphics_view_seam, invalidate_routes_through_the_command_mapper)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        const int before = platform->invalidations;
        view.invalidate(); // C# Handler?.Invoke(nameof(IGraphicsView.Invalidate))
        EXPECT_EQ(platform->invalidations, before + 1);
    }

    TEST(graphics_view_seam, background_only_invalidates_when_set) // C# MapBackground
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        const int before = platform->invalidations;
        view.set_background(std::make_shared<maui::graphics::solid_paint>(color(0.0F, 1.0F, 0.0F)));
        EXPECT_EQ(platform->invalidations, before + 1);
    }

    TEST(graphics_view_seam, replay_draws_the_drawable_into_a_recording_canvas) // the golden-op seat
    {
        graphics_view view;
        auto drawable = std::make_shared<rect_drawable>();
        view.set_drawable(drawable);

        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 90, 40));
        EXPECT_EQ(drawable->draw_count, 1);

        // golden ops: the staged fill color, then the rectangle fill over the dirty rect.
        ASSERT_EQ(canvas.ops().size(), 2U);
        const auto* staged = std::get_if<maui::graphics::canvas_ops::set_fill_color>(canvas.ops().data());
        ASSERT_NE(staged, nullptr);
        EXPECT_EQ(staged->value, color(1.0F, 0.0F, 0.0F));
        const auto* filled = std::get_if<fill_rectangle>(&canvas.ops()[1]);
        ASSERT_NE(filled, nullptr);
        EXPECT_EQ(filled->width, 90.0F);
        EXPECT_EQ(filled->height, 40.0F);
    }

    TEST(graphics_view_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<maui::core::i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<graphics_view>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<graphics_view_handler*>(handler.get()), nullptr);
    }

    // ---- box_view (BoxViewUnitTests.cs) ----

    TEST(box_view, test_constructor) // C# TestConstructor
    {
        box_view box;
        box.set_color(color(0.2F, 0.3F, 0.4F));
        box.set_width_request(20);
        box.set_height_request(30);

        EXPECT_EQ(box.color(), color(0.2F, 0.3F, 0.4F));
        const maui::graphics::size request =
            box.measure(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        EXPECT_EQ(request.width, 20.0);
        EXPECT_EQ(request.height, 30.0);
    }

    TEST(box_view, default_size) // C# DefaultSize
    {
        box_view box;
        const maui::graphics::size request =
            box.measure(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        EXPECT_EQ(request.width, 40.0);
        EXPECT_EQ(request.height, 40.0);
    }

    TEST(box_view, fill_materializes_the_color_paint) // C# IShapeView.Fill => Color?.AsPaint()
    {
        box_view box;
        EXPECT_FALSE(box.has_color());
        EXPECT_EQ(box.fill(), nullptr); // Color null → Fill null

        box.set_color(color(0.5F, 0.25F, 0.0F));
        ASSERT_NE(box.fill(), nullptr);
        EXPECT_EQ(box.fill()->background_color(), color(0.5F, 0.25F, 0.0F));
        EXPECT_EQ(box.shape(), static_cast<maui::graphics::i_shape*>(&box)); // Shape => this
        EXPECT_EQ(box.aspect(), maui::core::path_aspect::none);
        EXPECT_EQ(box.stroke(), nullptr);
        EXPECT_EQ(box.stroke_thickness(), 0.0);
    }

    TEST(box_view, path_for_bounds_is_the_rounded_rectangle)
    {
        box_view box;
        box.set_corner_radius(maui::graphics::corner_radius(4, 0, 0, 4));

        const maui::graphics::path_f path = box.path_for_bounds(maui::graphics::rect(0, 0, 50, 30));
        maui::graphics::path_f expected;
        expected.append_rounded_rectangle(0, 0, 50, 30, 4, 0, 0, 4);
        EXPECT_TRUE(path == expected);
    }

    TEST(box_view_seam, color_change_redraws_through_the_shape_handler)
    {
        box_view box;
        auto handler = std::make_shared<shape_view_handler>();
        box.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->drawable.shape_view(), &box);

        const int before = platform->invalidations;
        box.set_color(color(0.0F, 0.0F, 1.0F)); // BoxView.OnPropertyChanged → UpdateValue(Shape)
        EXPECT_GT(platform->invalidations, before);

        // replay: the box fills its rounded-rect path with the color paint.
        recording_canvas canvas;
        platform->replay(canvas, rect_f(0, 0, 40, 40));
        bool saw_fill_paint = false;
        for (const canvas_op& op : canvas.ops())
        {
            if (const auto* fill = std::get_if<maui::graphics::canvas_ops::set_fill_paint>(&op))
            {
                saw_fill_paint = true;
                EXPECT_EQ(fill->background_color, color(0.0F, 0.0F, 1.0F));
            }
        }
        EXPECT_TRUE(saw_fill_paint);
    }
} // namespace

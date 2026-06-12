// Tests for the shapes_page demo (src/samples/pages/shapes_page.hpp) — backend-agnostic: the page
// is pure cross-platform control wiring, so this suite compiles in every preset and proves the
// demo's structure (the stack hosting graphics_view/box_view/the shape family), the per-control
// decoration, and the canvas interaction → readout wiring without a hosting main.
#include "src/samples/pages/shapes_page.hpp"

#include <memory>
#include <vector>

#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/recording_canvas.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::shapes_page;

    TEST(shapes_page, builds_the_shape_tree)
    {
        shapes_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 8); // readout + canvas + box + rect/ellipse/line/polygon/path

        EXPECT_NE(demo.canvas().drawable(), nullptr);
        EXPECT_TRUE(demo.box().has_color());
        EXPECT_EQ(demo.rounded_rect().radius_x(), 10.0);
        ASSERT_NE(demo.rounded_rect().stroke(), nullptr);
        EXPECT_EQ(demo.divider().stroke_dash_pattern().size(), 2U);
        EXPECT_EQ(demo.star().points().size(), 5U);
        EXPECT_EQ(demo.star().fill_rule(), maui::controls::shapes::fill_rule::even_odd);
        ASSERT_NE(demo.marker().data(), nullptr);
        EXPECT_FALSE(demo.marker().data()->path_for_bounds({}).points().empty());
        EXPECT_TRUE(demo.marker().render_transform_matrix().has_value());
    }

    TEST(shapes_page, canvas_interaction_drives_the_readout)
    {
        shapes_page demo;
        maui::core::i_graphics_view& contract = demo.canvas();
        contract.send_start_interaction({maui::graphics::point_f(12, 34)});
        EXPECT_EQ(demo.readout().text(), "Touched at 12, 34");
    }

    TEST(shapes_page, the_demo_drawable_paints_the_framed_cross)
    {
        shapes_page demo;
        maui::graphics::recording_canvas canvas;
        demo.canvas().drawable()->draw(canvas, maui::graphics::rect_f(0, 0, 100, 100));
        // stroke size + color, the frame rectangle, the two diagonals.
        EXPECT_EQ(canvas.ops().size(), 5U);
    }
} // namespace

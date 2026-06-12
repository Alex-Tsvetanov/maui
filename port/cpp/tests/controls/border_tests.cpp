// Tests for the border control + its headless handler seam — ported from BorderUnitTests.cs
// (src/Controls/tests/Core.UnitTests: the content parenting trio) plus the Border.cs surface the C#
// suite leaves to its DeviceTests: the IBorderStroke defaults (thickness 1, Rectangle shape, butt/miter,
// miter limit 10, empty dash array), the CrossPlatformMeasure/Arrange insets (Padding + StrokeThickness),
// and the handler seam — the headless border_platform mirrors the hosted content and the resolved
// border_stroke_spec on every stroke-property push (the StrokeExtensions → UpdateMauiCALayer funnel),
// re-pushing the bounds-dependent shape when the arranged size changes (BorderHandler.PlatformArrange).
#include "maui/controls/border.hpp"

#include <memory>
#include <vector>

#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::border;
    using maui::core::border_handler;
    using maui::core::i_element_handler;
    using maui::core::thickness;
    using maui::graphics::color;
    using maui::graphics::line_cap;
    using maui::graphics::line_join;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::graphics::solid_paint;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation ----

    TEST(border, defaults_match_the_csharp_property_defaults)
    {
        const border view;
        EXPECT_EQ(view.content(), nullptr);
        EXPECT_EQ(view.padding(), thickness());
        EXPECT_EQ(view.stroke(), nullptr);
        EXPECT_EQ(view.stroke_thickness(), 1.0);
        // StrokeShape defaults to a Rectangle (never null).
        ASSERT_NE(view.shape(), nullptr);
        EXPECT_NE(dynamic_cast<maui::graphics::shapes::rectangle*>(view.shape()), nullptr);
        EXPECT_TRUE(view.stroke_dash_array().empty());
        EXPECT_TRUE(view.stroke_dash_pattern().empty());
        EXPECT_EQ(view.stroke_dash_offset(), 0.0F);
        EXPECT_EQ(view.stroke_line_cap(), line_cap::butt);    // PenLineCap.Flat
        EXPECT_EQ(view.stroke_line_join(), line_join::miter); // PenLineJoin.Miter
        EXPECT_EQ(view.stroke_miter_limit(), 10.0F);
    }

    TEST(border, content_is_parented_and_unparented) // C# ChildrenHaveParentsWhenContentIsSet
    {
        border view;
        mock_view child;

        view.set_content(child);
        EXPECT_EQ(view.content(), &child);
        EXPECT_EQ(child.logical_parent(), &view);

        view.set_content(nullptr);
        EXPECT_EQ(view.content(), nullptr); // C# HasNoVisualChildrenWhenNoContentIsSet
        EXPECT_EQ(child.logical_parent(), nullptr);
    }

    TEST(border, stroke_dash_pattern_materializes_the_dash_array)
    {
        border view;
        view.set_stroke_dash_array({4.0, 2.0});
        const std::vector<float> pattern = view.stroke_dash_pattern();
        ASSERT_EQ(pattern.size(), 2U);
        EXPECT_EQ(pattern[0], 4.0F);
        EXPECT_EQ(pattern[1], 2.0F);
    }

    // ---- measure/arrange: Padding + StrokeThickness insets (Border.CrossPlatformMeasure/Arrange) ----

    TEST(border, measure_insets_by_padding_plus_stroke_thickness)
    {
        border view;
        view.set_padding(thickness(10));
        view.set_stroke_thickness(5);
        mock_view child;
        child.configure({100, 40});
        view.set_content(child);

        // content 100x40 + (padding 10 + thickness 5) on all sides -> 130x70.
        const size measured = view.measure(1000, 1000);
        EXPECT_EQ(measured.width, 130.0);
        EXPECT_EQ(measured.height, 70.0);

        // The content was measured with the full inset subtracted from the constraints.
        EXPECT_EQ(child.last_measure_width, 970.0);
        EXPECT_EQ(child.last_measure_height, 970.0);
    }

    TEST(border, measure_with_no_content_is_the_inset_only)
    {
        border view;
        view.set_padding(thickness(10));
        view.set_stroke_thickness(5);
        const size measured = view.measure(1000, 1000);
        EXPECT_EQ(measured.width, 30.0);
        EXPECT_EQ(measured.height, 30.0);
    }

    TEST(border, arrange_places_content_within_stroke_then_padding)
    {
        border view;
        view.set_padding(thickness(10));
        view.set_stroke_thickness(5);
        mock_view child;
        child.configure({100, 40});
        view.set_content(child);

        view.measure(1000, 1000);
        view.arrange(rect(0, 0, 130, 70));

        // bounds inset by the stroke (5), then ArrangeContent applies the padding (10) within it.
        EXPECT_EQ(child.last_arrange, rect(15, 15, 100, 40));
    }

    // ---- the handler seam: the headless host mirrors content + the resolved stroke spec ----

    TEST(border_seam, attaching_handler_mirrors_content_and_stroke_spec)
    {
        border view;
        mock_view child;
        view.set_content(child);
        view.set_stroke(std::make_shared<solid_paint>(color(1.0F, 0.0F, 0.0F)));
        view.set_stroke_thickness(3);
        view.set_stroke_dash_array({2.0, 1.0});
        view.set_stroke_dash_offset(1.5);
        view.set_stroke_line_cap(line_cap::round);
        view.set_stroke_line_join(line_join::bevel);
        view.set_stroke_miter_limit(4);

        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        EXPECT_EQ(platform->hosted_content, &child);
        EXPECT_TRUE(platform->border.has_stroke);
        EXPECT_EQ(platform->border.stroke_color, color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->border.thickness, 3.0);
        ASSERT_EQ(platform->border.dash_pattern.size(), 2U);
        EXPECT_EQ(platform->border.dash_pattern[0], 2.0F);
        EXPECT_EQ(platform->border.dash_offset, 1.5F);
        EXPECT_EQ(platform->border.line_cap, line_cap::round);
        EXPECT_EQ(platform->border.line_join, line_join::bevel);
        EXPECT_EQ(platform->border.miter_limit, 4.0F);
        EXPECT_EQ(platform->border.shape, view.shape());
    }

    TEST(border_seam, every_stroke_property_change_refreshes_the_spec)
    {
        border view;
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->border.has_stroke);
        EXPECT_EQ(platform->border.thickness, 1.0); // the connect push carried the defaults

        view.set_stroke(std::make_shared<solid_paint>(color(0.0F, 0.0F, 1.0F)));
        EXPECT_TRUE(platform->border.has_stroke);
        EXPECT_EQ(platform->border.stroke_color, color(0.0F, 0.0F, 1.0F));

        view.set_stroke_thickness(7);
        EXPECT_EQ(platform->border.thickness, 7.0);

        auto shape = std::make_shared<maui::graphics::shapes::ellipse>();
        view.set_stroke_shape(shape);
        EXPECT_EQ(platform->border.shape, shape.get());
    }

    TEST(border_seam, arranging_a_new_size_repushes_the_bounds_dependent_border)
    {
        border view;
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Wipe the mirror, then arrange: the size change must re-push the border (the C#
        // PlatformArrange → UpdateValue(Shape) path); a same-size re-arrange must not.
        platform->border = maui::core::border_stroke_spec{};
        view.arrange(rect(0, 0, 100, 50));
        EXPECT_EQ(platform->border.shape, view.shape());

        platform->border = maui::core::border_stroke_spec{};
        view.arrange(rect(0, 0, 100, 50)); // unchanged size -> no re-push
        EXPECT_EQ(platform->border.shape, nullptr);
    }

    TEST(border_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<border>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<border_handler*>(handler.get()), nullptr);
    }
} // namespace

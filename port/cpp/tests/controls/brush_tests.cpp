// Tests for the controls-level Brush family (X1). Ported from src/Controls/tests/Core.UnitTests:
//   SolidColorBrushTests.cs / LinearGradientBrushTests.cs / RadialGradientBrushTests.cs /
//   BrushTypeConverterUnitTests.cs — the behavioral oracle for brush construction, IsEmpty /
//   IsNullOrEmpty / HasTransparency, value equality, the brush<->paint bridge, the XAML converter, and the
//   BindingContext / immutable-brush parent rules. Backend-agnostic (pure model + bridge + converter).

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/brush_paint_bridge.hpp"
#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/image_brush.hpp"
#include "maui/controls/brushes/immutable_brush.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/radial_gradient_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "maui/controls/brushes/brush_type_converter.hpp"
#include "maui/controls/grid.hpp"
#include <gtest/gtest.h>

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace
{
    using maui::controls::brush;
    using maui::controls::gradient_stop;
    using maui::controls::image_brush;
    using maui::controls::linear_gradient_brush;
    using maui::controls::radial_gradient_brush;
    using maui::controls::solid_color_brush;
    using maui::graphics::color;
    using maui::graphics::point;
    namespace colors = maui::graphics::colors;

    std::shared_ptr<gradient_stop> stop(color c, float offset)
    {
        return std::make_shared<gradient_stop>(c, offset);
    }

    // ====================================================================== SolidColorBrush

    TEST(solid_color_brush_test, constructor_color_is_null)
    {
        const solid_color_brush brush_value;
        EXPECT_FALSE(brush_value.color().has_value()); // C#: Assert.Null(Color)
    }

    TEST(solid_color_brush_test, constructor_using_color)
    {
        const solid_color_brush brush_value{colors::red};
        ASSERT_TRUE(brush_value.color().has_value());
        EXPECT_EQ(*brush_value.color(), colors::red);
    }

    TEST(solid_color_brush_test, empty_solid_color_brush)
    {
        const solid_color_brush empty;
        EXPECT_TRUE(empty.is_empty());
        EXPECT_FALSE(brush::red().is_empty());
    }

    TEST(solid_color_brush_test, null_or_empty_solid_color_brush)
    {
        EXPECT_TRUE(brush::is_null_or_empty(nullptr));
        const solid_color_brush empty;
        EXPECT_TRUE(brush::is_null_or_empty(&empty));
        EXPECT_FALSE(brush::is_null_or_empty(&brush::yellow()));
    }

    TEST(solid_color_brush_test, default_brushes)
    {
        const solid_color_brush& black = brush::black();
        ASSERT_TRUE(black.color().has_value());
        EXPECT_EQ(*black.color(), colors::black);

        const solid_color_brush& white = brush::white();
        ASSERT_TRUE(white.color().has_value());
        EXPECT_EQ(*white.color(), colors::white);
    }

    TEST(solid_color_brush_test, equals_compares_color_values) // dotnet/maui#27281
    {
        const color color1{1.0F, 0.0F, 0.0F, 1.0F};
        const color color2{1.0F, 0.0F, 0.0F, 1.0F};
        EXPECT_TRUE(color1 == color2);

        const solid_color_brush brush1{color1};
        const solid_color_brush brush2{color2};
        EXPECT_TRUE(brush1 == brush2);
    }

    TEST(solid_color_brush_test, has_transparency)
    {
        EXPECT_FALSE(brush::has_transparency(nullptr));

        const solid_color_brush empty;
        EXPECT_FALSE(brush::has_transparency(&empty)); // null color is not transparent

        const solid_color_brush red{colors::red};
        EXPECT_FALSE(brush::has_transparency(&red));

        const solid_color_brush transparent{colors::transparent};
        EXPECT_TRUE(brush::has_transparency(&transparent));

        const solid_color_brush semi{color::from_rgba(255, 0, 0, 0.5)};
        EXPECT_TRUE(brush::has_transparency(&semi));

        EXPECT_TRUE(brush::has_transparency(&brush::transparent()));
        EXPECT_FALSE(brush::has_transparency(&brush::black()));
    }

    // ====================================================================== LinearGradientBrush

    TEST(linear_gradient_brush_test, constructor_defaults)
    {
        const linear_gradient_brush brush_value;
        EXPECT_DOUBLE_EQ(brush_value.end_point().x, 1.0);
        EXPECT_DOUBLE_EQ(brush_value.end_point().y, 1.0);
        EXPECT_DOUBLE_EQ(brush_value.start_point().x, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.start_point().y, 0.0);
    }

    TEST(linear_gradient_brush_test, constructor_using_collection)
    {
        std::vector<std::shared_ptr<gradient_stop>> stops{stop(colors::red, 0.1F), stop(colors::orange, 0.8F)};
        const linear_gradient_brush brush_value{std::move(stops), point{0, 0}, point{0, 1}};
        EXPECT_FALSE(brush_value.gradient_stops().empty());
        EXPECT_DOUBLE_EQ(brush_value.end_point().x, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.end_point().y, 1.0);
    }

    TEST(linear_gradient_brush_test, is_empty)
    {
        const linear_gradient_brush empty;
        EXPECT_TRUE(empty.is_empty());

        linear_gradient_brush filled;
        filled.set_start_point(point{0, 0});
        filled.set_end_point(point{1, 0});
        filled.set_gradient_stops({stop(colors::orange, 0.1F), stop(colors::red, 0.8F)});
        EXPECT_FALSE(filled.is_empty());
    }

    TEST(linear_gradient_brush_test, null_or_empty)
    {
        EXPECT_TRUE(brush::is_null_or_empty(nullptr));
        const linear_gradient_brush empty;
        EXPECT_TRUE(brush::is_null_or_empty(&empty));

        linear_gradient_brush filled;
        filled.set_gradient_stops({stop(colors::orange, 0.1F), stop(colors::red, 0.8F)});
        EXPECT_FALSE(brush::is_null_or_empty(&filled));
    }

    TEST(linear_gradient_brush_test, null_or_empty_paint_with_empty_stops)
    {
        // C#: `Paint p = brush; p.IsNullOrEmpty()`. 2 stops with NULL color (default gradient_stop) → null
        // StartColor/EndColor → empty. brush_is_null_or_empty_as_paint computes this over the brush's
        // nullable colors (the value-type graphics paint cannot carry a null color — see the bridge header).
        linear_gradient_brush brush_value;
        brush_value.set_start_point(point{0, 0});
        brush_value.set_end_point(point{1, 0});
        brush_value.gradient_stops().add(std::make_shared<gradient_stop>());
        brush_value.gradient_stops().add(std::make_shared<gradient_stop>());
        EXPECT_TRUE(maui::controls::brush_is_null_or_empty_as_paint(&brush_value));
    }

    TEST(linear_gradient_brush_test, null_or_empty_paint_with_null_stops)
    {
        linear_gradient_brush brush_value;
        brush_value.gradient_stops().add(nullptr);
        brush_value.gradient_stops().add(nullptr);
        EXPECT_TRUE(maui::controls::brush_is_null_or_empty_as_paint(&brush_value));
    }

    TEST(linear_gradient_brush_test, non_empty_paint_with_a_null_stop)
    {
        linear_gradient_brush brush_value;
        brush_value.gradient_stops().add(stop(colors::red, 0.1F));
        brush_value.gradient_stops().add(nullptr);
        brush_value.gradient_stops().add(stop(colors::blue, 1.0F));
        EXPECT_FALSE(maui::controls::brush_is_null_or_empty_as_paint(&brush_value));
    }

    TEST(linear_gradient_brush_test, points)
    {
        linear_gradient_brush brush_value;
        brush_value.set_start_point(point{0, 0});
        brush_value.set_end_point(point{1, 0});
        EXPECT_DOUBLE_EQ(brush_value.start_point().x, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.start_point().y, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.end_point().x, 1.0);
        EXPECT_DOUBLE_EQ(brush_value.end_point().y, 0.0);
    }

    TEST(linear_gradient_brush_test, gradient_stops_count)
    {
        linear_gradient_brush brush_value;
        brush_value.set_gradient_stops({stop(colors::red, 0.1F), stop(colors::blue, 1.0F)});
        EXPECT_EQ(brush_value.gradient_stops().count(), 2U);
    }

    TEST(linear_gradient_brush_test, has_transparency)
    {
        EXPECT_FALSE(brush::has_transparency(nullptr));
        const linear_gradient_brush empty;
        EXPECT_FALSE(brush::has_transparency(&empty));

        linear_gradient_brush opaque;
        opaque.set_gradient_stops({stop(colors::red, 0.0F), stop(colors::blue, 1.0F)});
        EXPECT_FALSE(brush::has_transparency(&opaque));

        linear_gradient_brush transparent;
        transparent.set_gradient_stops({stop(colors::transparent, 0.0F), stop(colors::blue, 1.0F)});
        EXPECT_TRUE(brush::has_transparency(&transparent));

        linear_gradient_brush mixed;
        mixed.set_gradient_stops(
            {stop(colors::red, 0.0F), stop(color::from_rgba(0, 255, 0, 0.5), 0.5F), stop(colors::blue, 1.0F)});
        EXPECT_TRUE(brush::has_transparency(&mixed));
    }

    // ====================================================================== RadialGradientBrush

    TEST(radial_gradient_brush_test, constructor_empty_stops)
    {
        const radial_gradient_brush brush_value;
        EXPECT_EQ(brush_value.gradient_stops().count(), 0U);
    }

    TEST(radial_gradient_brush_test, constructor_using_collection)
    {
        std::vector<std::shared_ptr<gradient_stop>> stops{stop(colors::red, 0.1F), stop(colors::orange, 0.8F)};
        const radial_gradient_brush brush_value{std::move(stops), point{0, 0}, 10};
        EXPECT_FALSE(brush_value.gradient_stops().empty());
        EXPECT_DOUBLE_EQ(brush_value.center().x, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.center().y, 0.0);
        EXPECT_DOUBLE_EQ(brush_value.radius(), 10.0);
    }

    TEST(radial_gradient_brush_test, null_or_empty_paint_with_empty_stops)
    {
        radial_gradient_brush brush_value;
        brush_value.set_center(point{0, 0});
        brush_value.set_radius(10);
        brush_value.gradient_stops().add(std::make_shared<gradient_stop>());
        brush_value.gradient_stops().add(std::make_shared<gradient_stop>());
        EXPECT_TRUE(maui::controls::brush_is_null_or_empty_as_paint(&brush_value));
    }

    TEST(radial_gradient_brush_test, null_or_empty_paint_with_null_stops)
    {
        radial_gradient_brush brush_value;
        brush_value.gradient_stops().add(nullptr);
        brush_value.gradient_stops().add(nullptr);
        EXPECT_TRUE(maui::controls::brush_is_null_or_empty_as_paint(&brush_value));
    }

    TEST(radial_gradient_brush_test, is_empty)
    {
        const radial_gradient_brush empty;
        EXPECT_TRUE(empty.is_empty());

        radial_gradient_brush filled;
        filled.set_center(point{0, 0});
        filled.set_radius(10);
        filled.set_gradient_stops({stop(colors::orange, 0.1F), stop(colors::red, 0.8F)});
        EXPECT_FALSE(filled.is_empty());
    }

    TEST(radial_gradient_brush_test, radius)
    {
        radial_gradient_brush brush_value;
        brush_value.set_radius(20);
        EXPECT_DOUBLE_EQ(brush_value.radius(), 20.0);
    }

    TEST(radial_gradient_brush_test, gradient_stops_count)
    {
        radial_gradient_brush brush_value;
        brush_value.set_gradient_stops({stop(colors::red, 0.1F), stop(colors::blue, 1.0F)});
        brush_value.set_radius(20);
        EXPECT_EQ(brush_value.gradient_stops().count(), 2U);
    }

    TEST(radial_gradient_brush_test, has_transparency)
    {
        EXPECT_FALSE(brush::has_transparency(nullptr));
        const radial_gradient_brush empty;
        EXPECT_FALSE(brush::has_transparency(&empty));

        radial_gradient_brush transparent;
        transparent.set_gradient_stops({stop(colors::transparent, 0.0F), stop(colors::blue, 1.0F)});
        EXPECT_TRUE(brush::has_transparency(&transparent));
    }

    // ====================================================================== brush<->paint bridge

    TEST(brush_paint_bridge_test, solid_to_paint)
    {
        const solid_color_brush brush_value{colors::red};
        const auto paint = maui::controls::to_paint(brush_value);
        const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(paint.get());
        ASSERT_NE(solid, nullptr);
        EXPECT_EQ(solid->color(), colors::red);
    }

    TEST(brush_paint_bridge_test, linear_to_paint_carries_stops_and_points)
    {
        linear_gradient_brush brush_value;
        brush_value.set_start_point(point{0.2, 0.3});
        brush_value.set_end_point(point{0.8, 0.9});
        brush_value.set_gradient_stops({stop(colors::red, 0.0F), stop(colors::blue, 1.0F)});
        const auto paint = maui::controls::to_paint(brush_value);
        const auto* linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(paint.get());
        ASSERT_NE(linear, nullptr);
        EXPECT_EQ(linear->gradient_stops().size(), 2U);
        EXPECT_DOUBLE_EQ(linear->start_point().x, 0.2);
        EXPECT_DOUBLE_EQ(linear->end_point().y, 0.9);
    }

    TEST(brush_paint_bridge_test, color_implicit_to_solid_brush_round_trip)
    {
        // C# `implicit operator Brush(Color)` + `implicit operator Paint(Brush)`: a solid_paint round-trips.
        const auto solid = std::make_shared<maui::graphics::solid_paint>(colors::green);
        const auto brush_value = maui::controls::to_brush(solid.get());
        const auto* as_solid = dynamic_cast<const solid_color_brush*>(brush_value.get());
        ASSERT_NE(as_solid, nullptr);
        ASSERT_TRUE(as_solid->color().has_value());
        EXPECT_EQ(*as_solid->color(), colors::green);
    }

    TEST(brush_paint_bridge_test, null_brush_to_paint_is_null)
    {
        const std::shared_ptr<brush> none;
        EXPECT_EQ(maui::controls::to_paint(none), nullptr);
        EXPECT_EQ(maui::controls::to_brush(static_cast<const maui::graphics::paint*>(nullptr)), nullptr);
    }

    // ====================================================================== XAML converter

    TEST(brush_converter_test, convert_null_yields_null_color_solid)
    {
        const auto result = maui::controls::convert_brush("");
        const auto* solid = dynamic_cast<const solid_color_brush*>(result.get());
        ASSERT_NE(solid, nullptr);
        EXPECT_FALSE(solid->color().has_value());
    }

    TEST(brush_converter_test, css_color_definitions)
    {
        for (const char* definition : {"rgb(6, 201, 198)", "rgba(6, 201, 188, 0.2)", "hsl(6, 20%, 45%)",
                                       "hsla(6, 20%, 45%,0.75)", "rgb(100%, 32%, 64%)", "rgba(100%, 32%, 64%,0.27)"})
        {
            const auto result = maui::controls::convert_brush(definition);
            ASSERT_NE(result, nullptr) << definition;
            const auto* solid = dynamic_cast<const solid_color_brush*>(result.get());
            ASSERT_NE(solid, nullptr) << definition;
            EXPECT_TRUE(solid->color().has_value()) << definition;
        }
    }

    TEST(brush_converter_test, color_hex)
    {
        for (const char* hex : {"#ff00ff", "#00FF33", "#00FFff 40%"})
        {
            const auto result = maui::controls::convert_brush(hex);
            const auto* solid = dynamic_cast<const solid_color_brush*>(result.get());
            ASSERT_NE(solid, nullptr) << hex;
            EXPECT_TRUE(solid->color().has_value()) << hex;
        }
    }

    TEST(brush_converter_test, gradients)
    {
        const auto linear = maui::controls::convert_brush("linear-gradient(90deg, rgb(255, 0, 0),rgb(255, 153, 51))");
        EXPECT_NE(dynamic_cast<const linear_gradient_brush*>(linear.get()), nullptr);

        const auto radial = maui::controls::convert_brush(
            "radial-gradient(circle, rgb(255, 0, 0) 25%, rgb(0, 255, 0) 50%, rgb(0, 0, 255) 75%)");
        EXPECT_NE(dynamic_cast<const radial_gradient_brush*>(radial.get()), nullptr);
    }

    // ====================================================================== BindingContext + immutable

    TEST(brush_binding_context_test, propagates_into_stops)
    {
        // BrushTypeConverterUnitTests.TestBindingContextPropagation: setting the brush's context flows into
        // its stops (the stops are logical children, parented on add).
        const auto context = std::make_shared<int>(7);
        linear_gradient_brush brush_value;
        auto first = stop(colors::red, 0.1F);
        auto second = stop(colors::blue, 1.0F);
        brush_value.gradient_stops().add(first);
        brush_value.gradient_stops().add(second);
        brush_value.set_binding_context(context);
        EXPECT_EQ(first->binding_context<int>(), context);
        EXPECT_EQ(second->binding_context<int>(), context);
    }

    TEST(brush_binding_context_test, view_background_inherits_context_without_parenting)
    {
        // BrushTypeConverterUnitTests.TestBrushBindingContext + ImmutableBrushDoesntSetParent: a brush set as
        // a view's Background inherits the view's BindingContext but is NOT parented (its logical parent
        // stays null).
        const auto context = std::make_shared<int>(11);
        maui::controls::grid parent;
        parent.set_binding_context(context);
        auto brush_value = std::make_shared<linear_gradient_brush>();
        brush_value->gradient_stops().add(stop(colors::red, 0.1F));
        parent.set_background_brush(std::shared_ptr<brush>{brush_value});
        EXPECT_EQ(brush_value->binding_context<int>(), context);
        EXPECT_EQ(brush_value->logical_parent(), nullptr);
    }

    TEST(brush_binding_context_test, immutable_brush_does_not_set_parent)
    {
        maui::controls::grid grid;
        grid.set_background_brush(std::shared_ptr<brush>{&brush::green(), [](brush*) {}}); // non-owning alias
        EXPECT_EQ(brush::green().logical_parent(), nullptr);
    }

    TEST(brush_binding_context_test, immutable_brush_throws_on_parent)
    {
        // SolidColorBrush.Green.Parent = grid -> InvalidOperationException (port: std::logic_error via
        // ImmutableBrush.OnParentChangingCore == element::on_logical_parent_changing, the seam every
        // attach_logical_child runs through). A null parent (detach) is allowed.
        maui::controls::grid grid;
        EXPECT_THROW(brush::green().on_logical_parent_changing(&grid), std::logic_error);
        EXPECT_NO_THROW(brush::green().on_logical_parent_changing(nullptr));
    }

    TEST(gradient_stop_test, get_hash_code_does_not_throw)
    {
        const gradient_stop value;
        EXPECT_NO_THROW((void)value.get_hash_code());
    }

    TEST(image_brush_test, is_empty_when_no_source)
    {
        const image_brush value;
        EXPECT_TRUE(value.is_empty());
    }
} // namespace

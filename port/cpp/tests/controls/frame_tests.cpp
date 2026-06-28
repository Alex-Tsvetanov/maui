// Tests for the frame control — ported from FrameUnitTests.cs (src/Controls/tests/Core.UnitTests):
// the constructor defaults (no content, Padding 20), the content parenting pair, the measure insets
// (TestFrameLayout's 140x240 = content + Padding 20, no border until BorderColor is set), the
// style-supplied padding (SettingPaddingThroughStyle — a style setter overrides the DEFAULT padding),
// plus the facade translation the port documents in frame.hpp: BorderColor → a 1px solid stroke
// (measure then insets 21 per side), CornerRadius → a round_rectangle stroke shape (validated to -1 or
// >= 0), HasShadow → the canned iOS frame shadow (radius 5, opacity 0.8).
//
// The LayoutOptions alignment tests are not ported (no LayoutOptions surface — the same documented
// scope as content_page/content_view).
#include "maui/controls/frame.hpp"

#include <any>
#include <memory>
#include <optional>
#include <stdexcept>

#include "maui/controls/setter.hpp"
#include "maui/controls/style.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::frame;
    using maui::controls::setter;
    using maui::controls::style;
    using maui::core::border_handler;
    using maui::core::i_element_handler;
    using maui::core::thickness;
    using maui::graphics::color;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    TEST(frame, defaults_have_no_content_and_padding_20) // C# TestConstructor
    {
        const frame view;
        EXPECT_EQ(view.content(), nullptr);
        EXPECT_EQ(view.padding(), thickness(20, 20, 20, 20));
        EXPECT_FALSE(view.border_color().has_value());
        EXPECT_EQ(view.corner_radius(), -1.0F);
        EXPECT_TRUE(view.has_shadow());
    }

    TEST(frame, set_and_replace_child_parents) // C# TestSetChild / TestReplaceChild
    {
        frame view;
        mock_view first;
        mock_view second;

        view.set_content(first);
        EXPECT_EQ(view.content(), &first);
        EXPECT_EQ(first.logical_parent(), &view);

        view.set_content(second);
        EXPECT_EQ(first.logical_parent(), nullptr);
        EXPECT_EQ(view.content(), &second);
        EXPECT_EQ(second.logical_parent(), &view);
    }

    TEST(frame, does_not_throw_on_null_child) // C# TestDoesNotThrowOnSetNullChild
    {
        frame view;
        view.set_content(nullptr);
        EXPECT_EQ(view.content(), nullptr);
    }

    TEST(frame, measure_insets_by_the_default_padding) // C# TestFrameLayout (the measure half)
    {
        frame view;
        mock_view child;
        child.configure({100, 200});
        view.set_content(child);

        // content 100x200 + padding {20} on all sides (no border color -> no border inset) -> 140x240.
        const size measured = view.measure(10000, 10000);
        EXPECT_EQ(measured.width, 140.0);
        EXPECT_EQ(measured.height, 240.0);
    }

    TEST(frame, arrange_places_content_within_the_padding)
    {
        frame view;
        mock_view child;
        child.configure({100, 200});
        view.set_content(child);

        view.measure(10000, 10000);
        view.arrange(rect(0, 0, 140, 240));
        EXPECT_EQ(child.last_arrange, rect(20, 20, 100, 200));
    }

    // ---- the facade translation onto the border machinery ----

    TEST(frame, border_color_adds_a_one_pixel_solid_stroke)
    {
        frame view;
        EXPECT_EQ(view.stroke(), nullptr);
        EXPECT_EQ(view.stroke_thickness(), 0.0); // the facade keeps 0 until a BorderColor arrives

        view.set_border_color(color(1.0F, 0.0F, 0.0F));
        // Compare the whole optional (the analyzer-friendly unchecked-optional pattern).
        EXPECT_EQ(view.border_color(), std::optional<color>(color(1.0F, 0.0F, 0.0F)));
        ASSERT_NE(view.stroke(), nullptr);
        EXPECT_EQ(view.stroke()->background_color(), color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(view.stroke_thickness(), 1.0); // IBorderElement.BorderWidth => 1

        // Frame.CrossPlatformMeasure: inset = Padding + (BorderColor != null ? 1 : 0).
        mock_view child;
        child.configure({100, 200});
        view.set_content(child);
        const size measured = view.measure(10000, 10000);
        EXPECT_EQ(measured.width, 142.0);
        EXPECT_EQ(measured.height, 242.0);
    }

    TEST(frame, corner_radius_drives_the_stroke_shape)
    {
        frame view;
        view.set_corner_radius(8.0F);
        EXPECT_EQ(view.corner_radius(), 8.0F);
        auto* rounded = dynamic_cast<maui::graphics::shapes::round_rectangle*>(view.shape());
        ASSERT_NE(rounded, nullptr);
        EXPECT_EQ(rounded->corner_radius().top_left, 8.0);

        view.set_corner_radius(-1.0F); // back to the sentinel -> the plain rectangle
        EXPECT_EQ(dynamic_cast<maui::graphics::shapes::round_rectangle*>(view.shape()), nullptr);
    }

    TEST(frame, corner_radius_validates_like_csharp)
    {
        frame view;
        EXPECT_THROW(view.set_corner_radius(-2.0F), std::invalid_argument);
        EXPECT_EQ(view.corner_radius(), -1.0F); // unchanged
    }

    // The facade translation must fire on the bindable-property seam too — the path the XAML loader
    // (register_bindable_property → apply_setter) and data bindings take, NOT just the imperative
    // set_* methods. Before the propertyChanged fix, setting BorderColor/CornerRadius/HasShadow this
    // way left stroke/shape/shadow untranslated, so a loader-mounted Frame rendered no chrome on iOS.
    TEST(frame, bindable_property_seam_translates_onto_the_border_machinery)
    {
        using maui::core::setter_specificity;

        frame view;
        view.apply_setter("border_color", std::any(color(0.86F, 0.20F, 0.27F)),
                          setter_specificity::manual_value_setter);
        ASSERT_NE(view.stroke(), nullptr);
        EXPECT_EQ(view.stroke()->background_color(), color(0.86F, 0.20F, 0.27F));
        EXPECT_EQ(view.stroke_thickness(), 1.0);

        view.apply_setter("corner_radius", std::any(6.0F), setter_specificity::manual_value_setter);
        auto* rounded = dynamic_cast<maui::graphics::shapes::round_rectangle*>(view.shape());
        ASSERT_NE(rounded, nullptr);
        EXPECT_EQ(rounded->corner_radius().top_left, 6.0);

        view.apply_setter("has_shadow", std::any(false), setter_specificity::manual_value_setter);
        EXPECT_EQ(view.shadow(), nullptr);
    }

    TEST(frame, has_shadow_drives_the_canned_frame_shadow)
    {
        frame view;
        ASSERT_NE(view.shadow(), nullptr); // default true
        EXPECT_EQ(view.shadow()->radius(), 5.0);
        EXPECT_EQ(view.shadow()->opacity(), 0.8);

        view.set_has_shadow(false);
        EXPECT_EQ(view.shadow(), nullptr);

        view.set_has_shadow(true);
        ASSERT_NE(view.shadow(), nullptr);
        EXPECT_EQ(view.shadow()->radius(), 5.0);
    }

    TEST(frame, setting_padding_through_style) // C# SettingPaddingThroughStyle
    {
        frame view;
        auto sheet = std::make_shared<style>(style::of<frame>());
        sheet->add(setter::of(frame::padding_property(), thickness(0)));

        view.set_style(sheet);
        EXPECT_EQ(view.padding(), thickness(0));
    }

    TEST(frame_seam, handler_resolved_from_default_registry_is_the_border_handler)
    {
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<frame>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<border_handler*>(handler.get()), nullptr);
    }
} // namespace

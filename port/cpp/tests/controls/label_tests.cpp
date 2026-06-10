// Tests for the label control + its headless handler seam — the second control, confirming the handler
// recipe generalizes (a display-only control: properties flow virtual→native, no events).
#include "maui/controls/label.hpp"

#include <memory>

#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::core::i_element_handler;
    using maui::core::i_label;
    using maui::core::i_text;
    using maui::core::label_handler;
    using maui::core::text_alignment;

    TEST(label, text_defaults_empty_and_is_settable)
    {
        label control;
        EXPECT_EQ(control.text(), "");
        control.set_text("Hello");
        EXPECT_EQ(control.text(), "Hello");
    }

    TEST(label, usable_through_interface_references)
    {
        label control;
        control.set_text("Caption");
        i_label& as_label = control;
        i_text& as_text = control;
        EXPECT_EQ(as_label.text(), "Caption");
        EXPECT_EQ(as_text.text(), "Caption");
        EXPECT_EQ(as_label.horizontal_text_alignment(), text_alignment::start);
    }

    TEST(label_seam, attaching_handler_maps_initial_text)
    {
        label control;
        control.set_text("Start");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        EXPECT_EQ(handler->typed_platform_view()->text, "Start");
    }

    TEST(label_seam, setting_properties_maps_to_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_text("Changed");
        EXPECT_EQ(platform->text, "Changed");

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->text_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_font(maui::core::font::of_size("Arial", 14));
        EXPECT_EQ(platform->text_font.family(), "Arial");
        EXPECT_EQ(platform->text_font.size(), 14.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(platform->horizontal_alignment, text_alignment::center);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ(platform->vertical_alignment, text_alignment::end);

        control.set_character_spacing(3.0);
        EXPECT_EQ(platform->character_spacing, 3.0);

        control.set_text_decorations(maui::core::text_decorations::underline);
        EXPECT_EQ(platform->decorations, maui::core::text_decorations::underline);

        control.set_text_decorations(maui::core::text_decorations::none);
        EXPECT_EQ(platform->decorations, maui::core::text_decorations::none);
    }

    TEST(label_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<label>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<label_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        label control;
        control.set_text("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->text, "Registered");
    }
} // namespace

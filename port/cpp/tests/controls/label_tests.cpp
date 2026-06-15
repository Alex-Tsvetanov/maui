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
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::core::i_element_handler;
    using maui::core::i_label;
    using maui::core::i_text;
    using maui::core::label_handler;
    using maui::core::line_break_mode;
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

    // line_height + padding flow virtual→platform mirror (LabelHandler.Mapper LineHeight / Padding entries).
    TEST(label_seam, line_height_and_padding_map_to_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Default: LineHeight -1 ("unset"), zero padding (Label.LineHeightProperty default / empty Thickness).
        EXPECT_EQ(platform->line_height, -1.0);
        EXPECT_EQ(platform->padding.left, 0.0);

        control.set_line_height(1.5);
        EXPECT_EQ(platform->line_height, 1.5);

        control.set_padding(maui::core::thickness(4, 8, 12, 16));
        EXPECT_EQ(platform->padding.left, 4.0);
        EXPECT_EQ(platform->padding.top, 8.0);
        EXPECT_EQ(platform->padding.right, 12.0);
        EXPECT_EQ(platform->padding.bottom, 16.0);
    }

    // line_break_mode + max_lines flow virtual→platform mirror (LabelHandler.Mapper LineBreakMode /
    // MaxLines entries). The headless mirror keeps the raw view values; the SetLineBreakMode resolution
    // (truncation forces a single line) lives in the native iOS/AppKit seams.
    TEST(label_seam, line_break_mode_and_max_lines_map_to_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Defaults: Label.LineBreakMode WordWrap, Label.MaxLines -1 (unset).
        EXPECT_EQ(control.line_break_mode(), line_break_mode::word_wrap);
        EXPECT_EQ(control.max_lines(), -1);
        EXPECT_EQ(platform->line_break_mode_value, line_break_mode::word_wrap);
        EXPECT_EQ(platform->max_lines, -1);

        control.set_line_break_mode(line_break_mode::tail_truncation);
        EXPECT_EQ(platform->line_break_mode_value, line_break_mode::tail_truncation);
        EXPECT_EQ(platform->max_lines, -1); // MaxLines untouched (the mirror co-mirrors both)

        control.set_max_lines(3);
        EXPECT_EQ(platform->max_lines, 3);
        EXPECT_EQ(platform->line_break_mode_value, line_break_mode::tail_truncation);

        control.set_line_break_mode(line_break_mode::no_wrap);
        EXPECT_EQ(platform->line_break_mode_value, line_break_mode::no_wrap);
        EXPECT_EQ(platform->max_lines, 3);
    }

    // line_break_mode + max_lines are reachable through the i_label contract (the two new virtuals).
    TEST(label, line_break_mode_and_max_lines_through_interface)
    {
        label control;
        control.set_line_break_mode(line_break_mode::middle_truncation);
        control.set_max_lines(2);
        i_label& as_label = control;
        EXPECT_EQ(as_label.line_break_mode(), line_break_mode::middle_truncation);
        EXPECT_EQ(as_label.max_lines(), 2);
    }

    // Padding inflates the desired size: MauiLabel.SizeThatFits subtracts the insets before measuring and
    // adds them back, so the measured size grows by (left+right, top+bottom).
    TEST(label_seam, padding_inflates_desired_size)
    {
        label control;
        control.set_text("Hi"); // 2 chars * 7pt = 14 wide, one 16pt line
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size bare = handler->get_desired_size(1.0e9, 1.0e9);
        control.set_padding(maui::core::thickness(5, 6, 7, 8));
        const maui::graphics::size padded = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_DOUBLE_EQ(padded.width, bare.width + 12.0);   // 5 + 7
        EXPECT_DOUBLE_EQ(padded.height, bare.height + 14.0); // 6 + 8
    }

    // PreferredMaxLayoutWidth branch: an explicit virtual Width wraps the text to multiple lines, so the
    // measured height grows past a single line and the width clamps to the explicit Width.
    TEST(label_seam, explicit_width_wraps_to_multiple_lines)
    {
        label control;
        // 10 chars * 7pt = 70pt of text; an explicit 35pt width => 2 lines (ceil(70/35)).
        control.set_text("ABCDEFGHIJ");
        control.set_width_request(35);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size wrapped = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_DOUBLE_EQ(wrapped.width, 35.0);  // clamped to the explicit Width
        EXPECT_DOUBLE_EQ(wrapped.height, 32.0); // two 16pt lines
    }

    // Without an explicit Width an unconstrained measure stays a single line (the non-PreferredMaxLayoutWidth
    // branch: PlatformView.PreferredMaxLayoutWidth = 0).
    TEST(label_seam, unconstrained_measure_is_single_line)
    {
        label control;
        control.set_text("ABCDEFGHIJ");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size measured = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_DOUBLE_EQ(measured.width, 70.0);
        EXPECT_DOUBLE_EQ(measured.height, 16.0);
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

#pragma once
// maui::samples::label_page — ports LabelPage.xaml (+ LabelPage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::label. Mirrors the EXACT shape of
// value_controls_page.hpp / input_controls_page.hpp / formatted_text_page.hpp: the class OWNS its whole
// element tree as members, exposes page(), and uses only cross-platform maui:: API so it
// stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - Basic properties: a defaults label, a TextColor=Red label, a BackgroundColor=Cyan label,
//   - Horizontal text alignment: Start / Center / End / Justify (on light backgrounds),
//   - Vertical text alignment: Start / Center / End (with a HeightRequest so the alignment is visible),
//   - Formatted text spans: one label whose FormattedText is built from several styled spans
//     (plain / colored / strikethrough / big font), plus a "Change Formatted String" button that
//     replaces the spans at runtime (ChangeFormattedString_Clicked),
//   - Maximum lines: MaxLines=1 and MaxLines=2 on a long paragraph,
//   - Line break mode: NoWrap / WordWrap / HeadTruncation / MiddleTruncation / TailTruncation /
//     CharacterWrap on a long paragraph.
//
// note: the HTML-text labels, the per-span tap gestures (random-recolor), and the EllipseGeometry
// clipping demo are simplified/omitted — they depend on HTML text rendering, span gesture wiring, and
// clip-shape geometry that aren't the focus of this label gallery; the label's mapped text/alignment/
// max-lines/line-break/formatted-text surface is all exercised above.

#include <memory>
#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/formatted_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/span.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class label_page
    {
    public:
        label_page()
        {
            page_.set_title("Label");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            // A long paragraph reused for the MaxLines + LineBreakMode demos (the XAML Lorem ipsum).
            static const std::string lorem =
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt "
                "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
                "laboris nisi ut aliquip ex ea commodo consequat.";

            // ---- Basic properties ----
            defaults_label_.set_text("Defaults");

            text_color_label_.set_text("This text should be RED");
            text_color_label_.set_text_color(maui::graphics::colors::red);

            background_label_.set_text("This has a solid CYAN background color");
            background_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::cyan));

            // ---- Horizontal text alignment ----
            h_start_label_.set_text("This should be at the start of the line");
            h_start_label_.set_horizontal_text_alignment(maui::core::text_alignment::start);
            h_start_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));

            h_center_label_.set_text("This should be at the center of the line");
            h_center_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            h_center_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::silver));

            h_end_label_.set_text("This should be at the end of the line");
            h_end_label_.set_horizontal_text_alignment(maui::core::text_alignment::end);
            h_end_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));

            h_justify_label_.set_text(lorem);
            h_justify_label_.set_horizontal_text_alignment(maui::core::text_alignment::justify);
            h_justify_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::silver));

            // ---- Vertical text alignment (HeightRequest so the alignment is visible) ----
            v_start_label_.set_text("This should be at the start");
            v_start_label_.set_vertical_text_alignment(maui::core::text_alignment::start);
            v_start_label_.set_height_request(100);
            v_start_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));

            v_center_label_.set_text("This should be at the center");
            v_center_label_.set_vertical_text_alignment(maui::core::text_alignment::center);
            v_center_label_.set_height_request(100);
            v_center_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::silver));

            v_end_label_.set_text("This should be at the bottom");
            v_end_label_.set_vertical_text_alignment(maui::core::text_alignment::end);
            v_end_label_.set_height_request(100);
            v_end_label_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));

            // ---- Formatted text spans ----
            build_initial_formatted_text();
            change_formatted_button_.set_text("Change Formatted String");
            change_formatted_button_.clicked.connect([this] { change_formatted_text(); });

            // ---- Maximum lines ----
            max_lines_1_label_.set_text(lorem);
            max_lines_1_label_.set_max_lines(1);

            max_lines_2_label_.set_text(lorem);
            max_lines_2_label_.set_max_lines(2);

            // ---- Line break mode ----
            no_wrap_label_.set_text(lorem);
            no_wrap_label_.set_line_break_mode(maui::core::line_break_mode::no_wrap);

            word_wrap_label_.set_text(lorem);
            word_wrap_label_.set_line_break_mode(maui::core::line_break_mode::word_wrap);

            head_trunc_label_.set_text(lorem);
            head_trunc_label_.set_line_break_mode(maui::core::line_break_mode::head_truncation);

            middle_trunc_label_.set_text(lorem);
            middle_trunc_label_.set_line_break_mode(maui::core::line_break_mode::middle_truncation);

            tail_trunc_label_.set_text(lorem);
            tail_trunc_label_.set_line_break_mode(maui::core::line_break_mode::tail_truncation);

            char_wrap_label_.set_text(lorem);
            char_wrap_label_.set_line_break_mode(maui::core::line_break_mode::character_wrap);

            stack_.add(defaults_label_);
            stack_.add(text_color_label_);
            stack_.add(background_label_);
            stack_.add(h_start_label_);
            stack_.add(h_center_label_);
            stack_.add(h_end_label_);
            stack_.add(h_justify_label_);
            stack_.add(v_start_label_);
            stack_.add(v_center_label_);
            stack_.add(v_end_label_);
            stack_.add(formatted_label_);
            stack_.add(change_formatted_button_);
            stack_.add(max_lines_1_label_);
            stack_.add(max_lines_2_label_);
            stack_.add(no_wrap_label_);
            stack_.add(word_wrap_label_);
            stack_.add(head_trunc_label_);
            stack_.add(middle_trunc_label_);
            stack_.add(tail_trunc_label_);
            stack_.add(char_wrap_label_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& formatted_label()
        {
            return formatted_label_;
        }
        [[nodiscard]] maui::controls::button& change_formatted_button()
        {
            return change_formatted_button_;
        }

    private:
        // The initial multi-span FormattedText, mirroring the ORIGINAL LabelPage.xaml <FormattedString>
        // segmentation (and the shared twin): styled spans separated by standalone PLAIN one-space spans,
        // so the Cyan "Colors" background and the Strikethrough decoration never bleed into the
        // separators. "Colors" carries Cyan background + Navy text; "Big Font" is FontSize 20; the
        // trailing "Plain old Text" is plain (the original's tail span has no color).
        void build_initial_formatted_text()
        {
            auto formatted = std::make_shared<maui::controls::formatted_string>();

            const auto space_span = [] {
                auto separator = std::make_shared<maui::controls::span>();
                separator->set_text(" ");
                return separator;
            };

            auto plain = std::make_shared<maui::controls::span>();
            plain->set_text("Plain old Text");

            auto colored = std::make_shared<maui::controls::span>();
            colored->set_text("Colors");
            colored->set_background_color(maui::graphics::colors::cyan);
            colored->set_text_color(maui::graphics::colors::navy);

            auto strike = std::make_shared<maui::controls::span>();
            strike->set_text("Strikethrough");
            strike->set_text_decorations(maui::core::text_decorations::strikethrough);

            auto big = std::make_shared<maui::controls::span>();
            big->set_text("Big Font");
            big->set_font_size(20);

            auto tail = std::make_shared<maui::controls::span>();
            tail->set_text("Plain old Text");

            formatted->add_span(plain);
            formatted->add_span(space_span());
            formatted->add_span(colored);
            formatted->add_span(space_span());
            formatted->add_span(strike);
            formatted->add_span(space_span());
            formatted->add_span(big);
            formatted->add_span(space_span());
            formatted->add_span(tail);
            formatted_label_.set_formatted_text(formatted);
        }

        // ChangeFormattedString_Clicked: replace the FormattedText with two new spans (a plain + a bold).
        void change_formatted_text()
        {
            auto formatted = std::make_shared<maui::controls::formatted_string>();

            auto testing = std::make_shared<maui::controls::span>();
            testing->set_text("Testing");

            auto bold = std::make_shared<maui::controls::span>();
            bold->set_text("Bold");
            bold->set_font_attributes(maui::core::font_attributes::bold);

            formatted->add_span(testing);
            formatted->add_span(bold);
            formatted_label_.set_formatted_text(formatted);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label defaults_label_;
        maui::controls::label text_color_label_;
        maui::controls::label background_label_;
        maui::controls::label h_start_label_;
        maui::controls::label h_center_label_;
        maui::controls::label h_end_label_;
        maui::controls::label h_justify_label_;
        maui::controls::label v_start_label_;
        maui::controls::label v_center_label_;
        maui::controls::label v_end_label_;
        maui::controls::label formatted_label_;
        maui::controls::button change_formatted_button_;
        maui::controls::label max_lines_1_label_;
        maui::controls::label max_lines_2_label_;
        maui::controls::label no_wrap_label_;
        maui::controls::label word_wrap_label_;
        maui::controls::label head_trunc_label_;
        maui::controls::label middle_trunc_label_;
        maui::controls::label tail_trunc_label_;
        maui::controls::label char_wrap_label_;
    };
} // namespace maui::samples

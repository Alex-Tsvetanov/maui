#pragma once
// maui::samples::activity_indicator_page — ports ActivityIndicatorPage.xaml (+ ActivityIndicatorPage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::activity_indicator. Mirrors the EXACT shape
// of value_controls_page.hpp / button_page.hpp: the class OWNS its whole element tree as members,
// exposes page(), and uses only cross-platform maui:: API so it stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - a Default running indicator (IsRunning=True),
//   - a Styled indicator with Color="CornflowerBlue" (a plain named-color literal in the shared XAML twin
//     — see note),
//   - a Styled indicator with BackgroundColor=Yellow (here a solid_color_brush, as the button page does
//     for BackgroundColor),
//   - a Larger indicator (WidthRequest/HeightRequest=150),
//   - a Smaller indicator (WidthRequest/HeightRequest=10; XAML HorizontalOptions=Center omitted, see note),
//   - a Not-Running indicator (IsRunning=False),
//   - an "- End of page -" subhead label.
//
// note: the shared XAML twin (port/maui-reference/pages/activity_indicator.xaml) sets the "Color" cell's
// Color as a plain named-color literal (Color="CornflowerBlue"), NOT an AppThemeBinding — so this page
// matches it with the same named color rather than standing in for a themed accent (an earlier revision
// of this comment claimed the port had no app-theme surface as the reason for a hand-picked literal; that
// claim was stale even against the twin it was justifying, which never used AppThemeBinding to begin
// with). HorizontalOptions has no surface on the port's view, so the Smaller indicator is left at its
// layout default. The Headline / Subhead label styling likewise maps to plain labels.

#include <memory>
#include <string>

#include "maui/controls/activity_indicator.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class activity_indicator_page
    {
    public:
        activity_indicator_page()
        {
            page_.set_title("ActivityIndicator");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            // ---- Default (IsRunning=True) ----
            default_header_.set_text("Default");
            default_indicator_.set_is_running(true);

            // ---- Styled - Color="CornflowerBlue" ----
            // note: the shared XAML twin sets a plain named-color literal here (no AppThemeBinding — see
            // the file-top note), so this matches it with the same named color exactly.
            color_header_.set_text("Color"); // the shared XAML's label text (was verbose "Styled - ...")
            color_indicator_.set_is_running(true);
            color_indicator_.set_color(maui::graphics::colors::cornflower_blue);

            // ---- Styled - BackgroundColor=Yellow ----
            background_header_.set_text("BackgroundColor=Yellow"); // shared XAML label text
            background_indicator_.set_is_running(true);
            background_indicator_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::yellow));

            // ---- Larger (Width/HeightRequest=150) ----
            larger_header_.set_text("Larger");
            larger_indicator_.set_is_running(true);
            larger_indicator_.set_width_request(150);
            larger_indicator_.set_height_request(150);

            // ---- Smaller (Width/HeightRequest=10; XAML HorizontalOptions=Center) ----
            // note: HorizontalOptions has no surface on the port's view; the indicator keeps its default.
            smaller_header_.set_text("Smaller"); // shared XAML label text
            smaller_indicator_.set_is_running(true);
            smaller_indicator_.set_width_request(10);
            smaller_indicator_.set_height_request(10);

            // ---- Not Running (IsRunning=False) ----
            not_running_header_.set_text("Not Running");
            not_running_indicator_.set_is_running(false);

            // ---- End-of-page subhead ----
            end_label_.set_text("- End of page -");

            // Section headers render bold @18pt — mirrors maui-compare ActivityIndicatorPage.Headline().
            for (maui::controls::label* h : {&default_header_, &color_header_, &background_header_, &smaller_header_,
                                             &larger_header_, &not_running_header_})
            {
                h->set_font(maui::core::font::system_font_of_size(18.0, maui::core::font_weight::bold));
            }

            stack_.add(default_header_);
            stack_.add(default_indicator_);
            stack_.add(color_header_);
            stack_.add(color_indicator_);
            stack_.add(background_header_);
            stack_.add(background_indicator_);
            stack_.add(larger_header_);
            stack_.add(larger_indicator_);
            stack_.add(smaller_header_);
            stack_.add(smaller_indicator_);
            stack_.add(not_running_header_);
            stack_.add(not_running_indicator_);
            stack_.add(end_label_);
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
        [[nodiscard]] maui::controls::activity_indicator& default_indicator()
        {
            return default_indicator_;
        }
        [[nodiscard]] maui::controls::activity_indicator& color_indicator()
        {
            return color_indicator_;
        }
        [[nodiscard]] maui::controls::activity_indicator& larger_indicator()
        {
            return larger_indicator_;
        }
        [[nodiscard]] maui::controls::activity_indicator& not_running_indicator()
        {
            return not_running_indicator_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label default_header_;
        maui::controls::activity_indicator default_indicator_;
        maui::controls::label color_header_;
        maui::controls::activity_indicator color_indicator_;
        maui::controls::label background_header_;
        maui::controls::activity_indicator background_indicator_;
        maui::controls::label larger_header_;
        maui::controls::activity_indicator larger_indicator_;
        maui::controls::label smaller_header_;
        maui::controls::activity_indicator smaller_indicator_;
        maui::controls::label not_running_header_;
        maui::controls::activity_indicator not_running_indicator_;
        maui::controls::label end_label_;
    };
} // namespace maui::samples

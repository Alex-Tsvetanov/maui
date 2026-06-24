#pragma once
// maui::samples::button_page — ports ButtonPage.xaml (+ ButtonPage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::button. Mirrors the EXACT shape of
// value_controls_page.hpp / input_controls_page.hpp: the class OWNS its whole element tree as members,
// exposes page(), and uses only cross-platform maui:: API so it stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - a Default button, a Disabled button (IsEnabled=false), and a Clicked button whose taps bump a
//     readout (the OnButtonClicked handler → here a live count),
//   - a Command button (XAML Button.Command bound to ButtonCommand → here button::command), which drives
//     the same readout,
//   - BackgroundColor / TextColor / BorderColor (StrokeColor) / BorderWidth (StrokeThickness) /
//     CornerRadius / CharacterSpacing styling buttons,
//   - an ImageSource button whose Clicked toggles the image on/off (Button_Clicked), plus a
//     ContentLayout "position" button that cycles Left→Top→Right→Bottom (OnPositionChange) and
//     Decrease/Increase-Spacing buttons (OnDecreaseSpacing / OnIncreasingSpacing),
//   - a Padding button, and a Slider whose Value drives a button's BorderWidth (the XAML
//     {x:Reference BorderWidthSlider} binding → here a value_changed wire).
//
// note: ButtonPage's LineBreakMode buttons, the gradient/background-brush swap, the tooltip, the custom
// converter color binding, and HorizontalOptions are simplified or omitted — the port's button has no
// line_break_mode surface and the gallery focuses on the mapped button properties; the styling intent is
// preserved through the color/border/corner/spacing/image buttons above.

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/button_content_layout.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class button_page
    {
    public:
        button_page()
        {
            page_.set_title("Button");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            readout_.set_text("Taps: 0");

            // ---- Default ----
            default_button_.set_text("Button");

            // ---- Disabled (IsEnabled=false; its Clicked is gated off by send_clicked) ----
            disabled_button_.set_text("Button (disabled)");
            disabled_button_.set_is_enabled(false);

            // ---- Clicked (ButtonPage.OnButtonClicked → bump the readout) ----
            clicked_button_.set_text("Clicked");
            clicked_button_.clicked.connect([this] {
                ++tap_count_;
                update_readout();
            });

            // ---- Command (XAML Button.Command={Binding ButtonCommand}; here button::command) ----
            command_button_.set_text("Command");
            command_button_.command = [this] {
                ++tap_count_;
                update_readout();
            };

            // ---- BackgroundColor (Blue) ----
            background_button_.set_text("Button");
            background_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::blue));

            // ---- TextColor (White on Red) ----
            text_color_button_.set_text("Button");
            text_color_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::red));
            text_color_button_.set_text_color(maui::graphics::colors::white);

            // ---- BorderColor (StrokeColor=Red on Green, White text) ----
            border_color_button_.set_text("BorderColor");
            border_color_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            border_color_button_.set_text_color(maui::graphics::colors::white);
            border_color_button_.set_stroke_color(maui::graphics::colors::red);

            // ---- BorderWidth (StrokeThickness=4) ----
            border_width_button_.set_text("BorderWidth");
            border_width_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            border_width_button_.set_text_color(maui::graphics::colors::white);
            border_width_button_.set_stroke_color(maui::graphics::colors::red);
            border_width_button_.set_stroke_thickness(4);

            // ---- CornerRadius = 10 (Purple, White text) ----
            corner_radius_button_.set_text("CornerRadius");
            corner_radius_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::purple));
            corner_radius_button_.set_text_color(maui::graphics::colors::white);
            corner_radius_button_.set_corner_radius(10);

            // ---- CharacterSpacing = 20 (HotPink text) ----
            spacing_button_.set_text("Button");
            spacing_button_.set_character_spacing(20);
            spacing_button_.set_text_color(maui::graphics::colors::hot_pink);

            // ---- ImageSource + ContentLayout: toggle the image (Button_Clicked) ----
            image_button_.set_text("settings");
            image_button_.set_text_color(maui::graphics::colors::white);
            image_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::black));
            image_button_.set_content_layout(
                maui::controls::button_content_layout(maui::controls::button_content_layout::image_position::top, 10));
            // note: settings.png needs a bundled asset to display; the file source is set to a plausible
            // bundle-relative path (the port loads file sources synchronously when present).
            image_button_.set_image_source(maui::controls::image_source::from_file("settings.png"));
            image_button_.clicked.connect([this] {
                image_on_ = !image_on_;
                image_button_.set_image_source(image_on_ ? maui::controls::image_source::from_file("settings.png")
                                                         : nullptr);
            });

            // ---- ContentLayout positioning: cycle Left→Top→Right→Bottom (OnPositionChange) ----
            position_button_.set_text("settings");
            position_button_.set_text_color(maui::graphics::colors::white);
            position_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::black));
            position_button_.set_image_source(maui::controls::image_source::from_file("settings.png"));
            position_button_.set_content_layout(
                maui::controls::button_content_layout(maui::controls::button_content_layout::image_position::top, 10));
            position_button_.clicked.connect([this] { cycle_position(); });

            // ---- Decrease / Increase spacing on the positioning button (OnDecreaseSpacing/OnIncreasingSpacing) ----
            decrease_spacing_button_.set_text("Decrease Spacing");
            decrease_spacing_button_.set_text_color(maui::graphics::colors::white);
            decrease_spacing_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::black));
            decrease_spacing_button_.clicked.connect([this] { nudge_spacing(-1); });

            increase_spacing_button_.set_text("Increase Spacing");
            increase_spacing_button_.set_text_color(maui::graphics::colors::white);
            increase_spacing_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::black));
            increase_spacing_button_.clicked.connect([this] { nudge_spacing(+1); });

            // ---- Padding = 20,10 (LightGray background) ----
            padding_button_.set_text("Button");
            padding_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_gray));
            padding_button_.set_padding(maui::core::thickness(20, 10));

            // ---- Slider drives BorderWidth (XAML {x:Reference BorderWidthSlider}) ----
            border_width_demo_button_.set_text("BorderWidth Changing");
            border_width_demo_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            border_width_slider_.set_minimum(0);
            border_width_slider_.set_maximum(100);
            border_width_slider_.set_value(0);
            border_width_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                border_width_demo_button_.set_stroke_thickness(new_value);
            });

            stack_.add(readout_);
            stack_.add(default_button_);
            stack_.add(disabled_button_);
            stack_.add(clicked_button_);
            stack_.add(command_button_);
            stack_.add(background_button_);
            stack_.add(text_color_button_);
            stack_.add(border_color_button_);
            stack_.add(border_width_button_);
            stack_.add(corner_radius_button_);
            stack_.add(spacing_button_);
            stack_.add(image_button_);
            stack_.add(position_button_);
            stack_.add(decrease_spacing_button_);
            stack_.add(increase_spacing_button_);
            stack_.add(padding_button_);
            stack_.add(border_width_demo_button_);
            stack_.add(border_width_slider_);
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
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::button& clicked_button()
        {
            return clicked_button_;
        }
        [[nodiscard]] maui::controls::button& command_button()
        {
            return command_button_;
        }
        [[nodiscard]] maui::controls::button& image_button()
        {
            return image_button_;
        }
        [[nodiscard]] maui::controls::button& position_button()
        {
            return position_button_;
        }
        [[nodiscard]] maui::controls::slider& border_width_slider()
        {
            return border_width_slider_;
        }

    private:
        void update_readout()
        {
            char text[32];
            std::snprintf(text, sizeof(text), "Taps: %d", tap_count_);
            readout_.set_text(text);
        }

        // OnPositionChange: Left→Top→Right→Bottom→Left, keeping the current spacing.
        void cycle_position()
        {
            const auto current = position_button_.content_layout();
            auto next = static_cast<int>(current.position) + 1;
            if (next > static_cast<int>(maui::controls::button_content_layout::image_position::bottom))
            {
                next = 0;
            }
            position_button_.set_content_layout(maui::controls::button_content_layout(
                static_cast<maui::controls::button_content_layout::image_position>(next), current.spacing));
        }

        // OnDecreaseSpacing / OnIncreasingSpacing: keep the position, step the spacing by delta.
        void nudge_spacing(double delta)
        {
            const auto current = position_button_.content_layout();
            position_button_.set_content_layout(
                maui::controls::button_content_layout(current.position, current.spacing + delta));
        }

        int tap_count_ = 0;
        bool image_on_ = true;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::button default_button_;
        maui::controls::button disabled_button_;
        maui::controls::button clicked_button_;
        maui::controls::button command_button_;
        maui::controls::button background_button_;
        maui::controls::button text_color_button_;
        maui::controls::button border_color_button_;
        maui::controls::button border_width_button_;
        maui::controls::button corner_radius_button_;
        maui::controls::button spacing_button_;
        maui::controls::button image_button_;
        maui::controls::button position_button_;
        maui::controls::button decrease_spacing_button_;
        maui::controls::button increase_spacing_button_;
        maui::controls::button padding_button_;
        maui::controls::button border_width_demo_button_;
        maui::controls::slider border_width_slider_;
    };
} // namespace maui::samples

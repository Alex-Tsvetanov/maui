#pragma once
// maui::samples::image_button_page — ports ImageButtonPage.xaml (+ ImageButtonPage.xaml.cs)
//
// A self-contained, code-first demo page for the ImageButton control (the C# gallery-page convention,
// mirroring the input_controls_page / image_page pattern). The page OWNS its whole element tree;
// `page()` hands back the content_page; the generic mount (app_host.hpp) attaches every owned view's
// handler and hosts the tree.
//
// The C# page is a scroll of headline-labelled ImageButton variants. This port keeps the
// cross-platform-API subset and wires the demonstrated INTERACTIONS:
//   - Aspect: three buttons (AspectFit / AspectFill / Fill), each green-backed, each bumping a shared
//     click counter shown in the readout (the C# OnImageButtonClicked + InfoLabel).
//   - BorderColor / BorderWidth: a red-stroked button; a Slider (0..20, start 4) drives the stroke
//     thickness (the C# {Binding Source={x:Reference BorderWidthSlider}, Path=Value}).
//   - CornerRadius: fixed 0 / 10 plus a Slider (0..60, start 10) driving the corner radius.
//   - Custom Size: a small button whose Click resizes it to 100x100 (the C# OnResizeImageButtonClicked).
//   - Padding: a Slider (0..60, start 10) drives the padding (the C# Padding binding).
//   - Animated GIF: a gif source, plus a "Use Online Source" button swapping to a remote URI source
//     (the C# UseOnlineSource_Clicked → ImageSource.FromUri).
//   - Background (Gradient): an "Update / Remove Background" pair swapping a green↔purple solid brush
//     in for the gradient (the C# LinearGradientBrush; the color randomization collapses to a stable
//     toggle so the readout stays deterministic — headless-safe).
//
// Faithful best-effort deviations (// note:):
//   - HorizontalOptions="Center" / VerticalOptions have no cross-platform layer analog in the port's
//     view, so they are dropped (WidthRequest/HeightRequest are kept).
//   - cog.png / dotnet_bot.png / animated_heart.gif are bundled assets the headless backend has no
//     loader for; set_source mints the file source faithfully but the image won't rasterize headless
//     (the image control's documented convention).
//   - The C# VisualStateManager Pressed-scale block is markup-era VSM wiring; the control's
//     ChangeVisualState already drives Pressed, so no extra setup is reproduced here.
//   - The C# random gradient background collapses to a deterministic green↔purple solid toggle.

#include <cstdio>
#include <memory>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class image_button_page
    {
    public:
        image_button_page()
        {
            page_.set_title("ImageButton");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            readout_.set_text("0 ImageButton clicks");

            // ---- Aspect (AspectFit / AspectFill / Fill), each click bumps the shared counter ----
            fit_label_.set_text("AspectFit");
            configure_aspect_button(fit_button_, maui::core::aspect::aspect_fit, 200, 0);
            fill_label_.set_text("AspectFill");
            configure_aspect_button(fill_button_, maui::core::aspect::aspect_fill, 200, 100);
            stretch_label_.set_text("Fill");
            configure_aspect_button(stretch_button_, maui::core::aspect::fill, 200, 100);

            // ---- BorderColor + BorderWidth (slider-driven stroke thickness) ----
            border_label_.set_text("BorderColor + BorderWidth");
            border_button_.set_aspect(maui::core::aspect::aspect_fit);
            border_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            border_button_.set_stroke_color(maui::graphics::colors::red);
            border_button_.set_stroke_thickness(4);
            border_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            border_width_slider_.set_minimum(0);
            border_width_slider_.set_maximum(20);
            border_width_slider_.set_value(4);
            border_width_slider_.value_changed.connect(
                [this](double, double new_value) { border_button_.set_stroke_thickness(new_value); });

            // ---- CornerRadius (fixed + slider-driven) ----
            corner_label_.set_text("CornerRadius = 0 / 10 / slider");
            corner_zero_button_.set_aspect(maui::core::aspect::aspect_fit);
            corner_zero_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            corner_zero_button_.set_corner_radius(0);
            corner_zero_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::purple));
            corner_ten_button_.set_aspect(maui::core::aspect::aspect_fit);
            corner_ten_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            corner_ten_button_.set_corner_radius(10);
            corner_ten_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::purple));
            corner_slider_button_.set_aspect(maui::core::aspect::aspect_fit);
            corner_slider_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            corner_slider_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::purple));
            corner_slider_button_.set_corner_radius(10);
            corner_radius_slider_.set_minimum(0);
            corner_radius_slider_.set_maximum(60);
            corner_radius_slider_.set_value(10);
            corner_radius_slider_.value_changed.connect([this](double, double new_value) {
                corner_slider_button_.set_corner_radius(static_cast<int>(new_value));
            });

            // ---- Custom Size (Click resizes the button to 100x100) ----
            resize_label_.set_text("Custom Size (click to resize)");
            resize_button_.set_source(maui::controls::image_source::from_file("dotnet_bot.png"));
            resize_button_.set_width_request(40);
            resize_button_.set_height_request(40);
            resize_button_.clicked.connect([this] {
                resize_button_.set_width_request(100);
                resize_button_.set_height_request(100);
            });

            // ---- Padding (slider-driven) ----
            padding_label_.set_text("Padding (slider-driven)");
            padding_button_.set_aspect(maui::core::aspect::aspect_fit);
            padding_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            padding_button_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            padding_button_.set_padding(maui::core::thickness(10));
            padding_button_.clicked.connect([this] { bump_click_count(); });
            padding_slider_.set_minimum(0);
            padding_slider_.set_maximum(60);
            padding_slider_.set_value(10);
            padding_slider_.value_changed.connect(
                [this](double, double new_value) { padding_button_.set_padding(maui::core::thickness(new_value)); });

            // ---- Animated GIF + "Use Online Source" ----
            gif_label_.set_text("Animated GIF");
            gif_button_.set_source(maui::controls::image_source::from_file("animated_heart.gif"));
            online_source_button_.set_text("Use Online Source");
            online_source_button_.clicked.connect([this] {
                gif_button_.set_source(maui::controls::image_source::from_uri(
                    "https://raw.githubusercontent.com/dotnet/maui/main/src/Compatibility/ControlGallery/"
                    "src/iOS/GifTwo.gif"));
            });

            // ---- Background (gradient → deterministic green/purple toggle) ----
            background_label_.set_text("Background (toggle)");
            background_button_.set_aspect(maui::core::aspect::aspect_fit);
            background_button_.set_source(maui::controls::image_source::from_file("cog.png"));
            background_button_.set_padding(maui::core::thickness(0));
            apply_background();
            update_background_button_.set_text("Update Background");
            update_background_button_.clicked.connect([this] {
                background_is_green_ = !background_is_green_;
                apply_background();
            });
            remove_background_button_.set_text("Remove Background");
            remove_background_button_.clicked.connect([this] { background_button_.set_background(nullptr); });

            stack_.add(readout_);
            stack_.add(fit_label_);
            stack_.add(fit_button_);
            stack_.add(fill_label_);
            stack_.add(fill_button_);
            stack_.add(stretch_label_);
            stack_.add(stretch_button_);
            stack_.add(border_label_);
            stack_.add(border_button_);
            stack_.add(border_width_slider_);
            stack_.add(corner_label_);
            stack_.add(corner_zero_button_);
            stack_.add(corner_ten_button_);
            stack_.add(corner_slider_button_);
            stack_.add(corner_radius_slider_);
            stack_.add(resize_label_);
            stack_.add(resize_button_);
            stack_.add(padding_label_);
            stack_.add(padding_button_);
            stack_.add(padding_slider_);
            stack_.add(gif_label_);
            stack_.add(gif_button_);
            stack_.add(online_source_button_);
            stack_.add(background_label_);
            stack_.add(background_button_);
            stack_.add(update_background_button_);
            stack_.add(remove_background_button_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::image_button& fit_button()
        {
            return fit_button_;
        }
        [[nodiscard]] maui::controls::image_button& resize_button()
        {
            return resize_button_;
        }
        [[nodiscard]] maui::controls::slider& corner_radius_slider()
        {
            return corner_radius_slider_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] int click_total() const
        {
            return click_total_;
        }

    private:
        void configure_aspect_button(maui::controls::image_button& button, maui::core::aspect aspect, double width,
                                     double height)
        {
            button.set_aspect(aspect);
            button.set_source(maui::controls::image_source::from_file("cog.png"));
            button.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            button.set_padding(maui::core::thickness(16, 9, 16, 9));
            button.set_width_request(width);
            if (height > 0)
            {
                button.set_height_request(height);
            }
            button.clicked.connect([this] { bump_click_count(); });
        }

        void bump_click_count()
        {
            ++click_total_;
            char text[48];
            std::snprintf(text, sizeof(text), "%d ImageButton click%s", click_total_, click_total_ == 1 ? "" : "s");
            readout_.set_text(text);
        }

        void apply_background()
        {
            background_button_.set_background_brush(std::make_shared<maui::controls::solid_color_brush>(
                background_is_green_ ? maui::graphics::colors::green : maui::graphics::colors::purple));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label readout_;
        maui::controls::label fit_label_;
        maui::controls::image_button fit_button_;
        maui::controls::label fill_label_;
        maui::controls::image_button fill_button_;
        maui::controls::label stretch_label_;
        maui::controls::image_button stretch_button_;
        maui::controls::label border_label_;
        maui::controls::image_button border_button_;
        maui::controls::slider border_width_slider_;
        maui::controls::label corner_label_;
        maui::controls::image_button corner_zero_button_;
        maui::controls::image_button corner_ten_button_;
        maui::controls::image_button corner_slider_button_;
        maui::controls::slider corner_radius_slider_;
        maui::controls::label resize_label_;
        maui::controls::image_button resize_button_;
        maui::controls::label padding_label_;
        maui::controls::image_button padding_button_;
        maui::controls::slider padding_slider_;
        maui::controls::label gif_label_;
        maui::controls::image_button gif_button_;
        maui::controls::button online_source_button_;
        maui::controls::label background_label_;
        maui::controls::image_button background_button_;
        maui::controls::button update_background_button_;
        maui::controls::button remove_background_button_;

        int click_total_ = 0;
        bool background_is_green_ = true;
    };
} // namespace maui::samples

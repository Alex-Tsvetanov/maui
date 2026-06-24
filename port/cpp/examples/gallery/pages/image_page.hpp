#pragma once
// maui::samples::image_page — ports ImagePage.xaml (+ ImagePage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::image. Mirrors the EXACT shape of
// value_controls_page.hpp / input_controls_page.hpp: the class OWNS its whole element tree as members,
// exposes page(), and uses only cross-platform maui:: API so it stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - UriSource: an image whose Source is a remote URI (image_source::from_uri),
//   - FileSource: an image whose Source is a bundled file (image_source::from_file),
//   - Font image source: a glyph rendered from a font, both with and without auto-scaling
//     (image_source::from_font with a maui::core::font carrying the family/size/auto-scaling),
//   - Animating a gif: a toggle_switch whose IsToggled drives the gif's IsAnimationPlaying
//     (the XAML {x:Reference IsAnimationPlayingSwitch} binding → here a toggled wire),
//   - Stream source: an image whose Source is a bytes-provider (image_source::from_stream),
//   - Opacity (0.5): an image with reduced opacity over a black background,
//   - Animated GIF with Start/Stop + Use-Online-Source buttons (AnimationStartStop_Clicked /
//     UseOnlineSource_Clicked → toggle is_animation_playing / swap to a uri source).
//
// note: the on-disk/asset images (dotnet_bot.png, animated_heart.gif, settings.png) need bundled assets
// to actually display; file sources are set to plausible bundle-relative paths (the port loads file
// sources synchronously when present, uri/stream/font sources load asynchronously through the loader).
// The stream source here returns empty bytes — a real app would supply the decoded image bytes; the wiring
// (from_stream → handler load path) is faithful even though no pixels are produced headless.

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class image_page
    {
    public:
        image_page()
        {
            page_.set_title("Image");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            uri_caption_.set_text("UriSource");
            uri_image_.set_source(maui::controls::image_source::from_uri("https://aka.ms/campus.jpg"));

            file_caption_.set_text("FileSource");
            file_image_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::purple));
            file_image_.set_source(maui::controls::image_source::from_file("dotnet_bot.png"));

            // Font image source, auto-scaling disabled (Size=20, family Ionicons, a glyph).
            font_caption_.set_text("Font Image Source");
            font_image_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            font_image_.set_source(maui::controls::image_source::from_font(
                "", maui::core::font::of_size("Ionicons", 20).with_auto_scaling(false),
                maui::graphics::colors::white));

            // Font image source, auto-scaling enabled (Size=90).
            font_scaled_caption_.set_text("Font Image Source (auto-scaling)");
            font_scaled_image_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::green));
            font_scaled_image_.set_source(maui::controls::image_source::from_font(
                "", maui::core::font::of_size("Ionicons", 90).with_auto_scaling(true),
                maui::graphics::colors::white));

            // Animating a gif: the switch's IsToggled drives the gif's IsAnimationPlaying.
            gif_caption_.set_text("Animating a gif");
            animation_switch_.set_is_toggled(true);
            animation_switch_.toggled.connect([this](bool is_on) { switch_gif_.set_is_animation_playing(is_on); });
            switch_gif_.set_source(maui::controls::image_source::from_file("animated_heart.gif"));
            switch_gif_.set_width_request(200);
            switch_gif_.set_is_animation_playing(true);

            // Stream source: a bytes-provider source (here empty bytes — see the note above).
            stream_caption_.set_text("Stream Source");
            stream_image_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::light_pink));
            stream_image_.set_source(maui::controls::image_source::from_stream(
                [](const maui::core::cancellation_token& /*token*/) { return maui::core::image_bytes{}; }));

            // Opacity (0.5) over black.
            opacity_caption_.set_text("Opacity (0.5)");
            opacity_image_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::black));
            opacity_image_.set_source(maui::controls::image_source::from_file("dotnet_bot.png"));
            opacity_image_.set_opacity(0.5);

            // Animated GIF + Start/Stop + Use-Online-Source.
            animated_caption_.set_text("Animated GIF");
            animated_gif_.set_source(maui::controls::image_source::from_file("animated_heart.gif"));
            animated_gif_.set_is_animation_playing(true);

            start_stop_button_.set_text("Start/Stop");
            start_stop_button_.clicked.connect(
                [this] { animated_gif_.set_is_animation_playing(!animated_gif_.is_animation_playing()); });

            use_online_button_.set_text("Use Online Source");
            use_online_button_.clicked.connect([this] {
                animated_gif_.set_source(maui::controls::image_source::from_uri(
                    "https://raw.githubusercontent.com/dotnet/maui/126f47aaf9d5c01224f54fe1c6bfb1c8299cc2fe/"
                    "src/Compatibility/ControlGallery/src/iOS/GifTwo.gif"));
            });

            stack_.add(uri_caption_);
            stack_.add(uri_image_);
            stack_.add(file_caption_);
            stack_.add(file_image_);
            stack_.add(font_caption_);
            stack_.add(font_image_);
            stack_.add(font_scaled_caption_);
            stack_.add(font_scaled_image_);
            stack_.add(gif_caption_);
            stack_.add(animation_switch_);
            stack_.add(switch_gif_);
            stack_.add(stream_caption_);
            stack_.add(stream_image_);
            stack_.add(opacity_caption_);
            stack_.add(opacity_image_);
            stack_.add(animated_caption_);
            stack_.add(animated_gif_);
            stack_.add(start_stop_button_);
            stack_.add(use_online_button_);
            scroll_.set_content(stack_); // the stack overflows the viewport — wrap it in a scroll_view
            page_.set_content(scroll_);
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
        [[nodiscard]] maui::controls::image& uri_image()
        {
            return uri_image_;
        }
        [[nodiscard]] maui::controls::image& switch_gif()
        {
            return switch_gif_;
        }
        [[nodiscard]] maui::controls::toggle_switch& animation_switch()
        {
            return animation_switch_;
        }
        [[nodiscard]] maui::controls::image& animated_gif()
        {
            return animated_gif_;
        }
        [[nodiscard]] maui::controls::button& start_stop_button()
        {
            return start_stop_button_;
        }
        [[nodiscard]] maui::controls::button& use_online_button()
        {
            return use_online_button_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label uri_caption_;
        maui::controls::image uri_image_;
        maui::controls::label file_caption_;
        maui::controls::image file_image_;
        maui::controls::label font_caption_;
        maui::controls::image font_image_;
        maui::controls::label font_scaled_caption_;
        maui::controls::image font_scaled_image_;
        maui::controls::label gif_caption_;
        maui::controls::toggle_switch animation_switch_;
        maui::controls::image switch_gif_;
        maui::controls::label stream_caption_;
        maui::controls::image stream_image_;
        maui::controls::label opacity_caption_;
        maui::controls::image opacity_image_;
        maui::controls::label animated_caption_;
        maui::controls::image animated_gif_;
        maui::controls::button start_stop_button_;
        maui::controls::button use_online_button_;
    };
} // namespace maui::samples

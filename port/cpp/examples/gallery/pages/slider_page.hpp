#pragma once
// maui::samples::slider_page — ports SliderPage.xaml (+ .xaml.cs)
//
// Mirrors the MAUI gallery page: a vertical stack of headlined Slider states — Default, BackgroundColor
// (Blue), Background (yellow→green LinearGradientBrush), Minimum(5)/Maximum(15) with a value readout
// (ValueChanged → Label, the x:Reference binding), Disabled, MinimumTrackColor (LightBlue),
// MaximumTrackColor (Pink), ThumbColor (Orange), ThumbImageSource (toggled by a button), a custom
// tri-color slider, a Dynamic slider whose Min/Max are mutated by two buttons (UpdateInfo readout), and a
// Min==Max==Value edge case.
//
// The .xaml.cs behavior is preserved: OnValueChanged/OnDynamicValueChanged drive the readouts;
// ToggleImageSource swaps the thumb image source in/out (remembering the removed one); the Update Minimum/
// Maximum buttons set Minimum=4 / Maximum=8 on the dynamic slider then refresh its info label.
//
// Self-contained (the value_controls_page pattern): the page OWNS its whole element tree, exposes page().
// Headless-safe — only cross-platform maui:: API here.

#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class slider_page
    {
    public:
        slider_page()
        {
            page_.set_title("Slider");
            stack_.set_spacing(6);

            // --- Default ---
            default_headline_.set_text("Default");

            // --- BackgroundColor (Blue) ---
            bg_color_headline_.set_text("BackgroundColor");
            bg_color_slider_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));

            // --- Background (LinearGradientBrush yellow@0.1 → green@1.0, EndPoint 1,0) ---
            background_headline_.set_text("Background");
            {
                std::vector<std::shared_ptr<maui::controls::gradient_stop>> stops;
                stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::yellow, 0.1F));
                stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::green, 1.0F));
                background_slider_.set_background_brush(std::make_shared<maui::controls::linear_gradient_brush>(
                    std::move(stops), maui::graphics::point{0, 0}, maui::graphics::point{1, 0}));
            }

            // --- Minimum (5) and Maximum (15) — ValueChanged drives the readout (the x:Reference label) ---
            min_max_headline_.set_text("Minimum (5) and Maximum (15)");
            min_max_slider_.set_maximum(15);
            min_max_slider_.set_minimum(5);
            min_max_slider_.set_value(10); // the canonical shared slider.xaml shows Value="10" (mid-range)
            min_max_slider_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { update_min_max_readout(new_value); });
            update_min_max_readout(min_max_slider_.value());

            // --- Disabled ---
            disabled_headline_.set_text("Disabled");
            disabled_slider_.set_is_enabled(false);

            // --- MinimumTrackColor (LightBlue) ---
            min_track_headline_.set_text("MinimumTrackColor=LightBlue");
            min_track_slider_.set_minimum_track_color(maui::graphics::colors::light_blue);

            // --- MaximumTrackColor (Pink) ---
            max_track_headline_.set_text("MaximumTrackColor=Pink");
            max_track_slider_.set_maximum_track_color(maui::graphics::colors::pink);

            // --- ThumbColor (Orange) ---
            thumb_color_headline_.set_text("ThumbColor=Orange");
            thumb_color_slider_.set_thumb_color(maui::graphics::colors::orange);

            // --- ThumbImageSource (toggled in/out by the button — SliderPage.ToggleImageSource) ---
            thumb_image_headline_.set_text("ThumbImageSource=thumb_image.png");
            image_slider_.set_thumb_image_source(maui::controls::image_source::from_file("thumb_image.png"));
            toggle_image_button_.set_text("Toggle Image");
            toggle_image_button_.command = [this] { toggle_image_source(); };

            // --- Custom (Red min track, Green max track, Blue thumb) ---
            custom_headline_.set_text(
                "Custom Slider (Red MinimumTrackColor, Green MaximumTrackColor, Blue ThumbColor)");
            custom_slider_.set_minimum_track_color(maui::graphics::colors::red);
            custom_slider_.set_maximum_track_color(maui::graphics::colors::green);
            custom_slider_.set_thumb_color(maui::graphics::colors::blue);

            // --- Dynamic (Min 0, Max 10, Value 5) — buttons mutate Min/Max, readout updates ---
            dynamic_headline_.set_text("Dynamically update Slider");
            dynamic_slider_.set_minimum(0);
            dynamic_slider_.set_maximum(10);
            dynamic_slider_.set_value(5);
            dynamic_slider_.value_changed.connect(
                [this](double /*old_value*/, double /*new_value*/) { update_dynamic_info(); });
            update_min_button_.set_text("Update Minimum");
            update_min_button_.command = [this] {
                dynamic_slider_.set_minimum(4); // SliderPage.OnUpdateMinimumButtonClicked
                update_dynamic_info();
            };
            update_max_button_.set_text("Update Maximum");
            update_max_button_.command = [this] {
                dynamic_slider_.set_maximum(8); // SliderPage.OnUpdateMaximumButtonClicked
                update_dynamic_info();
            };
            dynamic_buttons_.add(update_min_button_);
            dynamic_buttons_.add(update_max_button_);
            update_dynamic_info(); // the ctor's UpdateInfo() seed

            // --- Edge case (Min==Max==Value==100) ---
            edge_headline_.set_text("Edge case - Same Minimum, Maximum and Value");
            // Apply Max before Min (the larger bound first) so the intermediate range stays valid, matching
            // slider's set-order independence; Value then coerces into [100,100].
            edge_slider_.set_maximum(100);
            edge_slider_.set_minimum(100);
            edge_slider_.set_value(100);

            // Section headers render bold @18pt — mirrors maui-compare SliderPage.Headline().
            for (maui::controls::label* h :
                 {&default_headline_, &bg_color_headline_, &background_headline_, &min_max_headline_,
                  &disabled_headline_, &min_track_headline_, &max_track_headline_, &thumb_color_headline_,
                  &thumb_image_headline_, &custom_headline_, &dynamic_headline_, &edge_headline_})
            {
                h->set_font(maui::core::font::system_font_of_size(18.0, maui::core::font_weight::bold));
            }

            stack_.add(default_headline_);
            stack_.add(default_slider_);
            stack_.add(bg_color_headline_);
            stack_.add(bg_color_slider_);
            stack_.add(background_headline_);
            stack_.add(background_slider_);
            stack_.add(min_max_headline_);
            stack_.add(min_max_slider_);
            stack_.add(min_max_readout_);
            stack_.add(disabled_headline_);
            stack_.add(disabled_slider_);
            stack_.add(min_track_headline_);
            stack_.add(min_track_slider_);
            stack_.add(max_track_headline_);
            stack_.add(max_track_slider_);
            stack_.add(thumb_color_headline_);
            stack_.add(thumb_color_slider_);
            stack_.add(thumb_image_headline_);
            stack_.add(image_slider_);
            stack_.add(toggle_image_button_);
            stack_.add(custom_headline_);
            stack_.add(custom_slider_);
            stack_.add(dynamic_headline_);
            stack_.add(dynamic_slider_);
            stack_.add(dynamic_info_);
            stack_.add(dynamic_buttons_);
            stack_.add(edge_headline_);
            stack_.add(edge_slider_);

            // The MAUI page wraps the stack in a ScrollView (the content is taller than a screen).
            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

    private:
        // SliderPage.OnValueChanged readout (the x:Reference {Binding Path=Value} label).
        void update_min_max_readout(double value)
        {
            char text[32];
            std::snprintf(text, sizeof(text), "%g", value);
            min_max_readout_.set_text(text);
        }

        // SliderPage.UpdateInfo(): "Minimum: …, Maximum: …, Value: …" for the dynamic slider.
        void update_dynamic_info()
        {
            char text[96];
            std::snprintf(text, sizeof(text), "Minimum: %g, Maximum: %g, Value: %g", dynamic_slider_.minimum(),
                          dynamic_slider_.maximum(), dynamic_slider_.value());
            dynamic_info_.set_text(text);
        }

        // SliderPage.ToggleImageSource: pull the source out (remembering it) on first press, restore it next.
        void toggle_image_source()
        {
            if (!saved_thumb_image_)
            {
                saved_thumb_image_ = image_slider_.thumb_image_source_value();
                image_slider_.set_thumb_image_source(nullptr);
            }
            else
            {
                image_slider_.set_thumb_image_source(saved_thumb_image_);
                saved_thumb_image_ = nullptr;
            }
        }

        std::shared_ptr<maui::core::i_image_source> saved_thumb_image_; // SliderPage._imageSource

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_headline_;
        maui::controls::slider default_slider_;
        maui::controls::label bg_color_headline_;
        maui::controls::slider bg_color_slider_;
        maui::controls::label background_headline_;
        maui::controls::slider background_slider_;
        maui::controls::label min_max_headline_;
        maui::controls::slider min_max_slider_;
        maui::controls::label min_max_readout_;
        maui::controls::label disabled_headline_;
        maui::controls::slider disabled_slider_;
        maui::controls::label min_track_headline_;
        maui::controls::slider min_track_slider_;
        maui::controls::label max_track_headline_;
        maui::controls::slider max_track_slider_;
        maui::controls::label thumb_color_headline_;
        maui::controls::slider thumb_color_slider_;
        maui::controls::label thumb_image_headline_;
        maui::controls::slider image_slider_;
        maui::controls::button toggle_image_button_;
        maui::controls::label custom_headline_;
        maui::controls::slider custom_slider_;
        maui::controls::label dynamic_headline_;
        maui::controls::slider dynamic_slider_;
        maui::controls::label dynamic_info_;
        maui::controls::horizontal_stack_layout dynamic_buttons_;
        maui::controls::button update_min_button_;
        maui::controls::button update_max_button_;
        maui::controls::label edge_headline_;
        maui::controls::slider edge_slider_;
    };
} // namespace maui::samples

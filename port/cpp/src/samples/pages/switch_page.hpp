#pragma once
// maui::samples::switch_page — ports SwitchPage.xaml (+ .xaml.cs)
//
// Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor
// (Blue), Background (a yellow→green LinearGradientBrush), Disabled, OnColor (Red), ThumbColor (Orange).
// The port's control is maui::controls::toggle_switch (`switch` is a C++ keyword). The original .xaml.cs
// has no behavior beyond InitializeComponent, so this page adds a small live readout: toggling the first
// switch updates a label, exercising the IsToggled property + Toggled event end-to-end.
//
// Self-contained (the value_controls_page pattern): the page OWNS its whole element tree, exposes page()
// and attach_handlers(maui_app). Headless-safe — only cross-platform maui:: API here.

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

#include <memory>
#include <vector>

#include "gallery_attach.hpp"

namespace maui::samples
{
    class switch_page
    {
    public:
        switch_page()
        {
            page_.set_title("Switch");
            stack_.set_spacing(6);

            // --- Default (also drives the live readout below) ---
            default_headline_.set_text("Default");
            readout_.set_text("Default switch is Off");
            default_switch_.toggled.connect(
                [this](bool is_on) { readout_.set_text(is_on ? "Default switch is On" : "Default switch is Off"); });

            // --- BackgroundColor (Blue) ---
            bg_color_headline_.set_text("BackgroundColor");
            bg_color_switch_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));

            // --- Background (LinearGradientBrush yellow@0.1 → green@1.0, EndPoint 1,0) ---
            background_headline_.set_text("Background");
            {
                std::vector<std::shared_ptr<maui::controls::gradient_stop>> stops;
                stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::yellow, 0.1F));
                stops.push_back(std::make_shared<maui::controls::gradient_stop>(maui::graphics::colors::green, 1.0F));
                auto brush = std::make_shared<maui::controls::linear_gradient_brush>(
                    std::move(stops), maui::graphics::point{0, 0}, maui::graphics::point{1, 0});
                background_switch_.set_background_brush(std::move(brush));
            }

            // --- Disabled ---
            disabled_headline_.set_text("Disabled");
            disabled_switch_.set_is_enabled(false);

            // --- OnColor (Red) ---
            on_color_headline_.set_text("OnColor");
            on_color_switch_.set_on_color(maui::graphics::colors::red);

            // --- ThumbColor (Orange) ---
            thumb_color_headline_.set_text("ThumbColor");
            thumb_color_switch_.set_thumb_color(maui::graphics::colors::orange);

            // Section headers render bold @18pt — mirrors maui-compare SwitchPage.Headline().
            for (maui::controls::label* h : {&default_headline_, &bg_color_headline_, &background_headline_,
                                             &disabled_headline_, &on_color_headline_, &thumb_color_headline_})
            {
                h->set_font(maui::core::font::system_font_of_size(18.0, maui::core::font_weight::bold));
            }

            stack_.add(default_headline_);
            stack_.add(default_switch_);
            stack_.add(readout_);
            stack_.add(bg_color_headline_);
            stack_.add(bg_color_switch_);
            stack_.add(background_headline_);
            stack_.add(background_switch_);
            stack_.add(disabled_headline_);
            stack_.add(disabled_switch_);
            stack_.add(on_color_headline_);
            stack_.add(on_color_switch_);
            stack_.add(thumb_color_headline_);
            stack_.add(thumb_color_switch_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view BOTTOM-UP (leaves → layout → page), then replay the host
        // commands the ctor fired before any handler existed (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, default_headline_, "default_headline_");
            gallery_attach_one(app, default_switch_, "default_switch_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, bg_color_headline_, "bg_color_headline_");
            gallery_attach_one(app, bg_color_switch_, "bg_color_switch_");
            gallery_attach_one(app, background_headline_, "background_headline_");
            gallery_attach_one(app, background_switch_, "background_switch_");
            gallery_attach_one(app, disabled_headline_, "disabled_headline_");
            gallery_attach_one(app, disabled_switch_, "disabled_switch_");
            gallery_attach_one(app, on_color_headline_, "on_color_headline_");
            gallery_attach_one(app, on_color_switch_, "on_color_switch_");
            gallery_attach_one(app, thumb_color_headline_, "thumb_color_headline_");
            gallery_attach_one(app, thumb_color_switch_, "thumb_color_switch_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_headline_;
        maui::controls::toggle_switch default_switch_;
        maui::controls::label readout_;
        maui::controls::label bg_color_headline_;
        maui::controls::toggle_switch bg_color_switch_;
        maui::controls::label background_headline_;
        maui::controls::toggle_switch background_switch_;
        maui::controls::label disabled_headline_;
        maui::controls::toggle_switch disabled_switch_;
        maui::controls::label on_color_headline_;
        maui::controls::toggle_switch on_color_switch_;
        maui::controls::label thumb_color_headline_;
        maui::controls::toggle_switch thumb_color_switch_;
    };
} // namespace maui::samples

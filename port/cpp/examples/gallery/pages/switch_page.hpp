#pragma once
// maui::samples::switch_page — ports SwitchPage.xaml (+ .xaml.cs)
//
// Mirrors the MAUI gallery page: a vertical stack of headlined Switch states — Default, BackgroundColor
// (Blue), Background (plain — see the note at the section), Disabled, OnColor (Red), ThumbColor (Orange).
// The port's control is maui::controls::toggle_switch (`switch` is a C++ keyword). The original .xaml.cs
// has no behavior beyond InitializeComponent, so this page adds a small live readout: toggling the first
// switch updates a label, exercising the IsToggled property + Toggled event end-to-end.
//
// Self-contained (the value_controls_page pattern): the page OWNS its whole element tree, exposes page().
// Headless-safe — only cross-platform maui:: API here.

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

#include <memory>
#include <vector>

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

            // --- Background — a plain switch, matching the canonical shared switch.xaml (and MAUI's
            // actual Catalyst render, which paints no gradient for a Switch Background) ---
            background_headline_.set_text("Background");

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

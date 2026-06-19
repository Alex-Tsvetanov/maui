#pragma once
// maui::samples::stepper_page — ports StepperPage.xaml (+ StepperPage.xaml.cs).
//
// A self-contained, code-first demo page for maui::controls::stepper. Mirrors the EXACT shape of
// value_controls_page.hpp / button_page.hpp: the class OWNS its whole element tree as members,
// exposes page() and attach_handlers(maui_app) (bottom-up, leaves first -> layout -> page, via the
// shared gallery_attach helpers), and uses only cross-platform maui:: API so it stays headless-safe.
//
// What the MAUI page demonstrates (reproduced here so the demo visibly exercises the control):
//   - a Default stepper (no bounds set),
//   - a Disabled stepper (IsEnabled=false) plus an Enable/Disable button that toggles its IsEnabled
//     and swaps its own text (OnEnableButtonClicked),
//   - a BackgroundColor=Red stepper (here a solid_color_brush, as the button page does for
//     BackgroundColor),
//   - a Minimum=5 / Maximum=25 stepper,
//   - an Increment=2 stepper over [0,100],
//   - a ValueChanged stepper over [0,100] whose value drives a readout label (OnValueChanged +
//     the XAML {x:Reference}.Value binding folded into one live label).
//
// note: StepperPage's Background LinearGradientBrush stepper is rendered here as a plain stepper with a
// "// note:" — the gallery keeps to mapped solid-color styling and does not invent a gradient. The
// Headline label styling (Style="{StaticResource Headline}") has no resource-dictionary surface in the
// port, so the section headers are plain labels.

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class stepper_page
    {
    public:
        stepper_page()
        {
            page_.set_title("Stepper");
            stack_.set_spacing(6);
            stack_.set_padding(maui::core::thickness(12));

            // ---- Default ----
            default_header_.set_text("Default");

            // ---- Disabled (IsEnabled=false) + Enable/Disable button (OnEnableButtonClicked) ----
            disabled_header_.set_text("Disabled");
            enable_stepper_.set_is_enabled(false);
            enable_button_.set_text("Enable Stepper");
            enable_button_.clicked.connect([this] { toggle_enable(); });

            // ---- BackgroundColor=Red (solid_color_brush, mirroring the button page) ----
            background_color_header_.set_text("BackgroundColor");
            background_color_stepper_.set_background_brush(
                std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::red));

            // ---- Background (LinearGradientBrush in XAML) ----
            // note: the XAML sets a Yellow->Green LinearGradientBrush here; the port's gallery keeps to
            // mapped solid-color styling and does not invent a gradient, so this stepper is left plain.
            background_header_.set_text("Background");

            // ---- Minimum (5) and Maximum (25) ----
            // The XAML applies Maximum then Minimum implicitly; the stepper coerces Value into the range.
            min_max_header_.set_text("Minimum (5) and Maximum (25)");
            min_max_stepper_.set_maximum(25);
            min_max_stepper_.set_minimum(5);

            // ---- Increment (2) over [0,100] ----
            increment_header_.set_text("Increment (2)");
            increment_stepper_.set_minimum(0);
            increment_stepper_.set_maximum(100);
            increment_stepper_.set_increment(2);

            // ---- ValueChanged over [0,100], Increment=1; value drives the readout label ----
            value_changed_header_.set_text("ValueChanged");
            value_changed_stepper_.set_minimum(0);
            value_changed_stepper_.set_maximum(100);
            value_changed_stepper_.set_increment(1);
            value_changed_stepper_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { update_readout(new_value); });
            update_readout(value_changed_stepper_.value());

            stack_.add(default_header_);
            stack_.add(default_stepper_);
            stack_.add(disabled_header_);
            stack_.add(enable_stepper_);
            stack_.add(enable_button_);
            stack_.add(background_color_header_);
            stack_.add(background_color_stepper_);
            stack_.add(background_header_);
            stack_.add(background_stepper_);
            stack_.add(min_max_header_);
            stack_.add(min_max_stepper_);
            stack_.add(increment_header_);
            stack_.add(increment_stepper_);
            stack_.add(value_changed_header_);
            stack_.add(value_changed_stepper_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, default_header_, "default_header_");
            gallery_attach_one(app, default_stepper_, "default_stepper_");
            gallery_attach_one(app, disabled_header_, "disabled_header_");
            gallery_attach_one(app, enable_stepper_, "enable_stepper_");
            gallery_attach_one(app, enable_button_, "enable_button_");
            gallery_attach_one(app, background_color_header_, "background_color_header_");
            gallery_attach_one(app, background_color_stepper_, "background_color_stepper_");
            gallery_attach_one(app, background_header_, "background_header_");
            gallery_attach_one(app, background_stepper_, "background_stepper_");
            gallery_attach_one(app, min_max_header_, "min_max_header_");
            gallery_attach_one(app, min_max_stepper_, "min_max_stepper_");
            gallery_attach_one(app, increment_header_, "increment_header_");
            gallery_attach_one(app, increment_stepper_, "increment_stepper_");
            gallery_attach_one(app, value_changed_header_, "value_changed_header_");
            gallery_attach_one(app, value_changed_stepper_, "value_changed_stepper_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
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
        [[nodiscard]] maui::controls::stepper& enable_stepper()
        {
            return enable_stepper_;
        }
        [[nodiscard]] maui::controls::button& enable_button()
        {
            return enable_button_;
        }
        [[nodiscard]] maui::controls::stepper& value_changed_stepper()
        {
            return value_changed_stepper_;
        }

    private:
        // OnEnableButtonClicked: flip the disabled stepper's IsEnabled and swap the button text.
        void toggle_enable()
        {
            if (enable_stepper_.is_enabled())
            {
                enable_stepper_.set_is_enabled(false);
                enable_button_.set_text("Enable Stepper");
            }
            else
            {
                enable_stepper_.set_is_enabled(true);
                enable_button_.set_text("Disable Stepper");
            }
        }

        // OnValueChanged + the {x:Reference ValueChangedStepper}.Value binding, folded into one readout.
        void update_readout(double value)
        {
            char text[32];
            std::snprintf(text, sizeof(text), "Value: %.0f", value);
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label default_header_;
        maui::controls::stepper default_stepper_;
        maui::controls::label disabled_header_;
        maui::controls::stepper enable_stepper_;
        maui::controls::button enable_button_;
        maui::controls::label background_color_header_;
        maui::controls::stepper background_color_stepper_;
        maui::controls::label background_header_;
        maui::controls::stepper background_stepper_;
        maui::controls::label min_max_header_;
        maui::controls::stepper min_max_stepper_;
        maui::controls::label increment_header_;
        maui::controls::stepper increment_stepper_;
        maui::controls::label value_changed_header_;
        maui::controls::stepper value_changed_stepper_;
        maui::controls::label readout_;
    };
} // namespace maui::samples

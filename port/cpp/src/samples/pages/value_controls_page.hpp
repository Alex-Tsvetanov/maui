#pragma once
// value_controls_page — a self-contained demo page for the W1-04 value-control set: toggle_switch,
// check_box, slider, stepper, progress_bar and activity_indicator on one vertical stack, wired
// together so every input drives a visible output (the C# gallery-page convention, code-first).
//
// The page OWNS its whole element tree (the sample_app pattern in maui_app_sample.mm). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts
// page() in a window; the headless/apple/ios test trees exercise the same controls directly.
//
// Interactions demonstrated:
//   - the slider's value drives the progress bar fill and the label readout (value_changed),
//   - the stepper steps the slider value (the two-way IRange seam),
//   - the toggle_switch starts/stops the activity_indicator (toggled → set_is_running),
//   - the check_box recolors the slider thumb while checked (checked_changed → thumb color).

#include <cstdio>
#include <string>

#include "maui/controls/activity_indicator.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class value_controls_page
    {
    public:
        value_controls_page()
        {
            page_.set_title("Value controls");
            stack_.set_spacing(12);

            readout_.set_text("Value: 25 / 100");

            // slider — the range input; its value drives the progress bar + readout.
            slider_.set_minimum(0);
            slider_.set_maximum(100);
            slider_.set_value(25);
            slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                progress_.set_progress(new_value / 100.0);
                update_readout(new_value);
            });

            // stepper — steps the same value in increments of 5 (two controls over one range).
            stepper_.set_minimum(0);
            stepper_.set_maximum(100);
            stepper_.set_increment(5);
            stepper_.set_value(25);
            stepper_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { slider_.set_value(new_value); });

            // progress_bar — the output gauge for the slider value.
            progress_.set_progress(0.25);

            // toggle_switch — starts/stops the busy spinner.
            busy_switch_.toggled.connect([this](bool is_on) { spinner_.set_is_running(is_on); });

            // check_box — recolors the slider thumb while checked.
            accent_check_.checked_changed.connect([this](bool checked) {
                slider_.set_thumb_color(checked ? maui::graphics::color(0.86F, 0.20F, 0.27F) : maui::graphics::color{});
            });

            stack_.add(readout_);
            stack_.add(slider_);
            stack_.add(stepper_);
            stack_.add(progress_);
            stack_.add(busy_switch_);
            stack_.add(accent_check_);
            stack_.add(spinner_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last) so each parent can
        // host its child's native view, then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, slider_, "slider_");
            gallery_attach_one(app, stepper_, "stepper_");
            gallery_attach_one(app, progress_, "progress_");
            gallery_attach_one(app, busy_switch_, "busy_switch_");
            gallery_attach_one(app, accent_check_, "accent_check_");
            gallery_attach_one(app, spinner_, "spinner_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::slider& value_slider()
        {
            return slider_;
        }
        [[nodiscard]] maui::controls::stepper& value_stepper()
        {
            return stepper_;
        }
        [[nodiscard]] maui::controls::progress_bar& progress()
        {
            return progress_;
        }
        [[nodiscard]] maui::controls::toggle_switch& busy_switch()
        {
            return busy_switch_;
        }
        [[nodiscard]] maui::controls::check_box& accent_check()
        {
            return accent_check_;
        }
        [[nodiscard]] maui::controls::activity_indicator& spinner()
        {
            return spinner_;
        }

    private:
        void update_readout(double value)
        {
            char text[48];
            std::snprintf(text, sizeof(text), "Value: %.0f / 100", value);
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::slider slider_;
        maui::controls::stepper stepper_;
        maui::controls::progress_bar progress_;
        maui::controls::toggle_switch busy_switch_;
        maui::controls::check_box accent_check_;
        maui::controls::activity_indicator spinner_;
    };
} // namespace maui::samples

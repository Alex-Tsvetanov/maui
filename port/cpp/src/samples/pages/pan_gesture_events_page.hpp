#pragma once
// maui::samples::pan_gesture_events_page — ports PanGestureEventsGallery.xaml (+ .xaml.cs)
//                                          (Maui.Controls.Sample.Pages.PanGestureEventsGallery).
//
// The C# page is a two-row Grid (RowDefinitions="*,*"): the top row is a GREEN Grid carrying a single
// PanGestureRecognizer (PanUpdated="OnPanGestureRecognizerUpdated") over a white InfoLabel; the bottom
// row is a plain RED Grid (a passive backdrop, present so the green pan target only fills the top half).
// The single handler writes the live pan state into the label:
//   `InfoLabel.Text = $"StatusType: {e.StatusType}, TotalX: {e.TotalX}, TotalY: {e.TotalY}"`
// i.e. it surfaces the PanUpdatedEventArgs status-machine (Started -> Running -> Completed) and the
// cumulative TotalX/TotalY translation as the gesture runs.
//
// This code-first port reproduces that shape:
//   - a green pan target Grid (target_) with a white readout label (info_label_) as its only child;
//   - a real maui::controls::pan_gesture_recognizer added to the target's GestureRecognizers collection
//     (the View seam — grid is a layout, hence a view, so it carries recognizers);
//   - a red backdrop Grid (backdrop_) for the bottom row, matching the oracle's two-row split;
//   - the PanUpdated handler formats the same "StatusType: <s>, TotalX: <x>, TotalY: <y>" readout off
//     the pan_updated_event_args (status_type / total_x / total_y).
//
// On a device the platform pan-gesture manager drives the recognizer through its
// i_pan_gesture_controller seam. The headless backend has NO native input, so the page's mount hook
// finishes with one deterministic synthetic pan drive through that same seam — Started -> Running(dx,dy)
// -> Completed, stamped with one gesture id minted from PanGestureRecognizer::current_id() (exactly how
// the gesture unit tests and the platform bridges drive it) — leaving the readout showing the final
// "StatusType: Completed, …" line in a static capture, with the running line's TotalX/TotalY having
// already exercised the cumulative-translation path.
//
// The page OWNS its whole element tree (the gestures_page / pointer_gesture_page pattern): public
// page().

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class pan_gesture_events_page
    {
    public:
        pan_gesture_events_page()
        {
            page_.set_title("PanGesture Events Gallery"); // Title="PanGesture Events Gallery"

            // ---- the outer two-row Grid (RowDefinitions="*,*") ----
            outer_.add_row_definition(maui::core::grid_length::star());
            outer_.add_row_definition(maui::core::grid_length::star());

            // ---- top row: the GREEN pan target Grid carrying the recognizer + white readout ----
            target_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green));

            info_label_.set_text(""); // InfoLabel starts blank (TextColor="White")
            info_label_.set_text_color(maui::graphics::colors::white);
            target_.add(info_label_);

            // The PanGestureRecognizer (PanUpdated="OnPanGestureRecognizerUpdated").
            pan_->pan_updated.connect([this](const maui::controls::pan_updated_event_args& e) { on_pan_updated(e); });
            target_.gesture_recognizers().add(pan_);

            // ---- bottom row: the plain RED backdrop Grid ----
            backdrop_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));

            outer_.set_row(target_, 0);
            outer_.add(target_);
            outer_.set_row(backdrop_, 1);
            outer_.add(backdrop_);
            page_.set_content(outer_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Headless has no native input, so drive one deterministic synthetic
        // pan so the readout reflects the wiring in a static capture. All per-control attach + re-host plumbing
        // is now the generic mount's job.
        void on_mounted(maui::hosting::maui_app& /*app*/)
        {
            drive_synthetic_pan();
        }

        // The owned views + recognizer, exposed for the hosting main / headless tests.
        [[nodiscard]] maui::controls::grid& target()
        {
            return target_;
        }
        [[nodiscard]] maui::controls::label& info_label()
        {
            return info_label_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pan_gesture_recognizer>& pan()
        {
            return pan_;
        }

        // One deterministic pan through the i_pan_gesture_controller seam (the same path the platform
        // bridge + the pan unit tests use): Started -> Running(12,-8) -> Completed, all stamped with one
        // gesture id. Leaves info_label_ on the final "StatusType: Completed, …" line, after the running
        // step has exercised the TotalX/TotalY readout.
        void drive_synthetic_pan()
        {
            const int pan_id = maui::controls::pan_gesture_recognizer::current_id().increment();
            pan_->send_pan_started(target_, pan_id);
            pan_->send_pan(target_, 12, -8, pan_id);
            pan_->send_pan_completed(target_, pan_id);
        }

    private:
        // OnPanGestureRecognizerUpdated: format the C# readout off the event args
        // ("StatusType: <s>, TotalX: <x>, TotalY: <y>").
        void on_pan_updated(const maui::controls::pan_updated_event_args& e)
        {
            char text[96];
            (void)std::snprintf(text, sizeof(text), "StatusType: %s, TotalX: %.0f, TotalY: %.0f",
                                status_name(e.status_type), e.total_x, e.total_y);
            info_label_.set_text(text);
        }

        // The GestureStatus.ToString() spelling the C# string interpolation prints.
        static const char* status_name(maui::core::gesture_status status)
        {
            switch (status)
            {
                case maui::core::gesture_status::started:
                    return "Started";
                case maui::core::gesture_status::running:
                    return "Running";
                case maui::core::gesture_status::completed:
                    return "Completed";
                case maui::core::gesture_status::canceled:
                    return "Canceled";
            }
            return "Started";
        }

        maui::controls::content_page page_;
        maui::controls::grid outer_; // the two-row "*,*" grid

        // top row: the green pan target + its white readout
        maui::controls::grid target_;
        maui::controls::label info_label_;

        // bottom row: the red backdrop
        maui::controls::grid backdrop_;

        // the recognizer (the collection co-owns it via shared_ptr; we keep a strong ref for the
        // synthetic drive + the test accessor)
        std::shared_ptr<maui::controls::pan_gesture_recognizer> pan_ =
            std::make_shared<maui::controls::pan_gesture_recognizer>();
    };
} // namespace maui::samples

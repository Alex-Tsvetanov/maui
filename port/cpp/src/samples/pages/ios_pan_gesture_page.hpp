#pragma once
// maui::samples::ios_pan_gesture_page — ports iOSPanGestureRecognizerPage.xaml (+ .xaml.cs)
//
// The C# iOSPanGestureRecognizerPage is a StackLayout with: a bold message Label (_messageLabel), a
// "Toggle Simultaneous Gesture Recognition" Button, and a grouped ListView of employees whose per-row age
// Label carries a PanGestureRecognizer. OnPanUpdated sets _messageLabel.Text = $"panned x:{TotalX}
// y:{TotalY}"; OnButtonClicked flips the iOSSpecific Application knob
// PanGestureRecognizerShouldRecognizeSimultaneously (Application.Current.On<iOS>().Set/Get...).
//
// The demonstrated pieces, both HEADLESS-SAFE:
//   1. A PanGestureRecognizer attached to a target Label's GestureRecognizers (the View seam). Its
//      pan_updated drives the readout to "panned x:<TotalX> y:<TotalY>" while RUNNING (exactly the C#
//      OnPanUpdated format), and shows the started/completed phases too. Headless has no native input, so
//      attach_handlers() issues one deterministic synthetic drive (started -> running(TotalX,TotalY) ->
//      completed) through the recognizer's i_pan_gesture_controller seam (send_pan_started / send_pan /
//      send_pan_completed) — the same path the platform bridges and the pan unit tests use — leaving the
//      readout non-blank on a static capture.
//   2. The iOSSpecific Application knob PanGestureRecognizerShouldRecognizeSimultaneously: the toggle
//      button flips it through application.on<ios>() (ios_specific::application::set_/get_...), exactly the
//      C# OnButtonClicked, and the readout reflects the new value. The page OWNS a controls::application so
//      the knob is exercised self-contained (the C# Application.Current global has no headless analogue;
//      the knob is a STORED platform-spec, observably identical — see ios_specific/application.hpp).
//
// note: the grouped ListView of employees (ItemsSource / DataTemplate / per-row age Label) is a
//       data-binding + ListView-grouping concern that is out of this code-first page's scope; the
//       demonstrated behavior (a PanGestureRecognizer reporting Total X/Y into the message Label) is
//       layout-independent, so this port attaches the recognizer to a single target Label rather than to a
//       templated row. The recognizer wiring + the iOSSpecific knob — the actual subject of the page — are
//       reproduced faithfully.
//
// Self-contained: the page OWNS its whole element tree, exposes page() and attach_handlers(maui_app).

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/thickness.hpp"
#include "maui/hosting/maui_app.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/application.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ios_pan_gesture_page
    {
    public:
        ios_pan_gesture_page()
        {
            page_.set_title("Pan Gesture Recognizer");
            stack_.set_margin(maui::core::thickness(20)); // XAML StackLayout Margin="20"
            stack_.set_spacing(12);

            // C# _messageLabel (FontAttributes="Bold") — the readout the pan + toggle both write into.
            message_.set_text("Pan the target. If you pan it, this Label will change.");

            // C# "Toggle Simultaneous Gesture Recognition" button — flips the iOSSpecific Application knob.
            toggle_.set_text("Toggle Simultaneous Gesture Recognition");
            toggle_.clicked.connect([this] { on_toggle_clicked(); });

            // The pan target (stands in for the C# per-row age Label that carries the recognizer).
            target_.set_text("Pan target");

            // ---- PanGestureRecognizer.PanUpdated → "panned x:<TotalX> y:<TotalY>" (the C# OnPanUpdated) ----
            pan_->pan_updated.connect([this](const maui::controls::pan_updated_event_args& e) {
                switch (e.status_type)
                {
                    case maui::core::gesture_status::started:
                        message_.set_text("Pan started");
                        break;
                    case maui::core::gesture_status::running: {
                        // EXACTLY the C# format: $"panned x:{e.TotalX} y:{e.TotalY}".
                        char text[80];
                        (void)std::snprintf(text, sizeof(text), "panned x:%g y:%g", e.total_x, e.total_y);
                        message_.set_text(text);
                        break;
                    }
                    case maui::core::gesture_status::completed:
                        // Leave the last "panned x:.. y:.." text in place; note completion in the status line.
                        refresh_status();
                        break;
                    case maui::core::gesture_status::canceled:
                        refresh_status();
                        break;
                }
            });

            // Attach the recognizer to the target's GestureRecognizers collection (the View seam) — exactly
            // the XAML <Label.GestureRecognizers><PanGestureRecognizer PanUpdated="OnPanUpdated"/>.
            target_.gesture_recognizers().add(pan_);

            refresh_status();

            stack_.add(message_);
            stack_.add(toggle_);
            stack_.add(target_);
            stack_.add(status_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view BOTTOM-UP, re-host the ctor-built tree, then — since headless
        // has no native input — issue one deterministic synthetic pan drive so the readout reflects the
        // wiring on a static capture (the recognizer needs no handler of its own; it rides the target view).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, message_, "message_");
            gallery_attach_one(app, toggle_, "toggle_");
            gallery_attach_one(app, target_, "target_");
            gallery_attach_one(app, status_, "status_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);

            drive_synthetic_pan();
        }

        // One deterministic pan through the i_pan_gesture_controller seam: started -> running(45,-12) ->
        // completed, all under one minted gesture id (the path the platform bridges + the pan unit tests
        // use). Leaves the readout on "panned x:45 y:-12".
        void drive_synthetic_pan()
        {
            const int id = maui::controls::pan_gesture_recognizer::current_id().increment();
            pan_->send_pan_started(target_, id);
            pan_->send_pan(target_, 45, -12, id);
            pan_->send_pan_completed(target_, id);
        }

        // ---- owned controls + recognizer, exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& message()
        {
            return message_;
        }
        [[nodiscard]] maui::controls::label& target()
        {
            return target_;
        }
        [[nodiscard]] maui::controls::button& toggle()
        {
            return toggle_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pan_gesture_recognizer>& pan()
        {
            return pan_;
        }

        // The iOSSpecific Application knob readback (the toggle target).
        [[nodiscard]] bool simultaneous_recognition() const
        {
            namespace ios_app = maui::controls::platform_configuration::ios_specific::application;
            return ios_app::get_pan_gesture_recognizer_should_recognize_simultaneously(app_);
        }

    private:
        // C# OnButtonClicked: Set(!Get()) on Application.Current.On<iOS>() — flip the simultaneous knob.
        void on_toggle_clicked()
        {
            namespace pc = maui::controls::platform_configuration;
            namespace ios_app = pc::ios_specific::application;
            const bool current =
                ios_app::get_pan_gesture_recognizer_should_recognize_simultaneously(app_.on<pc::ios>());
            ios_app::set_pan_gesture_recognizer_should_recognize_simultaneously(app_.on<pc::ios>(), !current);
            refresh_status();
        }

        void refresh_status()
        {
            namespace ios_app = maui::controls::platform_configuration::ios_specific::application;
            std::string text = "SimultaneousRecognition: ";
            text += ios_app::get_pan_gesture_recognizer_should_recognize_simultaneously(app_) ? "true" : "false";
            status_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label message_; // C# _messageLabel
        maui::controls::button toggle_;
        maui::controls::label target_; // carries the PanGestureRecognizer (stands in for the row age Label)
        maui::controls::label status_;

        // The page-owned application the iOSSpecific simultaneous-recognition knob is exercised on (stands
        // in for the C# Application.Current global — the knob is a stored platform-spec, observably identical).
        maui::controls::application app_;

        // The recognizer (the collection owns it via shared_ptr; we keep a strong ref for the synthetic
        // drive + the test accessor).
        std::shared_ptr<maui::controls::pan_gesture_recognizer> pan_ =
            std::make_shared<maui::controls::pan_gesture_recognizer>();
    };
} // namespace maui::samples

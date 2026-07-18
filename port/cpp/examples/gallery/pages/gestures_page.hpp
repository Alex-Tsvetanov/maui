#pragma once
// maui::samples::gestures_page — ports GesturesPage.xaml (+ .xaml.cs)
//
// The MAUI GesturesPage.xaml is a *gallery navigation* page: a CollectionView listing gesture-demo
// sections that the shell navigates into. That list-of-links structure is a Shell/navigation concern
// with no headless analog, so this code-first port instead builds the demo those sections lead to:
// a single target view carrying the full gesture-recognizer family, each recognizer wired to a
// readout label so the wiring is observable on one headless static-capture page.
//
// The recognizers are real maui::controls::*_gesture_recognizer instances added to the target's
// GestureRecognizers collection (View.gesture_recognizers().add). On a device the platform gesture
// manager drives them; headless, there is no native input, so the page's mount hook also issues one
// deterministic synthetic drive per recognizer through each one's Send* / i_*_gesture_controller
// seam (exactly how the gesture unit tests exercise them) — leaving the readout showing the last
// recognized gesture so the static capture is non-blank.
//
// Demonstrated (the recognizers exist; events drive the readout):
//   - TapGestureRecognizer.Tapped (NumberOfTapsRequired=1) -> "Tapped".
//   - PanGestureRecognizer.PanUpdated (the started/running/completed state machine) -> "Pan dx,dy".
//   - PinchGestureRecognizer.PinchUpdated (relative scale) -> "Pinch xN.NN".
//   - SwipeGestureRecognizer.Swiped (Direction Left|Right|Up|Down, Threshold) -> "Swiped <dir>".
//   - PointerGestureRecognizer.PointerEntered/Moved/Exited/Pressed/Released -> "Pointer <phase>".
//
// Self-contained (the value_controls_page / shapes_page pattern): the page OWNS its whole element
// tree, exposes page().

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class gestures_page
    {
    public:
        gestures_page()
        {
            page_.set_title("Gestures");
            stack_.set_spacing(12);
            stack_.set_padding(maui::core::thickness(12)); // shared XAML root <VerticalStackLayout Padding="12">

            instructions_.set_text("Gesture target (tap / pan / pinch / swipe / pointer)");
            readout_.set_text("Last gesture: (none)");

            // The target view every recognizer attaches to.
            target_.set_color(maui::graphics::colors::cornflower_blue);
            target_.set_corner_radius(maui::graphics::corner_radius(8));
            target_.set_height_request(160);

            // ---- TapGestureRecognizer (Tapped) ----
            tap_->set_number_of_taps_required(1);
            tap_->tapped.connect([this](const maui::controls::tapped_event_args&) { set_readout("Tapped"); });

            // ---- PanGestureRecognizer (PanUpdated state machine) ----
            pan_->pan_updated.connect([this](const maui::controls::pan_updated_event_args& e) {
                switch (e.status_type)
                {
                    case maui::core::gesture_status::started:
                        set_readout("Pan started");
                        break;
                    case maui::core::gesture_status::running: {
                        char text[64];
                        (void)std::snprintf(text, sizeof(text), "Pan %.0f,%.0f", e.total_x, e.total_y);
                        set_readout(text);
                        break;
                    }
                    case maui::core::gesture_status::completed:
                        set_readout("Pan completed");
                        break;
                    case maui::core::gesture_status::canceled:
                        set_readout("Pan canceled");
                        break;
                }
            });

            // ---- PinchGestureRecognizer (PinchUpdated, relative scale) ----
            pinch_->pinch_updated.connect([this](const maui::controls::pinch_gesture_updated_event_args& e) {
                if (e.status == maui::core::gesture_status::running)
                {
                    char text[64];
                    (void)std::snprintf(text, sizeof(text), "Pinch x%.2f", e.scale);
                    set_readout(text);
                }
            });

            // ---- SwipeGestureRecognizer (Swiped) — recognize all four directions. ----
            swipe_->set_direction(maui::core::swipe_direction::left | maui::core::swipe_direction::right |
                                  maui::core::swipe_direction::up | maui::core::swipe_direction::down);
            swipe_->swiped.connect([this](const maui::controls::swiped_event_args& e) {
                std::string text = "Swiped ";
                text += swipe_direction_name(e.direction);
                set_readout(text.c_str());
            });

            // ---- PointerGestureRecognizer (Entered / Moved / Exited / Pressed / Released) ----
            pointer_->pointer_entered.connect(
                [this](const maui::controls::pointer_event_args&) { set_readout("Pointer entered"); });
            pointer_->pointer_moved.connect(
                [this](const maui::controls::pointer_event_args&) { set_readout("Pointer moved"); });
            pointer_->pointer_pressed.connect(
                [this](const maui::controls::pointer_event_args&) { set_readout("Pointer pressed"); });
            pointer_->pointer_released.connect(
                [this](const maui::controls::pointer_event_args&) { set_readout("Pointer released"); });
            pointer_->pointer_exited.connect(
                [this](const maui::controls::pointer_event_args&) { set_readout("Pointer exited"); });

            // Add every recognizer to the target's GestureRecognizers collection (the View seam). Only ONE
            // pinch is allowed per view (View.ValidateGesture) — we add exactly one of each.
            target_.gesture_recognizers().add(tap_);
            target_.gesture_recognizers().add(pan_);
            target_.gesture_recognizers().add(pinch_);
            target_.gesture_recognizers().add(swipe_);
            target_.gesture_recognizers().add(pointer_);

            stack_.add(instructions_);
            stack_.add(target_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // No post-mount synthetic drive: the shared gestures.xaml is captured at REST, with the readout at its
        // static "Last gesture: (none)" text (the gesture recognizers' wiring is exercised by the gesture unit
        // tests, and drive_synthetic_gestures() below remains callable for interactive/manual use). Driving a
        // gesture at mount overwrote that resting readout (e.g. "Pointer exited"), diverging from MAUI.

        // The owned controls + recognizers, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::box_view& target()
        {
            return target_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::tap_gesture_recognizer>& tap()
        {
            return tap_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pan_gesture_recognizer>& pan()
        {
            return pan_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pinch_gesture_recognizer>& pinch()
        {
            return pinch_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::swipe_gesture_recognizer>& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pointer_gesture_recognizer>& pointer()
        {
            return pointer_;
        }

        // One deterministic drive per recognizer through its Send* / controller seam (the same path the
        // platform bridges and the gesture unit tests use). Leaves the readout on the last gesture.
        void drive_synthetic_gestures()
        {
            // Tap.
            tap_->send_tapped(target_, maui::graphics::point{80, 80});

            // Pan: started -> running(12,-8) -> completed (one gesture id).
            const int pan_id = maui::controls::pan_gesture_recognizer::current_id().increment();
            pan_->send_pan_started(target_, pan_id);
            pan_->send_pan(target_, 12, -8, pan_id);
            pan_->send_pan_completed(target_, pan_id);

            // Pinch: started -> running(scale 1.25) -> ended.
            pinch_->send_pinch_started(target_, maui::graphics::point{0.5, 0.5});
            pinch_->send_pinch(target_, 1.25, maui::graphics::point{0.5, 0.5});
            pinch_->send_pinch_ended(target_);

            // Swipe: feed a rightward pan past the default 100px threshold, then detect.
            swipe_->send_swipe(target_, 160, 0);
            (void)swipe_->detect_swipe(target_, maui::core::swipe_direction::right);

            // Pointer: entered -> moved -> pressed -> released -> exited.
            pointer_->send_pointer_entered(target_, maui::graphics::point{20, 20});
            pointer_->send_pointer_moved(target_, maui::graphics::point{40, 40});
            pointer_->send_pointer_pressed(target_, maui::graphics::point{40, 40});
            pointer_->send_pointer_released(target_, maui::graphics::point{40, 40});
            pointer_->send_pointer_exited(target_, maui::graphics::point{0, 0});
        }

    private:
        void set_readout(const char* gesture)
        {
            std::string text = "Last gesture: ";
            text += gesture;
            readout_.set_text(text);
        }

        // The combined-flag direction name for the swiped readout.
        static const char* swipe_direction_name(maui::core::swipe_direction direction)
        {
            using maui::core::swipe_direction;
            if (direction == swipe_direction::left)
            {
                return "Left";
            }
            if (direction == swipe_direction::right)
            {
                return "Right";
            }
            if (direction == swipe_direction::up)
            {
                return "Up";
            }
            if (direction == swipe_direction::down)
            {
                return "Down";
            }
            return "(combined)";
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label instructions_;
        maui::controls::box_view target_;
        maui::controls::label readout_;

        // The recognizers (the collection owns them via shared_ptr; we keep our own strong refs so the
        // synthetic drives and the test accessors can reach them).
        std::shared_ptr<maui::controls::tap_gesture_recognizer> tap_ =
            std::make_shared<maui::controls::tap_gesture_recognizer>();
        std::shared_ptr<maui::controls::pan_gesture_recognizer> pan_ =
            std::make_shared<maui::controls::pan_gesture_recognizer>();
        std::shared_ptr<maui::controls::pinch_gesture_recognizer> pinch_ =
            std::make_shared<maui::controls::pinch_gesture_recognizer>();
        std::shared_ptr<maui::controls::swipe_gesture_recognizer> swipe_ =
            std::make_shared<maui::controls::swipe_gesture_recognizer>();
        std::shared_ptr<maui::controls::pointer_gesture_recognizer> pointer_ =
            std::make_shared<maui::controls::pointer_gesture_recognizer>();
    };
} // namespace maui::samples

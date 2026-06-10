#pragma once
// maui::controls::swipe_gesture_recognizer    <=  Microsoft.Maui.Controls.SwipeGestureRecognizer
// maui::controls::i_swipe_gesture_controller  <=  Microsoft.Maui.Controls.ISwipeGestureController
// maui::controls::swiped_event_args           <=  Microsoft.Maui.Controls.SwipedEventArgs
// (+ the Internals.SwipeGestureExtensions rotation helpers, tested by SwipeGestureRecognizerTests.cs)
//
// Recognizes swipe gestures on the attached element. The platform bridges either detect natively and
// call send_swiped directly (UIKit's UISwipeGestureRecognizer), or accumulate pan totals through
// send_swipe and ask detect_swipe at gesture end (the Windows/Android model — and the AppKit synthesis,
// since AppKit has no swipe recognizer). detect_swipe checks the totals against the threshold along
// every requested direction and raises `swiped` with the detected direction(s). Ported from
// SwipeGestureRecognizer.cs + SwipedEventArgs.cs + ISwipeGestureController.cs +
// Internals/SwipeGestureExtensions.cs.
//
// Deviation (port-wide): Command/CommandParameter are not ported (no ICommand port yet — STATUS.md);
// SendSwiped's command-before-event order collapses to raising `swiped`, and SwipedEventArgs.Parameter
// is dropped with it.

#include <cmath>
#include <cstdint>

#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"
#include "maui/core/swipe_direction.hpp"

namespace maui::controls
{
    // The platform-bridge drive seam for swipe (ISwipeGestureController): feed the running pan totals,
    // then ask for detection when the gesture ends.
    class i_swipe_gesture_controller
    {
    public:
        virtual ~i_swipe_gesture_controller() = default;

        virtual void send_swipe(element& sender, double total_x, double total_y) = 0;
        [[nodiscard]] virtual bool detect_swipe(element& sender, maui::core::swipe_direction direction) = 0;

    protected:
        i_swipe_gesture_controller() = default;
        i_swipe_gesture_controller(const i_swipe_gesture_controller&) = default;
        i_swipe_gesture_controller(i_swipe_gesture_controller&&) = default;
        i_swipe_gesture_controller& operator=(const i_swipe_gesture_controller&) = default;
        i_swipe_gesture_controller& operator=(i_swipe_gesture_controller&&) = default;
    };

    // The payload of swipe_gesture_recognizer::swiped (SwipedEventArgs).
    struct swiped_event_args
    {
        maui::core::swipe_direction direction = maui::core::swipe_direction::none;
    };

    class swipe_gesture_recognizer final : public gesture_recognizer, public i_swipe_gesture_controller
    {
    public:
        // SwipeGestureRecognizer.DirectionProperty (default: no direction — C# default(SwipeDirection)).
        static const maui::core::bindable_property<maui::core::swipe_direction>& direction_property();
        // SwipeGestureRecognizer.ThresholdProperty (default 100 px — DefaultSwipeThreshold).
        static const maui::core::bindable_property<std::uint32_t>& threshold_property();

        // The direction(s) of swipes to recognize (SwipeGestureRecognizer.Direction).
        [[nodiscard]] maui::core::swipe_direction direction() const
        {
            return direction_.get();
        }
        void set_direction(maui::core::swipe_direction value)
        {
            direction_.set(value);
        }

        // The minimum distance in pixels the swipe must travel (SwipeGestureRecognizer.Threshold).
        [[nodiscard]] std::uint32_t threshold() const
        {
            return threshold_.get();
        }
        void set_threshold(std::uint32_t value)
        {
            threshold_.set(value);
        }

        // Occurs when a swipe gesture is recognized (SwipeGestureRecognizer.Swiped).
        maui::core::event<swiped_event_args> swiped;

        // SwipeGestureRecognizer.SendSwiped: raise `swiped` with the detected direction (C# executes the
        // command first; not ported — see the header comment).
        void send_swiped(element& sender, maui::core::swipe_direction detected_direction)
        {
            (void)sender;
            swiped.raise(swiped_event_args{.direction = detected_direction});
        }

        // ---- i_swipe_gesture_controller ----
        void send_swipe(element& sender, double total_x, double total_y) override
        {
            (void)sender;
            total_x_ = total_x;
            total_y_ = total_y;
        }
        // Check the accumulated totals against the threshold along every direction in `direction`,
        // raising `swiped` (with the combined detected direction) when any matches.
        [[nodiscard]] bool detect_swipe(element& sender, maui::core::swipe_direction direction) override
        {
            using maui::core::swipe_direction;
            bool detected = false;
            const auto threshold_px = static_cast<double>(threshold());
            auto detected_direction = swipe_direction::none;

            if (maui::core::is_left(direction) && total_x_ < -threshold_px)
            {
                detected = true;
                detected_direction |= swipe_direction::left;
            }
            if (maui::core::is_right(direction) && total_x_ > threshold_px)
            {
                detected = true;
                detected_direction |= swipe_direction::right;
            }
            if (maui::core::is_down(direction) && total_y_ > threshold_px)
            {
                detected = true;
                detected_direction |= swipe_direction::down;
            }
            if (maui::core::is_up(direction) && total_y_ < -threshold_px)
            {
                detected = true;
                detected_direction |= swipe_direction::up;
            }

            if (detected)
            {
                send_swiped(sender, detected_direction);
            }
            return detected;
        }

    private:
        double total_x_ = 0; // SwipeGestureRecognizer._totalX
        double total_y_ = 0; // SwipeGestureRecognizer._totalY
        maui::core::property<maui::core::swipe_direction> direction_{*this, direction_property()};
        maui::core::property<std::uint32_t> threshold_{*this, threshold_property()};
    };

    // ---- Internals.SwipeGestureExtensions ----

    // Normalize a rotation angle into [0, 360) (SwipeGestureExtensions.NormalizeRotation).
    [[nodiscard]] inline double normalize_rotation(double rotation)
    {
        return std::fmod(std::fmod(rotation, 360.0) + 360.0, 360.0);
    }

    // Rotate a swipe direction to compensate for the view's render rotation, so a "left" swipe on a
    // 90°-rotated view still reads as the user's left (SwipeGestureExtensions
    // .TransformSwipeDirectionForRotation). Non-finite rotations and rotations farther than 45° from a
    // cardinal angle return the direction unchanged.
    [[nodiscard]] maui::core::swipe_direction transform_swipe_direction_for_rotation(
        maui::core::swipe_direction direction, double rotation);
} // namespace maui::controls

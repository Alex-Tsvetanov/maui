#pragma once
// maui::controls::pan_gesture_recognizer    <=  Microsoft.Maui.Controls.PanGestureRecognizer
// maui::controls::i_pan_gesture_controller  <=  Microsoft.Maui.Controls.IPanGestureController
// maui::controls::pan_updated_event_args    <=  Microsoft.Maui.Controls.PanUpdatedEventArgs
// maui::controls::auto_id                   <=  Microsoft.Maui.Controls.Internals.AutoId
//
// A gesture recognizer for panning content. The platform bridges drive the recognizer through the
// i_pan_gesture_controller seam (SendPanStarted / SendPan / SendPanCompleted / SendPanCanceled), and
// each drive raises pan_updated with the matching gesture_status — the state machine the C# unit tests
// (PanGestureRecognizerUnitTests.cs) pin down. Ported from PanGestureRecognizer.cs +
// PanUpdatedEventArgs.cs + IPanGestureController.cs.

#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    // The platform-bridge drive seam for pan (IPanGestureController). The sender is the view the
    // recognizer is attached to (C#'s `Element sender`).
    class i_pan_gesture_controller
    {
    public:
        virtual ~i_pan_gesture_controller() = default;

        virtual void send_pan(element& sender, double total_x, double total_y, int gesture_id) = 0;
        virtual void send_pan_canceled(element& sender, int gesture_id) = 0;
        virtual void send_pan_completed(element& sender, int gesture_id) = 0;
        virtual void send_pan_started(element& sender, int gesture_id) = 0;

    protected:
        i_pan_gesture_controller() = default;
        i_pan_gesture_controller(const i_pan_gesture_controller&) = default;
        i_pan_gesture_controller(i_pan_gesture_controller&&) = default;
        i_pan_gesture_controller& operator=(const i_pan_gesture_controller&) = default;
        i_pan_gesture_controller& operator=(i_pan_gesture_controller&&) = default;
    };

    // The payload of pan_gesture_recognizer::pan_updated (PanUpdatedEventArgs). TotalX/TotalY are the
    // total translation since the gesture began (only meaningful while StatusType is running).
    struct pan_updated_event_args
    {
        maui::core::gesture_status status_type = maui::core::gesture_status::started;
        int gesture_id = -1;
        double total_x = 0;
        double total_y = 0;
    };

    // A monotonically increasing gesture-id minter (Internals.AutoId): Value reads the current id,
    // Increment moves to the next (returning the old one) — the platform bridges stamp each pan
    // gesture's events with one id and increment when it ends.
    class auto_id
    {
    public:
        [[nodiscard]] int value() const
        {
            return current_;
        }
        int increment()
        {
            const int old = current_;
            ++current_;
            return old;
        }

    private:
        int current_ = 0;
    };

    class pan_gesture_recognizer : public gesture_recognizer, public i_pan_gesture_controller
    {
    public:
        // PanGestureRecognizer.CurrentId — the shared id minter the platform bridges use.
        static auto_id& current_id();

        // PanGestureRecognizer.TouchPointsProperty (default 1).
        static const maui::core::bindable_property<int>& touch_points_property();

        // The number of touch points in the gesture (PanGestureRecognizer.TouchPoints).
        [[nodiscard]] int touch_points() const
        {
            return touch_points_.get();
        }
        void set_touch_points(int value)
        {
            touch_points_.set(value);
        }

        // Raised on every i_pan_gesture_controller drive (PanGestureRecognizer.PanUpdated).
        maui::core::event<pan_updated_event_args> pan_updated;

        // ---- i_pan_gesture_controller (each drive raises pan_updated with the matching status) ----
        void send_pan(element& sender, double total_x, double total_y, int gesture_id) override
        {
            (void)sender;
            pan_updated.raise(pan_updated_event_args{.status_type = maui::core::gesture_status::running,
                                                     .gesture_id = gesture_id,
                                                     .total_x = total_x,
                                                     .total_y = total_y});
        }
        void send_pan_canceled(element& sender, int gesture_id) override
        {
            (void)sender;
            pan_updated.raise(
                pan_updated_event_args{.status_type = maui::core::gesture_status::canceled, .gesture_id = gesture_id});
        }
        void send_pan_completed(element& sender, int gesture_id) override
        {
            (void)sender;
            pan_updated.raise(
                pan_updated_event_args{.status_type = maui::core::gesture_status::completed, .gesture_id = gesture_id});
        }
        void send_pan_started(element& sender, int gesture_id) override
        {
            (void)sender;
            pan_updated.raise(
                pan_updated_event_args{.status_type = maui::core::gesture_status::started, .gesture_id = gesture_id});
        }

    private:
        maui::core::property<int> touch_points_{*this, touch_points_property()};
    };
} // namespace maui::controls

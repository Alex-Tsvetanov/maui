#pragma once
// maui::controls::pinch_gesture_recognizer        <=  Microsoft.Maui.Controls.PinchGestureRecognizer
// maui::controls::i_pinch_gesture_controller      <=  Microsoft.Maui.Controls.IPinchGestureController
// maui::controls::pinch_gesture_updated_event_args <= Microsoft.Maui.Controls.PinchGestureUpdatedEventArgs
//
// Recognizer for pinch gestures. The platform bridges drive it through the i_pinch_gesture_controller
// seam; each drive raises pinch_updated and maintains the IsPinching flag exactly as
// PinchGestureRecognizer.cs does (started/running set it, ended/canceled clear it). Ported from
// PinchGestureRecognizer.cs + PinchGestureUpdatedEventArgs.cs + IPinchGestureController.cs.

#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/event.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    // The platform-bridge drive seam for pinch (IPinchGestureController). The sender is the view the
    // recognizer is attached to.
    class i_pinch_gesture_controller
    {
    public:
        virtual ~i_pinch_gesture_controller() = default;

        [[nodiscard]] virtual bool is_pinching() const = 0;
        virtual void set_is_pinching(bool value) = 0;

        virtual void send_pinch(element& sender, double scale, maui::graphics::point current_scale_point) = 0;
        virtual void send_pinch_canceled(element& sender) = 0;
        virtual void send_pinch_ended(element& sender) = 0;
        // C# spells the parameter `intialScalePoint` (a preserved typo); the port corrects the name.
        virtual void send_pinch_started(element& sender, maui::graphics::point initial_scale_point) = 0;

    protected:
        i_pinch_gesture_controller() = default;
        i_pinch_gesture_controller(const i_pinch_gesture_controller&) = default;
        i_pinch_gesture_controller(i_pinch_gesture_controller&&) = default;
        i_pinch_gesture_controller& operator=(const i_pinch_gesture_controller&) = default;
        i_pinch_gesture_controller& operator=(i_pinch_gesture_controller&&) = default;
    };

    // The payload of pinch_gesture_recognizer::pinch_updated (PinchGestureUpdatedEventArgs). Scale is
    // the RELATIVE scale since the last update (default 1); ScaleOrigin is the pinch center in
    // view-relative unit coordinates.
    struct pinch_gesture_updated_event_args
    {
        maui::core::gesture_status status = maui::core::gesture_status::started;
        double scale = 1;
        maui::graphics::point scale_origin;
    };

    class pinch_gesture_recognizer final : public gesture_recognizer, public i_pinch_gesture_controller
    {
    public:
        // Raised on every i_pinch_gesture_controller drive (PinchGestureRecognizer.PinchUpdated).
        maui::core::event<pinch_gesture_updated_event_args> pinch_updated;

        // ---- i_pinch_gesture_controller ----
        [[nodiscard]] bool is_pinching() const override
        {
            return is_pinching_;
        }
        void set_is_pinching(bool value) override
        {
            is_pinching_ = value;
        }
        void send_pinch(element& sender, double scale, maui::graphics::point current_scale_point) override
        {
            (void)sender;
            pinch_updated.raise(pinch_gesture_updated_event_args{
                .status = maui::core::gesture_status::running, .scale = scale, .scale_origin = current_scale_point});
            is_pinching_ = true;
        }
        void send_pinch_canceled(element& sender) override
        {
            (void)sender;
            pinch_updated.raise(pinch_gesture_updated_event_args{.status = maui::core::gesture_status::canceled});
            is_pinching_ = false;
        }
        void send_pinch_ended(element& sender) override
        {
            (void)sender;
            pinch_updated.raise(pinch_gesture_updated_event_args{.status = maui::core::gesture_status::completed});
            is_pinching_ = false;
        }
        void send_pinch_started(element& sender, maui::graphics::point initial_scale_point) override
        {
            (void)sender;
            pinch_updated.raise(pinch_gesture_updated_event_args{
                .status = maui::core::gesture_status::started, .scale = 1, .scale_origin = initial_scale_point});
            is_pinching_ = true;
        }

    private:
        bool is_pinching_ = false; // IPinchGestureController.IsPinching backing store
    };
} // namespace maui::controls

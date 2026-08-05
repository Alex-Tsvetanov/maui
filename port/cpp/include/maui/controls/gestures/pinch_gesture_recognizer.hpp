#pragma once
// maui::controls::pinch_gesture_recognizer        <=  Microsoft.Maui.Controls.PinchGestureRecognizer
// maui::controls::i_pinch_gesture_controller      <=  Microsoft.Maui.Controls.IPinchGestureController
// maui::controls::pinch_gesture_updated_event_args <= Microsoft.Maui.Controls.PinchGestureUpdatedEventArgs
//
// Recognizer for pinch gestures. The platform bridges drive it through the i_pinch_gesture_controller
// seam; each drive raises pinch_updated and maintains the IsPinching flag as PinchGestureRecognizer.cs
// does (started/running set it, ended/canceled clear it) — but LATCHED BEFORE the raise rather than
// after, a lifetime-forced deviation documented on latch_then_raise below. Ported from
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
            latch_then_raise(true, pinch_gesture_updated_event_args{.status = maui::core::gesture_status::running,
                                                                    .scale = scale,
                                                                    .scale_origin = current_scale_point});
        }
        void send_pinch_canceled(element& sender) override
        {
            (void)sender;
            latch_then_raise(false, pinch_gesture_updated_event_args{.status = maui::core::gesture_status::canceled});
        }
        void send_pinch_ended(element& sender) override
        {
            (void)sender;
            latch_then_raise(false, pinch_gesture_updated_event_args{.status = maui::core::gesture_status::completed});
        }
        void send_pinch_started(element& sender, maui::graphics::point initial_scale_point) override
        {
            (void)sender;
            latch_then_raise(true, pinch_gesture_updated_event_args{.status = maui::core::gesture_status::started,
                                                                    .scale = 1,
                                                                    .scale_origin = initial_scale_point});
        }

    private:
        // The ONE spine all four sends run through, so no send_* — present or future — can write
        // is_pinching_ after user code. C# writes the latch AFTER the Invoke (PinchGestureRecognizer.cs
        // :22/:33/:44/:55), which is safe only because the GC roots `this` for the whole method body. In
        // the port a PinchUpdated handler can destroy the pinched view, and the view's
        // gesture_recognizers() collection OWNS the recognizer — so the write lands in freed memory
        // (reproduced as a heap-use-after-free WRITE by
        // pinch_gesture_recognizer_test.a_handler_may_destroy_the_view_and_the_recognizer_mid_dispatch).
        // Caller-side rooting cannot be relied on: the android bridge resolves its controller as
        // `entry.lock().get()` (src/platform/android/gesture_platform_manager.cpp:377-387), whose strong
        // ref dies at the end of that expression, and sends through the raw pointer at :1203.
        //
        // DOCUMENTED DEVIATION (gesture_recognizer.hpp rule 3): the latch moves BEFORE the raise. No
        // consumer can observe the move — every framework read of IsPinching happens BETWEEN drives, in
        // the native callback deciding whether to send Ended/Canceled (GesturePlatformManager.iOS.cs
        // :305/:324/:328, EventTracker.cs:300/:322/:326), and by then both orderings have written the
        // same value. The only behavioral delta is re-entrant: a handler that drives a further pinch
        // phase now sees the NEW latch (a Started handler's nested Completed passes the is_pinching()
        // guard, where C# would suppress it) — untested in C#, and in the truthful direction, since a
        // pinch that IS in flight then reads as in flight. The port's bridges already latch their own
        // pinch bookkeeping before the raise for exactly this reason (24db16875a;
        // src/platform/apple/gesture_platform_manager.mm:296-320), citing MAUI's own sibling oracle —
        // GesturePlatformManager.iOS.cs:301-302 reads its pinch state after the send, the Android
        // PinchGestureHandler.cs:63/:67 reads it before.
        void latch_then_raise(bool pinching, const pinch_gesture_updated_event_args& args)
        {
            is_pinching_ = pinching; // the LAST touch of `this` — everything below may free it
            // raise() snapshots its handler list into locals before invoking any of them (event.hpp:69-82),
            // so it reads no member of `this` once user code is running.
            pinch_updated.raise(args);
        }

        bool is_pinching_ = false; // IPinchGestureController.IsPinching backing store
    };
} // namespace maui::controls

// maui::controls::gesture_platform_manager — the CROSS-PLATFORM half of the GestureManager /
// GesturePlatformManager pair (see the header): the handler-lifecycle bookkeeping
// (GestureManager.SetupGestureManager / DisconnectGestures), the LoadRecognizers attach/detach diff,
// and the synthetic dispatch that routes a synthesized gesture through the same controller-interface
// calls the platform bridges make. The native attach/detach per recognizer is the backend partial
// (src/platform/<backend>/gesture_platform_manager.{cpp,mm}).

#include "maui/controls/gestures/gesture_platform_manager.hpp"

#include <algorithm>
#include <memory>
#include <optional>

#include "maui/controls/element.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    void gesture_platform_manager::set_handler(maui::core::i_view_handler* handler, element& sender,
                                               gesture_recognizer_collection& recognizers)
    {
        sender_ = &sender;
        recognizers_ = &recognizers;
        if (handler_ == handler)
        {
            return; // GestureManager.SetupGestureManager: already set up and watching the right view
        }
        // A different (or no) platform view: tear down the previous attachments first
        // (GestureManager.DisconnectGestures → GesturePlatformManager.Dispose).
        native_detach_all();
        attached_.clear();
        handler_ = handler;
        if (handler_ != nullptr)
        {
            load_recognizers();
        }
    }

    void gesture_platform_manager::load_recognizers()
    {
        if (handler_ == nullptr || recognizers_ == nullptr)
        {
            return; // not connected (no platform view to attach to) — mirrors the null guards in C#
        }

        // Attach every collection recognizer not yet attached (the first LoadRecognizers loop).
        for (const auto& recognizer : recognizers_->items())
        {
            if (!is_attached(*recognizer))
            {
                attached_.push_back(recognizer);
                native_attach(recognizer);
            }
        }

        // Detach every attached recognizer no longer in the collection (the toRemove sweep).
        for (auto it = attached_.begin(); it != attached_.end();)
        {
            if (!recognizers_->contains(**it))
            {
                native_detach(**it);
                it = attached_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    bool gesture_platform_manager::is_attached(const gesture_recognizer& recognizer) const
    {
        return std::ranges::any_of(attached_, [&recognizer](const std::shared_ptr<gesture_recognizer>& item) {
            return item.get() == &recognizer;
        });
    }

    // ---- synthetic dispatch ----

    void gesture_platform_manager::synthetic_tap(int number_of_taps, buttons_mask button,
                                                 std::optional<maui::graphics::point> position)
    {
        if (sender_ == nullptr)
        {
            return;
        }
        for (const auto& recognizer : attached_)
        {
            auto* tap = dynamic_cast<tap_gesture_recognizer*>(recognizer.get());
            // The iOS bridge filters: NumberOfTapsRequired must match the native recognizer's count,
            // and the fired button must be within the recognizer's mask (ButtonMaskRequired).
            if (tap != nullptr && tap->number_of_taps_required() == number_of_taps && contains(tap->buttons(), button))
            {
                tap->send_tapped(*sender_, position);
            }
        }
    }

    void gesture_platform_manager::synthetic_pan(maui::core::gesture_status phase, double total_x, double total_y,
                                                 int touch_points)
    {
        using maui::core::gesture_status;
        if (sender_ == nullptr)
        {
            return;
        }
        for (const auto& recognizer : attached_)
        {
            auto* pan = dynamic_cast<pan_gesture_recognizer*>(recognizer.get());
            if (pan == nullptr || pan->touch_points() != touch_points)
            {
                continue; // the iOS bridge's NumberOfTouches != TouchPoints filter
            }
            auto& current_id = pan_gesture_recognizer::current_id();
            switch (phase)
            {
                case gesture_status::started:
                    pan->send_pan_started(*sender_, current_id.value());
                    break;
                case gesture_status::running:
                    pan->send_pan(*sender_, total_x, total_y, current_id.value());
                    break;
                case gesture_status::completed:
                    pan->send_pan_completed(*sender_, current_id.value());
                    current_id.increment();
                    break;
                case gesture_status::canceled:
                    pan->send_pan_canceled(*sender_, current_id.value());
                    current_id.increment();
                    break;
            }
        }
    }

    void gesture_platform_manager::synthetic_pinch(maui::core::gesture_status phase, double scale,
                                                   maui::graphics::point origin)
    {
        using maui::core::gesture_status;
        if (sender_ == nullptr)
        {
            return;
        }
        for (const auto& recognizer : attached_)
        {
            auto* pinch = dynamic_cast<i_pinch_gesture_controller*>(recognizer.get());
            if (pinch == nullptr)
            {
                continue;
            }
            switch (phase)
            {
                case gesture_status::started:
                    pinch->send_pinch_started(*sender_, origin);
                    break;
                case gesture_status::running:
                    pinch->send_pinch(*sender_, scale, origin);
                    break;
                case gesture_status::completed:
                    // The iOS bridge's Ended case only sends when a pinch is actually in flight.
                    if (pinch->is_pinching())
                    {
                        pinch->send_pinch_ended(*sender_);
                    }
                    break;
                case gesture_status::canceled:
                    if (pinch->is_pinching())
                    {
                        pinch->send_pinch_canceled(*sender_);
                    }
                    break;
            }
        }
    }

    void gesture_platform_manager::synthetic_swipe(double total_x, double total_y)
    {
        if (sender_ == nullptr)
        {
            return;
        }
        for (const auto& recognizer : attached_)
        {
            auto* swipe = dynamic_cast<swipe_gesture_recognizer*>(recognizer.get());
            if (swipe != nullptr)
            {
                swipe->send_swipe(*sender_, total_x, total_y);
                (void)swipe->detect_swipe(*sender_, swipe->direction());
            }
        }
    }

    void gesture_platform_manager::synthetic_pointer(pointer_event_kind kind,
                                                     std::optional<maui::graphics::point> position, buttons_mask button)
    {
        if (sender_ == nullptr)
        {
            return;
        }
        for (const auto& recognizer : attached_)
        {
            auto* pointer = dynamic_cast<pointer_gesture_recognizer*>(recognizer.get());
            if (pointer == nullptr)
            {
                continue;
            }
            // The iOS bridge filters press events by the recognizer's mask; hover events pass through.
            const bool is_hover = kind == pointer_event_kind::entered || kind == pointer_event_kind::exited ||
                                  kind == pointer_event_kind::moved;
            if (!is_hover && !contains(pointer->buttons(), button))
            {
                continue;
            }
            switch (kind)
            {
                case pointer_event_kind::entered:
                    pointer->send_pointer_entered(*sender_, position, button);
                    break;
                case pointer_event_kind::exited:
                    pointer->send_pointer_exited(*sender_, position, button);
                    break;
                case pointer_event_kind::moved:
                    pointer->send_pointer_moved(*sender_, position, button);
                    break;
                case pointer_event_kind::pressed:
                    pointer->send_pointer_pressed(*sender_, position, button);
                    break;
                case pointer_event_kind::released:
                    pointer->send_pointer_released(*sender_, position, button);
                    break;
            }
        }
    }
} // namespace maui::controls

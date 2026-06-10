#pragma once
// maui::controls::tap_gesture_recognizer  <=  Microsoft.Maui.Controls.TapGestureRecognizer
// maui::controls::tapped_event_args      <=  Microsoft.Maui.Controls.TappedEventArgs
//
// Recognizes tap gestures on the attached element. Ported from TapGestureRecognizer.cs +
// TappedEventArgs.cs. Surface deviations (documented, port-wide):
//   - Command/CommandParameter are not ported (ICommand has no port yet — STATUS.md); SendTapped's
//     command-before-event order collapses to raising `tapped`. TappedEventArgs.Parameter (the command
//     parameter echo) is dropped with it.
//   - C#'s TappedEventArgs.GetPosition(relativeTo) (an element-relative coordinate conversion closure)
//     is narrowed to a stored position relative to the view the recognizer is attached to — the value
//     the platform bridges read via locationInView(platformView). nullopt when unavailable.

#include <optional>
#include <utility>

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    // The payload of tap_gesture_recognizer::tapped (TappedEventArgs).
    struct tapped_event_args
    {
        // The button(s) the recognizer fired for (TappedEventArgs.Buttons — the recognizer's own mask).
        buttons_mask buttons = buttons_mask::primary;
        // The tap position relative to the attached view, when the platform bridge knows it.
        std::optional<maui::graphics::point> position;
    };

    class tap_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // TapGestureRecognizer.NumberOfTapsRequiredProperty (default 1).
        static const maui::core::bindable_property<int>& number_of_taps_required_property();
        // TapGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask>& buttons_property();

        // The number of taps required to trigger the gesture (TapGestureRecognizer.NumberOfTapsRequired).
        [[nodiscard]] int number_of_taps_required() const
        {
            return number_of_taps_required_.get();
        }
        void set_number_of_taps_required(int value)
        {
            number_of_taps_required_.set(value);
        }

        // The button mask that triggers the gesture (TapGestureRecognizer.Buttons).
        [[nodiscard]] buttons_mask buttons() const
        {
            return buttons_.get();
        }
        void set_buttons(buttons_mask value)
        {
            buttons_.set(value);
        }

        // Occurs when a tap gesture is recognized on the element (TapGestureRecognizer.Tapped).
        maui::core::event<tapped_event_args> tapped;

        // TapGestureRecognizer.SendTapped: the platform bridge (or the synthetic dispatch) calls this on
        // a recognized tap. The sender is the attached view; the port's events carry args only (no
        // sender), matching the other controls' events.
        void send_tapped(element& sender, std::optional<maui::graphics::point> position = std::nullopt)
        {
            (void)sender;
            tapped.raise(tapped_event_args{.buttons = buttons(), .position = position});
        }

    private:
        maui::core::property<int> number_of_taps_required_{*this, number_of_taps_required_property()};
        maui::core::property<buttons_mask> buttons_{*this, buttons_property()};
    };
} // namespace maui::controls

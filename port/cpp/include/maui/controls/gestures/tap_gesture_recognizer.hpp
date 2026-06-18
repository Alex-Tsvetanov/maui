#pragma once
// maui::controls::tap_gesture_recognizer  <=  Microsoft.Maui.Controls.TapGestureRecognizer
// maui::controls::tapped_event_args      <=  Microsoft.Maui.Controls.TappedEventArgs
//
// Recognizes tap gestures on the attached element. Ported from TapGestureRecognizer.cs +
// TappedEventArgs.cs. Surface deviations (documented, port-wide):
//   - Command/CommandParameter ARE ported (U-CMD): Command is an i_command (command.hpp), held by a
//     bindable property<shared_ptr<i_command>>; CommandParameter is C#'s `object`, the port's boxed
//     std::any. CommandParameter is a plain member with hand-rolled change notification (not a
//     property<std::any> — the typed property engine needs operator== on T, which std::any lacks; same
//     deviation as RadioButton.Value). send_tapped runs the C# command-before-event order:
//     `if (cmd != null && cmd.can_execute(p)) cmd.execute(p);` then raises `tapped`.
//   - TappedEventArgs.Parameter (the command-parameter echo into the event args) is included so the C#
//     CallbackPassesParameter behavior is observable: tapped_event_args carries the CommandParameter.
//   - C#'s TappedEventArgs.GetPosition(relativeTo) (an element-relative coordinate conversion closure)
//     is narrowed to a stored position relative to the view the recognizer is attached to — the value
//     the platform bridges read via locationInView(platformView). nullopt when unavailable.

#include <any>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "maui/controls/command.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/i_command.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/boxed_value.hpp"
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
        // TappedEventArgs.Parameter — the recognizer's CommandParameter echoed into the args (C#'s
        // `new TappedEventArgs(CommandParameter, …)`). The port's `object` is std::any; empty = C#'s null.
        std::any parameter;
    };

    class tap_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // TapGestureRecognizer.CommandProperty (default null).
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& command_property();
        // TapGestureRecognizer.NumberOfTapsRequiredProperty (default 1).
        static const maui::core::bindable_property<int>& number_of_taps_required_property();
        // TapGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask>& buttons_property();

        // The command invoked when the gesture is recognized (TapGestureRecognizer.Command). null = none.
        [[nodiscard]] const std::shared_ptr<i_command>& command() const
        {
            return command_.get();
        }
        void set_command(std::shared_ptr<i_command> value)
        {
            command_.set(std::move(value));
        }

        // The parameter passed to Command (TapGestureRecognizer.CommandParameter — C#'s `object`, boxed).
        // A plain member with hand-rolled change notification (see the header note); empty = C#'s null.
        [[nodiscard]] const std::any& command_parameter() const
        {
            return command_parameter_;
        }
        void set_command_parameter(std::any value)
        {
            if (maui::core::boxed_equals(command_parameter_, value))
            {
                return;
            }
            this->on_property_changing(command_parameter_property_name);
            command_parameter_ = std::move(value);
            this->on_property_changed(command_parameter_property_name);
        }

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
        // a recognized tap. C# order (preserved): run the command first —
        // `if (cmd != null && cmd.CanExecute(CommandParameter)) cmd.Execute(CommandParameter);` — then
        // raise `tapped`. The sender is the attached view; the port's events carry args only (no sender),
        // matching the other controls' events.
        void send_tapped(element& sender, std::optional<maui::graphics::point> position = std::nullopt)
        {
            (void)sender;
            if (const std::shared_ptr<i_command>& cmd = command(); cmd && cmd->can_execute(command_parameter_))
            {
                cmd->execute(command_parameter_);
            }

            tapped.raise(
                tapped_event_args{.buttons = buttons(), .position = position, .parameter = command_parameter_});
        }

    private:
        // The name CommandParameter notifies under (the C# CommandParameterProperty name). A plain member,
        // not a property<T>, so it carries its own notification key.
        static constexpr std::string_view command_parameter_property_name = "command_parameter";

        maui::core::property<std::shared_ptr<i_command>> command_{*this, command_property()};
        std::any command_parameter_; // TapGestureRecognizer.CommandParameter (see the header note)
        maui::core::property<int> number_of_taps_required_{*this, number_of_taps_required_property()};
        maui::core::property<buttons_mask> buttons_{*this, buttons_property()};
    };
} // namespace maui::controls

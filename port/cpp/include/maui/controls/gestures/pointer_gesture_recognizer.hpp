#pragma once
// maui::controls::pointer_gesture_recognizer  <=  Microsoft.Maui.Controls.PointerGestureRecognizer
// maui::controls::pointer_event_args          <=  Microsoft.Maui.Controls.PointerEventArgs
//
// Provides pointer gesture recognition and events: entered / exited / moved over the attached view
// (hover), plus pressed / released. Ported from PointerGestureRecognizer.cs + PointerEventArgs.cs.
// Deviations (documented, port-wide):
//   - The five Pointer*Command/Pointer*CommandParameter pairs ARE ported (U-CMD): each Command is an
//     i_command held by a bindable property<shared_ptr<i_command>>; each CommandParameter is C#'s
//     `object` (a plain std::any member with hand-rolled notification, like RadioButton.Value /
//     TapGestureRecognizer.CommandParameter). Each Send* runs the C# command-before-event order:
//     `if (cmd?.CanExecute(param) == true) cmd.Execute(param);` then raises the event.
//   - PointerEventArgs.GetPosition(relativeTo) / PlatformArgs are narrowed to a stored view-relative
//     position (nullopt when unavailable), like tapped_event_args.

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
    // The payload of the pointer events (PointerEventArgs). Button is the mouse button that triggered
    // the event; for pure hover events it stays primary (the C# default).
    struct pointer_event_args
    {
        buttons_mask button = buttons_mask::primary;
        std::optional<maui::graphics::point> position;
    };

    class pointer_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // PointerGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask>& buttons_property();
        // The five Pointer*CommandProperty descriptors (default null).
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_entered_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_exited_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_moved_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_pressed_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_released_command_property();

        // The mouse buttons that should trigger the pointer events (PointerGestureRecognizer.Buttons).
        [[nodiscard]] buttons_mask buttons() const
        {
            return buttons_.get();
        }
        void set_buttons(buttons_mask value)
        {
            buttons_.set(value);
        }

        // The five Pointer*Command / Pointer*CommandParameter pairs. Each command is an i_command held by
        // a bindable property; each parameter is C#'s `object` (a plain std::any member, hand-notified —
        // see the header note). Getters/setters mirror the C# property pairs.
        [[nodiscard]] const std::shared_ptr<i_command>& pointer_entered_command() const
        {
            return pointer_entered_command_.get();
        }
        void set_pointer_entered_command(std::shared_ptr<i_command> value)
        {
            pointer_entered_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& pointer_entered_command_parameter() const
        {
            return pointer_entered_command_parameter_;
        }
        void set_pointer_entered_command_parameter(std::any value)
        {
            set_command_parameter(pointer_entered_command_parameter_, std::move(value),
                                  "pointer_entered_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& pointer_exited_command() const
        {
            return pointer_exited_command_.get();
        }
        void set_pointer_exited_command(std::shared_ptr<i_command> value)
        {
            pointer_exited_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& pointer_exited_command_parameter() const
        {
            return pointer_exited_command_parameter_;
        }
        void set_pointer_exited_command_parameter(std::any value)
        {
            set_command_parameter(pointer_exited_command_parameter_, std::move(value),
                                  "pointer_exited_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& pointer_moved_command() const
        {
            return pointer_moved_command_.get();
        }
        void set_pointer_moved_command(std::shared_ptr<i_command> value)
        {
            pointer_moved_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& pointer_moved_command_parameter() const
        {
            return pointer_moved_command_parameter_;
        }
        void set_pointer_moved_command_parameter(std::any value)
        {
            set_command_parameter(pointer_moved_command_parameter_, std::move(value),
                                  "pointer_moved_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& pointer_pressed_command() const
        {
            return pointer_pressed_command_.get();
        }
        void set_pointer_pressed_command(std::shared_ptr<i_command> value)
        {
            pointer_pressed_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& pointer_pressed_command_parameter() const
        {
            return pointer_pressed_command_parameter_;
        }
        void set_pointer_pressed_command_parameter(std::any value)
        {
            set_command_parameter(pointer_pressed_command_parameter_, std::move(value),
                                  "pointer_pressed_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& pointer_released_command() const
        {
            return pointer_released_command_.get();
        }
        void set_pointer_released_command(std::shared_ptr<i_command> value)
        {
            pointer_released_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& pointer_released_command_parameter() const
        {
            return pointer_released_command_parameter_;
        }
        void set_pointer_released_command_parameter(std::any value)
        {
            set_command_parameter(pointer_released_command_parameter_, std::move(value),
                                  "pointer_released_command_parameter");
        }

        // Raised when the pointer enters / exits / moves within the view, initiates a press, or
        // releases a previous press (PointerGestureRecognizer.PointerEntered/Exited/Moved/Pressed/
        // Released).
        maui::core::event<pointer_event_args> pointer_entered;
        maui::core::event<pointer_event_args> pointer_exited;
        maui::core::event<pointer_event_args> pointer_moved;
        maui::core::event<pointer_event_args> pointer_pressed;
        maui::core::event<pointer_event_args> pointer_released;

        // The platform-bridge inbound channel (PointerGestureRecognizer.SendPointerEntered/…). C# order
        // (preserved): `if (cmd?.CanExecute(param) == true) cmd.Execute(param);` then raise the event. The
        // sender is the attached view; the port's events carry args only (no sender).
        void send_pointer_entered(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                  buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            run_command(pointer_entered_command(), pointer_entered_command_parameter_);
            pointer_entered.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_exited(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                 buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            run_command(pointer_exited_command(), pointer_exited_command_parameter_);
            pointer_exited.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_moved(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            run_command(pointer_moved_command(), pointer_moved_command_parameter_);
            pointer_moved.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_pressed(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                  buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            run_command(pointer_pressed_command(), pointer_pressed_command_parameter_);
            pointer_pressed.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_released(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                   buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            run_command(pointer_released_command(), pointer_released_command_parameter_);
            pointer_released.raise(pointer_event_args{.button = button, .position = position});
        }

    private:
        // C#'s `if (cmd?.CanExecute(param) == true) cmd.Execute(param);` — the shared command-run step.
        static void run_command(const std::shared_ptr<i_command>& cmd, const std::any& parameter)
        {
            if (cmd && cmd->can_execute(parameter))
            {
                cmd->execute(parameter);
            }
        }
        // The hand-rolled CommandParameter change-notification (a plain std::any member is not a
        // property<T>, so it carries its own boxed_equals check + on_property_changed key — like
        // RadioButton.Value).
        void set_command_parameter(std::any& slot, std::any value, std::string_view name)
        {
            if (maui::core::boxed_equals(slot, value))
            {
                return;
            }
            this->on_property_changing(name);
            slot = std::move(value);
            this->on_property_changed(name);
        }

        maui::core::property<buttons_mask> buttons_{*this, buttons_property()};
        maui::core::property<std::shared_ptr<i_command>> pointer_entered_command_{*this,
                                                                                  pointer_entered_command_property()};
        maui::core::property<std::shared_ptr<i_command>> pointer_exited_command_{*this,
                                                                                 pointer_exited_command_property()};
        maui::core::property<std::shared_ptr<i_command>> pointer_moved_command_{*this,
                                                                                pointer_moved_command_property()};
        maui::core::property<std::shared_ptr<i_command>> pointer_pressed_command_{*this,
                                                                                  pointer_pressed_command_property()};
        maui::core::property<std::shared_ptr<i_command>> pointer_released_command_{*this,
                                                                                   pointer_released_command_property()};
        std::any pointer_entered_command_parameter_;
        std::any pointer_exited_command_parameter_;
        std::any pointer_moved_command_parameter_;
        std::any pointer_pressed_command_parameter_;
        std::any pointer_released_command_parameter_;
    };
} // namespace maui::controls

#pragma once
// maui::controls::drop_gesture_recognizer  <=  Microsoft.Maui.Controls.DropGestureRecognizer
// maui::controls::drag_event_args          <=  Microsoft.Maui.Controls.DragEventArgs
// maui::controls::drop_event_args          <=  Microsoft.Maui.Controls.DropEventArgs
//
// A gesture recognizer that makes its attached element a drop TARGET. Ported from
// src/Controls/src/Core/DragAndDrop/{DropGestureRecognizer,DragEventArgs,DropEventArgs}.cs. The platform
// bridges (UIDropInteraction / NSDraggingDestination) drive it: send_drag_over while a drag hovers,
// send_drag_leave when it leaves, send_drop on release.
//
// Deviations (documented, port-wide):
//   - Command/CommandParameter (DragOverCommand / DragLeaveCommand / DropCommand + their parameters) ARE
//     ported (U-CMD): each command is an i_command held by a bindable property; each parameter is C#'s
//     `object` (a plain std::any member, hand-notified — like RadioButton.Value). As on the drag side,
//     C# calls `Command?.Execute(param)` with NO CanExecute gate, then raises the event — preserved here.
//   - GetPosition(relativeTo) + PlatformArgs are dropped (see drag_gesture_recognizer.hpp).
//   - SendDrop is `async Task` in C# (it awaits Data.GetTextAsync / GetImageAsync — both synchronously
//     completed). The port's data_package_view exposes those synchronously, so send_drop is plain void.
//   - SendDrop's text/image auto-injection (set the dropped text/image onto the Parent control via
//     TrySetValue / Image.Source) is ported for the cleanly-portable controls via drag_drop_data.hpp;
//     TimePicker/DatePicker injection is the documented gap there. The image-target injection
//     (Parent is Image/ImageButton/Button → .Source) is included.

#include <any>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/controls/command.hpp"
#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/i_command.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    // DragEventArgs — the payload of drag_over / drag_leave. Borrows the live DataPackage (a reference
    // type in C#; the bridge owns the package for the hover's duration). AcceptedOperation is the
    // drop-target's answer the bridge reads back (default Copy).
    class drag_event_args
    {
    public:
        // DragEventArgs(DataPackage). The package is borrowed (non-owning) — it outlives the args (the
        // bridge keeps it alive across the hover; tests keep it on the stack alongside the args).
        explicit drag_event_args(data_package& data) : data_(&data)
        {
        }

        // DragEventArgs.Data.
        [[nodiscard]] data_package& data()
        {
            return *data_;
        }
        [[nodiscard]] const data_package& data() const
        {
            return *data_;
        }

        // DragEventArgs.AcceptedOperation (default DataPackageOperation.Copy).
        [[nodiscard]] data_package_operation accepted_operation() const
        {
            return accepted_operation_;
        }
        void set_accepted_operation(data_package_operation value)
        {
            accepted_operation_ = value;
        }

    private:
        data_package* data_; // non-owning (see the ctor note)
        data_package_operation accepted_operation_ = data_package_operation::copy;
    };

    // DropEventArgs — the payload of drop. Carries (and owns) a read-only DataPackageView snapshot of the
    // dropped data (C#: DropEventArgs(DataPackageView)). Handled suppresses the auto-injection in SendDrop.
    class drop_event_args
    {
    public:
        // DropEventArgs(DataPackageView).
        explicit drop_event_args(data_package_view data) : data_(std::move(data))
        {
        }

        // DropEventArgs.Data (read-only view of the dropped package).
        [[nodiscard]] const data_package_view& data() const
        {
            return data_;
        }

        // DropEventArgs.Handled — when true, SendDrop skips its default text/image processing.
        [[nodiscard]] bool handled() const
        {
            return handled_;
        }
        void set_handled(bool value)
        {
            handled_ = value;
        }

    private:
        data_package_view data_;
        bool handled_ = false;
    };

    class drop_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // DropGestureRecognizer.AllowDropProperty (default true).
        static const maui::core::bindable_property<bool>& allow_drop_property();
        // DropGestureRecognizer.DragOverCommandProperty / DragLeaveCommandProperty / DropCommandProperty
        // (default null).
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& drag_over_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& drag_leave_command_property();
        static const maui::core::bindable_property<std::shared_ptr<i_command>>& drop_command_property();

        // Whether the element can accept dropped data (DropGestureRecognizer.AllowDrop).
        [[nodiscard]] bool allow_drop() const
        {
            return allow_drop_.get();
        }
        void set_allow_drop(bool value)
        {
            allow_drop_.set(value);
        }

        // DragOverCommand / DragLeaveCommand / DropCommand + their parameters (see the header note on
        // ordering — no CanExecute gate on the drop side).
        [[nodiscard]] const std::shared_ptr<i_command>& drag_over_command() const
        {
            return drag_over_command_.get();
        }
        void set_drag_over_command(std::shared_ptr<i_command> value)
        {
            drag_over_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& drag_over_command_parameter() const
        {
            return drag_over_command_parameter_;
        }
        void set_drag_over_command_parameter(std::any value)
        {
            set_command_parameter(drag_over_command_parameter_, std::move(value), "drag_over_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& drag_leave_command() const
        {
            return drag_leave_command_.get();
        }
        void set_drag_leave_command(std::shared_ptr<i_command> value)
        {
            drag_leave_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& drag_leave_command_parameter() const
        {
            return drag_leave_command_parameter_;
        }
        void set_drag_leave_command_parameter(std::any value)
        {
            set_command_parameter(drag_leave_command_parameter_, std::move(value), "drag_leave_command_parameter");
        }

        [[nodiscard]] const std::shared_ptr<i_command>& drop_command() const
        {
            return drop_command_.get();
        }
        void set_drop_command(std::shared_ptr<i_command> value)
        {
            drop_command_.set(std::move(value));
        }
        [[nodiscard]] const std::any& drop_command_parameter() const
        {
            return drop_command_parameter_;
        }
        void set_drop_command_parameter(std::any value)
        {
            set_command_parameter(drop_command_parameter_, std::move(value), "drop_command_parameter");
        }

        // DropGestureRecognizer.DragOver / DragLeave / Drop.
        maui::core::event<drag_event_args&> drag_over;
        maui::core::event<drag_event_args&> drag_leave;
        maui::core::event<drop_event_args&> drop;

        // DropGestureRecognizer.SendDragOver / SendDragLeave: run the command (C#: `Command?.Execute(param)`
        // — no CanExecute gate), then raise the event (the bridge reads args.AcceptedOperation back from
        // drag_over). const — a drop drive mutates no recognizer state (executing a command through the
        // shared_ptr does not touch the recognizer), unlike the drag-side sends that latch _isDragActive.
        void send_drag_over(drag_event_args& args) const
        {
            run_command(drag_over_command(), drag_over_command_parameter_);
            drag_over.raise(args);
        }
        void send_drag_leave(drag_event_args& args) const
        {
            run_command(drag_leave_command(), drag_leave_command_parameter_);
            drag_leave.raise(args);
        }

        // DropGestureRecognizer.SendDrop: a no-op when !AllowDrop; otherwise raise drop, then — unless
        // args.Handled — inject the dropped text/image onto the recognizer's Parent control (the drop
        // target). `parent` is the attached element (C#'s Parent); null skips the injection but still
        // raises drop. See drag_drop_data.hpp for the inject seam + its TimePicker/DatePicker gap.
        void send_drop(drop_event_args& args, element* parent = nullptr) const;

    private:
        // C#'s `Command?.Execute(param)` — the drop side runs the command unconditionally when set (NO
        // CanExecute gate, unlike Tap/Pointer's run_command). Shared by all three drop sends + send_drop.
        static void run_command(const std::shared_ptr<i_command>& cmd, const std::any& parameter)
        {
            if (cmd)
            {
                cmd->execute(parameter);
            }
        }
        // The hand-rolled CommandParameter change-notification (a plain std::any member, like
        // RadioButton.Value / pointer_gesture_recognizer).
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

        maui::core::property<bool> allow_drop_{*this, allow_drop_property()};
        maui::core::property<std::shared_ptr<i_command>> drag_over_command_{*this, drag_over_command_property()};
        maui::core::property<std::shared_ptr<i_command>> drag_leave_command_{*this, drag_leave_command_property()};
        maui::core::property<std::shared_ptr<i_command>> drop_command_{*this, drop_command_property()};
        std::any drag_over_command_parameter_;
        std::any drag_leave_command_parameter_;
        std::any drop_command_parameter_;
    };
} // namespace maui::controls

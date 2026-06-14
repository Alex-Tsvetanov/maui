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
//   - Command/CommandParameter (DragOverCommand / DragLeaveCommand / DropCommand + their parameters) are
//     not ported (no ICommand port yet — STATUS.md); each Send* collapses to raising its event.
//   - GetPosition(relativeTo) + PlatformArgs are dropped (see drag_gesture_recognizer.hpp).
//   - SendDrop is `async Task` in C# (it awaits Data.GetTextAsync / GetImageAsync — both synchronously
//     completed). The port's data_package_view exposes those synchronously, so send_drop is plain void.
//   - SendDrop's text/image auto-injection (set the dropped text/image onto the Parent control via
//     TrySetValue / Image.Source) is ported for the cleanly-portable controls via drag_drop_data.hpp;
//     TimePicker/DatePicker injection is the documented gap there. The image-target injection
//     (Parent is Image/ImageButton/Button → .Source) is included.

#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
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

        // Whether the element can accept dropped data (DropGestureRecognizer.AllowDrop).
        [[nodiscard]] bool allow_drop() const
        {
            return allow_drop_.get();
        }
        void set_allow_drop(bool value)
        {
            allow_drop_.set(value);
        }

        // DropGestureRecognizer.DragOver / DragLeave / Drop.
        maui::core::event<drag_event_args&> drag_over;
        maui::core::event<drag_event_args&> drag_leave;
        maui::core::event<drop_event_args&> drop;

        // DropGestureRecognizer.SendDragOver / SendDragLeave: raise the event (the bridge reads
        // args.AcceptedOperation back from drag_over). const — a drop drive mutates no recognizer state
        // (event::raise is const; the args + the target control carry the effect), unlike the drag-side
        // sends that latch _isDragActive.
        void send_drag_over(drag_event_args& args) const
        {
            drag_over.raise(args);
        }
        void send_drag_leave(drag_event_args& args) const
        {
            drag_leave.raise(args);
        }

        // DropGestureRecognizer.SendDrop: a no-op when !AllowDrop; otherwise raise drop, then — unless
        // args.Handled — inject the dropped text/image onto the recognizer's Parent control (the drop
        // target). `parent` is the attached element (C#'s Parent); null skips the injection but still
        // raises drop. See drag_drop_data.hpp for the inject seam + its TimePicker/DatePicker gap.
        void send_drop(drop_event_args& args, element* parent = nullptr) const;

    private:
        maui::core::property<bool> allow_drop_{*this, allow_drop_property()};
    };
} // namespace maui::controls

#pragma once
// maui::controls::drag_gesture_recognizer  <=  Microsoft.Maui.Controls.DragGestureRecognizer
// maui::controls::drag_starting_event_args <=  Microsoft.Maui.Controls.DragStartingEventArgs
// maui::controls::drop_completed_event_args <= Microsoft.Maui.Controls.DropCompletedEventArgs
//
// A gesture recognizer that makes its attached element a drag SOURCE. Ported from
// src/Controls/src/Core/DragAndDrop/{DragGestureRecognizer,DragStartingEventArgs,DropCompletedEventArgs}.cs.
// The platform bridges (UIDragInteraction / NSDraggingSource) drive it: send_drag_starting on a recognized
// drag (mints the args + DataPackage the bridge hands to the OS), send_drop_completed when the OS reports
// the drag ended.
//
// Deviations (documented, port-wide):
//   - Command/CommandParameter (DragStartingCommand / DropCompletedCommand + their parameters) are not
//     ported (no ICommand port yet — STATUS.md); each Send* collapses to raising its event. The C# tests
//     that assert the command fired are therefore not portable (the event-raise + the data-package
//     plumbing they also exercise IS pinned by the headless suite).
//   - DragStartingEventArgs.GetPosition(relativeTo) (an element-relative coordinate closure) and the
//     PlatformArgs (the UIKit/Windows drag-session handles) are dropped — no headless coordinate seam,
//     no synthetic native session (the no-synthetic-session deviation, also documented on the native
//     attach). DragStartingEventArgs.Handled (the obsolete Windows-only flag) IS modeled because
//     SendDragStarting branches on it.
//   - The text/image AUTO-extraction in SendDragStarting (Data.Text ??= element.GetStringValue();
//     Data.Image ??= (element as IImageElement).Source) is ported for the cleanly-portable controls via
//     the drag_drop_data.hpp seam — see send_drag_starting. TimePicker/DatePicker text extraction is the
//     one gap there (it needs .NET culture-default DateTime/TimeSpan ToString, which the port does not
//     reproduce); documented in drag_drop_data.hpp.

#include "maui/controls/data_package.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    // DragStartingEventArgs — the payload of drag_gesture_recognizer::drag_starting. Carries the mutable
    // DataPackage the handler fills in (Text / Image / custom Properties) before the bridge hands it to
    // the OS. The recognizer reads Cancel / Handled after the event to decide whether the drag proceeds.
    class drag_starting_event_args
    {
    public:
        drag_starting_event_args() = default;

        // DragStartingEventArgs.Data — the package that accompanies the drag source.
        [[nodiscard]] data_package& data()
        {
            return data_;
        }
        [[nodiscard]] const data_package& data() const
        {
            return data_;
        }

        // DragStartingEventArgs.Cancel — when true, the drag does not proceed (SendDragStarting returns
        // early without stamping the source or running the text/image fallback).
        [[nodiscard]] bool cancel() const
        {
            return cancel_;
        }
        void set_cancel(bool value)
        {
            cancel_ = value;
        }

        // DragStartingEventArgs.Handled (obsolete; Windows-only). Like Cancel, it short-circuits the
        // post-event processing and additionally suppresses the "DragSource" stamp.
        [[nodiscard]] bool handled() const
        {
            return handled_;
        }
        void set_handled(bool value)
        {
            handled_ = value;
        }

    private:
        data_package data_;
        bool cancel_ = false;
        bool handled_ = false;
    };

    // DropCompletedEventArgs — the payload of drag_gesture_recognizer::drop_completed. Empty in the port
    // (C#'s only members are the obsolete DropResult and the dropped PlatformArgs).
    class drop_completed_event_args
    {
    public:
        drop_completed_event_args() = default;
    };

    class drag_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // DragGestureRecognizer.CanDragProperty (default true).
        static const maui::core::bindable_property<bool>& can_drag_property();

        // Whether the attached element can be a drag source (DragGestureRecognizer.CanDrag).
        [[nodiscard]] bool can_drag() const
        {
            return can_drag_.get();
        }
        void set_can_drag(bool value)
        {
            can_drag_.set(value);
        }

        // DragGestureRecognizer.DragStarting / DropCompleted.
        maui::core::event<drag_starting_event_args&> drag_starting;
        maui::core::event<drop_completed_event_args> drop_completed;

        // DragGestureRecognizer.SendDragStarting: raise drag_starting (the handler fills in args.Data),
        // then — unless Cancel/Handled — stamp the source element into Data.PropertiesInternal["DragSource"]
        // and apply the text/image fallback. Arms the "drag active" latch SendDropCompleted gates on.
        // `sender` is the attached element (the drag source the fallback reads). Returns the args (the
        // bridge reads Data off them); the reference stays valid for the caller's expression (the args are
        // returned by value-stored move — see the out-of-line body).
        drag_starting_event_args send_drag_starting(element& sender);

        // DragGestureRecognizer.SendDropCompleted: raise drop_completed ONCE per drag (the _isDragActive
        // latch — Android fires Ended on every drop-handler view, but only the first gets through). A
        // call before any send_drag_starting (or a repeat after the first) is ignored.
        void send_drop_completed(const drop_completed_event_args& args);

    private:
        bool is_drag_active_ = false; // DragGestureRecognizer._isDragActive
        maui::core::property<bool> can_drag_{*this, can_drag_property()};
    };
} // namespace maui::controls

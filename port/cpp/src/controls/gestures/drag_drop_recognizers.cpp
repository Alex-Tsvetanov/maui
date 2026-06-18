// maui::controls — the send_* bodies for the drag & drop recognizers (drag_gesture_recognizer.hpp /
// drop_gesture_recognizer.hpp). These live in their own TU (not the recognizer headers) because they
// drive the cross-control text/image seam (drag_drop_data.hpp) — exactly as C#'s SendDragStarting /
// SendDrop call into ViewExtensions.GetStringValue / Element.TrySetValue. Ported from
// DragGestureRecognizer.cs (SendDragStarting / SendDropCompleted) and DropGestureRecognizer.cs (SendDrop)
// with the documented Command + image-extraction deviations (see the recognizer headers).

#include <algorithm>
#include <any>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/data_package.hpp"
#include "maui/controls/drag_drop_data.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/i_command.hpp" // (U-CMD) the *Command value type executed in the send_* bodies
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    namespace
    {
        // The key DragGestureRecognizer stamps the source element under (and DropGestureRecognizer reads
        // it back from) the package's INTERNAL property bag.
        constexpr std::string_view drag_source_key = "DragSource";

        // C#'s string.IsNullOrWhiteSpace: true for an unset value or one that is empty / all whitespace.
        bool is_null_or_white_space(const std::optional<std::string>& text)
        {
            if (!text.has_value())
            {
                return true;
            }
            return std::all_of(text->begin(), text->end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
        }
    } // namespace

    drag_starting_event_args drag_gesture_recognizer::send_drag_starting(element& sender)
    {
        drag_starting_event_args args;

        // DragStartingCommand?.Execute(DragStartingCommandParameter) — no CanExecute gate (see the header).
        if (const std::shared_ptr<i_command>& cmd = drag_starting_command())
        {
            cmd->execute(drag_starting_command_parameter());
        }
        drag_starting.raise(args);

        // if (!args.Handled) args.Data.PropertiesInternal.Add("DragSource", element);
        if (!args.handled())
        {
            args.data().properties_internal().set(std::string(drag_source_key), std::any(&sender));
        }

        // if (args.Cancel || args.Handled) return args;
        if (args.cancel() || args.handled())
        {
            return args;
        }

        is_drag_active_ = true;

        // if (args.Data.Image == null && element is IImageElement ie) args.Data.Image = ie.Source;
        // — the image AUTO-extraction is not ported (the image controls expose only a borrowed
        // i_image_source*, not the shared_ptr the package needs); a handler-set Data.Image is unaffected.

        // if (String.IsNullOrWhiteSpace(args.Data.Text)) args.Data.Text = element?.GetStringValue();
        if (is_null_or_white_space(args.data().text()))
        {
            if (auto extracted = get_string_value(sender))
            {
                args.data().set_text(std::move(extracted));
            }
        }

        return args;
    }

    void drag_gesture_recognizer::send_drop_completed(const drop_completed_event_args& args)
    {
        // DragGestureRecognizer.SendDropCompleted: the _isDragActive latch — only the first completion of
        // a given drag gets through (Android fires Ended on every drop-handler view).
        if (!is_drag_active_)
        {
            return;
        }
        is_drag_active_ = false;

        // DropCompletedCommand?.Execute(DropCompletedCommandParameter) — no CanExecute gate (see header).
        if (const std::shared_ptr<i_command>& cmd = drop_completed_command())
        {
            cmd->execute(drop_completed_command_parameter());
        }
        drop_completed.raise(args);
    }

    void drop_gesture_recognizer::send_drop(drop_event_args& args, element* parent) const
    {
        // DropGestureRecognizer.SendDrop: if (!AllowDrop) return;
        if (!allow_drop())
        {
            return;
        }

        // DropCommand?.Execute(DropCommandParameter) — no CanExecute gate (see the header).
        if (const std::shared_ptr<i_command>& cmd = drop_command())
        {
            cmd->execute(drop_command_parameter());
        }
        drop.raise(args);

        if (args.handled())
        {
            return;
        }

        const data_package_view& data_view = args.data();
        std::shared_ptr<maui::core::i_image_source> source_target = data_view.image();
        std::optional<std::string> text = data_view.text();

        // The dropped package's "DragSource" (the element the drag started on) is the text/image fallback.
        const element* drag_source = nullptr;
        if (const auto* stored = data_view.properties_internal().try_get_value(drag_source_key))
        {
            if (const auto* const* as_element = std::any_cast<element*>(stored))
            {
                drag_source = *as_element;
            }
        }
        if (drag_source != nullptr && is_null_or_white_space(text))
        {
            text = get_string_value(*drag_source);
        }

        if (parent == nullptr)
        {
            return; // C#'s Parent-null guard: no target control to inject onto.
        }

        // Parent is Image/ImageButton → set its Source from the dropped image (C#'s image-target ladder;
        // the Button.ImageSource branch is the documented gap in try_set_image_source).
        if (source_target != nullptr)
        {
            (void)try_set_image_source(*parent, std::move(source_target));
        }

        // Parent?.TrySetValue(text);
        if (text.has_value())
        {
            (void)try_set_string_value(*parent, *text);
        }
    }
} // namespace maui::controls

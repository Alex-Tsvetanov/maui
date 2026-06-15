#pragma once
// maui::core::i_time_picker  <=  Microsoft.Maui.ITimePicker
//
// The virtual-view contract for a time-of-day-selection view. Ported from
// src/Core/src/Core/ITimePicker.cs (ITimePicker : IView, ITextStyle). C#'s nullable TimeSpan? Time
// becomes std::optional<time_span>; Format is read-only on the contract, exactly as in C#.
//
// time() has BOTH accessors because the value flows both ways: the mapper pushes the virtual time to
// the native picker, and a native pick writes back through set_time (stored at from_handler
// specificity, like C#'s explicit ITimePicker.Time setter).
//
// is_open tracks whether the native dialog is visible and flows both ways (MapIsOpen pushes it native;
// the editing-begin/end callback writes it back — TimePickerHandler.iOS.cs sets IsFocused = IsOpen).
// The Opened/Closed events live on the control (TimePicker → time_picker.hpp).

#include <optional>
#include <string_view>

#include "maui/core/date_time.hpp"
#include "maui/core/i_text_style.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_time_picker : public i_view, public i_text_style
    {
    public:
        [[nodiscard]] virtual std::string_view format() const = 0;

        [[nodiscard]] virtual std::optional<time_span> time() const = 0;
        // Inbound channel: the handler writes the native pick back (FromHandler specificity).
        virtual void set_time(std::optional<time_span> value) = 0;

        [[nodiscard]] virtual bool is_open() const = 0;
        // Both ways: MapIsOpen pushes it native, the editing-begin/end callback writes it back.
        virtual void set_is_open(bool value) = 0;
    };
} // namespace maui::core

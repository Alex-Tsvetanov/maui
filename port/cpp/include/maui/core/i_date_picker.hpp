#pragma once
// maui::core::i_date_picker  <=  Microsoft.Maui.IDatePicker
//
// The virtual-view contract for a date-selection view. Ported from src/Core/src/Core/IDatePicker.cs
// (IDatePicker : IView, ITextStyle). C#'s nullable DateTime? properties become std::optional<date_time>.
//
// date() has BOTH accessors because the value flows both ways: the mapper pushes the virtual date to
// the native picker, and a native pick writes back through set_date (stored at from_handler
// specificity, like C#'s explicit IDatePicker.Date setter). format() likewise mirrors C#'s settable
// interface property. minimum_date()/maximum_date() are read-only on the contract, exactly as in C#.
//
// Deferred (documented, not stubbed): IsOpen + Opened/Closed (focus subsystem; see date_picker.hpp).

#include <optional>
#include <string_view>

#include "maui/core/date_time.hpp"
#include "maui/core/i_text_style.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_date_picker : public i_view, public i_text_style
    {
    public:
        [[nodiscard]] virtual std::string_view format() const = 0;
        virtual void set_format(std::string_view value) = 0;

        [[nodiscard]] virtual std::optional<date_time> date() const = 0;
        // Inbound channel: the handler writes the native pick back (FromHandler specificity).
        virtual void set_date(std::optional<date_time> value) = 0;

        [[nodiscard]] virtual std::optional<date_time> minimum_date() const = 0;
        [[nodiscard]] virtual std::optional<date_time> maximum_date() const = 0;
    };
} // namespace maui::core

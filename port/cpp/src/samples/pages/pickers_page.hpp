#pragma once
// pickers_page — a self-contained demo page for the W1-06 picker set: picker, date_picker and
// time_picker on one vertical stack, wired together so every selection drives a visible output (the
// C# gallery-page convention, code-first; the value_controls_page pattern).
//
// The page OWNS its whole element tree. It is backend-agnostic — a sample main attaches handlers
// bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios test trees
// exercise the same controls directly.
//
// Interactions demonstrated:
//   - the picker chooses a meeting room (Items-list flavor) and the readout echoes it
//     (selected_index_changed),
//   - the date_picker (clamped to the current year via Minimum/MaximumDate) and the time_picker feed
//     the same readout (date_selected / time_selected),
//   - the readout re-renders from all three on every change, formatting through the date_time
//     primitives the pickers share.

#include <optional>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/date_time.hpp"

namespace maui::samples
{
    class pickers_page
    {
    public:
        pickers_page()
        {
            page_.set_title("Pickers");
            stack_.set_spacing(12);

            // picker — the Items-list flavor with a placeholder Title.
            room_picker_.set_title("Pick a room");
            room_picker_.items().add("Auditorium");
            room_picker_.items().add("Boardroom");
            room_picker_.items().add("Cafeteria");
            room_picker_.selected_index_changed.connect([this] { update_readout(); });

            // date_picker — clamped to the current year.
            const int year = maui::core::date_time::today().year();
            meeting_date_.set_minimum_date(maui::core::date_time(year, 1, 1));
            meeting_date_.set_maximum_date(maui::core::date_time(year, 12, 31));
            meeting_date_.date_selected.connect(
                [this](const std::optional<maui::core::date_time>&, const std::optional<maui::core::date_time>&) {
                    update_readout();
                });

            // time_picker — 24h display format.
            meeting_time_.set_format("HH:mm");
            meeting_time_.set_time(maui::core::time_span(9, 0, 0));
            meeting_time_.time_selected.connect(
                [this](const std::optional<maui::core::time_span>&, const std::optional<maui::core::time_span>&) {
                    update_readout();
                });

            update_readout();

            stack_.add(room_picker_);
            stack_.add(meeting_date_);
            stack_.add(meeting_time_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::picker& room_picker()
        {
            return room_picker_;
        }
        [[nodiscard]] maui::controls::date_picker& meeting_date()
        {
            return meeting_date_;
        }
        [[nodiscard]] maui::controls::time_picker& meeting_time()
        {
            return meeting_time_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        void update_readout()
        {
            const int index = room_picker_.selected_index();
            std::string text = index >= 0 ? room_picker_.get_item(index) : std::string("No room");
            text += " on ";
            const auto date = meeting_date_.date();
            text += date ? maui::core::format_date_time(*date, "d") : std::string("(no date)");
            text += " at ";
            const auto time = meeting_time_.time();
            text += time ? maui::core::format_time_span(*time, meeting_time_.format()) : std::string("(no time)");
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::picker room_picker_;
        maui::controls::date_picker meeting_date_;
        maui::controls::time_picker meeting_time_;
        maui::controls::label readout_;
    };
} // namespace maui::samples

// Tests for the pickers_page demo (examples/gallery/pages/pickers_page.hpp) — backend-agnostic: the page
// is pure cross-platform control wiring, so this suite compiles in every preset and proves the
// demo's interactions (picker/date_picker/time_picker → readout) without a hosting main.
#include "examples/gallery/pages/pickers_page.hpp"

#include <string>

#include "maui/core/date_time.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::date_time;
    using maui::core::time_span;
    using maui::samples::pickers_page;

    TEST(pickers_page, builds_the_picker_stack)
    {
        pickers_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 4); // three pickers + the readout
        EXPECT_EQ(demo.room_picker().get_count(), 3);
        EXPECT_EQ(demo.room_picker().selected_index(), -1);
    }

    TEST(pickers_page, readout_starts_with_no_room_and_the_seeded_time)
    {
        pickers_page demo;
        const std::string today = maui::core::format_date_time(date_time::today(), "d");
        EXPECT_EQ(demo.readout().text(), "No room on " + today + " at 09:00");
    }

    TEST(pickers_page, picking_a_room_updates_the_readout)
    {
        pickers_page demo;
        demo.room_picker().set_selected_index(1);
        const std::string today = maui::core::format_date_time(date_time::today(), "d");
        EXPECT_EQ(demo.readout().text(), "Boardroom on " + today + " at 09:00");
    }

    TEST(pickers_page, date_and_time_changes_update_the_readout)
    {
        pickers_page demo;
        demo.room_picker().set_selected_index(0);
        const int year = date_time::today().year();
        demo.meeting_date().set_date(date_time(year, 6, 15));
        demo.meeting_time().set_time(time_span(14, 30, 0));
        const std::string date_text = maui::core::format_date_time(date_time(year, 6, 15), "d");
        EXPECT_EQ(demo.readout().text(), "Auditorium on " + date_text + " at 14:30");
    }

    TEST(pickers_page, date_clamps_to_the_current_year)
    {
        pickers_page demo;
        const int year = date_time::today().year();
        demo.meeting_date().set_date(date_time(year + 5, 1, 1));
        EXPECT_EQ(demo.meeting_date().date(), demo.meeting_date().maximum_date());
    }
} // namespace

// Tests for the maui::core::date_time / time_span primitives (date_time.hpp <= System.DateTime /
// System.TimeSpan, the picker-facing subset). Characterization against the documented C# behavior the
// pickers rely on: default values, .Date truncation, component extraction (incl. negative spans),
// Total* doubles, chronological ordering, and the DateTime.ToString format subset.
#include "maui/core/date_time.hpp"

#include <__chrono/duration.h> // include-cleaner: the chrono literals/durations live here on libc++
#include <chrono>

#include <gtest/gtest.h>

namespace
{
    using maui::core::date_time;
    using maui::core::format_date_time;
    using maui::core::format_time_span;
    using maui::core::time_span;
    using namespace std::chrono_literals;

    // ---- date_time ----

    TEST(date_time, default_is_year_one_january_first_midnight)
    {
        const date_time value;
        EXPECT_EQ(value.year(), 1);
        EXPECT_EQ(value.month(), 1U);
        EXPECT_EQ(value.day(), 1U);
        EXPECT_EQ(value.time_of_day(), 0ms);
    }

    TEST(date_time, components_round_trip)
    {
        const date_time value(2008, 5, 5);
        EXPECT_EQ(value.year(), 2008);
        EXPECT_EQ(value.month(), 5U);
        EXPECT_EQ(value.day(), 5U);
    }

    TEST(date_time, date_truncates_the_time_of_day)
    {
        const date_time value{date_time(2015, 7, 21).days(), 13h + 45min};
        EXPECT_EQ(value.time_of_day(), 13h + 45min);
        EXPECT_EQ(value.date(), date_time(2015, 7, 21));
        EXPECT_EQ(value.date().time_of_day(), 0ms);
    }

    TEST(date_time, orders_chronologically)
    {
        EXPECT_LT(date_time(1900, 1, 1), date_time(2100, 12, 31));
        EXPECT_LT(date_time(2020, 6, 15), date_time(2020, 6, 16));
        // Same day: the time-of-day breaks the tie.
        EXPECT_LT((date_time{date_time(2020, 6, 15).days(), 1h}), (date_time{date_time(2020, 6, 15).days(), 2h}));
        EXPECT_EQ(date_time(1970, 1, 1), date_time{std::chrono::sys_days{}});
    }

    TEST(date_time, today_is_now_truncated)
    {
        const date_time now = date_time::now();
        const date_time today = date_time::today();
        EXPECT_EQ(today.time_of_day(), 0ms);
        // now() was taken after today() resolved its day, but a midnight rollover between the two
        // calls is the only way they could differ — re-derive instead of flaking: today() must equal
        // either now().date() or the day before (the rollover edge).
        EXPECT_LE(now.date().days() - today.days(), std::chrono::days{1});
        EXPECT_GE(now.date(), today);
    }

    // ---- time_span ----

    TEST(time_span, default_is_zero)
    {
        const time_span value;
        EXPECT_EQ(value.value(), 0ms);
        EXPECT_EQ(value, time_span(0, 0, 0));
    }

    TEST(time_span, components_round_trip)
    {
        const time_span value(8, 30, 15);
        EXPECT_EQ(value.hours(), 8);
        EXPECT_EQ(value.minutes(), 30);
        EXPECT_EQ(value.seconds(), 15);
    }

    TEST(time_span, hours_overflow_a_day_into_total_hours)
    {
        const time_span value(1000, 0, 0); // the TestTimeOutOfRange probe value
        EXPECT_EQ(value.total_hours(), 1000.0);
        EXPECT_EQ(value.hours(), 1000 % 24); // the component wraps like TimeSpan.Hours
    }

    TEST(time_span, negative_span_has_negative_components_and_totals)
    {
        const time_span value(-1, 0, 0);
        EXPECT_EQ(value.hours(), -1);
        EXPECT_EQ(value.total_hours(), -1.0);
        EXPECT_LT(value.total_milliseconds(), 0.0);
    }

    TEST(time_span, orders_by_duration)
    {
        EXPECT_LT(time_span(9, 0, 0), time_span(17, 30, 0));
        EXPECT_LT(time_span(0, 0, 0), time_span(23, 59, 59));
    }

    // ---- format_date_time (the DateTime.ToString subset) ----

    TEST(date_time_format, standard_short_date)
    {
        EXPECT_EQ(format_date_time(date_time(2008, 5, 5), "d"), "5/5/2008");
        EXPECT_EQ(format_date_time(date_time(2011, 11, 30), "d"), "11/30/2011");
    }

    TEST(date_time_format, standard_long_date)
    {
        // 2008-05-05 was a Monday.
        EXPECT_EQ(format_date_time(date_time(2008, 5, 5), "D"), "Monday, May 5, 2008");
    }

    TEST(date_time_format, custom_date_patterns)
    {
        const date_time value(2006, 12, 20);
        EXPECT_EQ(format_date_time(value, "dd/MM/yyyy"), "20/12/2006");
        EXPECT_EQ(format_date_time(value, "yyyy-MM-dd"), "2006-12-20");
        EXPECT_EQ(format_date_time(value, "d.M.yy"), "20.12.06");
        EXPECT_EQ(format_date_time(value, "ddd MMM d"), "Wed Dec 20");
        EXPECT_EQ(format_date_time(value, "dddd, MMMM d"), "Wednesday, December 20");
    }

    TEST(date_time_format, standard_time_patterns)
    {
        const date_time afternoon{date_time(2020, 1, 1).days(), 17h + 30min + 5s};
        EXPECT_EQ(format_date_time(afternoon, "t"), "5:30 PM");
        EXPECT_EQ(format_date_time(afternoon, "T"), "5:30:05 PM");
        const date_time midnight{date_time(2020, 1, 1).days(), 0min};
        EXPECT_EQ(format_date_time(midnight, "t"), "12:00 AM"); // 12-hour clock: 0 renders as 12
        const date_time noon{date_time(2020, 1, 1).days(), 12h};
        EXPECT_EQ(format_date_time(noon, "t"), "12:00 PM");
    }

    TEST(date_time_format, custom_time_patterns)
    {
        const date_time morning{date_time(2020, 1, 1).days(), 8h + 5min + 9s};
        EXPECT_EQ(format_date_time(morning, "HH:mm"), "08:05");
        EXPECT_EQ(format_date_time(morning, "H:m:s"), "8:5:9");
        EXPECT_EQ(format_date_time(morning, "hh:mm tt"), "08:05 AM");
        EXPECT_EQ(format_date_time(morning, "h:mm t"), "8:05 A");
    }

    TEST(date_time_format, unknown_characters_pass_through_literally)
    {
        EXPECT_EQ(format_date_time(date_time(2006, 12, 20), "yyyy~MM~dd!"), "2006~12~20!");
    }

    // ---- format_time_span (TimeExtensions.ToFormattedString) ----

    TEST(time_span_format, default_format_is_short_time)
    {
        EXPECT_EQ(format_time_span(time_span(8, 30, 0), "t"), "8:30 AM");
        EXPECT_EQ(format_time_span(time_span(17, 30, 0), "t"), "5:30 PM");
        EXPECT_EQ(format_time_span(time_span(8, 30, 0), ""), "8:30 AM"); // empty falls back to "t"
    }

    TEST(time_span_format, custom_formats)
    {
        EXPECT_EQ(format_time_span(time_span(17, 30, 0), "HH:mm"), "17:30");
        EXPECT_EQ(format_time_span(time_span(23, 59, 59), "HH:mm:ss"), "23:59:59");
    }
} // namespace

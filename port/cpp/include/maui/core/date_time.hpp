#pragma once
// maui::core::date_time / maui::core::time_span  <=  System.DateTime / System.TimeSpan
//
// The port's stand-in for the two BCL date/time value types, scoped to EXACTLY what the pickers
// consume (W1-06): DatePicker holds a calendar date (DateTime coerced to .Date), TimePicker holds a
// time of day (TimeSpan validated to [0, 24h)), and both render display text through a DateTime-style
// format string. Built on std::chrono — the date is a sys_days (year_month_day-convertible), the
// time-of-day / elapsed time is a milliseconds duration. Minimal by design: NOT a general date
// library (no time zones, no calendars beyond the proleptic Gregorian std::chrono uses, no parsing).
//
// C# fidelity notes:
//   - default date_time = 0001-01-01 00:00 (default(DateTime)); default time_span = TimeSpan.Zero.
//   - date() truncates the time-of-day (DateTime.Date); components mirror Year/Month/Day/TimeOfDay.
//   - time_span components (hours/minutes/seconds) truncate toward zero like TimeSpan.Hours/...;
//     total_hours/total_milliseconds mirror the Total* doubles.
//   - now()/today() are LOCAL time, matching C# DateTime.Now/Today. They used to return the system UTC
//     clock, recorded here as a deviation whose "few-hour skew is immaterial" since the pickers only
//     clamp and format. It was NOT immaterial: measured on the android emulator at local 02:25 on
//     2026-08-22 (UTC+3), MAUI rendered 8/22/2026 and the port 8/21/2026 on the same device in the same
//     second. The offset comes from localtime_r/timegm (POSIX; _s/_mkgmtime on Windows), which keeps the
//     original no-libc++-tzdb constraint the deviation was written to satisfy.
//   - format_date_time ports the DateTime.ToString subset the pickers exercise (see its comment).

#include <chrono>
#include <compare>
#include <string>
#include <string_view>

namespace maui::core
{
    // maui::core::time_span  <=  System.TimeSpan (the picker-facing subset)
    class time_span
    {
    public:
        constexpr time_span() = default;
        // TimeSpan(hours, minutes, seconds) — components may be negative or overflow a day; the
        // pickers' validation (0 <= total < 24h) lives in the control, exactly as in C#.
        constexpr time_span(int hours, int minutes, int seconds)
            : value_(std::chrono::hours{hours} + std::chrono::minutes{minutes} + std::chrono::seconds{seconds})
        {
        }
        explicit constexpr time_span(std::chrono::milliseconds value) : value_(value)
        {
        }

        // The underlying duration (the port's "ticks").
        [[nodiscard]] constexpr std::chrono::milliseconds value() const
        {
            return value_;
        }

        // Component accessors (TimeSpan.Hours/Minutes/Seconds): truncated toward zero, hours wrap at
        // a day, minutes/seconds at their unit — negative spans yield negative components, like C#.
        [[nodiscard]] constexpr int hours() const
        {
            return static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(value_).count() % 24);
        }
        [[nodiscard]] constexpr int minutes() const
        {
            return static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(value_).count() % 60);
        }
        [[nodiscard]] constexpr int seconds() const
        {
            return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(value_).count() % 60);
        }

        // TimeSpan.TotalHours / TotalMilliseconds (the TimeProperty validateValue inputs).
        [[nodiscard]] constexpr double total_hours() const
        {
            return std::chrono::duration<double, std::ratio<3600>>(value_).count();
        }
        [[nodiscard]] constexpr double total_milliseconds() const
        {
            return std::chrono::duration<double, std::milli>(value_).count();
        }

        friend constexpr bool operator==(const time_span&, const time_span&) = default;
        friend constexpr auto operator<=>(const time_span&, const time_span&) = default;

    private:
        std::chrono::milliseconds value_{0};
    };

    // maui::core::date_time  <=  System.DateTime (the picker-facing subset)
    class date_time
    {
    public:
        // default(DateTime): 0001-01-01 00:00:00.
        constexpr date_time() = default;
        constexpr date_time(int year, unsigned month, unsigned day)
            : day_(std::chrono::sys_days{std::chrono::year{year} / std::chrono::month{month} / std::chrono::day{day}})
        {
        }
        explicit constexpr date_time(std::chrono::sys_days day, std::chrono::milliseconds time_of_day = {})
            : day_(day), time_(time_of_day)
        {
        }

        // DateTime.Today / DateTime.Now — on the UTC system clock (deviation documented above).
        [[nodiscard]] static date_time today();
        [[nodiscard]] static date_time now();

        // DateTime.Date: the same day at midnight (the DatePicker coercion truncation).
        [[nodiscard]] constexpr date_time date() const
        {
            return date_time{day_};
        }

        [[nodiscard]] constexpr std::chrono::sys_days days() const
        {
            return day_;
        }
        [[nodiscard]] constexpr std::chrono::year_month_day year_month_day() const
        {
            return std::chrono::year_month_day{day_};
        }
        [[nodiscard]] int year() const
        {
            return static_cast<int>(year_month_day().year());
        }
        [[nodiscard]] unsigned month() const
        {
            return static_cast<unsigned>(year_month_day().month());
        }
        [[nodiscard]] unsigned day() const
        {
            return static_cast<unsigned>(year_month_day().day());
        }
        // DateTime.TimeOfDay (as the raw duration).
        [[nodiscard]] constexpr std::chrono::milliseconds time_of_day() const
        {
            return time_;
        }

        // Chronological ordering: by day, then by time-of-day (member order is load-bearing).
        friend constexpr bool operator==(const date_time&, const date_time&) = default;
        friend constexpr auto operator<=>(const date_time&, const date_time&) = default;

    private:
        std::chrono::sys_days day_{std::chrono::year{1} / std::chrono::January / 1};
        std::chrono::milliseconds time_{0};
    };

    // DateTime.ToString(format) — the subset the pickers exercise, in the invariant (English) culture
    // with the en-US lean the iOS oracle hardcodes for times (TimePickerExtensions.iOS picks en-US
    // when the format contains 't'/'h'). Supported:
    //   standard: "d" (M/d/yyyy), "D" (dddd, MMMM d, yyyy), "t" (h:mm tt), "T" (h:mm:ss tt)
    //   custom:   d/dd/ddd/dddd, M/MM/MMM/MMMM, y/yy/yyy+/yyyy, h/hh, H/HH, m/mm, s/ss, t/tt;
    //             every other character is emitted literally.
    // UNSUPPORTED (documented, not silently wrong): the remaining standard specifiers (f/F/g/G/m/o/
    // r/s/u/y at full-string level), sub-second (f/F), era (g), time-zone (K/z), '\' escapes and
    // quoted literals, and culture-specific patterns — none are exercised by the ported oracle tests.
    [[nodiscard]] std::string format_date_time(const date_time& value, std::string_view format);

    // TimeSpan rendered as display text: ports TimeExtensions.ToFormattedString — anchor the span on
    // today's date and format with the DateTime patterns above (empty format falls back to "t").
    [[nodiscard]] std::string format_time_span(const time_span& value, std::string_view format);
} // namespace maui::core

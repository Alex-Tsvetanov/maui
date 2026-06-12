// maui::core::date_time — out-of-line definitions: the UTC-clock now()/today() factories and the
// DateTime.ToString format subset (see date_time.hpp for the supported/unsupported specifier list).
// The names are invariant-culture English (the en-US lean the iOS time oracle hardcodes).

#include "maui/core/date_time.hpp"

#include <__chrono/duration.h> // include-cleaner: std::chrono::days lives here on libc++
#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

namespace maui::core
{
    namespace
    {
        constexpr std::array<std::string_view, 12> k_month_names = {"January",   "February", "March",    "April",
                                                                    "May",       "June",     "July",     "August",
                                                                    "September", "October",  "November", "December"};
        // Indexed by std::chrono::weekday::c_encoding() (0 = Sunday).
        constexpr std::array<std::string_view, 7> k_weekday_names = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                                                                     "Thursday", "Friday", "Saturday"};

        void append_number(std::string& out, long long value, std::size_t min_digits)
        {
            const std::string digits = std::to_string(value);
            for (std::size_t pad = digits.size(); pad < min_digits; ++pad)
            {
                out += '0';
            }
            out += digits;
        }

        // Expand one specifier run (`repeat` consecutive copies of `specifier`) into `out`.
        void append_token(std::string& out, char specifier, std::size_t repeat, const date_time& value)
        {
            const std::chrono::year_month_day ymd = value.year_month_day();
            const auto time = value.time_of_day();
            const auto hours24 = std::chrono::duration_cast<std::chrono::hours>(time).count() % 24;
            switch (specifier)
            {
                case 'd':
                    if (repeat >= 4)
                    {
                        out += k_weekday_names.at(std::chrono::weekday{value.days()}.c_encoding());
                    }
                    else if (repeat == 3)
                    {
                        out += k_weekday_names.at(std::chrono::weekday{value.days()}.c_encoding()).substr(0, 3);
                    }
                    else
                    {
                        append_number(out, static_cast<unsigned>(ymd.day()), repeat);
                    }
                    break;
                case 'M':
                    if (repeat >= 4)
                    {
                        out += k_month_names.at(static_cast<unsigned>(ymd.month()) - 1);
                    }
                    else if (repeat == 3)
                    {
                        out += k_month_names.at(static_cast<unsigned>(ymd.month()) - 1).substr(0, 3);
                    }
                    else
                    {
                        append_number(out, static_cast<unsigned>(ymd.month()), repeat);
                    }
                    break;
                case 'y':
                    // C#: y/yy are the two-digit year (yy zero-padded), yyy+ the full year zero-padded.
                    if (repeat <= 2)
                    {
                        append_number(out, static_cast<int>(ymd.year()) % 100, repeat);
                    }
                    else
                    {
                        append_number(out, static_cast<int>(ymd.year()), repeat);
                    }
                    break;
                case 'h': {
                    const long long hour12 = hours24 % 12 == 0 ? 12 : hours24 % 12;
                    append_number(out, hour12, repeat);
                    break;
                }
                case 'H':
                    append_number(out, hours24, repeat);
                    break;
                case 'm':
                    append_number(out, std::chrono::duration_cast<std::chrono::minutes>(time).count() % 60, repeat);
                    break;
                case 's':
                    append_number(out, std::chrono::duration_cast<std::chrono::seconds>(time).count() % 60, repeat);
                    break;
                case 't':
                    out += repeat >= 2 ? (hours24 < 12 ? "AM" : "PM") : (hours24 < 12 ? "A" : "P");
                    break;
                default:
                    // Unsupported specifier (documented in date_time.hpp): emit literally.
                    out.append(repeat, specifier);
                    break;
            }
        }

        constexpr bool is_specifier(char candidate)
        {
            return candidate == 'd' || candidate == 'M' || candidate == 'y' || candidate == 'h' || candidate == 'H' ||
                   candidate == 'm' || candidate == 's' || candidate == 't';
        }
    } // namespace

    date_time date_time::now()
    {
        const auto now = std::chrono::system_clock::now();
        const auto day = std::chrono::floor<std::chrono::days>(now);
        return date_time{std::chrono::sys_days{day}, std::chrono::duration_cast<std::chrono::milliseconds>(now - day)};
    }

    date_time date_time::today()
    {
        return now().date();
    }

    std::string format_date_time(const date_time& value, std::string_view format)
    {
        // The standard single-character formats the pickers exercise, expanded to custom patterns.
        if (format == "d")
        {
            format = "M/d/yyyy"; // the invariant short-date pattern
        }
        else if (format == "D")
        {
            format = "dddd, MMMM d, yyyy"; // the en-US long-date pattern (iOS: NSDateFormatterStyle.Full)
        }
        else if (format == "t")
        {
            format = "h:mm tt"; // en-US short time (TimePickerExtensions.iOS picks en-US for 't'/'h')
        }
        else if (format == "T")
        {
            format = "h:mm:ss tt";
        }

        std::string out;
        out.reserve(format.size() + 8);
        for (std::size_t at = 0; at < format.size();)
        {
            const char current = format[at];
            if (is_specifier(current))
            {
                std::size_t run = 1;
                while (at + run < format.size() && format[at + run] == current)
                {
                    ++run;
                }
                append_token(out, current, run, value);
                at += run;
            }
            else
            {
                out += current;
                ++at;
            }
        }
        return out;
    }

    std::string format_time_span(const time_span& value, std::string_view format)
    {
        // TimeExtensions.ToFormattedString: DateTime.Today.Add(time).ToString(format); an empty format
        // falls back to the short-time pattern.
        if (format.empty())
        {
            format = "t";
        }
        const date_time anchored{date_time::today().days(),
                                 std::chrono::duration_cast<std::chrono::milliseconds>(value.value())};
        return format_date_time(anchored, format);
    }
} // namespace maui::core

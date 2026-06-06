#pragma once
// Internal parse helpers shared by the point/size/rect TryParse ports — NOT public API.
// C#'s TypeConverters call value.Split(',') with a fixed expected part count, then parse each part.
// These mirror that: trim whitespace, parse a whole numeric token, and split on exactly 1 or 3 commas.

#include <charconv>
#include <cstddef>
#include <string_view>
#include <system_error>

namespace maui::graphics::detail
{
    inline std::string_view ps_trim(std::string_view s)
    {
        std::size_t b = 0;
        std::size_t e = s.size();
        auto ws = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; };
        while (b < e && ws(s[b]))
        {
            ++b;
        }
        while (e > b && ws(s[e - 1]))
        {
            --e;
        }
        return s.substr(b, e - b);
    }

    template <class T>
    inline bool ps_parse_num(std::string_view s, T &out)
    {
        s = ps_trim(s);
        if (s.empty())
        {
            return false;
        }
        const char *begin = s.data();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) — from_chars needs [first,last)
        const char *end = begin + s.size();
        auto [ptr, ec] = std::from_chars(begin, end, out, std::chars_format::general);
        return ec == std::errc{} && ptr == end; // whole token consumed (rejects "100#", etc.)
    }

    // value.Split(',') of length 2: exactly one comma -> two parts.
    inline bool ps_split2(std::string_view s, std::string_view &a, std::string_view &b)
    {
        if (s.empty())
        {
            return false;
        }
        auto const c = s.find(',');
        if (c == std::string_view::npos)
        {
            return false;
        }
        a = s.substr(0, c);
        b = s.substr(c + 1);
        return b.find(',') == std::string_view::npos;
    }

    // value.Split(',') of length 4: exactly three commas -> four parts.
    inline bool ps_split4(std::string_view s, std::string_view &a, std::string_view &b, std::string_view &c,
                          std::string_view &d)
    {
        if (s.empty())
        {
            return false;
        }
        auto const i1 = s.find(',');
        if (i1 == std::string_view::npos)
        {
            return false;
        }
        auto const i2 = s.find(',', i1 + 1);
        if (i2 == std::string_view::npos)
        {
            return false;
        }
        auto const i3 = s.find(',', i2 + 1);
        if (i3 == std::string_view::npos)
        {
            return false;
        }
        a = s.substr(0, i1);
        b = s.substr(i1 + 1, i2 - i1 - 1);
        c = s.substr(i2 + 1, i3 - i2 - 1);
        d = s.substr(i3 + 1);
        return d.find(',') == std::string_view::npos; // > 4 parts -> fail
    }
}

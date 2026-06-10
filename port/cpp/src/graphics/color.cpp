// maui::graphics::color  <=  Microsoft.Maui.Graphics.Color  (+ ColorUtils.cs, Colors.cs)
#include "maui/graphics/color.hpp"

#include "maui/detail/charconv_compat.hpp" // FP from_chars (general) with the libc++ < 20 fallback
#include "maui/graphics/colors.hpp"        // MAUI_GRAPHICS_NAMED_COLORS (the parse table)
#include "maui/graphics/vector4.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>

namespace maui::graphics
{
    namespace
    {

        constexpr float k_epsilon = 0.0000000001F; // GeometryUtil.Epsilon

        float clamp01(float v)
        {
            return std::clamp(v, 0.0F, 1.0F);
        }

        std::string_view trim(std::string_view s)
        {
            std::size_t b = 0;
            std::size_t e = s.size();
            while (b < e && std::isspace(static_cast<unsigned char>(s[b])) != 0)
            {
                ++b;
            }
            while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1])) != 0)
            {
                --e;
            }
            return s.substr(b, e - b);
        }

        bool starts_with_ci(std::string_view s, std::string_view prefix)
        {
            if (s.size() < prefix.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < prefix.size(); ++i)
            {
                if (std::tolower(static_cast<unsigned char>(s[i])) !=
                    std::tolower(static_cast<unsigned char>(prefix[i])))
                {
                    return false;
                }
            }
            return true;
        }

        int hex_digit(char c)
        {
            if (c >= '0' && c <= '9')
            {
                return c - '0';
            }
            if (c >= 'a' && c <= 'f')
            {
                return (c - 'a') + 10;
            }
            if (c >= 'A' && c <= 'F')
            {
                return (c - 'A') + 10;
            }
            return -1;
        }

        // Parse exactly two hex digits (NumberStyles.AllowHexSpecifier: no sign, no whitespace).
        std::optional<int> parse_hex2(char hi, char lo)
        {
            const int h = hex_digit(hi);
            const int l = hex_digit(lo);
            if (h < 0 || l < 0)
            {
                return std::nullopt;
            }
            return (h * 16) + l;
        }

        // ColorUtils.FromArgb: #aarrggbb / #rrggbb / #argb / #rgb. nullopt only when a hex pair is
        // invalid (a valid-length string with bad digits — C# throws there, caught by TryParse).
        // Unknown lengths yield opaque black, matching C#.
        std::optional<std::array<float, 4>> argb_from_hex(std::string_view v)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 255;
            if (!v.empty())
            {
                if (v.front() == '#')
                {
                    v.remove_prefix(1);
                }
                if (v.size() == 6)
                {
                    const auto rr = parse_hex2(v[0], v[1]);
                    const auto gg = parse_hex2(v[2], v[3]);
                    const auto bb = parse_hex2(v[4], v[5]);
                    if (!rr || !gg || !bb)
                    {
                        return std::nullopt;
                    }
                    r = *rr;
                    g = *gg;
                    b = *bb;
                }
                else if (v.size() == 3)
                { // #RGB
                    const auto rr = parse_hex2(v[0], v[0]);
                    const auto gg = parse_hex2(v[1], v[1]);
                    const auto bb = parse_hex2(v[2], v[2]);
                    if (!rr || !gg || !bb)
                    {
                        return std::nullopt;
                    }
                    r = *rr;
                    g = *gg;
                    b = *bb;
                }
                else if (v.size() == 4)
                { // #ARGB
                    const auto aa = parse_hex2(v[0], v[0]);
                    const auto rr = parse_hex2(v[1], v[1]);
                    const auto gg = parse_hex2(v[2], v[2]);
                    const auto bb = parse_hex2(v[3], v[3]);
                    if (!aa || !rr || !gg || !bb)
                    {
                        return std::nullopt;
                    }
                    a = *aa;
                    r = *rr;
                    g = *gg;
                    b = *bb;
                }
                else if (v.size() == 8)
                { // #AARRGGBB
                    const auto aa = parse_hex2(v[0], v[1]);
                    const auto rr = parse_hex2(v[2], v[3]);
                    const auto gg = parse_hex2(v[4], v[5]);
                    const auto bb = parse_hex2(v[6], v[7]);
                    if (!aa || !rr || !gg || !bb)
                    {
                        return std::nullopt;
                    }
                    a = *aa;
                    r = *rr;
                    g = *gg;
                    b = *bb;
                }
                // other lengths: leave defaults -> opaque black
            }
            return std::array<float, 4>{static_cast<float>(r) / 255.0F, static_cast<float>(g) / 255.0F,
                                        static_cast<float>(b) / 255.0F, static_cast<float>(a) / 255.0F};
        }

        // ColorUtils.FromRgba: #rrggbbaa / #rrggbb / #rgba / #rgb (lengths 6/3 defer to ARGB ordering).
        std::optional<std::array<float, 4>> rgba_from_hex(std::string_view v)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 255;
            if (!v.empty())
            {
                if (v.front() == '#')
                {
                    v.remove_prefix(1);
                }
                if (v.size() == 6 || v.size() == 3)
                {
                    return argb_from_hex(v); // no alpha -> same as ARGB
                }
                if (v.size() == 4)
                { // #RGBA
                    const auto rr = parse_hex2(v[0], v[0]);
                    const auto gg = parse_hex2(v[1], v[1]);
                    const auto bb = parse_hex2(v[2], v[2]);
                    const auto aa = parse_hex2(v[3], v[3]);
                    if (!rr || !gg || !bb || !aa)
                    {
                        return std::nullopt;
                    }
                    r = *rr;
                    g = *gg;
                    b = *bb;
                    a = *aa;
                }
                else if (v.size() == 8)
                { // #RRGGBBAA
                    const auto rr = parse_hex2(v[0], v[1]);
                    const auto gg = parse_hex2(v[2], v[3]);
                    const auto bb = parse_hex2(v[4], v[5]);
                    const auto aa = parse_hex2(v[6], v[7]);
                    if (!rr || !gg || !bb || !aa)
                    {
                        return std::nullopt;
                    }
                    r = *rr;
                    g = *gg;
                    b = *bb;
                    a = *aa;
                }
            }
            return std::array<float, 4>{static_cast<float>(r) / 255.0F, static_cast<float>(g) / 255.0F,
                                        static_cast<float>(b) / 255.0F, static_cast<float>(a) / 255.0F};
        }

        bool try_parse_double(std::string_view s, double& out)
        {
            s = trim(s);
            if (s.empty())
            {
                return false;
            }
            const char* begin = s.data();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) -- std::from_chars takes a pointer range
            const char* end = begin + s.size();
            auto [ptr, ec] = maui::detail::from_chars_general(begin, end, out);
            return ec == std::errc{} && ptr == end; // entire token must be consumed
        }

        // ColorUtils.TryParseColorValue: optional trailing '%', clamp to [0,maxValue], normalize to 0-1.
        bool try_parse_color_value(std::string_view elem, int max_value, bool accept_percent, double& out)
        {
            elem = trim(elem);
            if (!elem.empty() && elem.back() == '%' && accept_percent)
            {
                max_value = 100;
                elem = elem.substr(0, elem.size() - 1);
            }
            double v = 0;
            if (try_parse_double(elem, v))
            {
                out = std::clamp(v, 0.0, static_cast<double>(max_value)) / max_value;
                return true;
            }
            return false;
        }

        bool try_parse_opacity(std::string_view elem, double& out)
        {
            double v = 0;
            if (try_parse_double(elem, v))
            {
                out = std::clamp(v, 0.0, 1.0);
                return true;
            }
            return false;
        }

        bool four_ranges(std::string_view v, std::string_view& q0, std::string_view& q1, std::string_view& q2,
                         std::string_view& q3)
        {
            const auto op = v.find('(');
            const auto cp = v.rfind(')');
            if (op == std::string_view::npos || cp == std::string_view::npos || cp < op)
            {
                return false;
            }
            v = v.substr(op + 1, cp - op - 1);
            auto i = v.find(',');
            if (i == std::string_view::npos)
            {
                return false;
            }
            q0 = v.substr(0, i);
            v = v.substr(i + 1);
            i = v.find(',');
            if (i == std::string_view::npos)
            {
                return false;
            }
            q1 = v.substr(0, i);
            v = v.substr(i + 1);
            i = v.find(',');
            if (i == std::string_view::npos)
            {
                return false;
            }
            q2 = v.substr(0, i);
            q3 = v.substr(i + 1);
            return !q3.contains(','); // a trailing comma means too many ranges
        }

        bool three_ranges(std::string_view v, std::string_view& t0, std::string_view& t1, std::string_view& t2)
        {
            const auto op = v.find('(');
            const auto cp = v.rfind(')');
            if (op == std::string_view::npos || cp == std::string_view::npos || cp < op)
            {
                return false;
            }
            v = v.substr(op + 1, cp - op - 1);
            auto i = v.find(',');
            if (i == std::string_view::npos)
            {
                return false;
            }
            t0 = v.substr(0, i);
            v = v.substr(i + 1);
            i = v.find(',');
            if (i == std::string_view::npos)
            {
                return false;
            }
            t1 = v.substr(0, i);
            t2 = v.substr(i + 1);
            return !t2.contains(',');
        }

        // ColorUtils.ConvertHslToRgb — float math mirrored exactly (double only in the * comparisons).
        std::array<float, 3> convert_hsl_to_rgb(float hue, float saturation, float luminosity)
        {
            if (luminosity == 0)
            {
                return {0, 0, 0};
            }
            if (saturation == 0)
            {
                return {luminosity, luminosity, luminosity};
            }

            const float temp2 = luminosity <= 0.5F ? luminosity * (1.0F + saturation)
                                                   : (luminosity + saturation) - (luminosity * saturation);
            const float temp1 = (2.0F * luminosity) - temp2;

            std::array<float, 3> t3 = {hue + (1.0F / 3.0F), hue, hue - (1.0F / 3.0F)};
            std::array<float, 3> clr = {0, 0, 0};
            for (int i = 0; i < 3; i++)
            {
                if (t3.at(i) < 0)
                {
                    t3.at(i) += 1.0F;
                }
                if (t3.at(i) > 1)
                {
                    t3.at(i) -= 1.0F;
                }
                if (6.0 * t3.at(i) < 1.0)
                {
                    clr.at(i) = temp1 + ((temp2 - temp1) * t3.at(i) * 6.0F);
                }
                else if (2.0 * t3.at(i) < 1.0)
                {
                    clr.at(i) = temp2;
                }
                else if (3.0 * t3.at(i) < 2.0)
                {
                    clr.at(i) = temp1 + ((temp2 - temp1) * ((2.0F / 3.0F) - t3.at(i)) * 6.0F);
                }
                else
                {
                    clr.at(i) = temp1;
                }
            }
            return {clr[0], clr[1], clr[2]};
        }

        // ColorUtils.ConvertHsvToRgb — note f/q/t are double in C#, p is float.
        std::array<float, 3> convert_hsv_to_rgb(float h, float s, float v)
        {
            h = clamp01(h);
            s = clamp01(s);
            v = clamp01(v);

            const float h6 = h * 6.0F;
            const int range = static_cast<int>(std::floor(static_cast<double>(h6))) % 6;
            const double f = static_cast<double>(h6) - std::floor(static_cast<double>(h6));
            const float p = v * (1 - s);
            const double q = v * (1 - (f * s));
            const double t = v * (1 - ((1 - f) * s));

            switch (range)
            {
                case 0:
                    return {v, static_cast<float>(t), p};
                case 1:
                    return {static_cast<float>(q), v, p};
                case 2:
                    return {p, v, static_cast<float>(t)};
                case 3:
                    return {p, static_cast<float>(q), v};
                case 4:
                    return {static_cast<float>(t), p, v};
                default:
                    return {v, p, static_cast<float>(q)};
            }
        }

        const std::unordered_map<std::string_view, std::uint32_t>& named_color_table()
        {
            // NOLINTNEXTLINE(cppcoreguidelines-macro-usage) — X-macro is the idiomatic single-source table.
#define MAUI_GRAPHICS_ENTRY(name, str, argb) {str, argb},
            static const std::unordered_map<std::string_view, std::uint32_t> table = {
                MAUI_GRAPHICS_NAMED_COLORS(MAUI_GRAPHICS_ENTRY)};
#undef MAUI_GRAPHICS_ENTRY
            return table;
        }

        // Color.GetNamedColor: case-insensitive; "default" is intentionally absent (-> not found).
        std::optional<color> named_color(std::string_view value)
        {
            std::string lowered;
            lowered.reserve(value.size());
            for (const char c : value)
            {
                lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            const auto& table = named_color_table();
            const auto it = table.find(lowered);
            if (it == table.end())
            {
                return std::nullopt;
            }
            return color::from_uint(it->second);
        }

        bool color_utils_try_parse(std::string_view value, float& red, float& green, float& blue, float& alpha)
        {
            red = green = blue = alpha = 0.0F;
            value = trim(value);
            if (value.empty())
            {
                return false;
            }

            if (value.front() == '#')
            {
                const auto res = argb_from_hex(value);
                if (!res)
                {
                    return false;
                }
                red = (*res)[0];
                green = (*res)[1];
                blue = (*res)[2];
                alpha = (*res)[3];
                return true;
            }

            std::string_view a;
            std::string_view b;
            std::string_view c;
            std::string_view d;
            if (starts_with_ci(value, "rgba"))
            {
                if (!four_ranges(value, a, b, c, d))
                {
                    return false;
                }
                double r = 0;
                double g = 0;
                double bl = 0;
                double al = 0;
                bool ok = try_parse_color_value(a, 255, true, r);
                ok &= try_parse_color_value(b, 255, true, g);
                ok &= try_parse_color_value(c, 255, true, bl);
                ok &= try_parse_opacity(d, al);
                if (!ok)
                {
                    return false;
                }
                red = static_cast<float>(r);
                green = static_cast<float>(g);
                blue = static_cast<float>(bl);
                alpha = static_cast<float>(al);
                return true;
            }
            if (starts_with_ci(value, "rgb"))
            {
                if (!three_ranges(value, a, b, c))
                {
                    return false;
                }
                double r = 0;
                double g = 0;
                double bl = 0;
                bool ok = try_parse_color_value(a, 255, true, r);
                ok &= try_parse_color_value(b, 255, true, g);
                ok &= try_parse_color_value(c, 255, true, bl);
                if (!ok)
                {
                    return false;
                }
                red = static_cast<float>(r);
                green = static_cast<float>(g);
                blue = static_cast<float>(bl);
                alpha = 1.0F;
                return true;
            }
            if (starts_with_ci(value, "hsla"))
            {
                if (!four_ranges(value, a, b, c, d))
                {
                    return false;
                }
                double h = 0;
                double s = 0;
                double l = 0;
                double al = 0;
                bool ok = try_parse_color_value(a, 360, false, h);
                ok &= try_parse_color_value(b, 100, true, s);
                ok &= try_parse_color_value(c, 100, true, l);
                ok &= try_parse_opacity(d, al);
                if (!ok)
                {
                    return false;
                }
                const auto rgb =
                    convert_hsl_to_rgb(static_cast<float>(h), static_cast<float>(s), static_cast<float>(l));
                red = rgb[0];
                green = rgb[1];
                blue = rgb[2];
                alpha = static_cast<float>(al);
                return true;
            }
            if (starts_with_ci(value, "hsl"))
            {
                if (!three_ranges(value, a, b, c))
                {
                    return false;
                }
                double h = 0;
                double s = 0;
                double l = 0;
                bool ok = try_parse_color_value(a, 360, false, h);
                ok &= try_parse_color_value(b, 100, true, s);
                ok &= try_parse_color_value(c, 100, true, l);
                if (!ok)
                {
                    return false;
                }
                const auto rgb =
                    convert_hsl_to_rgb(static_cast<float>(h), static_cast<float>(s), static_cast<float>(l));
                red = rgb[0];
                green = rgb[1];
                blue = rgb[2];
                alpha = 1.0F;
                return true;
            }
            if (starts_with_ci(value, "hsva"))
            {
                if (!four_ranges(value, a, b, c, d))
                {
                    return false;
                }
                double h = 0;
                double s = 0;
                double v = 0;
                double al = 0;
                bool ok = try_parse_color_value(a, 360, false, h);
                ok &= try_parse_color_value(b, 100, true, s);
                ok &= try_parse_color_value(c, 100, true, v);
                ok &= try_parse_opacity(d, al);
                if (!ok)
                {
                    return false;
                }
                const auto rgb =
                    convert_hsv_to_rgb(static_cast<float>(h), static_cast<float>(s), static_cast<float>(v));
                red = rgb[0];
                green = rgb[1];
                blue = rgb[2];
                alpha = static_cast<float>(al);
                return true;
            }
            if (starts_with_ci(value, "hsv"))
            {
                if (!three_ranges(value, a, b, c))
                {
                    return false;
                }
                double h = 0;
                double s = 0;
                double v = 0;
                bool ok = try_parse_color_value(a, 360, false, h);
                ok &= try_parse_color_value(b, 100, true, s);
                ok &= try_parse_color_value(c, 100, true, v);
                if (!ok)
                {
                    return false;
                }
                const auto rgb =
                    convert_hsv_to_rgb(static_cast<float>(h), static_cast<float>(s), static_cast<float>(v));
                red = rgb[0];
                green = rgb[1];
                blue = rgb[2];
                alpha = 1.0F;
                return true;
            }
            return false;
        }

        std::string hex_byte(float value)
        {
            const int v = static_cast<int>(255.0F * value); // truncates, matching C# (int) cast
            return std::format("{:02X}", v);
        }

    } // namespace

    // ---- constructors ----
    color::color(float gray) : red(clamp01(gray)), green(red), blue(red)
    {
    }
    color::color(float r, float g, float b) : red(clamp01(r)), green(clamp01(g)), blue(clamp01(b))
    {
    }
    color::color(float r, float g, float b, float a)
        : red(clamp01(r)), green(clamp01(g)), blue(clamp01(b)), alpha(clamp01(a))
    {
    }
    color::color(const vector4& v) : color(v.x, v.y, v.z, v.w) // RGBA, clamped by the 4-float ctor
    {
    }

    // ---- integer factories (0-255) ----
    color color::from_rgb(int r, int g, int b)
    {
        return {static_cast<float>(r) / 255.0F, static_cast<float>(g) / 255.0F, static_cast<float>(b) / 255.0F};
    }
    color color::from_rgba(int r, int g, int b, int a)
    {
        return {static_cast<float>(r) / 255.0F, static_cast<float>(g) / 255.0F, static_cast<float>(b) / 255.0F,
                static_cast<float>(a) / 255.0F};
    }

    color color::from_uint(std::uint32_t argb)
    {
        return from_rgba(static_cast<int>((argb & 0x00ff0000U) >> 16U), static_cast<int>((argb & 0x0000ff00U) >> 8U),
                         static_cast<int>(argb & 0x000000ffU), static_cast<int>((argb & 0xff000000U) >> 24U));
    }
    color color::from_int(int argb)
    {
        return from_uint(static_cast<std::uint32_t>(argb));
    }

    // ---- hex ----
    color color::from_argb(std::string_view color_as_hex)
    {
        const auto res = argb_from_hex(color_as_hex);
        const auto v = res.value_or(std::array<float, 4>{0, 0, 0, 1}); // bad hex -> black (public path)
        return {v[0], v[1], v[2], v[3]};
    }
    color color::from_rgba(std::string_view color_as_hex)
    {
        const auto res = rgba_from_hex(color_as_hex);
        const auto v = res.value_or(std::array<float, 4>{0, 0, 0, 1});
        return {v[0], v[1], v[2], v[3]};
    }
    color color::from_hex(std::string_view color_as_argb_hex)
    {
        return from_argb(color_as_argb_hex);
    }

    // ---- HSL / HSV ----
    color color::from_hsla(float h, float s, float l, float a)
    {
        const auto rgb = convert_hsl_to_rgb(h, s, l);
        return {rgb[0], rgb[1], rgb[2], a};
    }
    color color::from_hsla(double h, double s, double l, double a)
    {
        const auto rgb = convert_hsl_to_rgb(static_cast<float>(h), static_cast<float>(s), static_cast<float>(l));
        return {rgb[0], rgb[1], rgb[2], static_cast<float>(a)};
    }
    color color::from_hsva_unit(float h, float s, float v, float a)
    {
        const auto rgb = convert_hsv_to_rgb(h, s, v);
        return {rgb[0], rgb[1], rgb[2], a};
    }
    color color::from_hsv(int h, int s, int v)
    {
        return from_hsva_unit(static_cast<float>(h) / 360.0F, static_cast<float>(s) / 100.0F,
                              static_cast<float>(v) / 100.0F, 1.0F);
    }
    color color::from_hsva(int h, int s, int v, int a)
    {
        return from_hsva_unit(static_cast<float>(h) / 360.0F, static_cast<float>(s) / 100.0F,
                              static_cast<float>(v) / 100.0F, static_cast<float>(a) / 100.0F);
    }

    // ---- parse ----
    bool color::try_parse(std::string_view value, color& out)
    {
        float r = 0;
        float g = 0;
        float b = 0;
        float a = 0;
        if (color_utils_try_parse(value, r, g, b, a))
        {
            out = color(r, g, b, a);
            return true;
        }
        if (const auto nc = named_color(value))
        {
            out = *nc;
            return true;
        }
        return false;
    }
    color color::parse(std::string_view value)
    {
        color c;
        if (try_parse(value, c))
        {
            return c;
        }
        throw std::invalid_argument("Cannot convert \"" + std::string(value) + "\" into maui::graphics::color");
    }

    // ---- conversions ----
    void color::to_rgba(std::uint8_t& r, std::uint8_t& g, std::uint8_t& b, std::uint8_t& a) const
    {
        a = static_cast<std::uint8_t>(alpha * 255.0F);
        r = static_cast<std::uint8_t>(red * 255.0F);
        g = static_cast<std::uint8_t>(green * 255.0F);
        b = static_cast<std::uint8_t>(blue * 255.0F);
    }
    void color::to_rgb(std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const
    {
        std::uint8_t a = 0;
        to_rgba(r, g, b, a);
    }
    vector4 color::to_vector4() const
    {
        return {red, green, blue, alpha};
    }
    int color::to_int() const
    {
        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 0;
        to_rgba(r, g, b, a);
        const std::uint32_t argb = (static_cast<std::uint32_t>(a) << 24U) | (static_cast<std::uint32_t>(r) << 16U) |
                                   (static_cast<std::uint32_t>(g) << 8U) | static_cast<std::uint32_t>(b);
        return static_cast<int>(argb);
    }
    std::uint32_t color::to_uint() const
    {
        return static_cast<std::uint32_t>(to_int());
    }

    // ---- hex out ----
    std::string color::to_hex() const
    {
        return "#" + hex_byte(red) + hex_byte(green) + hex_byte(blue);
    }
    std::string color::to_argb_hex(bool include_alpha) const
    {
        if (include_alpha || alpha < 1)
        {
            return "#" + hex_byte(alpha) + hex_byte(red) + hex_byte(green) + hex_byte(blue);
        }
        return "#" + hex_byte(red) + hex_byte(green) + hex_byte(blue);
    }
    std::string color::to_rgba_hex(bool include_alpha) const
    {
        if (include_alpha || alpha < 1)
        {
            return "#" + hex_byte(red) + hex_byte(green) + hex_byte(blue) + hex_byte(alpha);
        }
        return "#" + hex_byte(red) + hex_byte(green) + hex_byte(blue);
    }

    // ---- HSL accessors / modifiers ----
    void color::to_hsl(float& h, float& s, float& l) const
    {
        const float r = red;
        const float g = green;
        const float b = blue;
        float v = std::max(r, g);
        v = std::max(v, b);
        float m = std::min(r, g);
        m = std::min(m, b);

        l = (m + v) / 2.0F;
        if (l <= 0.0F)
        {
            h = s = l = 0;
            return;
        }
        const float vm = v - m;
        s = vm;
        if (s > 0.0F)
        {
            s /= l <= 0.5F ? v + m : (2.0F - v) - m;
        }
        else
        {
            h = 0;
            s = 0;
            return;
        }

        const float r2 = (v - r) / vm;
        const float g2 = (v - g) / vm;
        const float b2 = (v - b) / vm;

        if (r == v)
        {
            h = g == m ? 5.0F + b2 : 1.0F - g2;
        }
        else if (g == v)
        {
            h = b == m ? 1.0F + r2 : 3.0F - b2;
        }
        else
        {
            h = r == m ? 3.0F + g2 : 5.0F - r2;
        }
        h /= 6.0F;
    }
    float color::get_luminosity() const
    {
        float v = std::max(red, green);
        v = std::max(v, blue);
        float m = std::min(red, green);
        m = std::min(m, blue);
        const float l = (m + v) / 2.0F;
        if (l <= 0.0F)
        {
            return 0;
        }
        return l;
    }
    float color::get_hue() const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        return h;
    }
    float color::get_saturation() const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        return s;
    }
    color color::with_luminosity(float luminosity) const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        return from_hsla(h, s, luminosity, alpha);
    }
    color color::add_luminosity(float delta) const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        l = clamp01(l + delta);
        return from_hsla(h, s, l, alpha);
    }
    color color::with_hue(float hue) const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        return from_hsla(hue, s, l, alpha);
    }
    color color::with_saturation(float saturation) const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        return from_hsla(h, saturation, l, alpha);
    }
    color color::get_complementary() const
    {
        float h = 0;
        float s = 0;
        float l = 0;
        to_hsl(h, s, l);
        h += 0.5F; // 180 degrees around the wheel
        h = std::fmod(h, 1.0F);
        return from_hsla(h, s, l);
    }

    // ---- alpha ----
    color color::with_alpha(float a) const
    {
        if (std::abs(a - alpha) < k_epsilon)
        {
            return *this;
        }
        return {red, green, blue, a};
    }
    color color::multiply_alpha(float multiply_by) const
    {
        return {red, green, blue, alpha * multiply_by};
    }

    std::string color::to_string() const
    {
        return std::format("[Color: Red={}, Green={}, Blue={}, Alpha={}]", red, green, blue, alpha);
    }

} // namespace maui::graphics

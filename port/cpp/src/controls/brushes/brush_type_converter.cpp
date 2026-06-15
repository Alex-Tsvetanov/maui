// maui::controls::convert_brush — the BrushTypeConverter port (header: brushes/brush_type_converter.hpp).
//
// A close port of src/Controls/src/Core/Brush/BrushTypeConverter.cs (ConvertFrom + GradientBrushParser).
// Color tokens (hex / named / rgb()/rgba()/hsl()/hsla()) resolve through maui::graphics::color::try_parse
// (the ColorTypeConverter analog). Gradient parsing reproduces the CSS grammar the C# parser accepts
// (linear-gradient(<deg|turn>, <stops>) / radial-gradient(circle [at x y], <stops>)), including the
// "#hex offset%" / "rgb(...) offset%" stop offsets and the default-direction fallbacks.

#include "maui/controls/brushes/brush_type_converter.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/radial_gradient_brush.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr std::string_view k_linear_gradient = "linear-gradient";
        constexpr std::string_view k_radial_gradient = "radial-gradient";
        constexpr std::string_view k_rgb = "rgb";
        constexpr std::string_view k_rgba = "rgba";
        constexpr std::string_view k_hsl = "hsl";
        constexpr std::string_view k_hsla = "hsla";

        [[nodiscard]] std::string_view trim(std::string_view s) noexcept
        {
            constexpr std::string_view ws = " \t\n\v\f\r";
            const auto b = s.find_first_not_of(ws);
            if (b == std::string_view::npos)
            {
                return {};
            }
            const auto e = s.find_last_not_of(ws);
            return s.substr(b, e - b + 1);
        }

        // Split on any of the delimiters, dropping empty parts (C# Split(.., RemoveEmptyEntries)).
        [[nodiscard]] std::vector<std::string> split_remove_empty(std::string_view s, std::string_view delims)
        {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= s.size())
            {
                const auto pos = s.find_first_of(delims, start);
                const auto end = pos == std::string_view::npos ? s.size() : pos;
                if (end > start)
                {
                    parts.emplace_back(s.substr(start, end - start));
                }
                if (pos == std::string_view::npos)
                {
                    break;
                }
                start = pos + 1;
            }
            return parts;
        }

        [[nodiscard]] bool try_parse_float(std::string_view s, float& out)
        {
            s = trim(s);
            if (s.empty())
            {
                return false;
            }
            float v = 0.0F;
            const char* first = s.data();
            const char* last = first + s.size();
            const auto [ptr, ec] = std::from_chars(first, last, v);
            if (ec == std::errc{} && ptr == last)
            {
                out = v;
                return true;
            }
            return false;
        }

        // C# GradientBrushParser.TryParseNumber(part, unit) — strips a trailing unit, parses the number.
        [[nodiscard]] bool try_parse_number(std::string_view part, std::string_view unit, float& out)
        {
            if (part.size() >= unit.size() && part.substr(part.size() - unit.size()) == unit)
            {
                return try_parse_float(part.substr(0, part.size() - unit.size()), out);
            }
            return false;
        }

        // C# GradientBrushParser.TryParseAngle — "<n>deg" (mod 360) or "<n>turn" (×360).
        [[nodiscard]] bool try_parse_angle(std::string_view part, double& angle)
        {
            float degrees = 0.0F;
            if (try_parse_number(part, "deg", degrees))
            {
                angle = std::fmod(static_cast<double>(degrees), 360.0);
                return true;
            }
            float turn = 0.0F;
            if (try_parse_number(part, "turn", turn))
            {
                angle = 360.0 * static_cast<double>(turn);
                return true;
            }
            angle = 0.0;
            return false;
        }

        // C# GradientBrushParser.TryParseOffset — "<n>%" (clamped to 1) or "<n>px".
        [[nodiscard]] bool try_parse_offset(std::string_view part, float& out)
        {
            float value = 0.0F;
            if (try_parse_number(part, "%", value))
            {
                out = std::min(value / 100.0F, 1.0F);
                return true;
            }
            if (try_parse_number(part, "px", out))
            {
                return true;
            }
            return false;
        }

        [[nodiscard]] bool try_parse_offsets(const std::vector<std::string>& parts, std::vector<float>& out)
        {
            out.clear();
            for (const auto& part : parts)
            {
                float offset = 0.0F;
                if (try_parse_offset(part, offset))
                {
                    out.push_back(offset);
                }
            }
            return !out.empty();
        }

        [[nodiscard]] std::optional<maui::graphics::color> try_color(std::string_view token)
        {
            maui::graphics::color parsed;
            if (maui::graphics::color::try_parse(token, parsed))
            {
                return parsed;
            }
            return std::nullopt;
        }

        [[nodiscard]] bool equals_ci(std::string_view a, std::string_view b)
        {
            if (a.size() != b.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < a.size(); ++i)
            {
                const auto ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
                const auto cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
                if (ca != cb)
                {
                    return false;
                }
            }
            return true;
        }

        // The CSS gradient parser (a port of GradientBrushParser). Returns null when no gradient was built.
        class gradient_brush_parser
        {
        public:
            [[nodiscard]] std::shared_ptr<gradient_brush> parse(std::string_view css)
            {
                if (trim(css).empty())
                {
                    return gradient_;
                }
                parts_ = split_remove_empty(css, "(),");
                position_ = 0;
                while (position_ < parts_.size())
                {
                    const std::string part = std::string{trim(part_at(position_))};

                    if (!part.empty() && part.front() == '#')
                    {
                        consume_color_token(part);
                    }

                    const auto color_parts = split_remove_empty(part, ".");
                    if (!color_parts.empty() && color_parts[0] == "Color")
                    {
                        consume_color_token(part);
                    }

                    if (equals_ci(part, k_rgb) || equals_ci(part, k_rgba) || equals_ci(part, k_hsl) ||
                        equals_ci(part, k_hsla))
                    {
                        consume_functional_color(part);
                    }

                    if (part == k_linear_gradient)
                    {
                        const std::string direction = std::string{trim(next_part())};
                        double angle = 0.0;
                        if (try_parse_angle(direction, angle))
                        {
                            create_linear_gradient(angle);
                        }
                        else
                        {
                            create_linear_gradient(0.0);
                            --position_;
                        }
                    }

                    if (part == k_radial_gradient)
                    {
                        create_radial_gradient(gradient_center());
                    }

                    ++position_;
                }
                return gradient_;
            }

        private:
            // The "#hex [offset]" / "Color.Name [offset]" branch.
            void consume_color_token(const std::string& part)
            {
                const auto words = split_remove_empty(part, " ");
                if (words.empty())
                {
                    return;
                }
                if (const auto color = try_color(words[0]))
                {
                    std::vector<float> offsets;
                    if (try_parse_offsets(words, offsets))
                    {
                        add_gradient_stops(*color, offsets);
                    }
                    else
                    {
                        add_gradient_stop(*color, std::nullopt);
                    }
                }
            }

            // The "rgb|rgba|hsl|hsla ( a , b , c [, d] ) [offset]" branch.
            void consume_functional_color(const std::string& part)
            {
                std::string color_string{part};
                color_string += '(';
                color_string += next_part();
                color_string += ',';
                color_string += next_part();
                color_string += ',';
                color_string += next_part();
                if (part == k_rgba || part == k_hsla)
                {
                    color_string += ',';
                    color_string += next_part();
                }
                color_string += ')';

                if (const auto color = try_color(color_string))
                {
                    const auto words = split_remove_empty(next_part(), " ");
                    std::vector<float> offsets;
                    if (try_parse_offsets(words, offsets))
                    {
                        add_gradient_stops(*color, offsets);
                    }
                    else
                    {
                        add_gradient_stop(*color, std::nullopt);
                        --position_;
                    }
                }
            }

            [[nodiscard]] std::string part_at(std::size_t index) const
            {
                return index >= parts_.size() ? std::string{} : parts_[index];
            }

            [[nodiscard]] std::string next_part()
            {
                ++position_;
                return part_at(position_);
            }

            void create_linear_gradient(double angle)
            {
                const auto coords = coordinates_by_angle(angle);
                auto linear = std::make_shared<linear_gradient_brush>();
                linear->set_start_point(coords.first);
                linear->set_end_point(coords.second);
                gradient_ = std::move(linear);
            }

            void create_radial_gradient(maui::graphics::point center)
            {
                auto radial = std::make_shared<radial_gradient_brush>();
                radial->set_center(center);
                gradient_ = std::move(radial);
            }

            void add_gradient_stop(maui::graphics::color color, std::optional<float> offset)
            {
                if (!gradient_)
                {
                    create_linear_gradient(0.0);
                }
                // C# defaults a missing offset to -1 (a sentinel the renderer reorders); the port keeps that.
                gradient_->gradient_stops().add(std::make_shared<gradient_stop>(color, offset.value_or(-1.0F)));
            }

            void add_gradient_stops(maui::graphics::color color, const std::vector<float>& offsets)
            {
                for (const float offset : offsets)
                {
                    add_gradient_stop(color, offset);
                }
            }

            [[nodiscard]] static std::pair<maui::graphics::point, maui::graphics::point> coordinates_by_angle(
                double angle)
            {
                if (angle == 90.0)
                {
                    return {maui::graphics::point{0, 1}, maui::graphics::point{0, 0}};
                }
                if (angle == 180.0)
                {
                    return {maui::graphics::point{1, 0}, maui::graphics::point{0, 0}};
                }
                if (angle == 270.0)
                {
                    return {maui::graphics::point{0, 0}, maui::graphics::point{0, 1}};
                }
                return {maui::graphics::point{0, 0}, maui::graphics::point{1, 0}}; // default / 360
            }

            [[nodiscard]] static maui::graphics::point position_by_direction(std::string_view direction)
            {
                if (direction == "left")
                {
                    return {0, 0.5};
                }
                if (direction == "right")
                {
                    return {1, 0.5};
                }
                if (direction == "top")
                {
                    return {0.5, 0};
                }
                if (direction == "bottom")
                {
                    return {0.5, 1};
                }
                return {0.5, 0.5}; // center / default
            }

            [[nodiscard]] maui::graphics::point gradient_center()
            {
                ++position_;
                const std::string part = std::string{trim(part_at(position_))};
                const auto words = split_remove_empty(part, " ");
                constexpr std::size_t center_pos = 1;
                if (words.size() > center_pos && words[center_pos].find("at") != std::string::npos)
                {
                    std::size_t idx = center_pos + 1;
                    const std::string dir_x = idx < words.size() ? std::string{trim(words[idx])} : std::string{};
                    ++idx;
                    const std::string dir_y = idx < words.size() ? std::string{trim(words[idx])} : std::string{};

                    float pos_x = 0.0F;
                    float pos_y = 0.0F;
                    const bool has_x = try_parse_offset(dir_x, pos_x);
                    const bool has_y = try_parse_offset(dir_y, pos_y);

                    maui::graphics::point fallback{0.5, 0.5};
                    if (!has_x && !dir_x.empty())
                    {
                        fallback = position_by_direction(dir_x);
                    }
                    if (!has_y && !dir_y.empty())
                    {
                        fallback = position_by_direction(dir_y);
                    }
                    return {has_x ? static_cast<double>(pos_x) : fallback.x,
                            has_y ? static_cast<double>(pos_y) : fallback.y};
                }
                return {0.5, 0.5};
            }

            std::shared_ptr<gradient_brush> gradient_;
            std::vector<std::string> parts_;
            std::size_t position_ = 0;
        };
    } // namespace

    std::shared_ptr<brush> convert_brush(std::string_view text)
    {
        // C# BrushTypeConverter.ConvertFrom (string path): trim, then dispatch.
        const std::string_view trimmed = trim(text);

        if (trimmed.starts_with(k_linear_gradient) || trimmed.starts_with(k_radial_gradient))
        {
            gradient_brush_parser parser;
            if (auto gradient = parser.parse(trimmed))
            {
                return gradient;
            }
        }

        if (trimmed.starts_with(k_rgb) || trimmed.starts_with(k_rgba) || trimmed.starts_with(k_hsl) ||
            trimmed.starts_with(k_hsla))
        {
            if (const auto color = try_color(trimmed))
            {
                return std::make_shared<solid_color_brush>(*color);
            }
            return std::make_shared<solid_color_brush>(std::optional<maui::graphics::color>{});
        }

        // A single token, or a "Color.Name" form (parts.Length == 1 || parts[0] == "Color").
        const auto parts = split_remove_empty(trimmed, ".");
        if (parts.size() == 1 || (parts.size() == 2 && parts[0] == "Color"))
        {
            if (const auto color = try_color(trimmed))
            {
                return std::make_shared<solid_color_brush>(*color);
            }
        }

        // C# tail: `return new SolidColorBrush(null)` — an unparseable / empty input yields a null-color brush.
        return std::make_shared<solid_color_brush>(std::optional<maui::graphics::color>{});
    }
} // namespace maui::controls

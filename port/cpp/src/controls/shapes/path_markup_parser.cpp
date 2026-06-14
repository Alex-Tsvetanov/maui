// maui::controls::shapes — the path markup / point list parsers, out-of-line. Ported 1:1 from
// PathFigureCollectionConverter.cs (the WPF abbreviated-geometry grammar — the C# local-function
// parser becomes a small state struct) and PointCollectionConverter.cs. Number tokens go through
// the port's invariant from_chars shim (Convert.ToDouble(InvariantCulture) in C#).

#include "maui/controls/shapes/path_markup_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_segment.hpp"
#include "maui/controls/shapes/sweep_direction.hpp"
#include "maui/detail/charconv_compat.hpp" // FP from_chars (general) with the libc++ < 20 fallback
#include "maui/graphics/point.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls::shapes
{
    namespace
    {
        constexpr bool allow_sign = true;
        constexpr bool allow_comma = true;

        // The C# local-function parser state (ParseToPathFigureCollection's captured locals).
        class figure_parser
        {
        public:
            figure_parser(path_figure_collection& figures, std::string_view path_string, std::size_t start_index)
                : figures_(&figures), text_(path_string), index_(start_index)
            {
            }

            void run()
            {
                std::shared_ptr<path_figure> figure;
                bool first = true;
                char last_cmd = ' ';

                while (read_token()) // an empty path is allowed in XAML
                {
                    const char cmd = token_;

                    if (first)
                    {
                        if (cmd != 'M' && cmd != 'm') // a path starts with M|m
                        {
                            throw bad_token();
                        }
                        first = false;
                    }

                    switch (cmd)
                    {
                        case 'm':
                        case 'M': {
                            // XAML allows multiple points after M/m (the extras become lines).
                            last_point_ = read_point(cmd, !allow_comma);

                            figure = std::make_shared<path_figure>();
                            figure->set_start_point(last_point_);
                            figures_->push_back(figure);

                            figure_started_ = true;
                            last_start_ = last_point_;
                            last_cmd = 'M';

                            while (is_number(allow_comma))
                            {
                                last_point_ = read_point(cmd, !allow_comma);
                                figure->segments().push_back(std::make_shared<line_segment>(last_point_));
                                last_cmd = 'L';
                            }
                            break;
                        }

                        case 'l':
                        case 'L':
                        case 'h':
                        case 'H':
                        case 'v':
                        case 'V': {
                            ensure_figure();
                            while (true)
                            {
                                switch (cmd)
                                {
                                    case 'l':
                                    case 'L':
                                        last_point_ = read_point(cmd, !allow_comma);
                                        break;
                                    case 'h':
                                        last_point_.x += read_number(!allow_comma);
                                        break;
                                    case 'H':
                                        last_point_.x = read_number(!allow_comma);
                                        break;
                                    case 'v':
                                        last_point_.y += read_number(!allow_comma);
                                        break;
                                    case 'V':
                                        last_point_.y = read_number(!allow_comma);
                                        break;
                                    default:
                                        break;
                                }
                                ensure_path_figure(figure)->segments().push_back(
                                    std::make_shared<line_segment>(last_point_));
                                if (!is_number(allow_comma))
                                {
                                    break;
                                }
                            }

                            last_cmd = 'L';
                            break;
                        }

                        case 'c':
                        case 'C': // cubic Bezier
                        case 's':
                        case 'S': // smooth cubic Bezier
                        {
                            ensure_figure();
                            while (true)
                            {
                                maui::graphics::point p;
                                if (cmd == 's' || cmd == 'S')
                                {
                                    p = last_cmd == 'C' ? reflect() : last_point_;
                                    second_last_point_ = read_point(cmd, !allow_comma);
                                }
                                else
                                {
                                    p = read_point(cmd, !allow_comma);
                                    second_last_point_ = read_point(cmd, allow_comma);
                                }
                                last_point_ = read_point(cmd, allow_comma);

                                ensure_path_figure(figure)->segments().push_back(
                                    std::make_shared<bezier_segment>(p, second_last_point_, last_point_));
                                last_cmd = 'C';
                                if (!is_number(allow_comma))
                                {
                                    break;
                                }
                            }
                            break;
                        }

                        case 'q':
                        case 'Q': // quadratic Bezier
                        case 't':
                        case 'T': // smooth quadratic Bezier
                        {
                            ensure_figure();
                            while (true)
                            {
                                if (cmd == 't' || cmd == 'T')
                                {
                                    second_last_point_ = last_cmd == 'Q' ? reflect() : last_point_;
                                    last_point_ = read_point(cmd, !allow_comma);
                                }
                                else
                                {
                                    second_last_point_ = read_point(cmd, !allow_comma);
                                    last_point_ = read_point(cmd, allow_comma);
                                }

                                ensure_path_figure(figure)->segments().push_back(
                                    std::make_shared<quadratic_bezier_segment>(second_last_point_, last_point_));
                                last_cmd = 'Q';
                                if (!is_number(allow_comma))
                                {
                                    break;
                                }
                            }
                            break;
                        }

                        case 'a':
                        case 'A': {
                            ensure_figure();
                            while (true)
                            {
                                // A 3,4 5, 0, 0, 6,7
                                const double w = read_number(!allow_comma);
                                const double h = read_number(allow_comma);
                                const double rotation = read_number(allow_comma);
                                const bool large = read_bool();
                                const bool sweep = read_bool();

                                last_point_ = read_point(cmd, allow_comma);

                                auto arc = std::make_shared<arc_segment>();
                                arc->set_size({w, h});
                                arc->set_rotation_angle(rotation);
                                arc->set_is_large_arc(large);
                                arc->set_sweep_direction(sweep ? sweep_direction::clockwise
                                                               : sweep_direction::counter_clockwise);
                                arc->set_point(last_point_);
                                ensure_path_figure(figure)->segments().push_back(std::move(arc));
                                if (!is_number(allow_comma))
                                {
                                    break;
                                }
                            }

                            last_cmd = 'A';
                            break;
                        }

                        case 'z':
                        case 'Z':
                            ensure_figure();
                            ensure_path_figure(figure)->set_is_closed(true);
                            figure_started_ = false;
                            last_cmd = 'Z';
                            // the reference point becomes the first point of the current figure
                            last_point_ = last_start_;
                            break;

                        default:
                            throw bad_token();
                    }
                }
            }

        private:
            [[nodiscard]] std::invalid_argument bad_token() const
            {
                return std::invalid_argument("UnexpectedToken \"" + std::string(text_) + "\" into " +
                                             std::to_string(index_ == 0 ? 0 : index_ - 1));
            }

            // C# EnsurePathFigure — a draw command before any M.
            [[nodiscard]] std::shared_ptr<path_figure> ensure_path_figure(
                const std::shared_ptr<path_figure>& figure) const
            {
                if (figure == nullptr)
                {
                    throw bad_token();
                }
                return figure;
            }

            void ensure_figure()
            {
                figure_started_ = true;
            }

            [[nodiscard]] maui::graphics::point reflect() const
            {
                return {(2 * last_point_.x) - second_last_point_.x, (2 * last_point_.y) - second_last_point_.y};
            }

            [[nodiscard]] bool more() const
            {
                return index_ < text_.size();
            }

            // C# SkipWhiteSpace(allowComma) — returns whether a comma was consumed.
            bool skip_white_space(bool allow_comma_here)
            {
                bool comma_met = false;
                while (more())
                {
                    const char ch = text_[index_];
                    switch (ch)
                    {
                        case ' ':
                        case '\n':
                        case '\r':
                        case '\t':
                            break;
                        case ',':
                            if (allow_comma_here)
                            {
                                comma_met = true;
                                allow_comma_here = false; // one comma only
                            }
                            else
                            {
                                throw bad_token();
                            }
                            break;
                        default:
                            // the C# IsWhiteSpace fast path: anything in (' ' .. 'z'] is not whitespace
                            if ((ch > ' ' && ch <= 'z') || std::isspace(static_cast<unsigned char>(ch)) == 0)
                            {
                                return comma_met;
                            }
                            break;
                    }
                    index_++;
                }
                return comma_met;
            }

            bool read_bool()
            {
                skip_white_space(allow_comma);
                if (more())
                {
                    token_ = text_[index_++];
                    if (token_ == '0')
                    {
                        return false;
                    }
                    if (token_ == '1')
                    {
                        return true;
                    }
                }
                throw bad_token();
            }

            bool read_token()
            {
                skip_white_space(!allow_comma);
                if (!more())
                {
                    return false;
                }
                token_ = text_[index_++];
                return true;
            }

            maui::graphics::point read_point(char cmd, bool allow_comma_here)
            {
                double x = read_number(allow_comma_here);
                double y = read_number(allow_comma);
                if (cmd >= 'a') // 'A' < 'a': lower case means relative
                {
                    x += last_point_.x;
                    y += last_point_.y;
                }
                return {x, y};
            }

            bool is_number(bool allow_comma_here)
            {
                const bool comma_met = skip_white_space(allow_comma_here);
                if (more())
                {
                    token_ = text_[index_];
                    // a valid number start ('I' = Infinity, 'N' = NaN)
                    if (token_ == '.' || token_ == '-' || token_ == '+' || (token_ >= '0' && token_ <= '9') ||
                        token_ == 'I' || token_ == 'N')
                    {
                        return true;
                    }
                }
                if (comma_met) // a comma is only allowed between numbers
                {
                    throw bad_token();
                }
                return false;
            }

            void skip_digits(bool sign_allowed)
            {
                if (sign_allowed && more() && (text_[index_] == '-' || text_[index_] == '+'))
                {
                    index_++;
                }
                while (more() && text_[index_] >= '0' && text_[index_] <= '9')
                {
                    index_++;
                }
            }

            double read_number(bool allow_comma_here)
            {
                if (!is_number(allow_comma_here))
                {
                    throw bad_token();
                }

                bool simple = true;
                std::size_t start = index_;

                // an optional sign (-NaN and friends are left for the numeric parse to reject)
                if (more() && (text_[index_] == '-' || text_[index_] == '+'))
                {
                    index_++;
                }

                if (more() && text_[index_] == 'I')
                {
                    // "Infinity" — consume up to its 8 characters, parse later.
                    index_ = std::min(index_ + 8, text_.size());
                    simple = false;
                }
                else if (more() && text_[index_] == 'N')
                {
                    // "NaN" — consume up to its 3 characters, parse later.
                    index_ = std::min(index_ + 3, text_.size());
                    simple = false;
                }
                else
                {
                    skip_digits(!allow_sign);
                    if (more() && text_[index_] == '.') // an optional period + more digits
                    {
                        simple = false;
                        index_++;
                        skip_digits(!allow_sign);
                    }
                    if (more() && (text_[index_] == 'E' || text_[index_] == 'e')) // an exponent
                    {
                        simple = false;
                        index_++;
                        skip_digits(allow_sign);
                    }
                }

                if (simple && index_ <= start + 8) // fits a hand-rolled 32-bit parse (C# fast path)
                {
                    int sign = 1;
                    if (text_[start] == '+')
                    {
                        start++;
                    }
                    else if (text_[start] == '-')
                    {
                        start++;
                        sign = -1;
                    }
                    int value = 0;
                    while (start < index_)
                    {
                        value = (value * 10) + (text_[start] - '0');
                        start++;
                    }
                    return value * sign;
                }

                std::string_view token = text_.substr(start, index_ - start);
                // C# Convert.ToDouble(InvariantCulture) accepts a leading '+'; from_chars does not.
                if (!token.empty() && token.front() == '+')
                {
                    token.remove_prefix(1);
                }
                double parsed = 0;
                const char* const first = token.data();
                const char* const last = std::next(first, static_cast<std::ptrdiff_t>(token.size()));
                const auto [ptr, ec] = maui::detail::from_chars_general(first, last, parsed);
                if (ec != std::errc{} || ptr != last)
                {
                    throw std::invalid_argument("UnexpectedToken \"" + std::to_string(start) + "\" into " +
                                                std::string(text_));
                }
                return parsed;
            }

            path_figure_collection* figures_; // non-owning: the caller-provided output sink
            std::string_view text_;
            std::size_t index_ = 0;
            bool figure_started_ = false;
            maui::graphics::point last_start_;
            maui::graphics::point last_point_;
            maui::graphics::point second_last_point_;
            char token_ = '\0';
        };
    } // namespace

    void parse_path_figure_collection(path_figure_collection& figures, std::string_view path_string)
    {
        std::size_t cur_index = 0;

        // Skip leading whitespace, then the optional fill-rule token: 'F' must be followed by 0|1.
        while (cur_index < path_string.size() && std::isspace(static_cast<unsigned char>(path_string[cur_index])) != 0)
        {
            cur_index++;
        }
        if (cur_index < path_string.size() && path_string[cur_index] == 'F')
        {
            cur_index++;
            while (cur_index < path_string.size() &&
                   std::isspace(static_cast<unsigned char>(path_string[cur_index])) != 0)
            {
                cur_index++;
            }
            if (cur_index == path_string.size() || (path_string[cur_index] != '0' && path_string[cur_index] != '1'))
            {
                throw std::invalid_argument("IllegalToken");
            }
            cur_index++;
        }

        figure_parser parser(figures, path_string, cur_index);
        parser.run();
    }

    path_geometry parse_path_geometry(std::string_view path_string)
    {
        // C# PathGeometryConverter.ConvertFrom: a fresh PathGeometry whose Figures are parsed from
        // the string (a null/empty input leaves the figures empty).
        path_geometry geometry;
        parse_path_figure_collection(geometry.figures(), path_string);
        return geometry;
    }

    point_collection parse_point_collection(std::string_view points_string)
    {
        // C# PointCollectionConverter.ConvertFrom: split on spaces and commas, pair up the numbers.
        point_collection points;
        double x = 0;
        bool has_x = false;

        std::size_t index = 0;
        while (index <= points_string.size())
        {
            // split on ' ' and ',' (C# string.Split keeps empty entries; they are skipped below)
            std::size_t end = points_string.find_first_of(" ,", index);
            if (end == std::string_view::npos)
            {
                end = points_string.size();
            }
            const std::string_view token = points_string.substr(index, end - index);
            index = end + 1;

            // C# IsNullOrWhiteSpace skip (split tokens cannot contain the separators themselves)
            const bool blank =
                std::ranges::all_of(token, [](char ch) { return std::isspace(static_cast<unsigned char>(ch)) != 0; });
            if (blank)
            {
                continue;
            }

            double number = 0;
            std::string_view numeric = token;
            if (!numeric.empty() && numeric.front() == '+') // double.TryParse accepts a leading '+'
            {
                numeric.remove_prefix(1);
            }
            const char* const first = numeric.data();
            const char* const last = std::next(first, static_cast<std::ptrdiff_t>(numeric.size()));
            const auto [ptr, ec] = maui::detail::from_chars_general(first, last, number);
            if (ec != std::errc{} || ptr != last)
            {
                throw std::invalid_argument("Cannot convert \"" + std::string(token) + "\" into a double");
            }

            if (!has_x)
            {
                x = number;
                has_x = true;
            }
            else
            {
                points.emplace_back(x, number);
                has_x = false;
            }
        }

        if (has_x)
        {
            throw std::invalid_argument("Cannot convert string into PointCollection");
        }
        return points;
    }
} // namespace maui::controls::shapes

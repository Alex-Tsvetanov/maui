// maui::graphics::path_builder  <=  Microsoft.Maui.Graphics.PathBuilder
#include "maui/graphics/path_builder.hpp"

#include <cctype>
#include <cstddef>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"

#include "detail/parse_util.hpp"

namespace maui::graphics
{
    namespace
    {

        bool is_letter(char c)
        {
            return std::isalpha(static_cast<unsigned char>(c)) != 0;
        }

        // GeometryUtil.GetOppositePoint: reflect oppositePoint across pivot.
        point_f get_opposite_point(const point_f &pivot, const point_f &opposite)
        {
            const float dx = opposite.x - pivot.x;
            const float dy = opposite.y - pivot.y;
            return {pivot.x - dx, pivot.y - dy};
        }

        std::string replace_all(std::string s, std::string_view from, std::string_view to)
        {
            if (from.empty())
            {
                return s;
            }
            std::size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos)
            {
                s.replace(pos, from.size(), to);
                pos += to.size();
            }
            return s;
        }

        std::string separate_letter_chars_with_spaces(std::string_view input)
        {
            std::string out;
            out.reserve(input.size() * 3);
            for (const char c : input)
            {
                if (is_letter(c))
                {
                    out.push_back(' ');
                    out.push_back(c);
                    out.push_back(' ');
                }
                else
                {
                    out.push_back(c);
                }
            }
            return out;
        }

        std::vector<std::string> split_on_separators(const std::string &s)
        {
            std::vector<std::string> out;
            std::string cur;
            auto flush = [&] {
                if (!cur.empty())
                {
                    out.push_back(cur);
                }
                cur.clear();
            };
            for (const char c : s)
            {
                if (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == ',')
                {
                    flush();
                }
                else
                {
                    cur.push_back(c);
                }
            }
            flush();
            return out;
        }

        bool try_parse_float_full(std::string_view v, float &out)
        {
            return detail::ps_parse_num(v, out);
        }

    } // namespace

    path_f path_builder::build(std::string_view definition)
    {
        if (definition.empty())
        {
            return path_f{};
        }
        path_builder builder;
        return builder.build_path(definition);
    }

    float path_builder::parse_float(std::string_view value)
    {
        float number = 0;
        if (try_parse_float_full(value, number))
        {
            return number;
        }

        // Illustrator sometimes exports malformed numbers like "5.96.88".
        std::vector<std::string> split;
        {
            std::string cur;
            for (const char c : value)
            {
                if (c == '.')
                {
                    split.push_back(cur);
                    cur.clear();
                }
                else
                {
                    cur.push_back(c);
                }
            }
            split.push_back(cur);
        }
        if (split.size() > 2)
        {
            const std::string combined = split[0] + "." + split[1];
            if (try_parse_float_full(combined, number))
            {
                return number;
            }
        }

        std::string numbers_only;
        for (const char c : value)
        {
            if (std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '.' || c == '-')
            {
                numbers_only.push_back(c);
            }
        }
        if (try_parse_float_full(numbers_only, number))
        {
            return number;
        }

        throw std::runtime_error("Error parsing '" + std::string(value) + "' as a float.");
    }

    bool path_builder::next_bool_value()
    {
        const std::string v = command_stack_.back();
        command_stack_.pop_back();
        return v == "1";
    }
    float path_builder::next_value()
    {
        if (command_stack_.empty())
        {
            throw std::runtime_error("path command stack underflow");
        }
        const std::string v = command_stack_.back();
        command_stack_.pop_back();
        return parse_float(v);
    }

    path_f path_builder::build_path(std::string_view path_as_string)
    {
        last_command_ = '~';
        last_curve_control_point_.reset();
        path_ = path_f{};
        command_stack_.clear();
        relative_point_ = point_f(0, 0);
        close_when_done_ = false;

        std::string s(path_as_string);
        s = replace_all(s, "Infinity", "0");
        s = separate_letter_chars_with_spaces(s);
        s = replace_all(s, "-", " -");
        s = replace_all(s, " E  -", "E-");
        s = replace_all(s, " e  -", "e-");

        const std::vector<std::string> args = split_on_separators(s);

        for (int i = static_cast<int>(args.size()) - 1; i >= 0; i--)
        {
            std::string entry = args[i];
            const char c = entry[0];
            if (is_letter(c))
            {
                if (entry.size() > 1)
                {
                    entry = entry.substr(1);
                    if (is_letter(entry[0]))
                    {
                        if (entry.size() > 1)
                        {
                            command_stack_.push_back(entry.substr(1));
                        }
                        command_stack_.emplace_back(1, entry[0]);
                    }
                    else
                    {
                        command_stack_.push_back(entry);
                    }
                }
                command_stack_.emplace_back(1, c);
            }
            else
            {
                command_stack_.push_back(entry);
            }
        }

        try
        {
            while (!command_stack_.empty())
            {
                const std::string top = command_stack_.back();
                command_stack_.pop_back();
                const char first = top[0];
                if (std::isdigit(static_cast<unsigned char>(first)) == 0 && first != '.' && first != '-' &&
                    first != 'e' && first != 'E')
                { // IsCommand
                    handle_command(top);
                }
                else
                {
                    command_stack_.push_back(top);
                    handle_command(std::string(1, last_command_));
                }
            }
            if (!path_.closed() && close_when_done_)
            {
                path_.close();
            }
        }
        // NOLINTNEXTLINE(bugprone-empty-catch) -- intentionally matches release C#: swallow malformed-path errors
        catch (const std::exception &)
        {
            // Return the partial path built so far, as PathBuilder.cs does outside DEBUG.
        }
        return path_;
    }

    void path_builder::handle_command(const std::string &command)
    {
        const char c = command[0];
        if (last_command_ != '~' && (std::isdigit(static_cast<unsigned char>(c)) != 0 || c == '-'))
        {
            std::optional<char> previous_command;
            if (!command_stack_.empty())
            {
                previous_command = command_stack_.back()[0];
            }

            switch (last_command_)
            {
                case 'M':
                    command_stack_.push_back(command);
                    handle_command('L');
                    break;
                case 'm':
                    command_stack_.push_back(command);
                    handle_command('l');
                    break;
                case 'L':
                    command_stack_.push_back(command);
                    handle_command('L');
                    break;
                case 'l':
                    command_stack_.push_back(command);
                    handle_command('l');
                    break;
                case 'H':
                    command_stack_.push_back(command);
                    handle_command('H');
                    break;
                case 'h':
                    command_stack_.push_back(command);
                    handle_command('h');
                    break;
                case 'V':
                    command_stack_.push_back(command);
                    handle_command('V');
                    break;
                case 'v':
                    command_stack_.push_back(command);
                    handle_command('v');
                    break;
                case 'C':
                    command_stack_.push_back(command);
                    handle_command('C');
                    break;
                case 'c':
                    command_stack_.push_back(command);
                    handle_command('c');
                    break;
                case 'S':
                    command_stack_.push_back(command);
                    handle_command('S');
                    break;
                case 's':
                    command_stack_.push_back(command);
                    handle_command('s');
                    break;
                case 'Q':
                    command_stack_.push_back(command);
                    handle_command('Q');
                    break;
                case 'q':
                    command_stack_.push_back(command);
                    handle_command('q');
                    break;
                case 'T':
                    command_stack_.push_back(command);
                    handle_command('T', previous_command);
                    break;
                case 't':
                    command_stack_.push_back(command);
                    handle_command('t', previous_command);
                    break;
                case 'A':
                    command_stack_.push_back(command);
                    handle_command('A');
                    break;
                case 'a':
                    command_stack_.push_back(command);
                    handle_command('a');
                    break;
                default:
                    break;
            }
        }
        else
        {
            handle_command(c);
        }
    }

    void path_builder::handle_command(char command, std::optional<char> previous_command)
    {
        switch (command)
        {
            case 'M':
                move_to(false);
                break;
            case 'm':
                move_to(true);
                if (last_command_ == '~')
                {
                    command = 'm';
                }
                break;
            case 'z':
            case 'Z':
                close_path();
                break;
            case 'L':
                line_to(false);
                break;
            case 'l':
                line_to(true);
                break;
            case 'Q':
                quad_to(false);
                break;
            case 'q':
                quad_to(true);
                break;
            case 'T':
                reflective_quad_to(false, previous_command);
                break;
            case 't':
                reflective_quad_to(true, previous_command);
                break;
            case 'C':
                curve_to(false);
                break;
            case 'c':
                curve_to(true);
                break;
            case 'S':
                smooth_curve_to(false);
                break;
            case 's':
                smooth_curve_to(true);
                break;
            case 'A':
                arc_to(false);
                break;
            case 'a':
                arc_to(true);
                break;
            case 'H':
                horizontal_line_to(false);
                break;
            case 'h':
                horizontal_line_to(true);
                break;
            case 'V':
                vertical_line_to(false);
                break;
            case 'v':
                vertical_line_to(true);
                break;
            default:
                break;
        }
        if (command != 'C' && command != 'c' && command != 's' && command != 'S')
        {
            last_curve_control_point_.reset();
        }
        last_command_ = command;
    }

    void path_builder::close_path()
    {
        path_.close();
        relative_point_ = last_move_to_;
    }

    void path_builder::move_to(bool is_relative)
    {
        if (path_.sub_path_count() == 1)
        {
            if (path_.first_point() == path_.last_point())
            {
                close_when_done_ = true;
            }
        }
        const float x = next_value();
        const float y = next_value();
        const point_f point = new_point(x, y, is_relative, true);
        path_.move_to(point);
        last_move_to_ = point;
    }

    void path_builder::line_to(bool is_relative)
    {
        const float x = next_value();
        const float y = next_value();
        path_.line_to(new_point(x, y, is_relative, true));
    }

    void path_builder::horizontal_line_to(bool is_relative)
    {
        const float x = next_value();
        path_.line_to(new_horizontal_point(x, is_relative, true));
    }

    void path_builder::vertical_line_to(bool is_relative)
    {
        const float y = next_value();
        path_.line_to(new_vertical_point(y, is_relative, true));
    }

    void path_builder::curve_to(bool is_relative)
    {
        const float p1x = next_value();
        const float p1y = next_value();
        const point_f point1 = new_point(p1x, p1y, is_relative, false);
        const float x = next_value();
        const float y = next_value();
        const bool is_quad = !command_stack_.empty() && is_letter(command_stack_.back()[0]);
        const point_f point2 = new_point(x, y, is_relative, is_quad);
        if (is_quad)
        {
            path_.quad_to(point1, point2);
            last_curve_control_point_ = point1;
        }
        else
        {
            const float p3x = next_value();
            const float p3y = next_value();
            const point_f point3 = new_point(p3x, p3y, is_relative, true);
            path_.curve_to(point1, point2, point3);
            last_curve_control_point_ = point2;
        }
    }

    void path_builder::quad_to(bool is_relative)
    {
        const float p1x = next_value();
        const float p1y = next_value();
        const point_f point1 = new_point(p1x, p1y, is_relative, false);
        const float p2x = next_value();
        const float p2y = next_value();
        const point_f point2 = new_point(p2x, p2y, is_relative, true);
        last_curve_control_point_ = point1;
        path_.quad_to(point1, point2);
    }

    void path_builder::reflective_quad_to(bool is_relative, std::optional<char> previous_command)
    {
        const point_f last_point = path_.last_point();
        point_f point1 = last_point;
        const point_f lccp = last_curve_control_point_.value_or(point_f{});
        if (previous_command && (*previous_command == 'Q' || *previous_command == 'q' || *previous_command == 'T' ||
                                 *previous_command == 't'))
        {
            const float dx = last_point.x - lccp.x;
            const float dy = last_point.y - lccp.y;
            point1 = point1.offset(dx, dy);
        }
        const float x = next_value();
        const float y = next_value();
        const point_f point2 = new_point(x, y, is_relative, true);
        last_curve_control_point_ = point1;
        path_.quad_to(point1, point2);
    }

    void path_builder::smooth_curve_to(bool is_relative)
    {
        std::optional<point_f> point1;
        const float p2x = next_value();
        const float p2y = next_value();
        const point_f point2 = new_point(p2x, p2y, is_relative, false);
        if (!last_curve_control_point_ && relative_point_)
        {
            point1 = get_opposite_point(*relative_point_, point2);
        }
        else if (relative_point_ && last_curve_control_point_)
        {
            point1 = get_opposite_point(*relative_point_, *last_curve_control_point_);
        }
        const float p3x = next_value();
        const float p3y = next_value();
        const point_f point3 = new_point(p3x, p3y, is_relative, true);
        if (point1)
        {
            path_.curve_to(*point1, point2, point3);
        }
        last_curve_control_point_ = point2;
    }

    void path_builder::arc_to(bool is_relative)
    {
        const point_f start = relative_point_.value_or(point_f{});
        const float rx = next_value();
        const float ry = next_value();
        const float r = next_value();
        const bool large_arc_flag = next_bool_value();
        const bool sweep_flag = next_bool_value();
        const float ex = next_value();
        const float ey = next_value();
        const point_f end_point = new_point(ex, ey, is_relative, false);

        path_f arc_path(start);
        arc_path.svg_arc_to(rx, ry, r, large_arc_flag, sweep_flag, end_point.x, end_point.y, start.x, start.y);

        for (int s = 0; s < arc_path.operation_count(); s++)
        {
            const auto type = arc_path.get_segment_type(s);
            const auto pts = arc_path.get_points_for_segment(s);
            if (type == path_operation::line)
            {
                path_.line_to(pts[0]);
            }
            else if (type == path_operation::cubic)
            {
                path_.curve_to(pts[0], pts[1], pts[2]);
            }
            else if (type == path_operation::quad)
            {
                path_.quad_to(pts[0], pts[1]);
            }
            // move: do nothing
        }
        relative_point_ = path_.last_point();
    }

    point_f path_builder::new_point(float x, float y, bool is_relative, bool is_reference)
    {
        point_f point;
        if (is_relative && relative_point_)
        {
            point = point_f(relative_point_->x + x, relative_point_->y + y);
        }
        else
        {
            point = point_f(x, y);
        }
        if (is_reference)
        {
            relative_point_ = point;
        }
        return point;
    }

    point_f path_builder::new_vertical_point(float y, bool is_relative, bool is_reference)
    {
        point_f point;
        if (is_relative && relative_point_)
        {
            point = point_f(relative_point_->x, relative_point_->y + y);
        }
        else if (relative_point_)
        {
            point = point_f(relative_point_->x, y);
        }
        if (is_reference)
        {
            relative_point_ = point;
        }
        return point;
    }

    point_f path_builder::new_horizontal_point(float x, bool is_relative, bool is_reference)
    {
        point_f point;
        if (is_relative && relative_point_)
        {
            point = point_f(relative_point_->x + x, relative_point_->y);
        }
        else if (relative_point_)
        {
            point = point_f(x, relative_point_->y);
        }
        if (is_reference)
        {
            relative_point_ = point;
        }
        return point;
    }

} // namespace maui::graphics

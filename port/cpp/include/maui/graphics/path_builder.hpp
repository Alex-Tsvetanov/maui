#pragma once
// maui::graphics::path_builder  <=  Microsoft.Maui.Graphics.PathBuilder
// Parses SVG-like path definitions (M/L/H/V/C/S/Q/T/A/Z, absolute + relative) into a path_f.
// Ported from src/Graphics/src/Graphics/PathBuilder.cs.

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"

namespace maui::graphics
{

    class path_builder
    {
    public:
        static path_f build(std::string_view definition); // empty path for empty/null input
        static float parse_float(std::string_view value); // tolerant: handles "5.96.88", trailing junk
        path_f build_path(std::string_view path_as_string);

    private:
        std::vector<std::string> command_stack_; // top == back()
        bool close_when_done_ = false;
        char last_command_ = '~';
        std::optional<point_f> last_curve_control_point_;
        std::optional<point_f> last_move_to_;
        path_f path_;
        std::optional<point_f> relative_point_;

        bool next_bool_value();
        float next_value();
        void handle_command(const std::string &command);
        void handle_command(char command, std::optional<char> previous_command = std::nullopt);
        void close_path();
        void move_to(bool is_relative);
        void line_to(bool is_relative);
        void horizontal_line_to(bool is_relative);
        void vertical_line_to(bool is_relative);
        void curve_to(bool is_relative);
        void quad_to(bool is_relative);
        void reflective_quad_to(bool is_relative, std::optional<char> previous_command);
        void smooth_curve_to(bool is_relative);
        void arc_to(bool is_relative);
        point_f new_point(float x, float y, bool is_relative, bool is_reference);
        point_f new_vertical_point(float y, bool is_relative, bool is_reference);
        point_f new_horizontal_point(float x, bool is_relative, bool is_reference);
    };

} // namespace maui::graphics

#pragma once
// maui::graphics::recording_canvas — the HEADLESS canvas: records every operation into a typed op
// list for golden-op assertions, while running the genuine AbstractCanvas state machinery.
//
// Port lineage (recorded in port/STATUS.md): MAUI's headless recorder is PictureCanvas
// (src/Graphics/src/Graphics/PictureCanvas.cs), which records closures for IPicture playback and
// bypasses AbstractCanvas entirely. The port's recorder instead derives from abstract_canvas — so
// the C# state semantics (save/restore stack, transform tracking, stroke-size limiting, the lazy
// dash-pattern flush, the rounded-rect radius clamp) are exercised headlessly exactly as a platform
// backend would see them — and records plain op structs (closures would be opaque to assertions).
// Differences vs. PictureCanvas, all deliberate:
//  - set_antialias IS recorded (PictureCanvas drops it: "not supported in a picture");
//  - reset_state IS recorded (PictureCanvas's ResetState is a no-op);
//  - dash patterns surface as the platform flush op (pattern + offset + stroke size) before the
//    next stroked draw, not as a property op at set time — the AbstractCanvas pipeline;
//  - ops::set_fill_paint stores the paint's background_color projection + the gradient rect (paint
//    is not clonable; a null paint stores white, like C#'s Colors.White.AsPaint() fallback).
//
// The op structs are a tight cluster with the recorder (namespace canvas_ops). Out-of-line
// definitions live in src/graphics/recording_canvas.cpp.

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "maui/graphics/abstract_canvas.hpp"
#include "maui/graphics/blend_mode.hpp"
#include "maui/graphics/canvas_state.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/horizontal_alignment.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text_flow.hpp"
#include "maui/graphics/vertical_alignment.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::graphics
{
    namespace canvas_ops
    {
        // ---- state-setter ops ----
        struct set_stroke_size // the post-limit size the platform receives
        {
            float value;
            bool operator==(const set_stroke_size&) const = default;
        };
        struct set_miter_limit
        {
            float value;
            bool operator==(const set_miter_limit&) const = default;
        };
        struct set_stroke_color
        {
            color value;
            bool operator==(const set_stroke_color&) const = default;
        };
        struct set_stroke_line_cap
        {
            line_cap value;
            bool operator==(const set_stroke_line_cap&) const = default;
        };
        struct set_stroke_line_join
        {
            line_join value;
            bool operator==(const set_stroke_line_join&) const = default;
        };
        // The AbstractCanvas dash flush: pattern + offset + the stroke size it was scaled for.
        struct set_stroke_dash_pattern
        {
            std::vector<float> pattern;
            float dash_offset;
            float stroke_size;
            bool operator==(const set_stroke_dash_pattern&) const = default;
        };
        struct set_fill_color
        {
            color value;
            bool operator==(const set_fill_color&) const = default;
        };
        struct set_font_color
        {
            color value;
            bool operator==(const set_font_color&) const = default;
        };
        struct set_font
        {
            maui::graphics::font value;
            bool operator==(const set_font&) const = default;
        };
        struct set_font_size
        {
            float value;
            bool operator==(const set_font_size&) const = default;
        };
        struct set_alpha
        {
            float value;
            bool operator==(const set_alpha&) const = default;
        };
        struct set_antialias
        {
            bool value;
            bool operator==(const set_antialias&) const = default;
        };
        struct set_blend_mode
        {
            maui::graphics::blend_mode value;
            bool operator==(const set_blend_mode&) const = default;
        };

        // ---- draw / fill ops ----
        struct draw_line
        {
            float x1, y1, x2, y2;
            bool operator==(const draw_line&) const = default;
        };
        struct draw_arc
        {
            float x, y, width, height, start_angle, end_angle;
            bool clockwise, closed;
            bool operator==(const draw_arc&) const = default;
        };
        struct fill_arc
        {
            float x, y, width, height, start_angle, end_angle;
            bool clockwise;
            bool operator==(const fill_arc&) const = default;
        };
        struct draw_rectangle
        {
            float x, y, width, height;
            bool operator==(const draw_rectangle&) const = default;
        };
        struct fill_rectangle
        {
            float x, y, width, height;
            bool operator==(const fill_rectangle&) const = default;
        };
        struct draw_rounded_rectangle // corner_radius is the post-clamp value
        {
            float x, y, width, height, corner_radius;
            bool operator==(const draw_rounded_rectangle&) const = default;
        };
        struct fill_rounded_rectangle
        {
            float x, y, width, height, corner_radius;
            bool operator==(const fill_rounded_rectangle&) const = default;
        };
        struct draw_ellipse
        {
            float x, y, width, height;
            bool operator==(const draw_ellipse&) const = default;
        };
        struct fill_ellipse
        {
            float x, y, width, height;
            bool operator==(const fill_ellipse&) const = default;
        };
        struct draw_path
        {
            path_f path;
            bool operator==(const draw_path&) const = default;
        };
        struct fill_path
        {
            path_f path;
            winding_mode winding;
            bool operator==(const fill_path&) const = default;
        };

        // ---- clip ops ----
        struct clip_path
        {
            path_f path;
            winding_mode winding;
            bool operator==(const clip_path&) const = default;
        };
        struct clip_rectangle
        {
            float x, y, width, height;
            bool operator==(const clip_rectangle&) const = default;
        };
        struct subtract_from_clip
        {
            float x, y, width, height;
            bool operator==(const subtract_from_clip&) const = default;
        };

        // ---- text ops ----
        struct draw_string
        {
            std::string value;
            float x, y;
            horizontal_alignment h_align;
            bool operator==(const draw_string&) const = default;
        };
        struct draw_string_in_bounds
        {
            std::string value;
            float x, y, width, height;
            horizontal_alignment h_align;
            vertical_alignment v_align;
            text_flow flow;
            float line_spacing_adjustment;
            bool operator==(const draw_string_in_bounds&) const = default;
        };
        struct draw_text // a value snapshot of the i_attributed_text argument
        {
            std::string text;
            std::vector<maui::graphics::text::attributed_text_run> runs;
            float x, y, width, height;
            bool operator==(const draw_text&) const = default;
        };

        // ---- transform ops ----
        struct rotate
        {
            float degrees;
            bool operator==(const rotate&) const = default;
        };
        struct rotate_at
        {
            float degrees;
            float x, y;
            bool operator==(const rotate_at&) const = default;
        };
        struct scale
        {
            float sx, sy;
            bool operator==(const scale&) const = default;
        };
        struct translate
        {
            float tx, ty;
            bool operator==(const translate&) const = default;
        };
        struct concatenate_transform
        {
            matrix3x2 transform;
            bool operator==(const concatenate_transform&) const = default;
        };

        // ---- state-stack ops ----
        struct save_state
        {
            bool operator==(const save_state&) const = default;
        };
        struct restore_state
        {
            bool operator==(const restore_state&) const = default;
        };
        struct reset_state
        {
            bool operator==(const reset_state&) const = default;
        };

        // ---- effect ops ----
        struct set_shadow
        {
            size_f offset;
            float blur;
            color shadow_color;
            bool operator==(const set_shadow&) const = default;
        };
        struct set_fill_paint // the background_color projection of the paint (see header note)
        {
            color background_color;
            rect_f rectangle;
            bool operator==(const set_fill_paint&) const = default;
        };
    } // namespace canvas_ops

    // Every recordable operation, as a closed sum (golden-op assertions switch on the alternative).
    using canvas_op = std::variant<
        canvas_ops::set_stroke_size, canvas_ops::set_miter_limit, canvas_ops::set_stroke_color,
        canvas_ops::set_stroke_line_cap, canvas_ops::set_stroke_line_join, canvas_ops::set_stroke_dash_pattern,
        canvas_ops::set_fill_color, canvas_ops::set_font_color, canvas_ops::set_font, canvas_ops::set_font_size,
        canvas_ops::set_alpha, canvas_ops::set_antialias, canvas_ops::set_blend_mode, canvas_ops::draw_line,
        canvas_ops::draw_arc, canvas_ops::fill_arc, canvas_ops::draw_rectangle, canvas_ops::fill_rectangle,
        canvas_ops::draw_rounded_rectangle, canvas_ops::fill_rounded_rectangle, canvas_ops::draw_ellipse,
        canvas_ops::fill_ellipse, canvas_ops::draw_path, canvas_ops::fill_path, canvas_ops::clip_path,
        canvas_ops::clip_rectangle, canvas_ops::subtract_from_clip, canvas_ops::draw_string,
        canvas_ops::draw_string_in_bounds, canvas_ops::draw_text, canvas_ops::rotate, canvas_ops::rotate_at,
        canvas_ops::scale, canvas_ops::translate, canvas_ops::concatenate_transform, canvas_ops::save_state,
        canvas_ops::restore_state, canvas_ops::reset_state, canvas_ops::set_shadow, canvas_ops::set_fill_paint>;

    class recording_canvas final : public abstract_canvas<canvas_state>
    {
    public:
        recording_canvas() = default;

        // The recorded op list, in call order.
        [[nodiscard]] const std::vector<canvas_op>& ops() const;
        // Drop the recorded ops (the state machinery is untouched).
        void clear();

        // ---- the i_canvas surface abstract_canvas leaves to the backend ----
        void set_miter_limit(float value) override;
        void set_stroke_color(const color& value) override;
        void set_stroke_line_cap(line_cap value) override;
        void set_stroke_line_join(line_join value) override;
        void set_fill_color(const color& value) override;
        void set_font_color(const color& value) override;
        void set_font(const font& value) override;
        void set_font_size(float value) override;
        void set_alpha(float value) override;
        void set_antialias(bool value) override;
        void set_blend_mode(blend_mode value) override;

        void fill_path(const path_f& path, winding_mode winding) override;
        void subtract_from_clip(float x, float y, float width, float height) override;
        void clip_path(const path_f& path, winding_mode winding = winding_mode::non_zero) override;
        void clip_rectangle(float x, float y, float width, float height) override;

        void fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                      bool clockwise) override;
        void fill_rectangle(float x, float y, float width, float height) override;
        void fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius) override;
        void fill_ellipse(float x, float y, float width, float height) override;

        void draw_string(std::string_view value, float x, float y, horizontal_alignment h_align) override;
        void draw_string(std::string_view value, float x, float y, float width, float height,
                         horizontal_alignment h_align, vertical_alignment v_align,
                         text_flow flow = text_flow::clip_bounds, float line_spacing_adjustment = 0) override;
        void draw_text(const text::i_attributed_text& value, float x, float y, float width, float height) override;

        // C# PictureCanvas.GetStringSize throws NotSupportedException — ported as std::logic_error.
        [[nodiscard]] size_f get_string_size(std::string_view value, const font& font, float font_size) const override;
        [[nodiscard]] size_f get_string_size(std::string_view value, const font& font, float font_size,
                                             horizontal_alignment h_align, vertical_alignment v_align) const override;

        void save_state() override;
        bool restore_state() override;
        void reset_state() override;

        void set_shadow(const size_f& offset, float blur, const color& shadow_color) override;
        void set_fill_paint(const paint* fill_paint, const rect_f& rectangle) override;

    protected:
        // ---- the abstract_canvas platform hooks: record the platform-level payloads ----
        void platform_set_stroke_size(float value) override;
        void platform_set_stroke_dash_pattern(const std::vector<float>& pattern, float stroke_dash_offset,
                                              float stroke_size) override;
        void platform_draw_line(float x1, float y1, float x2, float y2) override;
        void platform_draw_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                               bool clockwise, bool closed) override;
        void platform_draw_rectangle(float x, float y, float width, float height) override;
        void platform_draw_rounded_rectangle(float x, float y, float width, float height, float corner_radius) override;
        void platform_draw_ellipse(float x, float y, float width, float height) override;
        void platform_draw_path(const path_f& path) override;
        void platform_rotate(float degrees, float radians, float x, float y) override;
        void platform_rotate(float degrees, float radians) override;
        void platform_scale(float sx, float sy) override;
        void platform_translate(float tx, float ty) override;
        void platform_concatenate_transform(const matrix3x2& transform) override;

    private:
        std::vector<canvas_op> ops_;
    };
} // namespace maui::graphics

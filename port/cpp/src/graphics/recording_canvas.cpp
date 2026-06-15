// maui::graphics::recording_canvas — out-of-line definitions. See recording_canvas.hpp. Each member
// appends one op struct; the state-bearing members additionally run the abstract_canvas machinery
// (ported from AbstractCanvas.cs) so the headless recorder behaves exactly like a platform canvas.

#include "maui/graphics/recording_canvas.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "maui/graphics/abstract_canvas.hpp"
#include "maui/graphics/blend_mode.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/horizontal_alignment.hpp"
#include "maui/graphics/i_graphics_image.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/graphics/text/i_attributed_text.hpp"
#include "maui/graphics/text_flow.hpp"
#include "maui/graphics/vertical_alignment.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::graphics
{
    const std::vector<canvas_op>& recording_canvas::ops() const
    {
        return ops_;
    }

    void recording_canvas::clear()
    {
        ops_.clear();
    }

    void recording_canvas::set_miter_limit(float value)
    {
        ops_.emplace_back(canvas_ops::set_miter_limit{.value = value});
    }

    void recording_canvas::set_stroke_color(const color& value)
    {
        ops_.emplace_back(canvas_ops::set_stroke_color{.value = value});
    }

    void recording_canvas::set_stroke_line_cap(line_cap value)
    {
        ops_.emplace_back(canvas_ops::set_stroke_line_cap{.value = value});
    }

    void recording_canvas::set_stroke_line_join(line_join value)
    {
        ops_.emplace_back(canvas_ops::set_stroke_line_join{.value = value});
    }

    void recording_canvas::set_fill_color(const color& value)
    {
        ops_.emplace_back(canvas_ops::set_fill_color{.value = value});
    }

    void recording_canvas::set_font_color(const color& value)
    {
        ops_.emplace_back(canvas_ops::set_font_color{.value = value});
    }

    void recording_canvas::set_font(const font& value)
    {
        ops_.emplace_back(canvas_ops::set_font{.value = value});
    }

    void recording_canvas::set_font_size(float value)
    {
        ops_.emplace_back(canvas_ops::set_font_size{.value = value});
    }

    void recording_canvas::set_alpha(float value)
    {
        ops_.emplace_back(canvas_ops::set_alpha{.value = value});
    }

    void recording_canvas::set_antialias(bool value)
    {
        // Recorded (unlike PictureCanvas, which drops it — see the header note).
        ops_.emplace_back(canvas_ops::set_antialias{.value = value});
    }

    void recording_canvas::set_blend_mode(blend_mode value)
    {
        ops_.emplace_back(canvas_ops::set_blend_mode{.value = value});
    }

    void recording_canvas::fill_path(const path_f& path, winding_mode winding)
    {
        ops_.emplace_back(canvas_ops::fill_path{.path = path, .winding = winding});
    }

    void recording_canvas::subtract_from_clip(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::subtract_from_clip{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::clip_path(const path_f& path, winding_mode winding)
    {
        ops_.emplace_back(canvas_ops::clip_path{.path = path, .winding = winding});
    }

    void recording_canvas::clip_rectangle(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::clip_rectangle{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                                    bool clockwise)
    {
        ops_.emplace_back(canvas_ops::fill_arc{.x = x,
                                               .y = y,
                                               .width = width,
                                               .height = height,
                                               .start_angle = start_angle,
                                               .end_angle = end_angle,
                                               .clockwise = clockwise});
    }

    void recording_canvas::fill_rectangle(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::fill_rectangle{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius)
    {
        ops_.emplace_back(canvas_ops::fill_rounded_rectangle{
            .x = x, .y = y, .width = width, .height = height, .corner_radius = corner_radius});
    }

    void recording_canvas::fill_ellipse(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::fill_ellipse{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::draw_string(std::string_view value, float x, float y, horizontal_alignment h_align)
    {
        ops_.emplace_back(canvas_ops::draw_string{.value = std::string(value), .x = x, .y = y, .h_align = h_align});
    }

    void recording_canvas::draw_string(std::string_view value, float x, float y, float width, float height,
                                       horizontal_alignment h_align, vertical_alignment v_align, text_flow flow,
                                       float line_spacing_adjustment)
    {
        ops_.emplace_back(canvas_ops::draw_string_in_bounds{.value = std::string(value),
                                                            .x = x,
                                                            .y = y,
                                                            .width = width,
                                                            .height = height,
                                                            .h_align = h_align,
                                                            .v_align = v_align,
                                                            .flow = flow,
                                                            .line_spacing_adjustment = line_spacing_adjustment});
    }

    void recording_canvas::draw_text(const text::i_attributed_text& value, float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::draw_text{
            .text = value.text(), .runs = value.runs(), .x = x, .y = y, .width = width, .height = height});
    }

    size_f recording_canvas::get_string_size(std::string_view value, const font& font, float font_size) const
    {
        (void)value;
        (void)font;
        (void)font_size;
        // C# PictureCanvas.GetStringSize: throw new NotSupportedException().
        throw std::logic_error("recording_canvas::get_string_size is not supported");
    }

    size_f recording_canvas::get_string_size(std::string_view value, const font& font, float font_size,
                                             horizontal_alignment h_align, vertical_alignment v_align) const
    {
        (void)value;
        (void)font;
        (void)font_size;
        (void)h_align;
        (void)v_align;
        throw std::logic_error("recording_canvas::get_string_size is not supported");
    }

    void recording_canvas::save_state()
    {
        ops_.emplace_back(canvas_ops::save_state{});
        abstract_canvas::save_state();
    }

    bool recording_canvas::restore_state()
    {
        ops_.emplace_back(canvas_ops::restore_state{});
        return abstract_canvas::restore_state();
    }

    void recording_canvas::reset_state()
    {
        // Recorded (PictureCanvas's ResetState records nothing — see the header note).
        ops_.emplace_back(canvas_ops::reset_state{});
        abstract_canvas::reset_state();
    }

    void recording_canvas::set_shadow(const size_f& offset, float blur, const color& shadow_color)
    {
        ops_.emplace_back(canvas_ops::set_shadow{.offset = offset, .blur = blur, .shadow_color = shadow_color});
    }

    void recording_canvas::set_fill_paint(const paint* fill_paint, const rect_f& rectangle)
    {
        // C# SetFillPaint(null, ...) falls back to Colors.White.AsPaint(); the op stores the paint's
        // background_color projection (paints are not clonable — see the header note).
        const color background = fill_paint != nullptr ? fill_paint->background_color() : colors::white;
        ops_.emplace_back(canvas_ops::set_fill_paint{.background_color = background, .rectangle = rectangle});
    }

    void recording_canvas::platform_set_stroke_size(float value)
    {
        ops_.emplace_back(canvas_ops::set_stroke_size{.value = value});
    }

    void recording_canvas::platform_set_stroke_dash_pattern(const std::vector<float>& pattern, float stroke_dash_offset,
                                                            float stroke_size)
    {
        ops_.emplace_back(canvas_ops::set_stroke_dash_pattern{
            .pattern = pattern, .dash_offset = stroke_dash_offset, .stroke_size = stroke_size});
    }

    void recording_canvas::platform_draw_line(float x1, float y1, float x2, float y2)
    {
        ops_.emplace_back(canvas_ops::draw_line{.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2});
    }

    void recording_canvas::platform_draw_arc(float x, float y, float width, float height, float start_angle,
                                             float end_angle, bool clockwise, bool closed)
    {
        ops_.emplace_back(canvas_ops::draw_arc{.x = x,
                                               .y = y,
                                               .width = width,
                                               .height = height,
                                               .start_angle = start_angle,
                                               .end_angle = end_angle,
                                               .clockwise = clockwise,
                                               .closed = closed});
    }

    void recording_canvas::platform_draw_rectangle(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::draw_rectangle{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::platform_draw_rounded_rectangle(float x, float y, float width, float height,
                                                           float corner_radius)
    {
        ops_.emplace_back(canvas_ops::draw_rounded_rectangle{
            .x = x, .y = y, .width = width, .height = height, .corner_radius = corner_radius});
    }

    void recording_canvas::platform_draw_ellipse(float x, float y, float width, float height)
    {
        ops_.emplace_back(canvas_ops::draw_ellipse{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::platform_draw_path(const path_f& path)
    {
        ops_.emplace_back(canvas_ops::draw_path{.path = path});
    }

    void recording_canvas::platform_draw_image(const i_graphics_image& image, float x, float y, float width,
                                               float height)
    {
        // Geometry only; the image is caller-owned and not retained (see the header note).
        (void)image;
        ops_.emplace_back(canvas_ops::draw_image{.x = x, .y = y, .width = width, .height = height});
    }

    void recording_canvas::platform_rotate(float degrees, float radians, float x, float y)
    {
        (void)radians; // derivable from degrees; the op mirrors the ICanvas-level arguments
        ops_.emplace_back(canvas_ops::rotate_at{.degrees = degrees, .x = x, .y = y});
    }

    void recording_canvas::platform_rotate(float degrees, float radians)
    {
        (void)radians;
        ops_.emplace_back(canvas_ops::rotate{.degrees = degrees});
    }

    void recording_canvas::platform_scale(float sx, float sy)
    {
        ops_.emplace_back(canvas_ops::scale{.sx = sx, .sy = sy});
    }

    void recording_canvas::platform_translate(float tx, float ty)
    {
        ops_.emplace_back(canvas_ops::translate{.tx = tx, .ty = ty});
    }

    void recording_canvas::platform_concatenate_transform(const matrix3x2& transform)
    {
        ops_.emplace_back(canvas_ops::concatenate_transform{.transform = transform});
    }
} // namespace maui::graphics

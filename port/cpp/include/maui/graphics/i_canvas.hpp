#pragma once
// maui::graphics::i_canvas  <=  Microsoft.Maui.Graphics.ICanvas
//
// The platform-agnostic 2D drawing surface. Ported member for member from
// src/Graphics/src/Graphics/ICanvas.cs; C#'s set-only properties become set_* methods.
//
// Deliberate deviations (recorded in port/STATUS.md):
//  - Color parameters are the port's non-nullable color value type; C#'s null-color fallbacks
//    (StrokeColor null -> black, FillColor null -> white, FontColor null -> black) cannot arise —
//    callers pass the documented fallback constant directly.
//  - set_stroke_dash_pattern takes std::vector<float>; the empty vector stands in for C#'s null
//    (both clear the dash pattern on every backend).
//  - IFont is the collapsed maui::graphics::font value type (see font.hpp).
//  - set_fill_paint takes `const paint*` (nullable, like the C# parameter — null falls back to a
//    white solid fill).
//  - DrawImage(IImage, ...) is documented-deferred: Microsoft.Maui.Graphics.IImage (the drawing-layer
//    image) is not yet ported. Add draw_image with it.

#include <string>
#include <string_view>
#include <vector>

#include "maui/graphics/blend_mode.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/horizontal_alignment.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/graphics/text_flow.hpp"
#include "maui/graphics/vertical_alignment.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::graphics
{
    class paint;

    namespace text
    {
        class i_attributed_text;
    } // namespace text

    class i_canvas
    {
    public:
        virtual ~i_canvas() = default;

        // ---- state setters (C# set-only properties) ----

        // C# ICanvas.DisplayScale { get; set; } — the UI scaling factor.
        [[nodiscard]] virtual float display_scale() const = 0;
        virtual void set_display_scale(float value) = 0;

        // C# ICanvas.StrokeSize { set; }.
        virtual void set_stroke_size(float value) = 0;
        // C# ICanvas.MiterLimit { set; }.
        virtual void set_miter_limit(float value) = 0;
        // C# ICanvas.StrokeColor { set; }.
        virtual void set_stroke_color(const color& value) = 0;
        // C# ICanvas.StrokeLineCap { set; }.
        virtual void set_stroke_line_cap(line_cap value) = 0;
        // C# ICanvas.StrokeLineJoin { set; }.
        virtual void set_stroke_line_join(line_join value) = 0;
        // C# ICanvas.StrokeDashPattern { set; } — dash/gap lengths in stroke-size units.
        virtual void set_stroke_dash_pattern(std::vector<float> value) = 0;
        // C# ICanvas.StrokeDashOffset { set; }.
        virtual void set_stroke_dash_offset(float value) = 0;
        // C# ICanvas.FillColor { set; }.
        virtual void set_fill_color(const color& value) = 0;
        // C# ICanvas.FontColor { set; }.
        virtual void set_font_color(const color& value) = 0;
        // C# ICanvas.Font { set; }.
        virtual void set_font(const font& value) = 0;
        // C# ICanvas.FontSize { set; }.
        virtual void set_font_size(float value) = 0;
        // C# ICanvas.Alpha { set; }.
        virtual void set_alpha(float value) = 0;
        // C# ICanvas.Antialias { set; }.
        virtual void set_antialias(bool value) = 0;
        // C# ICanvas.BlendMode { set; }.
        virtual void set_blend_mode(blend_mode value) = 0;

        // ---- paths ----

        // C# ICanvas.DrawPath(PathF).
        virtual void draw_path(const path_f& path) = 0;
        // C# ICanvas.FillPath(PathF, WindingMode).
        virtual void fill_path(const path_f& path, winding_mode winding) = 0;

        // ---- clipping ----

        // C# ICanvas.SubtractFromClip(x, y, width, height).
        virtual void subtract_from_clip(float x, float y, float width, float height) = 0;
        // C# ICanvas.ClipPath(PathF, WindingMode = NonZero).
        virtual void clip_path(const path_f& path, winding_mode winding = winding_mode::non_zero) = 0;
        // C# ICanvas.ClipRectangle(x, y, width, height).
        virtual void clip_rectangle(float x, float y, float width, float height) = 0;

        // ---- shapes ----

        // C# ICanvas.DrawLine(x1, y1, x2, y2).
        virtual void draw_line(float x1, float y1, float x2, float y2) = 0;
        // C# ICanvas.DrawArc(x, y, width, height, startAngle, endAngle, clockwise, closed).
        virtual void draw_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                              bool clockwise, bool closed) = 0;
        // C# ICanvas.FillArc(x, y, width, height, startAngle, endAngle, clockwise).
        virtual void fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                              bool clockwise) = 0;
        // C# ICanvas.DrawRectangle(x, y, width, height).
        virtual void draw_rectangle(float x, float y, float width, float height) = 0;
        // C# ICanvas.FillRectangle(x, y, width, height).
        virtual void fill_rectangle(float x, float y, float width, float height) = 0;
        // C# ICanvas.DrawRoundedRectangle(x, y, width, height, cornerRadius).
        virtual void draw_rounded_rectangle(float x, float y, float width, float height, float corner_radius) = 0;
        // C# ICanvas.FillRoundedRectangle(x, y, width, height, cornerRadius).
        virtual void fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius) = 0;
        // C# ICanvas.DrawEllipse(x, y, width, height).
        virtual void draw_ellipse(float x, float y, float width, float height) = 0;
        // C# ICanvas.FillEllipse(x, y, width, height).
        virtual void fill_ellipse(float x, float y, float width, float height) = 0;

        // ---- text ----

        // C# ICanvas.DrawString(value, x, y, horizontalAlignment).
        virtual void draw_string(std::string_view value, float x, float y, horizontal_alignment h_align) = 0;
        // C# ICanvas.DrawString(value, x, y, width, height, horizontalAlignment, verticalAlignment,
        //                       textFlow = ClipBounds, lineSpacingAdjustment = 0).
        virtual void draw_string(std::string_view value, float x, float y, float width, float height,
                                 horizontal_alignment h_align, vertical_alignment v_align,
                                 text_flow flow = text_flow::clip_bounds, float line_spacing_adjustment = 0) = 0;
        // C# ICanvas.DrawText(IAttributedText, x, y, width, height).
        virtual void draw_text(const text::i_attributed_text& value, float x, float y, float width, float height) = 0;

        // C# ICanvas.GetStringSize(value, font, fontSize).
        [[nodiscard]] virtual size_f get_string_size(std::string_view value, const font& font,
                                                     float font_size) const = 0;
        // C# ICanvas.GetStringSize(value, font, fontSize, horizontalAlignment, verticalAlignment).
        [[nodiscard]] virtual size_f get_string_size(std::string_view value, const font& font, float font_size,
                                                     horizontal_alignment h_align,
                                                     vertical_alignment v_align) const = 0;

        // ---- transforms ----

        // C# ICanvas.Rotate(degrees, x, y) — clockwise for increasing angles, around (x, y).
        virtual void rotate(float degrees, float x, float y) = 0;
        // C# ICanvas.Rotate(degrees) — around the canvas origin.
        virtual void rotate(float degrees) = 0;
        // C# ICanvas.Scale(sx, sy).
        virtual void scale(float sx, float sy) = 0;
        // C# ICanvas.Translate(tx, ty).
        virtual void translate(float tx, float ty) = 0;
        // C# ICanvas.ConcatenateTransform(Matrix3x2).
        virtual void concatenate_transform(const matrix3x2& transform) = 0;

        // ---- graphics state ----

        // C# ICanvas.SaveState().
        virtual void save_state() = 0;
        // C# ICanvas.RestoreState() — true if a saved state was restored.
        virtual bool restore_state() = 0;
        // C# ICanvas.ResetState().
        virtual void reset_state() = 0;

        // ---- effects ----

        // C# ICanvas.SetShadow(SizeF offset, float blur, Color color).
        virtual void set_shadow(const size_f& offset, float blur, const color& shadow_color) = 0;
        // C# ICanvas.SetFillPaint(Paint paint, RectF rectangle) — null paint = solid white.
        virtual void set_fill_paint(const paint* fill_paint, const rect_f& rectangle) = 0;

    protected:
        i_canvas() = default;
        i_canvas(const i_canvas&) = default;
        i_canvas(i_canvas&&) = default;
        i_canvas& operator=(const i_canvas&) = default;
        i_canvas& operator=(i_canvas&&) = default;
    };
} // namespace maui::graphics

#pragma once
// maui::platform::android::android_canvas  <=  Microsoft.Maui.Graphics.Platform.PlatformCanvas
//   (src/Graphics/src/Graphics/Platforms/Android/PlatformCanvas.cs + PlatformCanvasState.cs — the
//    android.graphics.Canvas-backed ICanvas, the Android twin of the apple_shared coregraphics_canvas).
//
// A JNI bridge: an i_canvas over a borrowed android.graphics.Canvas (+ an owned android.graphics.Paint
// the FILL/STROKE styles toggle between, and a reusable android.graphics.Path the path/shape ops build
// into). It is the seam the Android shape render replays through — MauiShapeView.onDraw(Canvas) builds
// one of these over the incoming Canvas and calls shape_view_platform::replay → shape_drawable.draw,
// the same drawable the apple/ios drawRect hosts render (src/platform/apple/graphics_host.mm). This
// unblocks the whole shapes family on Android (ellipse/line/polyline/polygon/path/rectangle/rounded
// rectangle) — the GradientDrawable shortcut box_view_handler.cpp used only ever expressed a solid
// rounded box.
//
// Like the apple canvas it extends abstract_canvas<canvas_state> so the save/restore stack, stroke
// scaling, lazy dash flush and transform tracking come for free (AbstractCanvas.cs); only the platform_*
// hooks + the non-templated i_canvas methods translate to android.graphics. C#'s PlatformCanvas.Android
// likewise derives AbstractCanvas and pushes into a Paint/Canvas pair, so this mirrors its structure.
//
// COORDINATE CONVENTION (display_scale): the framework lays out + draws in density-independent POINTS
// (the shape drawable's path_for_bounds takes the host bounds in points), while android.graphics.Canvas
// draws in PIXELS. The bridge applies one canvas.scale(density, density) at construction (set via
// set_display_scale before the first draw — matching how the headless/apple canvases carry display_scale)
// so every point-coordinate op below lands at the right pixel. Stroke widths/dash lengths are in the same
// point units and scale with the CTM, exactly as on the apple backend.
//
// JNI doctrine (PROFILE §8, the button/box partials' recipe): cache jclass globally + jmethodID once via
// default_jni_cache, clear_pending(env) after every call so no Java pending-exception state leaks out.
// The Canvas is BORROWED (the Java onDraw owns it, valid for the call only); the Paint + Path are OWNED
// (global refs minted in the ctor, released in the dtor). The bridge is created + destroyed inside one
// onDraw, so it never crosses threads — but it still uses scoped_env so it is robust to being built from
// any attached thread.
//
// DEFERRED (documented, none reached by a shape page — shapes carry no text and the gallery's fills are
// solid/transparent; verify against PlatformCanvas.Android if a gradient/image shape page is added):
//   - gradient / pattern / image fills: set_fill_paint handles solid + falls back to the paint's
//     background_color for the rest (a flat fill). A LinearGradient/RadialGradient Shader port is the
//     follow-up. // TODO: gradient/image fills via android.graphics.{LinearGradient,RadialGradient,Bitmap}Shader.
//   - text ops (draw_string/draw_text/get_string_size): no shape renders text. They no-op / return {0,0}
//     with this TODO rather than wiring Canvas.drawText. // TODO: text via android.graphics.Canvas.drawText.
//   - draw_image: shapes never blit an image; deferred with the same marker.

#include <jni.h>

#include <string_view>
#include <vector>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/graphics/abstract_canvas.hpp"
#include "maui/graphics/blend_mode.hpp"
#include "maui/graphics/canvas_state.hpp"
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
} // namespace maui::graphics

namespace maui::platform::android
{
    // The android.graphics-backed i_canvas. Built over a borrowed android.graphics.Canvas; owns its
    // Paint + Path (global refs). Non-copyable/movable (it owns JNI globals + holds a borrowed Canvas).
    class android_canvas final : public maui::graphics::abstract_canvas<maui::graphics::canvas_state>
    {
    public:
        // env: a JNIEnv* for the calling thread (the onDraw thread). canvas: a BORROWED
        // android.graphics.Canvas local ref (owned by the Java caller). The bridge mints its own Paint +
        // Path; a JNI failure (no Paint class etc.) leaves the bridge inert (every op short-circuits on a
        // null canvas/paint), so the draw degrades to nothing rather than crashing.
        android_canvas(JNIEnv* env, jobject canvas);
        ~android_canvas() override;
        android_canvas(const android_canvas&) = delete;
        android_canvas(android_canvas&&) = delete;
        android_canvas& operator=(const android_canvas&) = delete;
        android_canvas& operator=(android_canvas&&) = delete;

        // display_scale carries the device density; set_display_scale also applies the matching
        // canvas.scale so the point→pixel mapping is in place before the first op (see the header note).
        void set_display_scale(float value) override;

        // ---- the i_canvas surface abstract_canvas leaves to the backend ----
        void set_miter_limit(float value) override;
        void set_stroke_color(const maui::graphics::color& value) override;
        void set_stroke_line_cap(maui::graphics::line_cap value) override;
        void set_stroke_line_join(maui::graphics::line_join value) override;
        void set_fill_color(const maui::graphics::color& value) override;
        void set_font_color(const maui::graphics::color& value) override;
        void set_font(const maui::graphics::font& value) override;
        void set_font_size(float value) override;
        void set_alpha(float value) override;
        void set_antialias(bool value) override;
        void set_blend_mode(maui::graphics::blend_mode value) override;

        void fill_path(const maui::graphics::path_f& path, maui::graphics::winding_mode winding) override;
        void subtract_from_clip(float x, float y, float width, float height) override;
        void clip_path(const maui::graphics::path_f& path,
                       maui::graphics::winding_mode winding = maui::graphics::winding_mode::non_zero) override;
        void clip_rectangle(float x, float y, float width, float height) override;

        void fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                      bool clockwise) override;
        void fill_rectangle(float x, float y, float width, float height) override;
        void fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius) override;
        void fill_ellipse(float x, float y, float width, float height) override;

        // Text ops — deferred (no shape renders text; see the header note). They no-op / return {0,0}.
        void draw_string(std::string_view value, float x, float y,
                         maui::graphics::horizontal_alignment h_align) override;
        void draw_string(std::string_view value, float x, float y, float width, float height,
                         maui::graphics::horizontal_alignment h_align, maui::graphics::vertical_alignment v_align,
                         maui::graphics::text_flow flow = maui::graphics::text_flow::clip_bounds,
                         float line_spacing_adjustment = 0) override;
        void draw_text(const maui::graphics::text::i_attributed_text& value, float x, float y, float width,
                       float height) override;
        [[nodiscard]] maui::graphics::size_f get_string_size(std::string_view value, const maui::graphics::font& font,
                                                             float font_size) const override;
        [[nodiscard]] maui::graphics::size_f get_string_size(std::string_view value, const maui::graphics::font& font,
                                                             float font_size,
                                                             maui::graphics::horizontal_alignment h_align,
                                                             maui::graphics::vertical_alignment v_align) const override;

        void save_state() override;
        bool restore_state() override;
        void reset_state() override;

        void set_shadow(const maui::graphics::size_f& offset, float blur,
                        const maui::graphics::color& shadow_color) override;
        void set_fill_paint(const maui::graphics::paint* fill_paint, const maui::graphics::rect_f& rectangle) override;

    protected:
        // ---- the abstract_canvas platform hooks (C# PlatformCanvas's Platform* overrides) ----
        void platform_set_stroke_size(float value) override;
        void platform_set_stroke_dash_pattern(const std::vector<float>& pattern, float stroke_dash_offset,
                                              float stroke_size) override;
        void platform_draw_line(float x1, float y1, float x2, float y2) override;
        void platform_draw_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                               bool clockwise, bool closed) override;
        void platform_draw_rectangle(float x, float y, float width, float height) override;
        void platform_draw_rounded_rectangle(float x, float y, float width, float height, float corner_radius) override;
        void platform_draw_ellipse(float x, float y, float width, float height) override;
        void platform_draw_path(const maui::graphics::path_f& path) override;
        void platform_draw_image(const maui::graphics::i_graphics_image& image, float x, float y, float width,
                                 float height) override;
        void platform_rotate(float degrees, float radians, float x, float y) override;
        void platform_rotate(float degrees, float radians) override;
        void platform_scale(float sx, float sy) override;
        void platform_translate(float tx, float ty) override;
        void platform_concatenate_transform(const maui::graphics::matrix3x2& transform) override;

    private:
        // Build `path` into the owned android.graphics.Path (cleared first); returns the Path jobject, or
        // nullptr on a JNI failure. The single path-conversion seam (path_f → android.graphics.Path).
        jobject build_path(const maui::graphics::path_f& path);
        // Fill `path_obj` honouring the winding rule (Path.setFillType WINDING/EVEN_ODD) with the fill
        // paint; used by fill_path and the clip path's even-odd fill rule.
        void fill_built_path(jobject path_obj, maui::graphics::winding_mode winding);
        // Set the fill paint's color (Paint.setColor(argb)); clears any staged fill mode.
        void apply_fill_color(const maui::graphics::color& value);

        JNIEnv* env_ = nullptr;          // the calling-thread env (the onDraw thread; valid for the bridge's life)
        jobject canvas_ = nullptr;       // BORROWED android.graphics.Canvas (the Java onDraw owns it)
        jobject fill_paint_ = nullptr;   // OWNED global ref — android.graphics.Paint, style FILL
        jobject stroke_paint_ = nullptr; // OWNED global ref — android.graphics.Paint, style STROKE
        jobject path_ = nullptr;         // OWNED global ref — reusable android.graphics.Path (rewind per build)
        jobject rectf_ = nullptr;        // OWNED global ref — reusable android.graphics.RectF (oval/round-rect)
        float font_size_ = 10.0F;
        maui::graphics::color font_color_{0, 0, 0, 1};
        maui::graphics::font font_;
    };
} // namespace maui::platform::android

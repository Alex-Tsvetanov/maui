#pragma once
// maui::platform::apple_shared::coregraphics_canvas  <=  Microsoft.Maui.Graphics.Platform.PlatformCanvas
//   (src/Graphics/src/Graphics/Platforms/MaciOS/PlatformCanvas.cs — the SHARED CoreGraphics canvas:
//    one implementation for both the AppKit and UIKit backends, exactly like C#'s MaciOS folder)
//   + coregraphics_canvas_state  <=  ...Platform.PlatformCanvasState (a tight cluster — the state
//     only exists for this canvas).
//
// A CGContextRef-backed i_canvas over the abstract_canvas state machinery. The header is pure
// C++/CoreGraphics (no AppKit/UIKit/ObjC types) so it can be included from any TU of either Apple
// backend; the definitions live in coregraphics_canvas.mm (CoreText needs the ObjC bridge).
//
// Ownership: the canvas does NOT own the CGContextRef (the caller/host does — C#'s Context property
// is likewise a plain reference slot); it DOES own the staged CGGradientRef (released on every
// fill-color/paint change, after each gradient draw, on restore_state and in the destructor).
//
// First-cut deviations (recorded in port/STATUS.md):
//  - PatternPaint/ImagePaint fills ARE now ported (unit U-IP): SetFillPaint detects them and the Fill*
//    methods route through FillWithPattern (CGPattern + ConstantSpacing tiling, the tile callback
//    re-enters a nested canvas to draw the pattern) / FillWithImage (CGPattern + NoDistortion tiling,
//    the tile callback blits the image's CGImage). DrawImage(IImage) was already ported. This is the
//    drawing-canvas (ICanvas Fill*) path; the view-background ImageBrush-as-Background path (the
//    MauiCALayer equivalent) stays deferred — C# iOS faithfully throws there.
//  - C#'s Func<CGColorSpace> getColorspace ctor knob is collapsed to DeviceRGB (the C# default).
//  - SetShadow ports the UIKit/Catalyst branch (no MONOMAC height negation — modern MAUI does not
//    define MONOMAC; hosts with flipped coordinates flip the CTM, not the shadow).
//  - Fonts resolve through CoreText (CTFontCreateWithName / the UI-font default + bold/italic
//    symbolic traits); C# FontExtensions' deeper family/weight resolution lands with a real
//    typography consumer. draw_text applies the bold/italic/underline/foreground/background run
//    attributes (AttributedTextExtensions.AsNSAttributedString's CoreText-compatible set);
//    strikethrough/sub/superscript/markers are deferred with it.

#include <CoreGraphics/CoreGraphics.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/graphics/abstract_canvas.hpp"
#include "maui/graphics/blend_mode.hpp"
#include "maui/graphics/canvas_state.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/font.hpp"
#include "maui/graphics/horizontal_alignment.hpp"
#include "maui/graphics/i_graphics_image.hpp"
#include "maui/graphics/i_pattern.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/graphics/text_flow.hpp"
#include "maui/graphics/vertical_alignment.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::platform::apple_shared
{
    // C# PlatformCanvasState: canvas_state plus the Shadowed flag (the gradient fill pre-fills the
    // shape when a shadow is active, so the shadow is cast by the clipped gradient too).
    class coregraphics_canvas_state final : public maui::graphics::canvas_state
    {
    public:
        [[nodiscard]] bool shadowed() const
        {
            return shadowed_;
        }
        void set_shadowed(bool value)
        {
            shadowed_ = value;
        }

    private:
        bool shadowed_ = false;
    };

    class coregraphics_canvas final : public maui::graphics::abstract_canvas<coregraphics_canvas_state>
    {
    public:
        coregraphics_canvas() = default;
        // C# PlatformCanvas(...) + Context = context.
        explicit coregraphics_canvas(CGContextRef context);
        ~coregraphics_canvas() override;
        coregraphics_canvas(const coregraphics_canvas&) = delete;
        coregraphics_canvas(coregraphics_canvas&&) = delete;
        coregraphics_canvas& operator=(const coregraphics_canvas&) = delete;
        coregraphics_canvas& operator=(coregraphics_canvas&&) = delete;

        // C# PlatformCanvas.Context { get; set; } — non-owning; setting installs the DeviceRGB
        // fill/stroke colorspaces and resets the state.
        [[nodiscard]] CGContextRef context() const;
        void set_context(CGContextRef context);

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

        void draw_string(std::string_view value, float x, float y,
                         maui::graphics::horizontal_alignment h_align) override;
        void draw_string(std::string_view value, float x, float y, float width, float height,
                         maui::graphics::horizontal_alignment h_align, maui::graphics::vertical_alignment v_align,
                         maui::graphics::text_flow flow = maui::graphics::text_flow::clip_bounds,
                         float line_spacing_adjustment = 0) override;
        void draw_text(const maui::graphics::text::i_attributed_text& value, float x, float y, float width,
                       float height) override;

        // C# PlatformStringSizeService: NSString.StringSize for the plain overload, the CoreText
        // framesetter for the aligned one — both answered here via CTLine/CTFramesetter bounds.
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
        // Which gradient DrawGradient should paint (C# tracks this via the _paint reference; the
        // port snapshots the geometry — paints are not retained).
        enum class staged_gradient : unsigned char
        {
            none,
            linear,
            radial
        };

        void release_gradient();
        // C# PlatformCanvas.FillWithGradient(Func<bool> action).
        void fill_with_gradient(const std::function<bool()>& add_shape);
        // C# PlatformCanvas.DrawGradient — paints and releases the staged gradient.
        void draw_gradient();
        // C# PlatformCanvas.FillWithPattern(x, y, drawingAction) — install a CGPattern (ConstantSpacing
        // tiling) whose tile callback re-enters fill_pattern_canvas_ and calls fill_pattern_->draw, then
        // run the fill action under the pattern fill colorspace.
        void fill_with_pattern(float x, float y, const std::function<void()>& drawing_action);
        // C# PlatformCanvas.FillWithImage(x, y, drawingAction) — install a CGPattern (NoDistortion tiling)
        // whose tile callback blits fill_image_'s CGImage, then run the fill action.
        void fill_with_image(float x, float y, const std::function<void()>& drawing_action);
        // C# PlatformCanvas.DrawPatternCallback — bind the nested canvas to the tile context and draw.
        void draw_pattern_callback(CGContextRef tile_context, maui::graphics::i_pattern* fill_pattern);
        // The CGPattern tile callbacks (C function pointers). Static members so they reach the private
        // draw_pattern_callback / nested canvas; the per-tile payload arrives via the CGPattern `info`.
        static void pattern_tile_callback(void* info, CGContextRef tile_context);
        static void image_tile_callback(void* info, CGContextRef tile_context);
        void draw_string_at(std::string_view value, float x, float y);

        CGContextRef context_ = nullptr; // non-owning
        bool antialias_ = true;
        maui::graphics::color font_color_{0, 0, 0, 1}; // C# default Colors.Black
        maui::graphics::font font_;
        float font_size_ = 10.0F;

        CGGradientRef gradient_ = nullptr; // owned
        staged_gradient gradient_kind_ = staged_gradient::none;
        maui::graphics::rect_f gradient_rectangle_{};
        maui::graphics::point gradient_start_{};  // linear: relative start point
        maui::graphics::point gradient_end_{};    // linear: relative end point
        maui::graphics::point gradient_center_{}; // radial: relative center
        double gradient_radius_ = 0;              // radial: relative radius
        float gradient_min_alpha_ = 1;            // min(StartColor.A, EndColor.A) for the shadow pre-fill

        // C# _fillPattern / _fillImage — the staged pattern/image fill (non-owning; the paint's pattern
        // or image is owned by the caller, read during the fill only). At most one of the three fill
        // kinds (gradient / pattern / image) is active at a time, mirroring SetFillPaint.
        maui::graphics::i_pattern* fill_pattern_ = nullptr;      // non-owning (the pattern_paint owns it)
        maui::graphics::i_graphics_image* fill_image_ = nullptr; // non-owning (the caller owns the image)
        // C# _fillPatternCanvas — the nested canvas the pattern tile callback draws into (lazily created,
        // reused across tiles/fills). unique_ptr because coregraphics_canvas is non-movable + final.
        std::unique_ptr<coregraphics_canvas> fill_pattern_canvas_;
    };
} // namespace maui::platform::apple_shared

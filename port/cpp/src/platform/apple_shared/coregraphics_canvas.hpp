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
//  - PatternPaint/ImagePaint fills + DrawImage(IImage) are deferred — those paint kinds and the
//    drawing-layer IImage are not yet ported (FillWithPattern/FillWithImage/DrawImageCallback land
//    with them).
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
#include <string_view>
#include <vector>

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
    };
} // namespace maui::platform::apple_shared

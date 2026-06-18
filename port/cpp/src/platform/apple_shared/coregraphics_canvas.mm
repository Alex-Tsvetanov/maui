// maui::platform::apple_shared::coregraphics_canvas — the CoreGraphics i_canvas, shared by the
// apple (AppKit) and ios (UIKit) backends. See coregraphics_canvas.hpp. Ported from
// src/Graphics/src/Graphics/Platforms/MaciOS/PlatformCanvas.cs (+ PlatformStringSizeService.cs and
// Text/AttributedTextExtensions.cs for the CoreText pieces). Objective-C++ with ARC for the
// Foundation objects; the CoreFoundation/CoreGraphics refs follow the Create rule explicitly.

#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

#include "coregraphics_canvas.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>
#include <vector>

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/i_graphics_image.hpp"
#include "maui/graphics/i_pattern.hpp"
#include "maui/graphics/image_paint.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/pattern_paint.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text/i_attributed_text.hpp"
#include "maui/graphics/text/text_attributes.hpp"

namespace maui::platform::apple_shared
{
    namespace
    {
        // COPY of the path_f -> CGPath walk in src/platform/apple/apple_visual_ops.hpp /
        // src/platform/ios/ios_visual_ops.hpp (both port GraphicsExtensions.AsCGPath; the two are
        // already identical). Copied rather than included because those headers import AppKit/UIKit
        // respectively — this TU must stay backend-neutral. DEDUP for the coordinator: hoist the
        // walk into apple_shared and have both visual-ops headers include it.
        // CF_RETURNS_RETAINED: +1-owned (the Create rule) — the caller must CGPathRelease it.
        CGMutablePathRef path_to_cg_path(const maui::graphics::path_f& path) CF_RETURNS_RETAINED
        {
            CGMutablePathRef cg = CGPathCreateMutable();
            int point_index = 0;
            int arc_angle_index = 0;
            int arc_clockwise_index = 0;
            const auto& operations = path.segment_types();
            for (const auto type : operations)
            {
                switch (type)
                {
                    case maui::graphics::path_operation::move: {
                        const maui::graphics::point_f p = path[point_index++];
                        CGPathMoveToPoint(cg, nullptr, p.x, p.y);
                        break;
                    }
                    case maui::graphics::path_operation::line: {
                        const maui::graphics::point_f p = path[point_index++];
                        CGPathAddLineToPoint(cg, nullptr, p.x, p.y);
                        break;
                    }
                    case maui::graphics::path_operation::quad: {
                        const maui::graphics::point_f control = path[point_index++];
                        const maui::graphics::point_f end = path[point_index++];
                        CGPathAddQuadCurveToPoint(cg, nullptr, control.x, control.y, end.x, end.y);
                        break;
                    }
                    case maui::graphics::path_operation::cubic: {
                        const maui::graphics::point_f control1 = path[point_index++];
                        const maui::graphics::point_f control2 = path[point_index++];
                        const maui::graphics::point_f end = path[point_index++];
                        CGPathAddCurveToPoint(cg, nullptr, control1.x, control1.y, control2.x, control2.y, end.x,
                                              end.y);
                        break;
                    }
                    case maui::graphics::path_operation::arc: {
                        const maui::graphics::point_f top_left = path[point_index++];
                        const maui::graphics::point_f bottom_right = path[point_index++];
                        const float start_angle = path.get_arc_angle(arc_angle_index++);
                        const float end_angle = path.get_arc_angle(arc_angle_index++);
                        const bool clockwise = path.get_arc_clockwise(arc_clockwise_index++);
                        const CGFloat cx = (bottom_right.x + top_left.x) / 2;
                        const CGFloat cy = (bottom_right.y + top_left.y) / 2;
                        const CGFloat radius = (bottom_right.x - top_left.x) / 2;
                        constexpr double deg_to_rad = M_PI / 180.0;
                        CGPathAddArc(cg, nullptr, cx, cy, radius, -start_angle * deg_to_rad, -end_angle * deg_to_rad,
                                     !clockwise);
                        break;
                    }
                    case maui::graphics::path_operation::close:
                        CGPathCloseSubpath(cg);
                        break;
                }
            }
            return cg;
        }

        // Never nil: invalid UTF-8 falls back to the empty string (the NSString init is the
        // nullable step; CoreText takes non-null strings).
        NSString* to_ns_string(std::string_view value)
        {
            NSString* const result = [[NSString alloc] initWithBytes:value.data()
                                                              length:value.size()
                                                            encoding:NSUTF8StringEncoding];
            return result != nil ? result : @"";
        }

        // C# `new CGColor(r, g, b, a)` — a DeviceRGB color. +1-owned. Nullable by the CG contract
        // (callers guard); in practice DeviceRGB construction never fails.
        CGColorRef to_cg_color(const maui::graphics::color& value) CF_RETURNS_RETAINED
        {
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            const std::array<CGFloat, 4> components{value.red, value.green, value.blue, value.alpha};
            CGColorRef cg = CGColorCreate(space, components.data());
            CGColorSpaceRelease(space);
            return cg;
        }

        // C# PlatformCanvas.BlendMode setter's switch, CGBlendMode for CGBlendMode.
        CGBlendMode to_cg_blend_mode(maui::graphics::blend_mode value)
        {
            using maui::graphics::blend_mode;
            switch (value)
            {
                case blend_mode::clear:
                    return kCGBlendModeClear;
                case blend_mode::color:
                    return kCGBlendModeColor;
                case blend_mode::color_burn:
                    return kCGBlendModeColorBurn;
                case blend_mode::color_dodge:
                    return kCGBlendModeColorDodge;
                case blend_mode::copy:
                    return kCGBlendModeCopy;
                case blend_mode::darken:
                    return kCGBlendModeDarken;
                case blend_mode::destination_atop:
                    return kCGBlendModeDestinationAtop;
                case blend_mode::destination_in:
                    return kCGBlendModeDestinationIn;
                case blend_mode::destination_out:
                    return kCGBlendModeDestinationOut;
                case blend_mode::destination_over:
                    return kCGBlendModeDestinationOver;
                case blend_mode::difference:
                    return kCGBlendModeDifference;
                case blend_mode::exclusion:
                    return kCGBlendModeExclusion;
                case blend_mode::hard_light:
                    return kCGBlendModeHardLight;
                case blend_mode::hue:
                    return kCGBlendModeHue;
                case blend_mode::lighten:
                    return kCGBlendModeLighten;
                case blend_mode::luminosity:
                    return kCGBlendModeLuminosity;
                case blend_mode::multiply:
                    return kCGBlendModeMultiply;
                case blend_mode::overlay:
                    return kCGBlendModeOverlay;
                case blend_mode::plus_darker:
                    return kCGBlendModePlusDarker;
                case blend_mode::plus_lighter:
                    return kCGBlendModePlusLighter;
                case blend_mode::saturation:
                    return kCGBlendModeSaturation;
                case blend_mode::screen:
                    return kCGBlendModeScreen;
                case blend_mode::soft_light:
                    return kCGBlendModeSoftLight;
                case blend_mode::source_atop:
                    return kCGBlendModeSourceAtop;
                case blend_mode::source_in:
                    return kCGBlendModeSourceIn;
                case blend_mode::source_out:
                    return kCGBlendModeSourceOut;
                case blend_mode::xor_:
                    return kCGBlendModeXOR;
                case blend_mode::normal:
                default:
                    return kCGBlendModeNormal;
            }
        }

        // C# IFont.ToCTFont / FontExtensions.GetDefaultCTFont (first cut, see the header note):
        // the default font is the system UI font; bold (weight >= 700) / italic map to symbolic
        // traits. +1-owned.
        CTFontRef to_ct_font(const maui::graphics::font& value, float size) CF_RETURNS_RETAINED
        {
            CTFontRef base = nullptr;
            if (value.is_default())
            {
                base = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr);
            }
            else
            {
                NSString* const name = to_ns_string(value.name());
                base = CTFontCreateWithName((__bridge CFStringRef)name, size, nullptr);
            }
            if (base == nullptr)
            {
                // CTFontCreateUIFontForLanguage is nullable; CTFontCreateWithName is not — fall
                // back through it so the result is non-null on every path.
                base = CTFontCreateWithName(CFSTR("Helvetica"), size, nullptr);
            }

            CTFontSymbolicTraits traits = 0;
            if (value.weight() >= maui::graphics::font_weights::bold)
            {
                traits |= kCTFontTraitBold;
            }
            if (value.style_type() != maui::graphics::font_style_type::normal)
            {
                traits |= kCTFontTraitItalic;
            }
            if (traits != 0)
            {
                CTFontRef derived = CTFontCreateCopyWithSymbolicTraits(base, size, nullptr, traits, traits);
                if (derived != nullptr)
                {
                    CFRelease(base);
                    return derived;
                }
            }
            return base;
        }

        // C# font.WithSymbolicTraits(...) for the per-run Bold/Italic attributes. +1-owned.
        CTFontRef with_symbolic_traits(CTFontRef base, float size, CTFontSymbolicTraits traits) CF_RETURNS_RETAINED
        {
            CTFontRef derived = CTFontCreateCopyWithSymbolicTraits(base, size, nullptr, traits, traits);
            if (derived != nullptr)
            {
                return derived;
            }
            return static_cast<CTFontRef>(CFRetain(base));
        }

        // The CTParagraphStyle carrying the horizontal alignment (DrawStringInPlatformPath's
        // CTParagraphStyleSettings switch). +1-owned.
        CTParagraphStyleRef to_ct_paragraph_style(maui::graphics::horizontal_alignment h_align) CF_RETURNS_RETAINED
        {
            CTTextAlignment alignment = kCTTextAlignmentLeft;
            switch (h_align)
            {
                case maui::graphics::horizontal_alignment::center:
                    alignment = kCTTextAlignmentCenter;
                    break;
                case maui::graphics::horizontal_alignment::right:
                    alignment = kCTTextAlignmentRight;
                    break;
                case maui::graphics::horizontal_alignment::justified:
                    alignment = kCTTextAlignmentJustified;
                    break;
                case maui::graphics::horizontal_alignment::left:
                default:
                    break;
            }
            const std::array<CTParagraphStyleSetting, 1> settings{
                CTParagraphStyleSetting{
                    .spec = kCTParagraphStyleSpecifierAlignment, .valueSize = sizeof(alignment), .value = &alignment},
            };
            return CTParagraphStyleCreate(settings.data(), settings.size());
        }

        // C# PlatformStringSizeService.GetTextSize(CTFrame): per-line typographic bounds folded
        // into (max line width) x (maxY - minY over origin±ascent/descent).
        maui::graphics::rect_f get_text_size(CTFrameRef frame)
        {
            float min_y = std::numeric_limits<float>::max();
            float max_y = std::numeric_limits<float>::lowest();
            float width = 0;

            CFArrayRef lines = CTFrameGetLines(frame);
            const CFIndex count = CFArrayGetCount(lines);
            std::vector<CGPoint> origins(static_cast<std::size_t>(std::max<CFIndex>(count, 1)));
            CTFrameGetLineOrigins(frame, CFRangeMake(0, 0), origins.data());

            for (CFIndex i = 0; i < count; i++)
            {
                const auto* const line = static_cast<CTLineRef>(CFArrayGetValueAtIndex(lines, i)); // borrowed
                CGFloat ascent = 0;
                CGFloat descent = 0;
                CGFloat leading = 0;
                const auto line_width =
                    static_cast<float>(CTLineGetTypographicBounds(line, &ascent, &descent, &leading));

                width = std::max(width, line_width);

                const CGPoint origin = origins[static_cast<std::size_t>(i)];
                min_y = std::min(min_y, static_cast<float>(origin.y - ascent));
                max_y = std::max(max_y, static_cast<float>(origin.y + descent));
            }

            if (count == 0)
            {
                return {0, 0, 0, 0};
            }
            return {0, min_y, width, std::max(0.0F, max_y - min_y)};
        }

        // C# AttributedTextExtensions.AsNSAttributedString (the CoreText-compatible attribute set;
        // strikethrough/sub-/superscript/markers are documented-deferred — header note).
        NSAttributedString* as_ct_attributed_string(const maui::graphics::text::i_attributed_text& value,
                                                    const maui::graphics::font& context_font, float context_font_size,
                                                    const maui::graphics::color& context_font_color)
        {
            CTFontRef default_font = to_ct_font(context_font, context_font_size);
            CGColorRef default_color = to_cg_color(context_font_color);
            NSMutableDictionary* const default_attributes = [NSMutableDictionary dictionary];
            default_attributes[(__bridge id)kCTFontAttributeName] = (__bridge id)default_font;
            if (default_color != nullptr) // CGColorCreate is nullable by contract; never null here
            {
                default_attributes[(__bridge id)kCTForegroundColorAttributeName] = (__bridge id)default_color;
            }
            NSMutableAttributedString* const result =
                [[NSMutableAttributedString alloc] initWithString:to_ns_string(value.text())
                                                       attributes:default_attributes];

            for (const maui::graphics::text::attributed_text_run& run : value.runs())
            {
                NSMutableDictionary* const attributes = [NSMutableDictionary dictionary];
                const maui::graphics::text::text_attributes& bag = run.attributes();

                CTFontSymbolicTraits traits = 0;
                if (bag.get_bold())
                {
                    traits |= kCTFontTraitBold;
                }
                if (bag.get_italic())
                {
                    traits |= kCTFontTraitItalic;
                }
                if (traits != 0)
                {
                    CTFontRef run_font = with_symbolic_traits(default_font, context_font_size, traits);
                    attributes[(__bridge id)kCTFontAttributeName] = (__bridge id)run_font;
                    CFRelease(run_font); // the dictionary retains it
                }

                if (bag.get_underline())
                {
                    attributes[(__bridge id)kCTUnderlineStyleAttributeName] =
                        [NSNumber numberWithInt:kCTUnderlineStyleSingle];
                }

                if (const auto foreground = bag.get_foreground_color())
                {
                    CGColorRef cg = to_cg_color(maui::graphics::color::parse(*foreground));
                    if (cg != nullptr)
                    {
                        attributes[(__bridge id)kCTForegroundColorAttributeName] = (__bridge id)cg;
                        CGColorRelease(cg); // the dictionary retains it
                    }
                }

                if (const auto background = bag.get_background_color())
                {
                    CGColorRef cg = to_cg_color(maui::graphics::color::parse(*background));
                    if (cg != nullptr)
                    {
                        attributes[(__bridge id)kCTBackgroundColorAttributeName] = (__bridge id)cg;
                        CGColorRelease(cg);
                    }
                }

                if (attributes.count > 0)
                {
                    [result addAttributes:attributes
                                    range:NSMakeRange(static_cast<NSUInteger>(run.start()),
                                                      static_cast<NSUInteger>(run.length()))];
                }
            }

            CGColorRelease(default_color);
            CFRelease(default_font);
            return result;
        }

        // C# GeometryUtil.DegreesToRadians + PlatformCanvas.NormalizeAngle (to [0, 2pi)).
        float to_radians(float degrees)
        {
            return static_cast<float>(std::numbers::pi) * degrees / 180.0F;
        }
        float normalize_angle(float angle)
        {
            constexpr auto two_pi = static_cast<float>(2 * std::numbers::pi);
            return std::fmod(std::fmod(angle, two_pi) + two_pi, two_pi);
        }

        // CgContextExtensions.AddRoundedRectangle — the arc-to-point rounded rectangle walk.
        void add_rounded_rectangle(CGContextRef context, CGFloat x, CGFloat y, CGFloat width, CGFloat height,
                                   CGFloat corner_radius)
        {
            CGFloat final_corner_radius = corner_radius;

            const CGRect rect = CGRectMake(x, y, width, height);

            if (final_corner_radius > rect.size.width)
            {
                final_corner_radius = rect.size.width / 2;
            }
            if (final_corner_radius > rect.size.height)
            {
                final_corner_radius = rect.size.height / 2;
            }

            const CGFloat min_x = rect.origin.x;
            const CGFloat min_y = rect.origin.y;
            const CGFloat max_x = min_x + rect.size.width;
            const CGFloat max_y = min_y + rect.size.height;
            const CGFloat mid_x = min_x + (rect.size.width / 2);
            const CGFloat mid_y = min_y + (rect.size.height / 2);

            CGContextMoveToPoint(context, min_x, mid_y);
            CGContextAddArcToPoint(context, min_x, min_y, mid_x, min_y, final_corner_radius);
            CGContextAddArcToPoint(context, max_x, min_y, max_x, mid_y, final_corner_radius);
            CGContextAddArcToPoint(context, max_x, max_y, mid_x, max_y, final_corner_radius);
            CGContextAddArcToPoint(context, min_x, max_y, min_x, mid_y, final_corner_radius);
            CGContextClosePath(context);
        }

        // C# PlatformCanvas.DrawStringInPlatformPath — the CoreText frame draw with the vertical
        // alignment translate. text_flow/lineSpacingAdjustment are accepted-but-unused exactly as
        // in C# (the parameters never reach the framesetter there either).
        void draw_string_in_path(CGContextRef context, CGPathRef path, std::string_view value,
                                 maui::graphics::horizontal_alignment h_align,
                                 maui::graphics::vertical_alignment v_align, const maui::graphics::font& font,
                                 float font_size, const maui::graphics::color& font_color, float ix, float iy)
        {
            const CGRect rect = CGPathGetPathBoundingBox(path);

            CGContextSaveGState(context);
            CGContextTranslateCTM(context, 0, rect.size.height);
            CGContextScaleCTM(context, 1, -1);

            // C# sets TextMatrix = identity (its subsequent TextMatrix.Translate(ix, iy) mutates a
            // returned copy — a C# no-op, not ported).
            CGContextSetTextMatrix(context, CGAffineTransformIdentity);

            CTFontRef ct_font = to_ct_font(font, font_size);

            float aligned_iy = iy;
            if (v_align == maui::graphics::vertical_alignment::center)
            {
                aligned_iy += -static_cast<float>(CTFontGetDescent(ct_font) / 2);
            }
            else if (v_align == maui::graphics::vertical_alignment::bottom)
            {
                aligned_iy += -static_cast<float>(CTFontGetDescent(ct_font));
            }

            CGColorRef foreground = to_cg_color(font_color);
            CTParagraphStyleRef paragraph_style = to_ct_paragraph_style(h_align);
            NSMutableDictionary* const attributes = [NSMutableDictionary dictionary];
            attributes[(__bridge id)kCTFontAttributeName] = (__bridge id)ct_font;
            if (foreground != nullptr) // CGColorCreate is nullable by contract; never null here
            {
                attributes[(__bridge id)kCTForegroundColorAttributeName] = (__bridge id)foreground;
            }
            attributes[(__bridge id)kCTParagraphStyleAttributeName] = (__bridge id)paragraph_style;
            NSAttributedString* const attributed_string = [[NSAttributedString alloc] initWithString:to_ns_string(value)
                                                                                          attributes:attributes];

            CTFramesetterRef framesetter =
                CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)attributed_string);
            CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), path, nullptr);

            if (frame != nullptr)
            {
                if (v_align != maui::graphics::vertical_alignment::top)
                {
                    const maui::graphics::rect_f text_frame_size = get_text_size(frame);
                    if (text_frame_size.height > 0)
                    {
                        const float dy =
                            v_align == maui::graphics::vertical_alignment::bottom
                                ? static_cast<float>(rect.size.height) - text_frame_size.height + aligned_iy
                                : ((static_cast<float>(rect.size.height) - text_frame_size.height) / 2) + aligned_iy;
                        CGContextTranslateCTM(context, -ix, -dy);
                    }
                }
                else
                {
                    CGContextTranslateCTM(context, -ix, -aligned_iy);
                }

                CTFrameDraw(frame, context);
                CFRelease(frame);
            }

            CFRelease(framesetter);
            CFRelease(paragraph_style);
            CGColorRelease(foreground);
            CFRelease(ct_font);

            CGContextRestoreGState(context);
        }
    } // namespace

    coregraphics_canvas::coregraphics_canvas(CGContextRef context)
    {
        set_context(context);
    }

    coregraphics_canvas::~coregraphics_canvas()
    {
        release_gradient();
    }

    CGContextRef coregraphics_canvas::context() const
    {
        return context_;
    }

    void coregraphics_canvas::set_context(CGContextRef context)
    {
        // C# Context setter: install the colorspace (DeviceRGB — the collapsed getColorspace
        // default) and reset the state. Non-owning.
        context_ = context;
        if (context_ != nullptr)
        {
            CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
            CGContextSetFillColorSpace(context_, colorspace);
            CGContextSetStrokeColorSpace(context_, colorspace);
            CGColorSpaceRelease(colorspace);
        }
        reset_state();
    }

    void coregraphics_canvas::set_miter_limit(float value)
    {
        CGContextSetMiterLimit(context_, value);
    }

    void coregraphics_canvas::set_stroke_color(const maui::graphics::color& value)
    {
        CGContextSetRGBStrokeColor(context_, value.red, value.green, value.blue, value.alpha);
    }

    void coregraphics_canvas::set_stroke_line_cap(maui::graphics::line_cap value)
    {
        switch (value)
        {
            case maui::graphics::line_cap::round:
                CGContextSetLineCap(context_, kCGLineCapRound);
                break;
            case maui::graphics::line_cap::square:
                CGContextSetLineCap(context_, kCGLineCapSquare);
                break;
            case maui::graphics::line_cap::butt:
            default:
                CGContextSetLineCap(context_, kCGLineCapButt);
                break;
        }
    }

    void coregraphics_canvas::set_stroke_line_join(maui::graphics::line_join value)
    {
        switch (value)
        {
            case maui::graphics::line_join::round:
                CGContextSetLineJoin(context_, kCGLineJoinRound);
                break;
            case maui::graphics::line_join::bevel:
                CGContextSetLineJoin(context_, kCGLineJoinBevel);
                break;
            case maui::graphics::line_join::miter:
            default:
                CGContextSetLineJoin(context_, kCGLineJoinMiter);
                break;
        }
    }

    void coregraphics_canvas::set_fill_color(const maui::graphics::color& value)
    {
        // C# FillColor setter: install the color and clear any staged gradient / pattern / image fill.
        CGContextSetRGBFillColor(context_, value.red, value.green, value.blue, value.alpha);
        release_gradient();
        gradient_kind_ = staged_gradient::none;
        fill_pattern_ = nullptr;
        fill_image_ = nullptr;
    }

    void coregraphics_canvas::set_font_color(const maui::graphics::color& value)
    {
        font_color_ = value;
    }

    void coregraphics_canvas::set_font(const maui::graphics::font& value)
    {
        font_ = value;
    }

    void coregraphics_canvas::set_font_size(float value)
    {
        font_size_ = value;
    }

    void coregraphics_canvas::set_alpha(float value)
    {
        CGContextSetAlpha(context_, value);
    }

    void coregraphics_canvas::set_antialias(bool value)
    {
        antialias_ = value;
    }

    void coregraphics_canvas::set_blend_mode(maui::graphics::blend_mode value)
    {
        CGContextSetBlendMode(context_, to_cg_blend_mode(value));
    }

    void coregraphics_canvas::release_gradient()
    {
        if (gradient_ != nullptr)
        {
            CGGradientRelease(gradient_);
            gradient_ = nullptr;
        }
    }

    void coregraphics_canvas::fill_with_gradient(const std::function<bool()>& add_shape)
    {
        // C# FillWithGradient: when shadowed, pre-fill the shape with white at the gradient's
        // minimum alpha so the (clipped) gradient still casts a shadow.
        if (current_state().shadowed() && gradient_kind_ != staged_gradient::none)
        {
            const maui::graphics::color color = maui::graphics::colors::white.with_alpha(gradient_min_alpha_);
            CGContextSetRGBFillColor(context_, color.red, color.green, color.blue, color.alpha);
            (void)add_shape();
            CGContextFillPath(context_);
        }

        CGContextSaveGState(context_);
        if (add_shape())
        {
            CGContextClip(context_);
        }

        draw_gradient();
        CGContextRestoreGState(context_);
    }

    void coregraphics_canvas::draw_gradient()
    {
        // C# DrawGradient — one-shot: paints the staged gradient over gradient_rectangle_ and
        // releases it.
        constexpr CGGradientDrawingOptions options =
            kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation;

        if (gradient_kind_ == staged_gradient::linear && gradient_ != nullptr)
        {
            const float x1 =
                gradient_rectangle_.x + (static_cast<float>(gradient_start_.x) * gradient_rectangle_.width);
            const float y1 =
                gradient_rectangle_.y + (static_cast<float>(gradient_start_.y) * gradient_rectangle_.height);
            const float x2 = gradient_rectangle_.x + (static_cast<float>(gradient_end_.x) * gradient_rectangle_.width);
            const float y2 = gradient_rectangle_.y + (static_cast<float>(gradient_end_.y) * gradient_rectangle_.height);

            CGContextDrawLinearGradient(context_, gradient_, CGPointMake(x1, y1), CGPointMake(x2, y2), options);
        }
        else if (gradient_kind_ == staged_gradient::radial && gradient_ != nullptr)
        {
            const float center_x =
                (static_cast<float>(gradient_center_.x) * gradient_rectangle_.width) + gradient_rectangle_.x;
            const float center_y =
                (static_cast<float>(gradient_center_.y) * gradient_rectangle_.height) + gradient_rectangle_.y;
            const CGPoint center = CGPointMake(center_x, center_y);

            float radius =
                static_cast<float>(gradient_radius_) * std::max(gradient_rectangle_.height, gradient_rectangle_.width);
            if (radius == 0)
            {
                radius = std::hypot(gradient_rectangle_.width, gradient_rectangle_.height);
            }

            CGContextDrawRadialGradient(context_, gradient_, center, 0, center, radius, options);
        }

        release_gradient();
        gradient_kind_ = staged_gradient::none;
    }

    namespace
    {
        // The info payload threaded through CGPattern's callbacks. CGPattern invokes the draw callback
        // (immediately for bitmap/screen contexts) with this `info`; the structs are stack locals in the
        // FillWith* methods that outlive the synchronous fill, so the release callback is a no-op.
        struct pattern_callback_info
        {
            coregraphics_canvas* outer;
            maui::graphics::i_pattern* pattern;
        };

        struct image_callback_info
        {
            maui::graphics::i_graphics_image* image;
        };
    } // namespace

    // Pattern tile callback: re-enter the outer canvas's nested fill_pattern_canvas_ bound to the tile
    // context, then call pattern->draw — mirrors C# DrawPatternCallback.
    void coregraphics_canvas::pattern_tile_callback(void* info, CGContextRef tile_context)
    {
        auto* const payload = static_cast<pattern_callback_info*>(info);
        payload->outer->draw_pattern_callback(tile_context, payload->pattern);
    }

    // Image tile callback: blit the image's CGImage into the tile rect (image logical Width x Height at
    // the origin) — mirrors C# DrawImageCallback. The CTM flip lives in the pattern matrix, so the
    // callback draws upright.
    void coregraphics_canvas::image_tile_callback(void* info, CGContextRef tile_context)
    {
        auto* const payload = static_cast<image_callback_info*>(info);
        auto* const cg_image = static_cast<CGImageRef>(payload->image->to_platform_image());
        if (cg_image != nullptr)
        {
            const CGRect rect = CGRectMake(0, 0, payload->image->width(), payload->image->height());
            CGContextDrawImage(tile_context, rect, cg_image);
        }
    }

    void coregraphics_canvas::draw_pattern_callback(CGContextRef tile_context, maui::graphics::i_pattern* fill_pattern)
    {
        // C# DrawPatternCallback: reset the dash, (lazily) create the nested canvas, bind it to the tile
        // context and replay the pattern's tile.
        if (fill_pattern == nullptr)
        {
            return;
        }
        CGContextSetLineDash(tile_context, 0, nullptr, 0);
        if (fill_pattern_canvas_ == nullptr)
        {
            fill_pattern_canvas_ = std::make_unique<coregraphics_canvas>();
        }
        fill_pattern_canvas_->set_context(tile_context);
        fill_pattern->draw(*fill_pattern_canvas_);
    }

    void coregraphics_canvas::fill_with_pattern(float x, float y, const std::function<void()>& drawing_action)
    {
        // C# FillWithPattern (PlatformCanvas.cs:708-743). The pattern colorspace is base-less
        // (CreatePattern(null)) — the pattern is colored, supplying its own colors.
        CGContextSaveGState(context_);
        CGContextSetPatternPhase(context_, CGSizeZero); // start the tile at the fill origin

        CGColorSpaceRef colorspace = CGColorSpaceCreatePattern(nullptr);
        CGContextSetFillColorSpace(context_, colorspace);

        const CGRect pattern_rect = CGRectMake(0, 0, fill_pattern_->width(), fill_pattern_->height());

        const maui::graphics::matrix3x2& m = current_state().transform();
        const CGAffineTransform current_transform = CGAffineTransformMake(m.m11, m.m12, m.m21, m.m22, m.m31, m.m32);
        const CGAffineTransform transform =
            CGAffineTransformConcat(CGAffineTransformMakeTranslation(x, y), current_transform);

        pattern_callback_info info{.outer = this, .pattern = fill_pattern_};
        const CGPatternCallbacks callbacks{.version = 0, .drawPattern = &pattern_tile_callback, .releaseInfo = nullptr};
        CGPatternRef pattern =
            CGPatternCreate(&info, pattern_rect, transform, fill_pattern_->step_x(), fill_pattern_->step_y(),
                            kCGPatternTilingConstantSpacing, /*isColored*/ true, &callbacks);

        const std::array<CGFloat, 1> alpha{1};
        CGContextSetFillPattern(context_, pattern, alpha.data());
        drawing_action();

        CGPatternRelease(pattern);
        CGColorSpaceRelease(colorspace);
        CGContextRestoreGState(context_);
    }

    void coregraphics_canvas::fill_with_image(float x, float y, const std::function<void()>& drawing_action)
    {
        // C# FillWithImage (PlatformCanvas.cs:745-777). The pattern colorspace wraps the base DeviceRGB
        // (the collapsed getColorspace default); the tile matrix flips Y so the bottom-up CGImage tiles
        // upright.
        CGContextSaveGState(context_);
        CGContextSetPatternPhase(context_, CGSizeZero);

        CGColorSpaceRef base_colorspace = CGColorSpaceCreateDeviceRGB();
        CGColorSpaceRef colorspace = CGColorSpaceCreatePattern(base_colorspace);
        CGContextSetFillColorSpace(context_, colorspace);

        const CGRect pattern_rect = CGRectMake(0, 0, fill_image_->width(), fill_image_->height());

        const maui::graphics::matrix3x2& m = current_state().transform();
        const CGAffineTransform current_transform = CGAffineTransformMake(m.m11, m.m12, m.m21, m.m22, m.m31, m.m32);
        CGAffineTransform transform =
            CGAffineTransformConcat(CGAffineTransformMakeTranslation(x, y), current_transform);
        transform = CGAffineTransformConcat(transform, CGAffineTransformMake(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F));

        image_callback_info info{.image = fill_image_};
        const CGPatternCallbacks callbacks{.version = 0, .drawPattern = &image_tile_callback, .releaseInfo = nullptr};
        CGPatternRef pattern =
            CGPatternCreate(&info, pattern_rect, transform, fill_image_->width(), fill_image_->height(),
                            kCGPatternTilingNoDistortion, /*isColored*/ true, &callbacks);

        const std::array<CGFloat, 1> alpha{1};
        CGContextSetFillPattern(context_, pattern, alpha.data());
        drawing_action();

        CGPatternRelease(pattern);
        CGColorSpaceRelease(colorspace);
        CGColorSpaceRelease(base_colorspace);
        CGContextRestoreGState(context_);
    }

    void coregraphics_canvas::set_fill_paint(const maui::graphics::paint* fill_paint,
                                             const maui::graphics::rect_f& rectangle)
    {
        gradient_rectangle_ = rectangle;

        // C# SetFillPaint clears the staged gradient + _fillPattern + _fillImage up front.
        release_gradient();
        gradient_kind_ = staged_gradient::none;
        fill_pattern_ = nullptr;
        fill_image_ = nullptr;

        // C# SetFillPaint(null) -> Colors.White.AsPaint() (a solid white fill).
        if (fill_paint == nullptr)
        {
            set_fill_color(maui::graphics::colors::white);
            return;
        }

        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(fill_paint))
        {
            set_fill_color(solid->color());
            return;
        }

        if (const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(fill_paint))
        {
            // Build the CGGradient from the stops, in property order exactly like C#.
            const std::vector<maui::graphics::gradient_stop>& stops = gradient->gradient_stops();
            std::vector<CGFloat> components;
            components.reserve(stops.size() * 4);
            std::vector<CGFloat> locations;
            locations.reserve(stops.size());
            for (const maui::graphics::gradient_stop& stop : stops)
            {
                const maui::graphics::color stop_color = stop.color();
                components.push_back(stop_color.red);
                components.push_back(stop_color.green);
                components.push_back(stop_color.blue);
                components.push_back(stop_color.alpha);
                locations.push_back(stop.offset());
            }

            CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
            gradient_ =
                CGGradientCreateWithColorComponents(colorspace, components.data(), locations.data(), locations.size());
            CGColorSpaceRelease(colorspace);

            gradient_min_alpha_ = std::min(gradient->start_color().alpha, gradient->end_color().alpha);

            if (const auto* const linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(fill_paint))
            {
                gradient_kind_ = staged_gradient::linear;
                gradient_start_ = linear->start_point();
                gradient_end_ = linear->end_point();
            }
            else if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(fill_paint))
            {
                gradient_kind_ = staged_gradient::radial;
                gradient_center_ = radial->center();
                gradient_radius_ = radial->radius();
            }
            else
            {
                // A bare gradient_paint kind we cannot position — C# would fall to its
                // BackgroundColor; do the same.
                release_gradient();
                set_fill_color(fill_paint->background_color());
            }
            return;
        }

        // C# `else if (paint is PatternPaint patternPaint) _fillPattern = patternPaint.Pattern;`.
        if (const auto* const pattern = dynamic_cast<const maui::graphics::pattern_paint*>(fill_paint))
        {
            fill_pattern_ = pattern->pattern();
            return;
        }

        // C# `else if (paint is ImagePaint imagePaint) _fillImage = imagePaint.Image;`.
        if (const auto* const image = dynamic_cast<const maui::graphics::image_paint*>(fill_paint))
        {
            fill_image_ = image->image();
            return;
        }

        // C#'s final else: FillColor = paint.BackgroundColor (an unrecognized paint kind).
        set_fill_color(fill_paint->background_color());
    }

    void coregraphics_canvas::platform_set_stroke_size(float value)
    {
        CGContextSetLineWidth(context_, value);
    }

    void coregraphics_canvas::platform_set_stroke_dash_pattern(const std::vector<float>& pattern,
                                                               float stroke_dash_offset, float stroke_size)
    {
        // C# PlatformSetStrokeDashPattern: null clears; otherwise the pattern and offset are scaled
        // by the (possibly limit-adjusted) stroke size.
        if (pattern.empty())
        {
            CGContextSetLineDash(context_, 0, nullptr, 0);
            return;
        }

        float actual_stroke_size = stroke_size;
        if (limit_stroke_scaling_enabled())
        {
            const float stroke_limit = assigned_stroke_limit();
            const float scale = current_state().scale();
            const float scaled_stroke_size = scale * actual_stroke_size;
            if (scaled_stroke_size < stroke_limit)
            {
                actual_stroke_size = stroke_limit / scale;
            }
        }

        std::vector<CGFloat> actual_dash_pattern;
        actual_dash_pattern.reserve(pattern.size());
        for (const float dash : pattern)
        {
            actual_dash_pattern.push_back(dash * actual_stroke_size);
        }

        CGContextSetLineDash(context_, stroke_dash_offset * actual_stroke_size, actual_dash_pattern.data(),
                             actual_dash_pattern.size());
    }

    void coregraphics_canvas::platform_draw_line(float x1, float y1, float x2, float y2)
    {
        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, false);
        }

        CGContextMoveToPoint(context_, x1, y1);
        CGContextAddLineToPoint(context_, x2, y2);
        CGContextStrokePath(context_);

        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, true);
        }
    }

    void coregraphics_canvas::platform_draw_arc(float x, float y, float width, float height, float start_angle,
                                                float end_angle, bool clockwise, bool closed)
    {
        const CGRect rect = CGRectMake(x, y, width, height);

        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, false);
        }

        const float start_angle_in_radians = normalize_angle(to_radians(-start_angle));
        const float end_angle_in_radians = normalize_angle(to_radians(-end_angle));

        if (width == height)
        {
            CGContextAddArc(context_, CGRectGetMidX(rect), CGRectGetMidY(rect), rect.size.width / 2,
                            start_angle_in_radians, end_angle_in_radians, static_cast<int>(!clockwise));
            if (closed)
            {
                CGContextClosePath(context_);
            }
            CGContextStrokePath(context_);
        }
        else
        {
            const CGFloat cx = CGRectGetMidX(rect);
            const CGFloat cy = CGRectGetMidY(rect);
            CGAffineTransform transform = CGAffineTransformMakeTranslation(cx, cy);
            transform =
                CGAffineTransformConcat(CGAffineTransformMakeScale(1, rect.size.height / rect.size.width), transform);

            CGMutablePathRef path = CGPathCreateMutable();
            CGPathAddArc(path, &transform, 0, 0, rect.size.width / 2, start_angle_in_radians, end_angle_in_radians,
                         !clockwise);
            if (closed)
            {
                CGPathCloseSubpath(path);
            }

            CGContextAddPath(context_, path);
            CGContextStrokePath(context_);
            CGPathRelease(path);
        }

        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, true);
        }
    }

    void coregraphics_canvas::fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                                       bool clockwise)
    {
        const CGRect rect = CGRectMake(x, y, width, height);

        // C# FillArc normalizes by adding 2pi while negative (slightly different from DrawArc's
        // modulo normalization — quirk preserved).
        float start_angle_in_radians = to_radians(-start_angle);
        float end_angle_in_radians = to_radians(-end_angle);
        constexpr auto two_pi = static_cast<float>(2 * std::numbers::pi);
        while (start_angle_in_radians < 0)
        {
            start_angle_in_radians += two_pi;
        }
        while (end_angle_in_radians < 0)
        {
            end_angle_in_radians += two_pi;
        }

        if (width == height)
        {
            const auto add_arc = [&] {
                CGContextAddArc(context_, CGRectGetMidX(rect), CGRectGetMidY(rect), rect.size.width / 2,
                                start_angle_in_radians, end_angle_in_radians, static_cast<int>(!clockwise));
            };
            if (gradient_kind_ != staged_gradient::none)
            {
                fill_with_gradient([&] {
                    add_arc();
                    return true;
                });
            }
            else if (fill_pattern_ != nullptr)
            {
                add_arc();
                fill_with_pattern(x, y, [&] { CGContextFillPath(context_); });
            }
            else if (fill_image_ != nullptr)
            {
                add_arc();
                fill_with_image(x, y, [&] { CGContextFillPath(context_); });
            }
            else
            {
                add_arc();
                CGContextFillPath(context_);
            }
        }
        else
        {
            const CGFloat cx = CGRectGetMidX(rect);
            const CGFloat cy = CGRectGetMidY(rect);
            CGAffineTransform transform = CGAffineTransformMakeTranslation(cx, cy);
            transform =
                CGAffineTransformConcat(CGAffineTransformMakeScale(1, rect.size.height / rect.size.width), transform);
            CGMutablePathRef path = CGPathCreateMutable();
            CGPathAddArc(path, &transform, 0, 0, rect.size.width / 2, start_angle_in_radians, end_angle_in_radians,
                         !clockwise);

            if (gradient_kind_ != staged_gradient::none)
            {
                fill_with_gradient([&] {
                    CGContextAddPath(context_, path);
                    return true;
                });
            }
            else if (fill_pattern_ != nullptr)
            {
                CGContextAddPath(context_, path);
                fill_with_pattern(x, y, [&] { CGContextFillPath(context_); });
            }
            else if (fill_image_ != nullptr)
            {
                CGContextAddPath(context_, path);
                fill_with_image(x, y, [&] { CGContextFillPath(context_); });
            }
            else
            {
                CGContextAddPath(context_, path);
                CGContextFillPath(context_);
            }

            CGPathRelease(path);
        }
    }

    void coregraphics_canvas::platform_draw_rectangle(float x, float y, float width, float height)
    {
        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, false);
        }
        CGContextStrokeRect(context_, CGRectMake(x, y, width, height));
        if (!antialias_)
        {
            CGContextSetShouldAntialias(context_, true);
        }
    }

    void coregraphics_canvas::fill_rectangle(float x, float y, float width, float height)
    {
        const CGRect rect = CGRectMake(x, y, width, height);

        if (gradient_kind_ != staged_gradient::none)
        {
            fill_with_gradient([&] {
                CGContextAddRect(context_, rect);
                return true;
            });
        }
        else if (fill_pattern_ != nullptr)
        {
            fill_with_pattern(x, y, [&] { CGContextFillRect(context_, rect); });
        }
        else if (fill_image_ != nullptr)
        {
            fill_with_image(x, y, [&] { CGContextFillRect(context_, rect); });
        }
        else
        {
            CGContextFillRect(context_, rect);
        }
    }

    void coregraphics_canvas::platform_draw_rounded_rectangle(float x, float y, float width, float height,
                                                              float corner_radius)
    {
        add_rounded_rectangle(context_, x, y, width, height, corner_radius);
        CGContextDrawPath(context_, kCGPathStroke);
    }

    void coregraphics_canvas::fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius)
    {
        // C# FillRoundedRectangle clamps the radius itself (the AbstractCanvas clamp only covers
        // the DRAW path); the if-chains collapse to std::min.
        float radius = std::min(corner_radius, std::abs(height / 2));
        radius = std::min(radius, std::abs(width / 2));

        if (gradient_kind_ != staged_gradient::none)
        {
            fill_with_gradient([&] {
                add_rounded_rectangle(context_, x, y, width, height, radius);
                return true;
            });
        }
        else if (fill_pattern_ != nullptr)
        {
            add_rounded_rectangle(context_, x, y, width, height, radius);
            fill_with_pattern(x, y, [&] { CGContextFillPath(context_); });
        }
        else if (fill_image_ != nullptr)
        {
            add_rounded_rectangle(context_, x, y, width, height, radius);
            fill_with_image(x, y, [&] { CGContextFillPath(context_); });
        }
        else
        {
            add_rounded_rectangle(context_, x, y, width, height, radius);
            CGContextFillPath(context_);
        }
    }

    void coregraphics_canvas::platform_draw_ellipse(float x, float y, float width, float height)
    {
        CGContextStrokeEllipseInRect(context_, CGRectMake(x, y, width, height));
    }

    void coregraphics_canvas::fill_ellipse(float x, float y, float width, float height)
    {
        const CGRect rect = CGRectMake(x, y, width, height);

        if (gradient_kind_ != staged_gradient::none)
        {
            fill_with_gradient([&] {
                CGContextAddEllipseInRect(context_, rect);
                return true;
            });
        }
        else if (fill_pattern_ != nullptr)
        {
            fill_with_pattern(x, y, [&] { CGContextFillEllipseInRect(context_, rect); });
        }
        else if (fill_image_ != nullptr)
        {
            fill_with_image(x, y, [&] { CGContextFillEllipseInRect(context_, rect); });
        }
        else
        {
            CGContextFillEllipseInRect(context_, rect);
        }
    }

    void coregraphics_canvas::subtract_from_clip(float x, float y, float width, float height)
    {
        // C# SubtractFromClip: clip to (current clip bounding box) XOR (inner rect) via EOClip.
        const CGRect clip_bounding_box = CGContextGetClipBoundingBox(context_);
        const CGRect inner_clip = CGRectMake(x, y, width, height);

        CGMutablePathRef clip = CGPathCreateMutable();
        CGPathAddRect(clip, nullptr, clip_bounding_box);
        CGPathAddRect(clip, nullptr, inner_clip);

        CGContextAddPath(context_, clip);
        CGContextEOClip(context_);
        CGPathRelease(clip);
    }

    void coregraphics_canvas::platform_draw_path(const maui::graphics::path_f& path)
    {
        // The CGPath is rebuilt per call (C# caches it on PathF.PlatformPath; the port's conversion
        // is stateless — cache when a profiling need shows up).
        CGPathRef platform_path = path_to_cg_path(path);

        if (gradient_kind_ != staged_gradient::none)
        {
            fill_with_gradient([&] {
                CGContextAddPath(context_, platform_path);
                CGContextReplacePathWithStrokedPath(context_);
                return true;
            });
        }
        else
        {
            CGContextAddPath(context_, platform_path);
            CGContextDrawPath(context_, kCGPathStroke);
        }

        CGPathRelease(platform_path);
    }

    void coregraphics_canvas::platform_draw_image(const maui::graphics::i_graphics_image& image, float x, float y,
                                                  float width, float height)
    {
        // C# PlatformCanvas.DrawImage (PlatformCanvas.cs:658-676): grab the CGImage off the platform
        // image; the canvas uses a top-left origin while CoreGraphics blits bottom-up, so flip the CTM
        // (ScaleCTM(1,-1) + TranslateCTM(0,-height)) before the draw. _rect.Y is -y exactly per C#.
        // The drawing layer hands the handle out as an opaque void*; cast it back to the CGImageRef
        // the blit needs.
        auto* cg_image = static_cast<CGImageRef>(image.to_platform_image());
        if (cg_image == nullptr)
        {
            return;
        }

        const CGRect rect = CGRectMake(x, -y, width, height);

        CGContextSaveGState(context_);
        CGContextScaleCTM(context_, 1, -1);
        CGContextTranslateCTM(context_, 0, -rect.size.height);
        CGContextDrawImage(context_, rect, cg_image);
        CGContextRestoreGState(context_);
    }

    void coregraphics_canvas::clip_path(const maui::graphics::path_f& path, maui::graphics::winding_mode winding)
    {
        CGPathRef platform_path = path_to_cg_path(path);
        CGContextAddPath(context_, platform_path);

        if (winding == maui::graphics::winding_mode::even_odd)
        {
            CGContextEOClip(context_);
        }
        else
        {
            CGContextClip(context_);
        }
        CGPathRelease(platform_path);
    }

    void coregraphics_canvas::clip_rectangle(float x, float y, float width, float height)
    {
        CGContextAddRect(context_, CGRectMake(x, y, width, height));
        CGContextClip(context_);
    }

    void coregraphics_canvas::fill_path(const maui::graphics::path_f& path, maui::graphics::winding_mode winding)
    {
        CGPathRef platform_path = path_to_cg_path(path);
        const bool even_odd = winding == maui::graphics::winding_mode::even_odd;

        if (gradient_kind_ != staged_gradient::none)
        {
            fill_with_gradient([&] {
                CGContextAddPath(context_, platform_path);
                if (even_odd)
                {
                    CGContextEOClip(context_);
                    return false;
                }
                return true;
            });
        }
        else if (fill_pattern_ != nullptr || fill_image_ != nullptr)
        {
            // C# uses the path's bounding-box origin as the pattern/image phase, not (0, 0).
            const CGRect bounding_box = CGPathGetBoundingBox(platform_path);
            const auto origin_x = static_cast<float>(bounding_box.origin.x);
            const auto origin_y = static_cast<float>(bounding_box.origin.y);
            const auto fill_action = [&] {
                if (even_odd)
                {
                    CGContextEOFillPath(context_);
                }
                else
                {
                    CGContextFillPath(context_);
                }
            };
            CGContextAddPath(context_, platform_path);
            if (fill_pattern_ != nullptr)
            {
                fill_with_pattern(origin_x, origin_y, fill_action);
            }
            else
            {
                fill_with_image(origin_x, origin_y, fill_action);
            }
        }
        else
        {
            CGContextAddPath(context_, platform_path);
            if (even_odd)
            {
                CGContextEOFillPath(context_);
            }
            else
            {
                CGContextFillPath(context_);
            }
        }

        CGPathRelease(platform_path);
    }

    void coregraphics_canvas::draw_string(std::string_view value, float x, float y,
                                          maui::graphics::horizontal_alignment h_align)
    {
        float aligned_x = x;
        if (h_align == maui::graphics::horizontal_alignment::right)
        {
            const maui::graphics::size_f size = get_string_size(value, font_, font_size_);
            aligned_x -= size.width;
        }
        else if (h_align == maui::graphics::horizontal_alignment::center)
        {
            const maui::graphics::size_f size = get_string_size(value, font_, font_size_);
            aligned_x -= size.width / 2.0F;
        }
        draw_string_at(value, aligned_x, y);
    }

    void coregraphics_canvas::draw_string_at(std::string_view value, float x, float y)
    {
        // C# private DrawString(value, x, y): one CTLine at the text matrix (1, 0, 0, -1, x, y).
        CTFontRef ct_font = to_ct_font(font_, font_size_);
        CGColorRef foreground = to_cg_color(font_color_);
        NSMutableDictionary* const attributes = [NSMutableDictionary dictionary];
        attributes[(__bridge id)kCTFontAttributeName] = (__bridge id)ct_font;
        if (foreground != nullptr) // CGColorCreate is nullable by contract; never null here
        {
            attributes[(__bridge id)kCTForegroundColorAttributeName] = (__bridge id)foreground;
        }
        NSAttributedString* const attributed_string = [[NSAttributedString alloc] initWithString:to_ns_string(value)
                                                                                      attributes:attributes];
        CTLineRef text_line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributed_string);

        CGContextSetTextMatrix(context_, CGAffineTransformMake(1, 0, 0, -1, x, y));
        CTLineDraw(text_line, context_);

        CFRelease(text_line);
        CGColorRelease(foreground);
        CFRelease(ct_font);
    }

    void coregraphics_canvas::draw_string(std::string_view value, float x, float y, float width, float height,
                                          maui::graphics::horizontal_alignment h_align,
                                          maui::graphics::vertical_alignment v_align, maui::graphics::text_flow flow,
                                          float line_spacing_adjustment)
    {
        (void)flow;                    // accepted-but-unused in C#'s bounded DrawString too
        (void)line_spacing_adjustment; // likewise

        if (width == 0 || height == 0 || value.empty())
        {
            return;
        }

        CGMutablePathRef path = CGPathCreateMutable();
        const CGRect rect = CGRectMake(x, -y, width, height);
        CGPathAddRect(path, nullptr, rect);

        CGContextSaveGState(context_);
        draw_string_in_path(context_, path, value, h_align, v_align, font_, font_size_, font_color_, 0, 0);
        CGContextRestoreGState(context_);
        CGPathRelease(path);
    }

    void coregraphics_canvas::draw_text(const maui::graphics::text::i_attributed_text& value, float x, float y,
                                        float width, float height)
    {
        // C# DrawText -> static DrawAttributedText(context, value, rect, font, size, color).
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddRect(path, nullptr, CGRectMake(x, y, width, height));

        const CGRect rect = CGPathGetPathBoundingBox(path);

        CGContextSaveGState(context_);
        CGContextTranslateCTM(context_, 0, rect.size.height);
        CGContextScaleCTM(context_, 1, -1);
        CGContextTranslateCTM(context_, 0, CGRectGetMinY(rect) * -2);

        CGContextSetTextMatrix(context_, CGAffineTransformIdentity);

        NSAttributedString* const attributed_string = as_ct_attributed_string(value, font_, font_size_, font_color_);

        CTFramesetterRef framesetter =
            CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)attributed_string);
        CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), path, nullptr);

        if (frame != nullptr)
        {
            // C#'s verticalAlignment is hard-wired to Top here -> TranslateCTM(0, 0), a no-op.
            CTFrameDraw(frame, context_);
            CFRelease(frame);
        }

        CFRelease(framesetter);
        CGContextRestoreGState(context_);
        CGPathRelease(path);
    }

    maui::graphics::size_f coregraphics_canvas::get_string_size(std::string_view value,
                                                                const maui::graphics::font& font, float font_size) const
    {
        // C# Mac PlatformStringSizeService.GetStringSize uses NSString.StringSize; the CoreText
        // typographic bounds of a single CTLine are the shared-framework equivalent.
        CTFontRef ct_font = to_ct_font(font, font_size);
        NSDictionary* const attributes = @{(__bridge id)kCTFontAttributeName : (__bridge id)ct_font};
        NSAttributedString* const attributed_string = [[NSAttributedString alloc] initWithString:to_ns_string(value)
                                                                                      attributes:attributes];
        CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributed_string);

        CGFloat ascent = 0;
        CGFloat descent = 0;
        CGFloat leading = 0;
        const double width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

        CFRelease(line);
        CFRelease(ct_font);

        return {static_cast<float>(width), static_cast<float>(ascent + descent + leading)};
    }

    maui::graphics::size_f coregraphics_canvas::get_string_size(std::string_view value,
                                                                const maui::graphics::font& font, float font_size,
                                                                maui::graphics::horizontal_alignment h_align,
                                                                maui::graphics::vertical_alignment v_align) const
    {
        // C# MaciOS PlatformStringSizeService.GetStringSize(value, font, size, h, v): scale the
        // font size under 10, lay the text out in a 512x512 frame, scale the bounds back.
        (void)v_align; // C# accepts-but-ignores the vertical alignment in this measurement

        float effective_font_size = font_size;
        float factor = 1;
        while (effective_font_size > 10)
        {
            effective_font_size /= 10;
            factor *= 10;
        }

        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddRect(path, nullptr, CGRectMake(0, 0, 512, 512));
        CGPathCloseSubpath(path);

        CTFontRef ct_font = to_ct_font(font, effective_font_size);
        CTParagraphStyleRef paragraph_style = to_ct_paragraph_style(h_align);
        NSDictionary* const attributes = @{
            (__bridge id)kCTFontAttributeName : (__bridge id)ct_font,
            (__bridge id)kCTParagraphStyleAttributeName : (__bridge id)paragraph_style,
        };
        NSAttributedString* const attributed_string = [[NSAttributedString alloc] initWithString:to_ns_string(value)
                                                                                      attributes:attributes];

        CTFramesetterRef framesetter =
            CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)attributed_string);
        CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), path, nullptr);

        maui::graphics::rect_f text_bounds{0, 0, 0, 0};
        if (frame != nullptr)
        {
            text_bounds = get_text_size(frame);
            CFRelease(frame);
        }

        CFRelease(framesetter);
        CFRelease(paragraph_style);
        CFRelease(ct_font);
        CGPathRelease(path);

        return {text_bounds.width * factor, text_bounds.height * factor};
    }

    void coregraphics_canvas::save_state()
    {
        abstract_canvas::save_state();
        CGContextSaveGState(context_);
    }

    bool coregraphics_canvas::restore_state()
    {
        const bool success = abstract_canvas::restore_state();

        release_gradient();
        gradient_kind_ = staged_gradient::none;
        fill_pattern_ = nullptr;
        fill_image_ = nullptr;
        CGContextRestoreGState(context_);

        return success;
    }

    void coregraphics_canvas::reset_state()
    {
        abstract_canvas::reset_state();

        release_gradient();
        gradient_kind_ = staged_gradient::none;
        fill_pattern_ = nullptr;
        fill_image_ = nullptr;

        font_color_ = maui::graphics::colors::black;
    }

    void coregraphics_canvas::set_shadow(const maui::graphics::size_f& offset, float blur,
                                         const maui::graphics::color& shadow_color)
    {
        // C# SetShadow without the legacy-MONOMAC height negation (see the header note). The
        // null-color SetShadow branch cannot arise — color is a value type.
        const CGSize size = CGSizeMake(offset.width, offset.height);
        CGColorRef cg_color = to_cg_color(shadow_color);
        CGContextSetShadowWithColor(context_, size, blur, cg_color);
        CGColorRelease(cg_color);

        current_state().set_shadowed(true);
    }

    void coregraphics_canvas::platform_rotate(float degrees, float radians, float x, float y)
    {
        (void)degrees;
        CGContextTranslateCTM(context_, x, y);
        CGContextRotateCTM(context_, radians);
        CGContextTranslateCTM(context_, -x, -y);
    }

    void coregraphics_canvas::platform_rotate(float degrees, float radians)
    {
        (void)degrees;
        CGContextRotateCTM(context_, radians);
    }

    void coregraphics_canvas::platform_scale(float sx, float sy)
    {
        CGContextScaleCTM(context_, sx, sy);
    }

    void coregraphics_canvas::platform_translate(float tx, float ty)
    {
        CGContextTranslateCTM(context_, tx, ty);
    }

    void coregraphics_canvas::platform_concatenate_transform(const maui::graphics::matrix3x2& transform)
    {
        // C# Matrix3x2.AsCGAffineTransform: (m11, m12, m21, m22, m31, m32) -> (a, b, c, d, tx, ty).
        CGContextConcatCTM(context_, CGAffineTransformMake(transform.m11, transform.m12, transform.m21, transform.m22,
                                                           transform.m31, transform.m32));
    }
} // namespace maui::platform::apple_shared

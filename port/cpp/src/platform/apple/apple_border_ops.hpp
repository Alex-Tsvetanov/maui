#pragma once
// Shared AppKit operations for the border-stroke layer — the platform side of border_handler's
// update_border (the C# StrokeExtensions → MauiCALayer funnel). Objective-C++ only — include
// exclusively from .mm files compiled as Objective-C++ (it references NSView / CAShapeLayer / CGPath).
//
// apply_border_stroke ports MauiCALayer's DrawInContext recipe onto STOCK layers (the documented
// adaptation — AppKit gets no custom drawing layer):
//   - The container layer is MASKED to the border shape via the existing apply_clip (the
//     "we are clipping the outer" step of MauiCALayer.DrawBorder + the ContentView content mask,
//     collapsed into one mask — it bounds the background, the hosted content, and the stroke).
//   - A CAShapeLayer sublayer (tagged k_border_layer_name) strokes the shape's path_for_bounds at
//     DOUBLE the stroke thickness; the mask cuts the outer half, leaving the INNER `thickness`-wide
//     border — exactly MauiCALayer.DrawBorder's `ctx.SetLineWidth(2 * _strokeThickness)` trick.
//   - Dash lengths and the dash phase scale by the thickness (MauiCALayer.SetBorderDash:
//     `dashArray[i] = thickness * array[i]`, `SetLineDash(_strokeDashOffset * _strokeThickness, …)`),
//     an odd-length dash array is doubled (the C# odd-array copy), and the miter limit follows
//     `_strokeMiterLimit * _strokeThickness / 4` (the doubled line width's compensation).
// No shape, no stroke brush, or a non-positive thickness removes the stroke sublayer (DrawBorder's
// `_strokeThickness <= 0` early-out; the brush-less layer draws no border).
//
// Reuses apple_visual_ops.hpp's path_to_cg_path (the GraphicsExtensions.AsCGPath walk) + apply_clip.
// `bounds` is the view's LOCAL bounds (origin 0) — pass `view.bounds`, as the update_clip callers do.

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <cstddef>
#include <vector>

#include "maui/core/border_handler.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

#include "apple_conversions.hpp"
#include "apple_visual_ops.hpp"

namespace maui::platform::apple
{
    // The name tagging the stroke sublayer apply_border_stroke installs (find/replace/remove key).
    inline NSString* const k_border_layer_name = @"maui.border.stroke";

    // The previously-installed stroke sublayer, or nil.
    inline CAShapeLayer* find_border_layer(CALayer* layer)
    {
        NSArray<CALayer*>* const sublayers = layer.sublayers;
        for (NSUInteger i = 0; i < sublayers.count; i++)
        {
            CALayer* const sub = sublayers[i];
            if ([sub.name isEqualToString:k_border_layer_name] && [sub isKindOfClass:[CAShapeLayer class]])
            {
                return (CAShapeLayer*)sub;
            }
        }
        return nil;
    }

    // MauiCALayer.SetBorderLineCap / SetBorderLineJoin: the CG enum mapping, as CAShapeLayer constants.
    inline CAShapeLayerLineCap border_line_cap(maui::graphics::line_cap value)
    {
        switch (value)
        {
            case maui::graphics::line_cap::round:
                return kCALineCapRound;
            case maui::graphics::line_cap::square:
                return kCALineCapSquare;
            case maui::graphics::line_cap::butt:
                break;
        }
        return kCALineCapButt;
    }

    inline CAShapeLayerLineJoin border_line_join(maui::graphics::line_join value)
    {
        switch (value)
        {
            case maui::graphics::line_join::round:
                return kCALineJoinRound;
            case maui::graphics::line_join::bevel:
                return kCALineJoinBevel;
            case maui::graphics::line_join::miter:
                break;
        }
        return kCALineJoinMiter;
    }

    // MauiCALayer.SetBorderDash: scale every dash length by the thickness; double an odd-length array.
    inline NSArray<NSNumber*>* border_dash_pattern(const std::vector<float>& dashes, double thickness)
    {
        if (dashes.empty())
        {
            return nil;
        }
        const std::size_t length = dashes.size() % 2 == 0 ? dashes.size() : 2 * dashes.size();
        NSMutableArray<NSNumber*>* const pattern = [NSMutableArray arrayWithCapacity:length];
        for (std::size_t i = 0; i < length; i++)
        {
            [pattern addObject:@(thickness * static_cast<double>(dashes[i % dashes.size()]))];
        }
        return pattern;
    }

    inline void apply_border_stroke(void* native, const maui::core::border_stroke_spec& spec,
                                    maui::graphics::rect bounds)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge NSView*)native;
        view.wantsLayer = YES;
        CALayer* const layer = view.layer;
        if (layer == nil)
        {
            return;
        }

        // The shape mask: clips background + content + the stroke's outer half (see the header).
        apply_clip(native, spec.shape, bounds);

        CAShapeLayer* stroke_layer = find_border_layer(layer);
        const bool draws_border = spec.has_stroke && spec.thickness > 0 && spec.shape != nullptr;
        if (!draws_border)
        {
            [stroke_layer removeFromSuperlayer];
            return;
        }

        if (stroke_layer == nil)
        {
            stroke_layer = [CAShapeLayer layer];
            stroke_layer.name = k_border_layer_name;
            [layer addSublayer:stroke_layer];
        }
        stroke_layer.frame = CGRectMake(bounds.x, bounds.y, bounds.width, bounds.height);
        stroke_layer.fillColor = nil; // stroke only — the background is the container layer's

        const maui::graphics::path_f path = spec.shape->path_for_bounds(bounds);
        CGPathRef cg = path_to_cg_path(path);
        stroke_layer.path = cg;
        CGPathRelease(cg);

        stroke_layer.strokeColor = to_ns_color(spec.stroke_color).CGColor;
        // DOUBLE the width — the mask cuts the outer half (MauiCALayer.DrawBorder).
        stroke_layer.lineWidth = static_cast<CGFloat>(2 * spec.thickness);
        stroke_layer.lineCap = border_line_cap(spec.line_cap);
        stroke_layer.lineJoin = border_line_join(spec.line_join);
        stroke_layer.miterLimit = static_cast<CGFloat>(static_cast<double>(spec.miter_limit) * spec.thickness / 4.0);
        stroke_layer.lineDashPattern = border_dash_pattern(spec.dash_pattern, spec.thickness);
        stroke_layer.lineDashPhase = static_cast<CGFloat>(static_cast<double>(spec.dash_offset) * spec.thickness);
    }
} // namespace maui::platform::apple

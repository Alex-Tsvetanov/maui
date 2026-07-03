#pragma once
// Shared UIKit operations for the border-stroke layer — the iOS twin of apple_border_ops.hpp (the
// platform side of border_handler's update_border; the C# StrokeExtensions → MauiCALayer funnel).
// Objective-C++ only — include exclusively from .mm files compiled as Objective-C++.
//
// apply_border_stroke ports MauiCALayer's DrawInContext recipe onto STOCK layers (the documented
// adaptation — no custom drawing layer):
//   - The container layer is MASKED to the border shape via the existing apply_clip (the
//     "we are clipping the outer" step of MauiCALayer.DrawBorder + the ContentView content mask,
//     collapsed into one mask — it bounds the background, the hosted content, and the stroke).
//   - A CAShapeLayer sublayer (tagged k_border_layer_name) strokes the shape's path_for_bounds at
//     DOUBLE the stroke thickness; the mask cuts the outer half, leaving the INNER `thickness`-wide
//     border — MauiCALayer.DrawBorder's `ctx.SetLineWidth(2 * _strokeThickness)` trick.
//   - Dash lengths and the dash phase scale by the thickness (MauiCALayer.SetBorderDash /
//     `SetLineDash(_strokeDashOffset * _strokeThickness, …)`), an odd-length dash array is doubled,
//     and the miter limit follows `_strokeMiterLimit * _strokeThickness / 4`.
// No shape, no stroke brush, or a non-positive thickness removes the stroke sublayer.
//
// Reuses ios_visual_ops.hpp's path_to_cg_path (the GraphicsExtensions.AsCGPath walk) + apply_clip.
// `bounds` is the view's LOCAL bounds (origin 0) — pass `view.bounds`, as the update_clip callers do.

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <cstddef>
#include <vector>

#include "maui/core/border_handler.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"

namespace maui::platform::ios
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

    // The name tagging the border's shadow layer — a SIBLING of the host in the host's superlayer (see
    // apply_border_shadow for why the shadow can't live on the masked host layer).
    inline NSString* const k_border_shadow_layer_name = @"maui.border.shadow";

    // The border's shadow sibling, searched among the host's siblings (the host's superlayer's sublayers).
    inline CALayer* find_border_shadow_layer(CALayer* host_layer)
    {
        CALayer* const super_layer = host_layer.superlayer;
        if (super_layer == nil)
        {
            return nil;
        }
        for (CALayer* const sub in super_layer.sublayers)
        {
            if ([sub.name isEqualToString:k_border_shadow_layer_name])
            {
                return sub;
            }
        }
        return nil;
    }

    // The alpha SILHOUETTE CoreAnimation casts the border shadow from — the +1-owned CGPath the caller must
    // release. MAUI casts the shadow off the border layer's RENDERED CONTENT (ShadowExtensions.SetShadow sets
    // NO ShadowPath, so the silhouette is whatever the MauiCALayer drew): an opaque Background fills the whole
    // shape (a solid drop shadow), a stroke-only Border (transparent fill) casts only its stroke RING. The
    // sibling layer has no drawn content, so reproduce that silhouette as an explicit shadowPath: the filled
    // shape when `has_fill`, else the shape stroked to a thin ring (the exact ring width is imperceptible
    // under the blur, so the visible `thickness` band is close enough).
    inline CGPathRef border_shadow_silhouette_path(const maui::core::border_stroke_spec& spec, bool has_fill,
                                                   maui::graphics::rect bounds)
    {
        const maui::graphics::path_f path =
            spec.shape->path_for_bounds(maui::graphics::rect{0.0, 0.0, bounds.width, bounds.height});
        CGPathRef filled = path_to_cg_path(path); // +1 owned
        if (has_fill)
        {
            return filled; // caller releases
        }
        const CGFloat w = static_cast<CGFloat>(spec.thickness > 0 ? spec.thickness : 1.0);
        const CGFloat miter = static_cast<CGFloat>(spec.miter_limit > 0 ? spec.miter_limit : 10.0);
        CGPathRef ring = CGPathCreateCopyByStrokingPath(filled, nullptr, w, kCGLineCapButt, kCGLineJoinMiter, miter);
        CGPathRelease(filled);
        return ring != nullptr ? ring : path_to_cg_path(path); // stroking can fail on a degenerate path
    }

    // Position the shadow sibling over the host and set its shadowPath to the border silhouette (fill vs stroke
    // ring). Called on every arrange / layout so the shadow tracks the host's bounds. No-op when there is no
    // shadow sibling.
    inline void reframe_border_shadow(void* native, const maui::core::border_stroke_spec& spec, bool has_fill,
                                      maui::graphics::rect bounds)
    {
        if (native == nullptr)
        {
            return;
        }
        UIView* const host = (__bridge UIView*)native;
        CALayer* const shadow_layer = find_border_shadow_layer(host.layer);
        if (shadow_layer == nil)
        {
            return;
        }
        shadow_layer.frame = host.layer.frame; // track the host's position within the shared superlayer
        if (spec.shape != nullptr && bounds.width > 0 && bounds.height > 0)
        {
            CGPathRef cg = border_shadow_silhouette_path(spec, has_fill, bounds); // +1 owned; layer copies
            shadow_layer.shadowPath = cg;
            CGPathRelease(cg);
        }
    }

    // Install / update / remove the border's Shadow. The host's OWN layer is masked to the shape by
    // apply_clip, and a CALayer WITH a mask cannot cast a shadow (the iOS limitation MAUI works around by
    // hanging the shadow on a WrapperView above the clipped content). Mirror that: draw the shadow on an
    // UNMASKED sibling layer inserted behind the host in the host's superlayer, its shadowPath set to the
    // border SILHOUETTE (filled shape when `has_fill`, else the stroke ring — see border_shadow_silhouette_path,
    // which mirrors MAUI casting off the layer's rendered content). A null shadow (or one with no paint) removes
    // the sibling. The sibling can only be created once the host has a superlayer (i.e. after mount) — arrange
    // re-invokes this so the deferred create lands.
    inline void apply_border_shadow(void* native, const maui::core::i_shadow* shadow,
                                    const maui::core::border_stroke_spec& spec, bool has_fill,
                                    maui::graphics::rect bounds)
    {
        if (native == nullptr)
        {
            return;
        }
        UIView* const host = (__bridge UIView*)native;
        CALayer* const host_layer = host.layer;
        CALayer* const super_layer = host_layer.superlayer;
        CALayer* shadow_layer = find_border_shadow_layer(host_layer);

        const maui::graphics::paint* const paint = (shadow != nullptr) ? shadow->paint() : nullptr;
        if (paint == nullptr || super_layer == nil)
        {
            [shadow_layer removeFromSuperlayer]; // nil-safe; also the no-superlayer (pre-mount) case
            return;
        }
        if (shadow_layer == nil)
        {
            shadow_layer = [CALayer layer];
            shadow_layer.name = k_border_shadow_layer_name;
            shadow_layer.backgroundColor = UIColor.clearColor.CGColor; // shadowPath casts the shadow, not content
            [super_layer insertSublayer:shadow_layer below:host_layer];
        }
        // ShadowExtensions scalar push (C# sets ShadowRadius = Radius / 2, like apply_shadow).
        shadow_layer.shadowColor = to_ui_color(paint->background_color()).CGColor;
        shadow_layer.shadowOpacity = static_cast<float>(shadow->opacity());
        shadow_layer.shadowRadius = static_cast<CGFloat>(shadow->radius() / 2.0);
        const maui::graphics::point offset = shadow->offset();
        shadow_layer.shadowOffset = CGSizeMake(static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y));
        reframe_border_shadow(native, spec, has_fill, bounds);
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
        auto* const view = (__bridge UIView*)native;
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
        // The Border content is hosted as a SUBVIEW (set_content -> addSubview), whose layer composites
        // ABOVE this stroke sublayer at the default zPosition 0 (the content is added after the stroke), so
        // the content painted over the border stroke. MAUI draws the border stroke ON TOP of the content
        // (MauiCALayer.DrawInContext strokes last), so lift the stroke above the content via zPosition.
        stroke_layer.zPosition = 1;
        stroke_layer.frame = CGRectMake(bounds.x, bounds.y, bounds.width, bounds.height);
        stroke_layer.fillColor = nil; // stroke only — the background is the container layer's

        const maui::graphics::path_f path = spec.shape->path_for_bounds(bounds);
        CGPathRef cg = path_to_cg_path(path);
        stroke_layer.path = cg;
        CGPathRelease(cg);

        stroke_layer.strokeColor = to_ui_color(spec.stroke_color).CGColor;
        // DOUBLE the width — the mask cuts the outer half (MauiCALayer.DrawBorder).
        stroke_layer.lineWidth = static_cast<CGFloat>(2 * spec.thickness);
        stroke_layer.lineCap = border_line_cap(spec.line_cap);
        stroke_layer.lineJoin = border_line_join(spec.line_join);
        stroke_layer.miterLimit = static_cast<CGFloat>(static_cast<double>(spec.miter_limit) * spec.thickness / 4.0);
        stroke_layer.lineDashPattern = border_dash_pattern(spec.dash_pattern, spec.thickness);
        stroke_layer.lineDashPhase = static_cast<CGFloat>(static_cast<double>(spec.dash_offset) * spec.thickness);
    }
} // namespace maui::platform::ios

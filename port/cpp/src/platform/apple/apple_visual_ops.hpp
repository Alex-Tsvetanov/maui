#pragma once
// Shared AppKit operations for the generic-IView visual layer — the platform side of the shared
// view_mapper's map_background / map_shadow / map_clip (view_mapper.cpp). Objective-C++ only — include
// exclusively from .mm files compiled as Objective-C++ (it references NSView / CALayer / CAShapeLayer /
// CGPath / CGColor).
//
// - apply_background ports Microsoft.Maui.Platform.PaintExtensions (iOS): a SolidPaint sets the backing
//   layer's backgroundColor (to_ns_color(...).CGColor); a null paint clears it. Only SolidPaint is honored
//   (gradients deferred — matching this unit's paint model); other paint kinds clear the color.
// - apply_shadow ports ShadowExtensions.SetShadow / ClearShadow (iOS): it sets the backing layer's
//   ShadowColor / ShadowOpacity / ShadowRadius (= Radius / 2, exactly as C#) / ShadowOffset; a null shadow
//   (or null paint) clears the shadow.
// - apply_clip ports WrapperView.SetClip (iOS): it converts shape->path_for_bounds(bounds) to a CGPath
//   (path_to_cg_path below, a faithful walk of GraphicsExtensions.AsCGPath) and installs it as a
//   CAShapeLayer set as the view layer's mask; a null shape removes the mask.
//
// These helpers exist for the per-control retrofit (the coordinator wires them into each control's
// platform update_background / update_shadow / update_clip overrides); this unit provides them but does
// not itself call them from any control.

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <cmath>

#include "maui/core/i_shadow.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

#include "apple_conversions.hpp"

namespace maui::platform::apple
{
    // Walk a path_f's operations/points into a CGPath, a faithful port of
    // GraphicsExtensions.AsCGPath (MoveToPoint / AddLineToPoint / AddQuadCurveToPoint / AddCurveToPoint /
    // CloseSubpath per operation). The Arc operation is mapped via CGPathAddArc on the ellipse's center
    // (the C# AsCGPath builds it through a CGAffineTransform scaling y; here we use the analytic center +
    // radius, equivalent for the axis-aligned arcs the shape builders emit). CF_RETURNS_RETAINED: the
    // returned path is +1-owned (the Create rule) — the caller must CGPathRelease it.
    inline CGMutablePathRef path_to_cg_path(const maui::graphics::path_f& path) CF_RETURNS_RETAINED
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
                    CGPathAddCurveToPoint(cg, nullptr, control1.x, control1.y, control2.x, control2.y, end.x, end.y);
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
                    // AsCGPath negates the angles (screen-space y is flipped) and walks counter-clockwise
                    // when the path is clockwise; CGPathAddArc's last arg is the clockwise flag.
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

    inline void apply_background(void* native, const maui::graphics::paint* p)
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
        // Only a SolidPaint maps to a layer background color (gradients deferred); anything else clears it.
        const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(p);
        if (solid == nullptr)
        {
            layer.backgroundColor = nil;
            return;
        }
        layer.backgroundColor = to_ns_color(solid->color()).CGColor;
    }

    inline void apply_shadow(void* native, const maui::core::i_shadow* s)
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
        // A null shadow (or one without a paint) clears the shadow (ShadowExtensions.ClearShadow).
        const maui::graphics::paint* const paint = (s != nullptr) ? s->paint() : nullptr;
        if (paint == nullptr)
        {
            layer.shadowColor = CGColorGetConstantColor(kCGColorClear);
            layer.shadowRadius = 0;
            layer.shadowOffset = CGSizeZero;
            layer.shadowOpacity = 0;
            return;
        }
        layer.shadowColor = to_ns_color(paint->background_color()).CGColor;
        layer.shadowOpacity = static_cast<float>(s->opacity());
        // C# sets ShadowRadius = Radius / 2.
        layer.shadowRadius = static_cast<CGFloat>(s->radius() / 2.0);
        const maui::graphics::point offset = s->offset();
        layer.shadowOffset = CGSizeMake(static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y));
    }

    inline void apply_clip(void* native, const maui::graphics::i_shape* shape, maui::graphics::rect bounds)
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
        // A null shape removes any existing mask (WrapperView.SetClip with clip == null).
        if (shape == nullptr)
        {
            layer.mask = nil;
            return;
        }
        const maui::graphics::path_f path = shape->path_for_bounds(bounds);
        // path_to_cg_path returns a +1-owned path (CF_RETURNS_RETAINED); the CAShapeLayer's `path` (a copy
        // property) retains its own reference, so we release ours right after assigning. The annotation lets
        // the analyzer see the Create/Release pairing. CGPathRef is `const CGPath*` (no extra const local).
        CGPathRef cg = path_to_cg_path(path);
        CAShapeLayer* const mask = [CAShapeLayer layer];
        mask.path = cg;
        CGPathRelease(cg);
        layer.mask = mask;
    }
} // namespace maui::platform::apple

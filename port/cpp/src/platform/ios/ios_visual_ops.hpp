#pragma once
// Shared UIKit operations for the generic-IView visual layer — the platform side of the shared
// view_mapper's map_background / map_shadow / map_clip (view_mapper.cpp), and the iOS twin of
// apple_visual_ops.hpp. Objective-C++ only — include exclusively from .mm files compiled as
// Objective-C++ (it references UIView / CALayer / CAShapeLayer / CGPath / CGColor).
//
// Unlike the AppKit twin (which ADAPTED these), the C# originals ARE the iOS implementations, so the
// helpers below port them directly:
// - apply_background ports Microsoft.Maui.Platform PaintExtensions (PaintExtensions.iOS.cs): a
//   SolidPaint sets the backing layer's backgroundColor (to_ui_color(...).CGColor); a
//   LinearGradientPaint / RadialGradientPaint installs a CAGradientLayer (axial / radial) as a sublayer
//   of the backing layer, sized to the view bounds, with colors[] + locations[] from the ordered stops
//   (CreateCALayer / GetCAGradientLayer*); a null paint clears both. The gradient sublayer is tagged
//   (k_gradient_layer_name) so repeated calls replace/remove it cleanly (switching paint kinds never
//   leaves a stale gradient behind).
// - apply_shadow ports ShadowExtensions.SetShadow / ClearShadow (ShadowExtensions.cs, iOS): it sets the
//   backing layer's ShadowColor / ShadowOpacity / ShadowRadius (= Radius / 2, exactly as C#) /
//   ShadowOffset; a null shadow (or null paint) clears the shadow.
// - apply_clip ports WrapperView.SetClip (WrapperView.cs, iOS): it converts shape->path_for_bounds(
//   bounds) to a CGPath (path_to_cg_path below, a faithful walk of GraphicsExtensions.AsCGPath) and
//   installs it as a CAShapeLayer set as the view layer's mask; a null shape removes the mask.
//
// UIKit difference from the AppKit twin: a UIView is ALWAYS layer-backed (view.layer is nonnull), so
// there is no wantsLayer request / nil-layer guard here. These helpers exist for the per-control
// retrofit (the coordinator wires them into each control's platform update_background / update_shadow /
// update_clip overrides); this unit wires its three container structs and provides them for the rest.

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/core/image_source_paint.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/image_source_services.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

#include "ios_conversions.hpp"

namespace maui::platform::ios
{
    // Walk a path_f's operations/points into a CGPath, a faithful port of
    // GraphicsExtensions.AsCGPath (MoveToPoint / AddLineToPoint / AddQuadCurveToPoint / AddCurveToPoint /
    // CloseSubpath per operation) — the same CG-only walk as the AppKit twin (CoreGraphics is shared by
    // both platforms; only the view types differ). The Arc operation is mapped via CGPathAddArc on the
    // ellipse's center (the C# AsCGPath builds it through a CGAffineTransform scaling y; here we use the
    // analytic center + radius, equivalent for the axis-aligned arcs the shape builders emit).
    // CF_RETURNS_RETAINED: the returned path is +1-owned (the Create rule) — the caller must
    // CGPathRelease it.
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

    // The name tagging the gradient sublayer apply_background installs, so a later call can find and
    // replace/remove it (CALayer.name is a plain string slot; the wrapper-view flow uses sublayers).
    inline NSString* const k_gradient_layer_name = @"maui.background.gradient";

    // The name tagging the image-source background sublayer (an ImageSourcePaint fill) — C#'s
    // ViewExtensions.BackgroundLayerName. apply_background installs/removes it by this tag.
    inline NSString* const k_image_layer_name = @"maui.background.image";

    // Port of PaintExtensions.iOS.cs GetCAGradientLayerColors: the ordered stops' colors as CGColors. A
    // fully transparent stop (Colors.Transparent) borrows its neighbor's color at alpha 0 (so the gradient
    // fades to the adjacent hue rather than to black), exactly as C#.
    inline NSArray* gradient_layer_colors(const std::vector<maui::graphics::gradient_stop>& stops)
    {
        if (stops.empty())
        {
            return @[];
        }
        NSMutableArray* const colors = [NSMutableArray arrayWithCapacity:stops.size()];
        for (std::size_t index = 0; index < stops.size(); index++)
        {
            const maui::graphics::color stop_color = stops[index].color();
            // A fully transparent stop borrows its neighbor's color at alpha 0 (C# GetCAGradientLayerColors:
            // gradientStops[index == 0 ? index + 1 : index - 1].Color). With a single stop there is no
            // neighbor — C# would index out of range here; we keep the stop's own (transparent) color, which
            // avoids UB and is the only meaningful color for a one-stop gradient.
            const bool has_neighbor = stops.size() > 1;
            if (stop_color == maui::graphics::color(0, 0, 0, 0) && has_neighbor) // Colors.Transparent
            {
                const std::size_t neighbor = (index == 0) ? index + 1 : index - 1;
                const maui::graphics::color borrowed = stops[neighbor].color().with_alpha(0.0F);
                [colors addObject:(__bridge id)to_ui_color(borrowed).CGColor];
            }
            else
            {
                [colors addObject:(__bridge id)to_ui_color(stop_color).CGColor];
            }
        }
        return colors;
    }

    // Port of PaintExtensions.iOS.cs GetCAGradientLayerLocations: the ordered stops' offsets as NSNumbers.
    // If there is more than one stop and any offset is non-zero, the offsets are used directly; otherwise the
    // stops are spread evenly (step = 1/count), keeping a leading zero-offset stop at its computed slot.
    inline NSArray<NSNumber*>* gradient_layer_locations(const std::vector<maui::graphics::gradient_stop>& stops)
    {
        if (stops.empty())
        {
            return @[];
        }
        const bool any_non_zero =
            std::ranges::any_of(stops, [](const maui::graphics::gradient_stop& s) { return s.offset() != 0; });
        if (stops.size() > 1 && any_non_zero)
        {
            NSMutableArray<NSNumber*>* const locations = [NSMutableArray arrayWithCapacity:stops.size()];
            for (const auto& stop : stops)
            {
                [locations addObject:@(stop.offset())];
            }
            return locations;
        }

        const int item_count = static_cast<int>(stops.size());
        const float step = 1.0F / static_cast<float>(item_count);
        NSMutableArray<NSNumber*>* const locations = [NSMutableArray arrayWithCapacity:stops.size()];
        for (int index = 0; index < item_count; index++)
        {
            const float location = step * static_cast<float>(index);
            const bool set_location = !std::ranges::any_of(
                stops, [location](const maui::graphics::gradient_stop& s) { return s.offset() > location; });
            const float offset = stops[static_cast<std::size_t>(index)].offset();
            // [NSNumber numberWithFloat:] (not the @(...) boxing literal) for these bare-identifier values:
            // it boxes the same NSNumber while reading cleanly as a method call.
            if (offset == 0 && set_location)
            {
                [locations addObject:[NSNumber numberWithFloat:location]];
            }
            else
            {
                [locations addObject:[NSNumber numberWithFloat:offset]];
            }
        }
        return locations;
    }

    // Port of PaintExtensions.iOS.cs GetRadialGradientPaintEndPoint: derive the radial layer's endPoint from
    // the center and radius, clamped to [0,1]. (When a coordinate is exactly 1 the radius is subtracted
    // rather than added, keeping the endPoint inside the unit square.)
    inline CGPoint radial_gradient_end_point(maui::graphics::point center, double radius)
    {
        double x = center.x == 1 ? center.x - radius : center.x + radius;
        x = std::clamp(x, 0.0, 1.0);
        double y = center.y == 1 ? center.y - radius : center.y + radius;
        y = std::clamp(y, 0.0, 1.0);
        return CGPointMake(x, y);
    }

    // Remove any sublayer this helper previously installed under `name` (idempotent cleanup before re-apply).
    inline void remove_background_named_layer(CALayer* layer, NSString* name)
    {
        // Snapshot the sublayers (a copy), then remove the tagged ones — mutating layer.sublayers under a
        // fast-enumeration would be unsafe; index iteration over the copy also keeps the loop variable
        // initialized.
        NSArray<CALayer*>* const sublayers = [layer.sublayers copy];
        for (NSUInteger i = 0; i < sublayers.count; i++)
        {
            CALayer* const sub = sublayers[i];
            if ([sub.name isEqualToString:name])
            {
                [sub removeFromSuperlayer];
            }
        }
    }

    inline void remove_background_gradient_layer(CALayer* layer)
    {
        remove_background_named_layer(layer, k_gradient_layer_name);
    }

    // Install `source`'s image as the view's background layer (C# ViewExtensions.UpdateBackgroundImageSource
    // Async): a named sublayer whose contents is the source's CGImage, sized to the view bounds with resize
    // gravity (C# StaticCALayer { ContentsGravity = GravityResize }). The base color is cleared (C#
    // BackgroundColor = UIColor.Clear). Returns whether an image layer was installed (the caller skips the
    // solid/gradient paths when it was). Ports GetRequiredImageSourceService + GetImageAsync + .CGImage (run
    // synchronously — the apple/ios services produce their image on the calling thread; a fully-async refresh
    // is the X1 brushes unit's concern).
    //
    // LIFETIME: the whole install runs INSIDE the load completion, while the image_source_result still owns
    // the UIImage — `layer.contents = cgImage` retains the CGImage before the result is destroyed. UIImage's
    // .CGImage is owned by the UIImage (NOT autoreleased), so extracting it and using it after the UIImage's
    // CFRelease would be a UAF; installing it on-the-spot avoids that.
    inline bool apply_image_source_background_layer(CALayer* layer, maui::core::i_image_source* source)
    {
        remove_background_named_layer(layer, k_image_layer_name);
        if (source == nullptr || source->is_empty())
        {
            return false;
        }
        maui::core::image_source_service_registry& registry = maui::core::default_image_source_service_registry();
        maui::core::register_default_image_source_services(registry);
        const std::shared_ptr<maui::core::i_image_source_service> service = registry.resolve(*source);
        if (!service)
        {
            return false;
        }

        bool installed = false;
        const maui::core::cancellation_token token;
        service->load(*source, token, [&installed, layer](maui::core::image_source_result result) {
            if (!result.loaded() || result.image() == nullptr)
            {
                return;
            }
            UIImage* const image = (__bridge UIImage*)result.image();
            CGImageRef const cg_image = image.CGImage;
            if (cg_image == nullptr)
            {
                return;
            }
            CALayer* const image_layer = [CALayer layer];
            image_layer.name = k_image_layer_name;
            image_layer.contents = (__bridge id)cg_image; // retains the CGImage while the UIImage is alive
            image_layer.frame = layer.bounds;
            image_layer.contentsGravity = kCAGravityResize;
            layer.backgroundColor = nil; // C# clears the solid background while the image layer is shown
            [layer addSublayer:image_layer];
            installed = true;
        });
        return installed;
    }

    inline void apply_background(void* native, const maui::graphics::paint* p)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge UIView*)native;
        CALayer* const layer = view.layer; // a UIView is always layer-backed (nonnull)

        // An ImageSourcePaint installs the source's image as a backing sublayer (C# PageExtensions /
        // ViewExtensions.UpdateBackgroundImageSourceAsync branch). Remove any stale gradient, then render
        // the image; the base color is cleared by the helper. If installed, the solid/gradient paths below
        // are skipped.
        if (const auto* const image_paint = dynamic_cast<const maui::core::image_source_paint*>(p))
        {
            remove_background_gradient_layer(layer);
            if (apply_image_source_background_layer(layer, image_paint->image_source()))
            {
                return;
            }
            layer.backgroundColor = nil; // no image resolved → clear (the image layer was already removed)
            return;
        }
        // A non-image paint (incl. null) removes any previously-installed image sublayer.
        remove_background_named_layer(layer, k_image_layer_name);

        // A gradient paint installs a CAGradientLayer sublayer (axial/radial) sized to the view bounds.
        const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(p);
        if (gradient != nullptr)
        {
            layer.backgroundColor = nil; // the gradient sublayer carries the fill, not the base color
            remove_background_gradient_layer(layer);

            CAGradientLayer* const gradient_layer = [CAGradientLayer layer];
            gradient_layer.name = k_gradient_layer_name;
            gradient_layer.contentsGravity = kCAGravityResizeAspectFill;
            gradient_layer.frame = layer.bounds; // sized to the backing layer's bounds

            const std::vector<maui::graphics::gradient_stop> ordered = gradient->get_sorted_stops();

            if (const auto* const linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(gradient))
            {
                gradient_layer.type = kCAGradientLayerAxial;
                gradient_layer.startPoint = CGPointMake(linear->start_point().x, linear->start_point().y);
                gradient_layer.endPoint = CGPointMake(linear->end_point().x, linear->end_point().y);
            }
            else if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(gradient))
            {
                gradient_layer.type = kCAGradientLayerRadial;
                gradient_layer.startPoint = CGPointMake(radial->center().x, radial->center().y);
                gradient_layer.endPoint = radial_gradient_end_point(radial->center(), radial->radius());
                gradient_layer.cornerRadius = static_cast<CGFloat>(radial->radius());
            }

            if (!ordered.empty())
            {
                gradient_layer.colors = gradient_layer_colors(ordered);
                gradient_layer.locations = gradient_layer_locations(ordered);
            }

            [layer addSublayer:gradient_layer];
            return;
        }

        // Any non-gradient call removes a previously-installed gradient sublayer first.
        remove_background_gradient_layer(layer);

        // A SolidPaint maps to the backing layer's backgroundColor; anything else (incl. null) clears it.
        const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(p);
        if (solid == nullptr)
        {
            layer.backgroundColor = nil;
            return;
        }
        layer.backgroundColor = to_ui_color(solid->color()).CGColor;
    }

    inline void apply_shadow(void* native, const maui::core::i_shadow* s)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge UIView*)native;
        CALayer* const layer = view.layer; // a UIView is always layer-backed (nonnull)
        // A null shadow (or one without a paint) clears the shadow (ShadowExtensions.ClearShadow).
        const maui::graphics::paint* const paint = (s != nullptr) ? s->paint() : nullptr;
        if (paint == nullptr)
        {
            layer.shadowColor = UIColor.clearColor.CGColor; // C#: new CGColor(0, 0, 0, 0)
            layer.shadowRadius = 0;
            layer.shadowOffset = CGSizeZero;
            layer.shadowOpacity = 0;
            return;
        }
        layer.shadowColor = to_ui_color(paint->background_color()).CGColor;
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
        auto* const view = (__bridge UIView*)native;
        CALayer* const layer = view.layer; // a UIView is always layer-backed (nonnull)
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
} // namespace maui::platform::ios

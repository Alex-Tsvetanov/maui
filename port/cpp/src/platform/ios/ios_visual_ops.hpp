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
#import <objc/runtime.h>

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
#include "maui/core/thickness.hpp"
#include "maui/core/view_platform_base.hpp"
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
#include "maui/graphics/system_background_paint.hpp"

#include "ios_conversions.hpp"

namespace maui::platform::ios
{
    // Port of ImageViewExtensions.SizeThatFitsImage (Platform/iOS/ImageViewExtensions.cs). UIImageView's
    // own -sizeThatFits: ALWAYS returns the image's natural dimensions, ignoring the constraint (the C#
    // comment: "Calling SizeThatFits on an ImageView always returns the image's dimensions, so we need to
    // call the extension method"), which left AspectFit images measured to full natural height in a
    // width-constrained stack. This computes the aspect-aware fit: ScaleAspectFit scales the image down by
    // the smaller of the two constraint ratios; ScaleToFill/Center just clamp each axis to the constraint.
    // `padding` mirrors the C# Thickness param (button content insets; zero for a plain Image).
    inline CGSize size_that_fits_image(UIImageView* image_view, CGSize constraints, maui::core::thickness padding = {})
    {
        UIImage* const image = image_view.image;
        if (image == nil)
        {
            return CGSizeMake(0, 0); // no image → takes up no space (C# returns CGSize.Empty)
        }
        const CGSize image_size = image.size;
        const double image_width = image_size.width;
        const double image_height = image_size.height;
        const double horizontal_thickness = padding.left + padding.right;
        const double vertical_thickness = padding.top + padding.bottom;
        const double width_constraint = constraints.width - horizontal_thickness;
        const double height_constraint = constraints.height - vertical_thickness;
        const double constrained_width = std::min(image_width, width_constraint);
        const double constrained_height = std::min(image_height, height_constraint);
        if (image_view.contentMode == UIViewContentModeScaleAspectFit && image_width > 0 && image_height > 0)
        {
            const double width_ratio = constrained_width / image_width;
            const double height_ratio = constrained_height / image_height;
            const double scale_factor = std::min(width_ratio, height_ratio);
            return CGSizeMake(static_cast<CGFloat>(image_width * scale_factor + horizontal_thickness),
                              static_cast<CGFloat>(image_height * scale_factor + vertical_thickness));
        }
        return CGSizeMake(static_cast<CGFloat>(constrained_width + horizontal_thickness),
                          static_cast<CGFloat>(constrained_height + vertical_thickness));
    }

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

    // The name tagging the bar-background layer apply_bar_background installs — the analog of C#'s
    // BrushExtensions.BackgroundLayer ("BackgroundLayer"), kept distinct from the generic-IView background
    // tags so a tab bar's brush fill is found/replaced independently. (tabbed_page_handler.mm.)
    inline NSString* const k_bar_background_layer_name = @"maui.bar.background";

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

    // Resize the named gradient / image background sublayers that apply_background installed to the view's
    // CURRENT layer bounds — the layout-time counterpart to apply_background's install-time
    // `gradient_layer.frame = layer.bounds` / `image_layer.frame = layer.bounds`. apply_background runs at
    // property-sync time, before the view has been arranged, so for a view that is NOT continuously laid
    // out by a parent the fill layer would be left zero-sized (invisible). Native MAUI views re-sync their
    // background layer on layoutSubviews (MauiView/MauiPicker etc.); call this from a view's layoutSubviews
    // override so a gradient/image fill tracks the bounds. Only `frame` is touched — colors / locations /
    // type / start+endPoint / contents / contentsGravity carry the fill and are set at install time
    // (CAGradientLayer start/endPoint are unit-square fractions and need no rescale).
    inline void resize_background_layers(void* native)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge UIView*)native;
        CALayer* const layer = view.layer;                           // a UIView is always layer-backed (nonnull)
        NSArray<CALayer*>* const sublayers = [layer.sublayers copy]; // snapshot, as remove_background_named_layer
        for (NSUInteger i = 0; i < sublayers.count; i++)
        {
            CALayer* const sub = sublayers[i];
            if ([sub.name isEqualToString:k_gradient_layer_name] || [sub.name isEqualToString:k_image_layer_name])
            {
                sub.frame = layer.bounds;
            }
        }
    }

    // Port of BrushExtensions.UpdateBackground / GetBackgroundLayer (iOS — Controls/Platform): install a
    // brush fill, expressed as a graphics::paint, as a CALayer at the BOTTOM (index 0) of `layer`. The old
    // bar-background layer (k_bar_background_layer_name) is removed first so a kind switch (solid↔gradient)
    // or a stop change never leaves a stale layer behind. A SolidPaint becomes a plain CALayer carrying
    // backgroundColor; a Linear/Radial gradient becomes an axial/radial CAGradientLayer (colors+locations
    // from the ordered stops); a null/empty paint just removes the layer (Brush.IsNullOrEmpty → return).
    // The layer is sized to `frame` (the C# `Frame = control.Bounds`).
    inline void apply_bar_background(CALayer* layer, const maui::graphics::paint* p, CGRect frame)
    {
        if (layer == nullptr)
        {
            return;
        }
        remove_background_named_layer(layer, k_bar_background_layer_name); // C# RemoveBackgroundLayer first

        if (const auto* const gradient = dynamic_cast<const maui::graphics::gradient_paint*>(p))
        {
            CAGradientLayer* const gradient_layer = [CAGradientLayer layer];
            gradient_layer.name = k_bar_background_layer_name;
            gradient_layer.contentsGravity = kCAGravityResizeAspectFill;
            gradient_layer.frame = frame;

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

            const std::vector<maui::graphics::gradient_stop> ordered = gradient->get_sorted_stops();
            if (!ordered.empty())
            {
                gradient_layer.colors = gradient_layer_colors(ordered);
                gradient_layer.locations = gradient_layer_locations(ordered);
            }
            [layer insertSublayer:gradient_layer atIndex:0]; // C# InsertBackgroundLayer(..., index: 0)
            return;
        }

        // A SolidPaint → a plain CALayer carrying the color (C# StaticCALayer { BackgroundColor = … }).
        const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(p);
        if (solid == nullptr)
        {
            return; // null / empty / unsupported brush → only the removal above (Brush.IsNullOrEmpty path)
        }
        CALayer* const solid_layer = [CALayer layer];
        solid_layer.name = k_bar_background_layer_name;
        solid_layer.contentsGravity = kCAGravityResizeAspectFill;
        solid_layer.frame = frame;
        solid_layer.backgroundColor = to_ui_color(solid->color()).CGColor;
        [layer insertSublayer:solid_layer atIndex:0];
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
            // Same C# LayerExtensions.InsertBackgroundLayer rule as the gradient path: zPosition = -1 +
            // bottom insertion so an image background fill stays behind the control's own sublayers and
            // subviews after a UIKit-driven re-layout.
            image_layer.zPosition = -1;
            [layer insertSublayer:image_layer atIndex:0];
            installed = true;
        });
        return installed;
    }

    // Resolve UIColor.systemBackground against a SPECIFIC view's trait collection, NOT the global one.
    // `UIColor.systemBackgroundColor.CGColor` resolves the dynamic color against UITraitCollection.current,
    // which — outside a view draw/layout callback (apply_background runs at map time) — is the SCREEN's
    // trait (e.g. the Mac's dark appearance under Catalyst), so it would bake a black CGColor onto the
    // layer even when the window forces Light (host_run.mm overrideUserInterfaceStyle). The frame fill is
    // on a CALayer (for the rounded-corner clip), and a CALayer never auto-resolves a dynamic color, so we
    // resolve MANUALLY against the view's own trait — which inherits the window's forced appearance — the
    // way MAUI's FrameRenderer relies on its platform view's appearance.
    inline CGColorRef system_background_cg_color(UIView* view)
    {
        UITraitCollection* const traits = view != nil ? view.traitCollection : UITraitCollection.currentTraitCollection;
        return [UIColor.systemBackgroundColor resolvedColorWithTraitCollection:traits].CGColor;
    }

    // The associated-object key holding the UITraitUserInterfaceStyle change registration installed for a
    // system_background_paint fill, so a live light/dark flip re-resolves the layer color (the solid
    // background is set once at map/update time and is NOT re-applied on layoutSubviews, which only resizes
    // gradient/image sublayers). A non-marker paint removes the registration.
    inline const void* k_system_bg_trait_key = &k_system_bg_trait_key;

    // Install (idempotently) or remove the trait-change re-resolution for a system-background fill.
    inline void set_system_background_trait_tracking(UIView* view, bool track)
    {
        if (view == nil)
        {
            return;
        }
        if (!track)
        {
            objc_setAssociatedObject(view, k_system_bg_trait_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            return;
        }
        if (objc_getAssociatedObject(view, k_system_bg_trait_key) != nil)
        {
            return; // already registered — keep the single token
        }
        if (@available(iOS 17, *))
        {
            __weak UIView* const weak_view = view;
            id<UITraitChangeRegistration> const registration = [view
                registerForTraitChanges:@[ UITraitUserInterfaceStyle.class ]
                            withHandler:^(__kindof id<UITraitEnvironment> /*env*/, UITraitCollection* /*previous*/) {
                              UIView* const strong = weak_view;
                              if (strong != nil)
                              {
                                  strong.layer.backgroundColor = system_background_cg_color(strong);
                              }
                            }];
            objc_setAssociatedObject(view, k_system_bg_trait_key, registration, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
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
            set_system_background_trait_tracking(view, false); // leaving any system-background fill
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
            set_system_background_trait_tracking(view, false); // leaving any system-background fill
            layer.backgroundColor = nil;                       // the gradient sublayer carries the fill
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

            // C# LayerExtensions.InsertBackgroundLayer(backgroundLayer, 0): set zPosition = -1 BEFORE
            // inserting and add the layer at the BOTTOM of the sublayer stack (index 0). The negative
            // zPosition is the load-bearing part — it keeps the brush fill behind the control's own
            // sublayers AND its thumb / track / text / bezel SUBVIEWS "even if UIKit reorganizes the
            // sublayer array during layout passes" (the C# comment). A plain addSublayer (top of stack,
            // zPosition 0) let the gradient land in front of the UISlider/UISwitch thumb (R2a) and the
            // date/time picker field content (R2b) once UIKit re-laid the control out.
            gradient_layer.zPosition = -1;
            [layer insertSublayer:gradient_layer atIndex:0];
            return;
        }

        // Any non-gradient call removes a previously-installed gradient sublayer first.
        remove_background_gradient_layer(layer);

        // A SolidPaint maps to the backing layer's backgroundColor; anything else (incl. null) clears it.
        // NOTE: this sets layer.backgroundColor (not the UIView property) deliberately — some native hosts
        // (the layout container view) mis-render when their UIView.backgroundColor is set directly. Controls
        // that draw their own chrome (UITextField RoundedRect, UISearchBar) need the UIView property to tint;
        // those handlers override update_background to set the native control's backgroundColor for solids.
        const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(p);
        if (solid == nullptr)
        {
            set_system_background_trait_tracking(view, false);
            layer.backgroundColor = nil;
            return;
        }
        // The legacy Frame's default fill: system_background_paint resolves to the DYNAMIC system
        // background (UIColor.systemBackground = white in light, ~black in dark) against the VIEW's own
        // trait — the compatibility FrameRenderer.SetupLayer sets ColorExtensions.BackgroundColor (also
        // UIColor.systemBackground) when BackgroundColor is null. Register for trait changes so a live
        // light/dark flip re-resolves the layer. On Mac Catalyst this file is the Catalyst handler (aliased
        // iOS backend), so the frame adapts there too. A plain solid_paint keeps its static color.
        if (dynamic_cast<const maui::graphics::system_background_paint*>(p) != nullptr)
        {
            layer.backgroundColor = system_background_cg_color(view);
            set_system_background_trait_tracking(view, true);
            return;
        }
        set_system_background_trait_tracking(view, false);
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

    // Bounds-dependent clip re-application (the leaf-control twin of resize_background_layers).
    //
    // apply_clip sizes the CAShapeLayer mask to the view bounds AT PUSH TIME — which for a leaf control
    // is map time, before the first layout, when layer.bounds is still 0×0 (the mask would clip the whole
    // view away). C# never hits this because WrapperView.SetClip is re-run from WrapperView.LayoutSubviews
    // against the live frame on every pass (WrapperView.cs:118). The port has no per-control WrapperView,
    // so each leaf handler re-applies the mask itself from its layout hook. To re-stroke without a back-
    // reference to the C++ handler (a UIKit-driven autoresize / rotation never routes through the handler),
    // store_clip_shape stashes the clip geometry — a NON-owning borrow the control keeps alive, refreshed
    // on every update_clip incl. the null clear, exactly like MauiIosBorder.borderRefresh captures its
    // spec — and reapply_clip re-runs apply_clip against the view's CURRENT bounds.
    namespace detail
    {
        // Associated-object key (its address is the key) holding an NSValue box of the const i_shape*
        // borrow. NSValue retains the box, not the C++ object; the control owns the shape's lifetime.
        inline const char k_clip_shape_key = 0;
    } // namespace detail

    inline void store_clip_shape(void* native, const maui::graphics::i_shape* shape)
    {
        if (native == nullptr)
        {
            return;
        }
        UIView* const view = (__bridge UIView*)native;
        // A null shape clears the stash (no re-application on later layouts), matching the null-clip path.
        NSValue* const boxed = shape == nullptr ? nil : [NSValue valueWithPointer:static_cast<const void*>(shape)];
        objc_setAssociatedObject(view, &detail::k_clip_shape_key, boxed, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    inline void reapply_clip(void* native)
    {
        if (native == nullptr)
        {
            return;
        }
        UIView* const view = (__bridge UIView*)native;
        NSValue* const boxed = objc_getAssociatedObject(view, &detail::k_clip_shape_key);
        if (boxed == nil)
        {
            return; // no clip stashed → nothing to re-frame (and never touch an unmasked view)
        }
        const auto* const shape = static_cast<const maui::graphics::i_shape*>([boxed pointerValue]);
        const CGRect bounds = view.bounds; // WrapperView.SetClip's RectF(0, 0, frame.Width, frame.Height)
        apply_clip(native, shape,
                   maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // The combined push a leaf handler's update_clip calls: re-mask against the CURRENT bounds AND stash
    // the geometry so the control's layout hook (MauiIos*.layoutSubviews / platform_arrange) can re-frame
    // it after the real layout pass. `bounds` is the view's live bounds at map time (0×0 before layout).
    inline void apply_and_store_clip(void* native, const maui::graphics::i_shape* shape, maui::graphics::rect bounds)
    {
        apply_clip(native, shape, bounds);
        store_clip_shape(native, shape);
    }

    // apply_transform is a faithful port of Microsoft.Maui.Platform.TransformationExtensions
    // .UpdateTransformation (src/Core/src/Platform/iOS/TransformationExtensions.cs): it rebuilds the WHOLE
    // CATransform3D from the ten ITransform scalars (so any single change re-applies the full transform —
    // matching the shared map_transform, which always passes the complete transform_spec). The C# original
    // keys the anchor-relative offset on view.Frame.Width/Height; here we read the laid-out size from the
    // view's backing layer bounds (set by the handler's platform_arrange), exactly as the AppKit twin
    // apple_view_ops.hpp does. This is the UIKit-native original the AppKit version was adapted from, so the
    // math is identical (the AppKit file ADAPTED it; both share CoreAnimation, only the view type differs).
    inline void apply_transform(void* native, const maui::core::transform_spec& t)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge UIView*)native;
        CALayer* const layer = view.layer; // a UIView is always layer-backed (nonnull)

        const auto anchor_x = static_cast<CGFloat>(t.anchor_x);
        const auto anchor_y = static_cast<CGFloat>(t.anchor_y);
        const auto translation_x = static_cast<CGFloat>(t.translation_x);
        const auto translation_y = static_cast<CGFloat>(t.translation_y);
        const auto rotation_x = static_cast<CGFloat>(t.rotation_x);
        const auto rotation_y = static_cast<CGFloat>(t.rotation_y);
        const auto rotation = static_cast<CGFloat>(t.rotation);
        // The uniform Scale multiplies the per-axis factors; the z-scale is the uniform Scale alone
        // (TransformationExtensions: scaleX = ScaleX * Scale, scaleY = ScaleY * Scale).
        const auto scale = static_cast<CGFloat>(t.scale);
        const CGFloat scale_x = static_cast<CGFloat>(t.scale_x) * scale;
        const CGFloat scale_y = static_cast<CGFloat>(t.scale_y) * scale;

        // The anchor-relative offset uses the laid-out size (C# uses view.Frame; the backing layer's bounds
        // is the post-arrange analog, same as the AppKit twin).
        const CGFloat width = layer.bounds.size.width;
        const CGFloat height = layer.bounds.size.height;

        constexpr double epsilon = 0.001;

        CATransform3D transform = CATransform3DIdentity;

        // Position is relative to the anchor point.
        if (std::abs(anchor_x - 0.5) > epsilon)
        {
            transform = CATransform3DTranslate(transform, (anchor_x - 0.5) * width, 0, 0);
        }
        if (std::abs(anchor_y - 0.5) > epsilon)
        {
            transform = CATransform3DTranslate(transform, 0, (anchor_y - 0.5) * height, 0);
        }

        if (std::abs(translation_x) > epsilon || std::abs(translation_y) > epsilon)
        {
            transform = CATransform3DTranslate(transform, translation_x, translation_y, 0);
        }

        // Setting m34 (perspective) also stops the layer from pixel-aligning; only do it when there is an
        // out-of-plane rotation (C# gates on rotationX/rotationY % 180).
        if (std::abs(std::fmod(rotation_y, 180.0)) > epsilon || std::abs(std::fmod(rotation_x, 180.0)) > epsilon)
        {
            transform.m34 = 1.0 / -400.0;
        }

        constexpr double deg_to_rad = M_PI / 180.0;
        if (std::abs(std::fmod(rotation_x, 360.0)) > epsilon)
        {
            transform = CATransform3DRotate(transform, rotation_x * deg_to_rad, 1.0, 0.0, 0.0);
        }
        if (std::abs(std::fmod(rotation_y, 360.0)) > epsilon)
        {
            transform = CATransform3DRotate(transform, rotation_y * deg_to_rad, 0.0, 1.0, 0.0);
        }
        transform = CATransform3DRotate(transform, rotation * deg_to_rad, 0.0, 0.0, 1.0);

        if (std::abs(scale_x - 1.0) > epsilon || std::abs(scale_y - 1.0) > epsilon)
        {
            transform = CATransform3DScale(transform, scale_x, scale_y, scale);
        }

        layer.anchorPoint = CGPointMake(anchor_x, anchor_y);
        layer.transform = transform;
    }
} // namespace maui::platform::ios

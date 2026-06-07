#pragma once
// Shared AppKit operations for the generic-IView render transform and flow direction — the platform
// side of the shared view_mapper's map_transform / map_flow_direction (view_mapper.cpp). Objective-C++
// only — include exclusively from .mm files compiled as Objective-C++ (it references NSView / CALayer /
// CATransform3D).
//
// apply_transform is a faithful port of Microsoft.Maui.Platform.TransformationExtensions
// .UpdateTransformation (src/Core/src/Platform/iOS/TransformationExtensions.cs): it rebuilds the WHOLE
// CATransform3D from the ten ITransform scalars (so any single change re-applies the full transform —
// matching the shared map_transform, which always passes the complete transform_spec). The C# original
// keys the anchor-relative offset on view.Frame.Width/Height; here we read the laid-out size from the
// NSView's own bounds (set by the handler's platform_arrange), the AppKit analog of view.Frame.
//
// apply_flow_direction mirrors ViewExtensions.UpdateFlowDirection but for AppKit: it sets the view's
// NSView.userInterfaceLayoutDirection (the AppKit counterpart of iOS's SemanticContentAttribute).
// MatchParent has no AppKit "unspecified" constant, so it resolves to the application-wide default
// layout direction — the AppKit analog of inheriting the ambient direction.
//
// These helpers exist for the per-control retrofit (the coordinator wires them into each control's
// platform update_transform / update_flow_direction overrides); this unit provides them but does not
// itself call them from any control.

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <cmath>

#include "maui/core/flow_direction.hpp"
#include "maui/core/view_platform_base.hpp"

namespace maui::platform::apple
{
    // Rebuild the render transform from all ten scalars and assign it to the view's backing layer
    // (the layer is created on demand via wantsLayer, as the existing button stroke path already does).
    inline void apply_transform(void* native, const maui::core::transform_spec& t)
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

        // The anchor-relative offset uses the laid-out size (C# uses view.Frame; AppKit's bounds is the
        // post-arrange analog).
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

    inline void apply_flow_direction(void* native, maui::core::flow_direction fd)
    {
        if (native == nullptr)
        {
            return;
        }
        auto* const view = (__bridge NSView*)native;
        switch (fd)
        {
            case maui::core::flow_direction::left_to_right:
                view.userInterfaceLayoutDirection = NSUserInterfaceLayoutDirectionLeftToRight;
                break;
            case maui::core::flow_direction::right_to_left:
                view.userInterfaceLayoutDirection = NSUserInterfaceLayoutDirectionRightToLeft;
                break;
            case maui::core::flow_direction::match_parent:
                // No AppKit "unspecified" — inherit the application-wide default layout direction.
                view.userInterfaceLayoutDirection = NSApplication.sharedApplication.userInterfaceLayoutDirection;
                break;
        }
    }
} // namespace maui::platform::apple

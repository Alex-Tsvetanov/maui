// shape_view_handler — Apple (AppKit / macOS) platform recipe: REUSES the graphics_view drawing
// host (graphics_host.hpp), pointed at the platform's own shape_drawable — exactly C#'s
// MauiShapeView : PlatformGraphicsView with Drawable = new ShapeDrawable(shapeView)
// (ShapeViewHandler.iOS.cs + ShapeViewExtensions.cs). Compiled as Objective-C++ with ARC for the
// `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "graphics_host.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"

namespace
{
    NSView* as_host(void* native)
    {
        return (__bridge NSView*)native;
    }
} // namespace

namespace maui::core
{
    shape_view_platform::~shape_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_drawable_host
            native = nullptr;
        }
    }

    // The portable replay seat (the headless twin; the REAL drawing runs through drawRect).
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    // The generic-IView property pushes (a plain NSView host; is_enabled keeps the base mirror).
    void shape_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void shape_view_platform::update_opacity(double value)
    {
        as_host(native).alphaValue = value;
    }

    void shape_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void shape_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void shape_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void shape_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void shape_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void shape_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = as_host(native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void shape_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void shape_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<shape_view_platform> shape_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<shape_view_platform>();
        platform->native = maui::platform::apple::create_drawable_host();
        // The host renders the platform's own ShapeDrawable (C# MauiShapeView.Drawable).
        maui::platform::apple::drawable_host_set_drawable(platform->native, &platform->drawable);
        return platform;
    }

    // C# UpdateShape (ShapeViewExtensions): re-point the drawable at the virtual view + redraw.
    void shape_view_handler::update_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->drawable.update_shape_view(virtual_view());
        refresh_drawable_state();
        platform->invalidations++;
        maui::platform::apple::drawable_host_invalidate(platform->native);
    }

    // C# InvalidateShape: refresh the drawable pushes + redraw.
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
        maui::platform::apple::drawable_host_invalidate(platform->native);
    }

    void shape_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

// border_handler — Apple (AppKit / macOS) platform recipe: a plain NSView host that holds the single
// content child as a subview and carries the border on its layer — the shape mask plus the
// CAShapeLayer stroke built by apple_border_ops.hpp (the MauiCALayer recipe over stock layers; the
// ops header documents the adaptation). Translated from BorderHandler.iOS.cs (CreatePlatformView →
// the ContentView host; UpdateContent → clear + re-parent the content's native view) + the
// StrokeExtensions funnel (every stroke map → one UpdateMauiCALayer refresh → update_border here).
// Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_border_ops.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    NSView* as_host(void* native)
    {
        return (__bridge NSView*)native;
    }

    // The child's native NSView via its view-handler's native_view() (the content_page helper twin).
    NSView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }
} // namespace

namespace maui::core
{
    border_platform::~border_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper via view_platform_base). The host is a
    // plain NSView; is_enabled keeps the base mirror, and update_clip stays with the base mirror too —
    // the border shape owns the layer mask (border_handler.hpp).
    void border_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void border_platform::update_opacity(double value)
    {
        as_host(native).alphaValue = value;
    }

    void border_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void border_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void border_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void border_platform::update_background(const maui::graphics::paint* value)
    {
        // The container layer's fill; the border shape mask bounds it to the shape (the C# MauiCALayer
        // draws the background into the same clipped layer).
        maui::platform::apple::apply_background(native, value);
    }

    void border_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void border_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void border_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        NSView* const host = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)host; // the void* slot owns one reference
        return platform;
    }

    void border_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = as_host(platform->native);

        // C# UpdateContent: ClearSubviews, then re-parent the content's native view.
        NSArray<NSView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (NSView* const subview = native_child(*platform->hosted_content))
        {
            [subview removeFromSuperview];
            [host addSubview:subview];
        }
    }

    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // Mirror the resolved stroke surface, then push it onto the host's layer (the bounds-dependent
        // stroke path uses the view's LOCAL bounds, like the update_clip callers).
        platform->border = make_border_stroke_spec(*virtual_view());
        const NSRect bounds = as_host(platform->native).bounds;
        maui::platform::apple::apply_border_stroke(
            platform->native, platform->border,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void border_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

// border_handler — iOS (UIKit) platform recipe: a plain UIView host that holds the single content
// child as a subview and carries the border on its layer — the shape mask plus the CAShapeLayer
// stroke built by ios_border_ops.hpp (the MauiCALayer recipe over stock layers; the ops header
// documents the adaptation). Ported from BorderHandler.iOS.cs (CreatePlatformView → the ContentView
// host; UpdateContent → ClearSubviews + re-parent the content's native view) + the StrokeExtensions
// funnel (every stroke map → one UpdateMauiCALayer refresh → update_border here). Compiled as
// Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "ios_border_ops.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }

    // The child's native UIView via its view-handler's native_view() (the content_page helper twin).
    UIView* native_child(maui::core::i_view& child)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(child.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
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

    // The generic-IView property pushes. is_enabled, transform and flow_direction keep the base
    // mirrors (the content_page partial's scope); update_clip stays with the base mirror too — the
    // border shape owns the layer mask (border_handler.hpp).
    void border_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void border_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
    }

    void border_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void border_platform::update_background(const maui::graphics::paint* value)
    {
        // The container layer's fill; the border shape mask bounds it to the shape (the C# MauiCALayer
        // draws the background into the same clipped layer).
        maui::platform::ios::apply_background(native, value);
    }

    void border_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void border_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void border_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        UIView* const host = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
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
        UIView* const host = as_host(platform->native);

        // C# UpdateContent: ClearSubviews, then re-parent the content's native view.
        NSArray<UIView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        if (UIView* const subview = native_child(*platform->hosted_content))
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
        const CGRect bounds = as_host(platform->native).bounds;
        maui::platform::ios::apply_border_stroke(
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
        [as_host(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

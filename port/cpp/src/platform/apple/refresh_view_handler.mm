// refresh_view_handler — Apple (AppKit / macOS) platform recipe: a plain NSView host for the scrollable
// Content. AppKit has NO native pull-to-refresh control (UIRefreshControl is UIKit-only), so this is a
// DOCUMENTED DEVIATION: IsRefreshing drives a stored spinner-overlay flag on the host (the platform
// mirror — a real NSProgressIndicator overlay can be layered later); the spinner color + enabled flag are
// stored mirrors; request_refresh() still writes IsRefreshing=true back through the virtual view (the
// MauiRefreshViewProxy.OnRefresh twin) so the programmatic/test path matches every backend. The host is
// a FLIPPED NSView (create_flipped_host: isFlipped=YES, top-left origin — like content_page, so the
// content's top-down arrange renders top-down). The generic-IView property pushes mirror
// content_page_handler.mm. Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "flipped_container.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSView* as_host(void* native)
    {
        return (__bridge NSView*)native;
    }

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
    refresh_view_platform::~refresh_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    void refresh_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void refresh_view_platform::update_opacity(double value)
    {
        as_host(native).alphaValue = value;
    }

    void refresh_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void refresh_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void refresh_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void refresh_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void refresh_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void refresh_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void refresh_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void refresh_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<refresh_view_platform> refresh_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<refresh_view_platform>();
        // A flipped (top-left origin) host so the content's top-down arrange renders top-down.
        platform->native = maui::platform::apple::create_flipped_host(); // retained — the void* slot owns one ref
        return platform;
    }

    void refresh_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = as_host(platform->native);

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

    // AppKit has no native spinner control; mirror the refreshing flag (a real NSProgressIndicator overlay
    // can be layered later). The documented deviation.
    void refresh_view_handler::update_is_refreshing()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refreshing = virtual_view()->is_refreshing();
    }

    void refresh_view_handler::update_refresh_color()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        const maui::graphics::paint* const paint = virtual_view()->refresh_color();
        if (paint != nullptr)
        {
            platform->has_refresh_color = true;
            platform->refresh_color_argb = paint->background_color().to_uint();
        }
        else
        {
            platform->has_refresh_color = false;
            platform->refresh_color_argb = 0;
        }
    }

    void refresh_view_handler::update_is_refresh_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->refresh_enabled = virtual_view()->is_refresh_enabled();
    }

    void refresh_view_handler::request_refresh()
    {
        if (auto* view = virtual_view())
        {
            view->set_is_refreshing(true);
        }
    }

    void refresh_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

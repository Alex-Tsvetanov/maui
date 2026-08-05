// graphics_view_handler — iOS (UIKit) platform recipe: the UIView drawing host twin of the apple
// partial (graphics_host.hpp over coregraphics_canvas) carrying the PlatformTouchGraphicsView touch
// plumbing. ConnectHandler/DisconnectHandler point the host's touch events at the virtual view
// (PlatformTouchGraphicsView.Connect/Disconnect). Translated from GraphicsViewHandler.iOS.cs + the
// touch recipe. Compiled as Objective-C++ with ARC for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "graphics_host.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/i_graphics_view.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }
} // namespace

namespace maui::core
{
    // The teardown that must run whether the handler is DISCONNECTED or merely DESTROYED. The native
    // view outlives the handler in any real app (a superview retains it) and the trampolines it keeps
    // in its associated objects carry RAW handler pointers; nothing calls disconnect_handler() when a
    // handler is destroyed (there is no ~view_handler doing it), so the platform dtor has to run this
    // too or the next native callback dereferences freed memory. Idempotent: disconnect_handler()
    // destroys the platform right after calling it, so both paths run on the same object.
    namespace
    {
        void detach_trampolines(graphics_view_platform& platform)
        {
            maui::platform::ios::drawable_host_set_interaction_target(platform.native, nullptr);
        }
    } // namespace

    graphics_view_platform::~graphics_view_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_drawable_host
            native = nullptr;
        }
    }

    // The portable replay seat (the headless twin; the REAL drawing runs through drawRect).
    void graphics_view_platform::replay(maui::graphics::i_canvas& canvas,
                                        const maui::graphics::rect_f& dirty_rect) const
    {
        if (drawable == nullptr)
        {
            return;
        }
        drawable->draw(canvas, dirty_rect);
    }

    // The generic-IView property pushes (a plain UIView host — the border ios partial's scope).
    void graphics_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void graphics_view_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
    }

    void graphics_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void graphics_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void graphics_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void graphics_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_host(native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void graphics_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void graphics_view_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }

    std::unique_ptr<graphics_view_platform> graphics_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<graphics_view_platform>();
        platform->native = maui::platform::ios::create_drawable_host();
        return platform;
    }

    // PlatformTouchGraphicsView.Connect: point the host's touch plumbing at the virtual view so
    // touchesBegan/Moved/Ended/Cancelled route into send_start/drag/end/cancel_interaction (non-owning
    // borrow — the view owns the handler that owns the host).
    void graphics_view_handler::on_connect_handler(graphics_view_platform& platform)
    {
        maui::platform::ios::drawable_host_set_interaction_target(platform.native, virtual_view());
    }

    // PlatformTouchGraphicsView.Disconnect: clear the borrow before the view goes away.
    void graphics_view_handler::on_disconnect_handler(graphics_view_platform& platform)
    {
        detach_trampolines(platform);
    }

    // C# UpdateDrawable: point the host at VirtualView.Drawable (and redraw).
    void graphics_view_handler::update_drawable()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->drawable = virtual_view() != nullptr ? virtual_view()->drawable() : nullptr;
        platform->invalidations++;
        maui::platform::ios::drawable_host_set_drawable(platform->native, platform->drawable);
    }

    // C# InvalidateDrawable.
    void graphics_view_handler::invalidate_drawable()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->invalidations++;
        maui::platform::ios::drawable_host_invalidate(platform->native);
    }

    void graphics_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_host(platform->native).frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void graphics_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

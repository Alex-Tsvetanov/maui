// swipe_view_handler — Apple (AppKit / macOS) platform recipe: a plain NSView host that holds the swipe
// Content as a subview, plus the swipe state machine driven by the shared cross-platform
// maui::core::swipe_machine. AppKit has NO built-in swipe-to-reveal control (the iOS MauiSwipeView is a
// UIPanGestureRecognizer-driven custom view), so the AppKit reveal is a DOCUMENTED DEVIATION: the host
// is a FLIPPED NSView host (create_flipped_host: isFlipped=YES, top-left origin — like content_page, so
// the content's top-down arrange renders top-down), the content is hosted as on content_page, and the
// machine is driven by the programmatic open/close + the cross-platform synthetic offsets (the same
// begin/swipe/end entry points the headless backend uses; a future NSPanGestureRecognizer can call them
// to add the real drag visual). The generic-IView property pushes mirror content_page_handler.mm.
// Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "flipped_container.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_machine.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/visibility.hpp"
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
    swipe_view_platform::~swipe_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    void swipe_view_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void swipe_view_platform::update_opacity(double value)
    {
        as_host(native).alphaValue = value;
    }

    void swipe_view_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void swipe_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void swipe_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void swipe_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void swipe_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void swipe_view_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void swipe_view_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void swipe_view_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<swipe_view_platform> swipe_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_view_platform>();
        // A flipped (top-left origin) host so the content's top-down arrange renders top-down.
        platform->native = maui::platform::apple::create_flipped_host(); // retained — the void* slot owns one ref
        return platform;
    }

    void swipe_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = as_host(platform->native);

        // C# MauiSwipeView.UpdateContent: clear the prior content, then re-parent the new content's NSView.
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

    void swipe_view_handler::update_transition_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->transition = virtual_view()->transition_mode();
    }

    void swipe_view_handler::update_items()
    {
        // C# MapLeftItems/... are empty; the machine reads the live collections on each swipe.
    }

    void swipe_view_handler::programmatically_open(const swipe_view_open_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::programmatically_open(platform->state, *view, request);
    }

    void swipe_view_handler::reset_swipe(bool /*animated*/)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::reset_swipe(platform->state, *view);
    }

    void swipe_view_handler::begin_swipe(swipe_direction direction)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        swipe_machine::begin_swipe(platform->state, direction);
    }

    void swipe_view_handler::swipe_to(double offset)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::swipe_to(platform->state, *view, offset);
    }

    void swipe_view_handler::end_swipe()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        swipe_machine::end_swipe(platform->state, *view);
    }

    void swipe_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

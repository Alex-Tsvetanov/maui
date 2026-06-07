// content_page_handler — Apple (AppKit / macOS) platform recipe: a plain NSView host that holds the
// single content child as a subview. The real-native twin of the headless partial. Translated from
// ContentViewHandler.iOS.cs (UIKit's ContentView : MauiView → a plain AppKit NSView host — no
// NSViewController, kept minimal): set_content clears the host's subviews and re-parents the content's
// native view (C#'s UpdateContent: ClearSubviews + AddSubview(content.ToPlatform())). The control frames
// the content within the padding via the content's own platform_arrange; the host itself is framed by
// platform_arrange. Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_view_ops.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSView* as_host(void* native)
    {
        return (__bridge NSView*)native;
    }

    // The child's native NSView, via its view-handler's native_view() (nil if the child is unattached or
    // its handler has no native view). native_view() returns the real NSView the pimpl owns — not the
    // pimpl pointer that platform_view() returns. Mirrors layout_handler.mm's native_child helper.
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
    content_page_platform::~content_page_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // host is a plain NSView; is_enabled has no NSView equivalent, so it is left to the base mirror.
    void content_page_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void content_page_platform::update_opacity(double value)
    {
        as_host(native).alphaValue = value;
    }

    void content_page_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_host(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<content_page_platform> content_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<content_page_platform>();
        NSView* const host = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)host; // the void* slot owns one reference
        return platform;
    }

    void content_page_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = as_host(platform->native);

        // C# UpdateContent: clear the old content when reused. Snapshot the subviews (removeFromSuperview
        // mutates the live array) and tear them down without an Obj-C fast-enumeration loop (which
        // clang-tidy's init-variables check misreads as uninitialized).
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

    maui::graphics::size content_page_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // A content view computes its own size through the control (which ports MeasureContent), so the
        // handler reports nothing here.
        return {0, 0};
    }

    void content_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_host(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void content_page_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void content_page_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }
} // namespace maui::core

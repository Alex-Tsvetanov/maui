// content_page_handler — iOS (UIKit) platform recipe: a plain UIView host that holds the single content
// child as a subview. The real-native twin of the headless partial, ported DIRECTLY from
// ContentViewHandler.iOS.cs (the same oracle the AppKit twin in src/platform/apple/
// content_page_handler.mm was adapted from): set_content clears the host's subviews and re-parents the
// content's native view (C#'s UpdateContent: ClearSubviews + platformView.RemoveFromSuperview +
// AddSubview). C#'s ContentView (a MauiView subclass carrying CrossPlatformLayout) collapses to a plain
// UIView because the control frames the content within the padding via the content's own
// platform_arrange (the port's MeasureContent/ArrangeContent live on the control); for the same reason
// C#'s InvalidateAncestorsMeasures / InvalidateMeasure calls have no analog. The host itself is framed
// by platform_arrange. Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIView* as_host(void* native)
    {
        return (__bridge UIView*)native;
    }

    // The child's native UIView, via its view-handler's native_view() (nil if the child is unattached or
    // its handler has no native view). native_view() returns the real UIView the pimpl owns — not the
    // pimpl pointer that platform_view() returns. Mirrors layout_handler.mm's native_child helper.
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
    content_page_platform::~content_page_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // host is a plain UIView; is_enabled has no UIView equivalent, so it is left to the base mirror.
    void content_page_platform::update_visibility(maui::core::visibility value)
    {
        as_host(native).hidden = value != maui::core::visibility::visible;
    }

    void content_page_platform::update_opacity(double value)
    {
        as_host(native).alpha = value;
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
        UIView* const host = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
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
        UIView* const host = as_host(platform->native);

        // C# UpdateContent: clear the old content when reused (ClearSubviews). Snapshot the subviews
        // (removeFromSuperview mutates the live array) and tear them down without an Obj-C
        // fast-enumeration loop (which clang-tidy's init-variables check misreads as uninitialized).
        NSArray<UIView*>* const snapshot = [host.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->hosted_content == nullptr)
        {
            return;
        }
        // C#: platformView.RemoveFromSuperview() before AddSubview (re-parent from any previous host).
        if (UIView* const subview = native_child(*platform->hosted_content))
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
        [as_host(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Background / shadow / clip pushed to the native view's layer via the shared ios_visual_ops helpers
    // (the direct PaintExtensions / ShadowExtensions / WrapperView.SetClip ports). `native` is this
    // struct's UIView handle.
    void content_page_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void content_page_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void content_page_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the page host via the shared
    // ios_semantics_ops helpers (SemanticExtensions.UpdateSemantics / ViewExtensions.
    // UpdateInputTransparent). `native` is this struct's UIView handle.
    void content_page_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void content_page_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }
} // namespace maui::core

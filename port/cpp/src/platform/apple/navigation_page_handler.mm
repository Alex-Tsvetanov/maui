// navigation_page_handler — Apple (AppKit / macOS) platform recipe: a plain NSView container that hosts
// the navigation stack's current (top-most) page as its single subview, swapping that subview on each
// push/pop. The real-native twin of the headless partial. AppKit has NO UINavigationController (iOS's
// host) — translated to a plain NSView container that swaps the current page's native view with NO
// animation: host_current clears the container's subviews and re-parents the new top page's native view
// (mirroring iOS's SetViewControllers, minus the navigation chrome). The cross-platform handler reports
// completion (IStackNavigation.NavigationFinished) synchronously after this returns. Compiled as
// Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_view_ops.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSView* as_container(void* native)
    {
        return (__bridge NSView*)native;
    }

    // The page's native NSView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). native_view() returns the real NSView the pimpl owns — not the
    // pimpl pointer that platform_view() returns. Mirrors layout_handler.mm's native_child helper.
    NSView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }
} // namespace

namespace maui::core
{
    navigation_page_platform::~navigation_page_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // container is a plain NSView; is_enabled has no NSView equivalent, so it is left to the base mirror.
    void navigation_page_platform::update_visibility(maui::core::visibility value)
    {
        as_container(native).hidden = value != maui::core::visibility::visible;
    }

    void navigation_page_platform::update_opacity(double value)
    {
        as_container(native).alphaValue = value;
    }

    void navigation_page_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_container(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<navigation_page_platform> navigation_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<navigation_page_platform>();
        NSView* const container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)container; // the void* slot owns one reference
        return platform;
    }

    void navigation_page_handler::host_current(i_view* top)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const container = as_container(platform->native);

        // Swap the current page: clear the old subview(s), then add the new top page's native view. Snapshot
        // the subviews (removeFromSuperview mutates the live array) and tear them down without an Obj-C
        // fast-enumeration loop (which clang-tidy's init-variables check misreads as uninitialized).
        NSArray<NSView*>* const snapshot = [container.subviews copy];
        [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];

        platform->hosted_page = top;
        if (top == nullptr)
        {
            return;
        }
        if (NSView* const subview = native_child(*top))
        {
            [subview removeFromSuperview];
            [container addSubview:subview];
        }
    }

    maui::graphics::size navigation_page_handler::get_desired_size(double /*width_constraint*/,
                                                                   double /*height_constraint*/) const
    {
        // The navigation page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void navigation_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const container = as_container(platform->native);
        [container setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
        // The current page fills the container (origin at 0,0 in the container's coordinate space).
        if (platform->hosted_page != nullptr)
        {
            if (NSView* const subview = native_child(*platform->hosted_page))
            {
                [subview setFrame:NSMakeRect(0, 0, frame.width, frame.height)];
            }
        }
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void navigation_page_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void navigation_page_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }
} // namespace maui::core

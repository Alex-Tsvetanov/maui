// flyout_page_handler — Apple (AppKit / macOS) platform recipe: a real NSSplitViewController with a
// SIDEBAR NSSplitViewItem hosting the flyout pane and a content item hosting the detail pane (the
// W1-10 task's NSSplitViewController choice). The real-native twin of the headless partial:
//   - set_panes rebuilds the two split view items — each pane's native NSView becomes a wrapper
//     NSViewController's view (sidebarWithViewController for the flyout, the plain item for the
//     detail);
//   - update_presentation drives the sidebar item's `collapsed` from IsPresented and pins it
//     (canCollapse = NO) while the computed FlyoutBehavior is Locked (split mode);
//   - DEVIATIONS (documented): one-way virtual→native — a user drag-collapse of the sidebar is not
//     observed back into IsPresented (no displayMode chrome on AppKit; the i_flyout_view
//     set_flyout_is_presented seam stands ready for a future observer); IsGestureEnabled stays a
//     mirror (no swipe gesture on macOS).
// Compiled as Objective-C++ with ARC only for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/flyout_behavior.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/core/i_flyout_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSSplitViewController* as_controller(void* handle)
    {
        return (__bridge NSSplitViewController*)handle;
    }

    // The pane's native NSView, via its view-handler's native_view() (nil if the pane is unattached or
    // its handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    NSView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }

    // A wrapper NSViewController whose view is the pane's native NSView (or an empty NSView for an
    // unattached / unset pane — NSSplitViewItem requires a view controller).
    NSViewController* make_pane_controller(maui::core::i_view* pane)
    {
        NSViewController* const wrapper = [[NSViewController alloc] init];
        NSView* pane_view = nil;
        if (pane != nullptr)
        {
            pane_view = native_child(*pane);
        }
        wrapper.view = pane_view != nil ? pane_view : [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        return wrapper;
    }
} // namespace

namespace maui::core
{
    flyout_page_platform::~flyout_page_platform()
    {
        // Release the retained AppKit handles (each balances a __bridge_retained below).
        if (flyout_host != nullptr)
        {
            CFRelease(flyout_host);
            flyout_host = nullptr;
        }
        if (detail_host != nullptr)
        {
            CFRelease(detail_host);
            detail_host = nullptr;
        }
        if (controller != nullptr)
        {
            CFRelease(controller);
            controller = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native);
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    // The host is the controller's plain NSView; is_enabled keeps the base mirror.
    void flyout_page_platform::update_visibility(maui::core::visibility value)
    {
        as_controller(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void flyout_page_platform::update_opacity(double value)
    {
        as_controller(controller).view.alphaValue = value;
    }

    void flyout_page_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_controller(controller).view.accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<flyout_page_platform> flyout_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<flyout_page_platform>();
        NSSplitViewController* const split = [[NSSplitViewController alloc] init];
        platform->controller = (__bridge_retained void*)split;  // the slot owns one reference
        platform->native = (__bridge_retained void*)split.view; // forces the view to load; owns one ref
        return platform;
    }

    void flyout_page_handler::set_panes(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        const auto* flyout = dynamic_cast<i_flyout_view*>(&view);
        if (flyout == nullptr)
        {
            return;
        }
        platform->hosted_flyout = flyout->flyout_view();
        platform->hosted_detail = flyout->flyout_detail();

        // Rebuild the two split view items: the flyout as the SIDEBAR item, the detail as the content.
        NSViewController* const flyout_controller = make_pane_controller(platform->hosted_flyout);
        NSViewController* const detail_controller = make_pane_controller(platform->hosted_detail);
        NSSplitViewItem* const sidebar = [NSSplitViewItem sidebarWithViewController:flyout_controller];
        NSSplitViewItem* const content = [NSSplitViewItem splitViewItemWithViewController:detail_controller];

        NSSplitViewController* const split = as_controller(platform->controller);
        split.splitViewItems = @[ sidebar, content ];

        // Re-point the retained wrapper slots at the new pane controllers.
        if (platform->flyout_host != nullptr)
        {
            CFRelease(platform->flyout_host);
        }
        platform->flyout_host = (__bridge_retained void*)flyout_controller;
        if (platform->detail_host != nullptr)
        {
            CFRelease(platform->detail_host);
        }
        platform->detail_host = (__bridge_retained void*)detail_controller;
    }

    void flyout_page_handler::update_presentation(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        const auto* flyout = dynamic_cast<i_flyout_view*>(&view);
        if (flyout == nullptr)
        {
            return;
        }
        platform->presented = flyout->flyout_is_presented();
        platform->behavior = flyout->flyout_behavior_value();

        NSSplitViewController* const split = as_controller(platform->controller);
        if (split.splitViewItems.count == 0)
        {
            return; // no panes hosted yet — set_panes realizes the presentation afterwards
        }
        NSSplitViewItem* const sidebar = split.splitViewItems.firstObject;
        // Locked (split mode) pins the flyout beside the detail; otherwise IsPresented collapses it.
        sidebar.canCollapse = platform->behavior != flyout_behavior::locked ? YES : NO;
        sidebar.collapsed = platform->presented || platform->behavior == flyout_behavior::locked ? NO : YES;
    }

    maui::graphics::size flyout_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The flyout page sizes from its panes, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void flyout_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = (__bridge NSView*)platform->native;
        [host setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

// flyout_page_handler — iOS (UIKit) platform recipe: a real double-column UISplitViewController whose
// PRIMARY column hosts the flyout pane and SECONDARY column the detail (the W1-10 task's
// UISplitViewController choice; the C# FlyoutViewHandler.iOS maps are empty stubs — the compat
// PhoneFlyoutPageRenderer owns the chrome there — so the renderer's intent is ported onto the handler
// seam): each pane's native UIView is wrapped in a child UIViewController set as the column's view
// controller (the child-VC composition the task asserts).
//
// Presentation: IsPresented → preferredDisplayMode (oneBesideSecondary when presented, secondaryOnly
// when hidden); the computed Locked behavior (split mode) pins the flyout beside the detail
// (oneBesideSecondary regardless + tile split behavior); IsGestureEnabled → presentsWithGesture.
//
// DEVIATION (documented): the native→virtual presented sync (the displayModeButton / show-hide
// delegate callbacks) needs a live UIWindow + transition coordinator, which the bundle-less simulator
// test process cannot host (the navigation_page_handler.mm precedent) — the i_flyout_view
// set_flyout_is_presented seam is in place and exercised directly by the tests.
//
// Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

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
    UISplitViewController* as_controller(void* handle)
    {
        return (__bridge UISplitViewController*)handle;
    }

    // The pane's native UIView, via its view-handler's native_view() (nil if the pane is unattached or
    // its handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    UIView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // A wrapper UIViewController whose view is the pane's native UIView (or an empty UIView for an
    // unattached / unset pane — a split column needs a view controller).
    UIViewController* make_pane_controller(maui::core::i_view* pane)
    {
        UIViewController* const wrapper = [[UIViewController alloc] init];
        UIView* pane_view = nil;
        if (pane != nullptr)
        {
            pane_view = native_child(*pane);
        }
        wrapper.view = pane_view != nil ? pane_view : [[UIView alloc] initWithFrame:CGRectZero];
        return wrapper;
    }
} // namespace

namespace maui::core
{
    flyout_page_platform::~flyout_page_platform()
    {
        // Release the retained UIKit handles (each balances a __bridge_retained below).
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
    // The host is the controller's plain UIView; is_enabled keeps the base mirror.
    void flyout_page_platform::update_visibility(maui::core::visibility value)
    {
        as_controller(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void flyout_page_platform::update_opacity(double value)
    {
        as_controller(controller).view.alpha = value;
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
        UISplitViewController* const split =
            [[UISplitViewController alloc] initWithStyle:UISplitViewControllerStyleDoubleColumn];
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

        // Rebuild the two columns: the flyout as the PRIMARY column, the detail as the SECONDARY.
        UIViewController* const flyout_controller = make_pane_controller(platform->hosted_flyout);
        UIViewController* const detail_controller = make_pane_controller(platform->hosted_detail);

        UISplitViewController* const split = as_controller(platform->controller);
        [split setViewController:flyout_controller forColumn:UISplitViewControllerColumnPrimary];
        [split setViewController:detail_controller forColumn:UISplitViewControllerColumnSecondary];

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
        platform->gesture_enabled = flyout->flyout_is_gesture_enabled();

        UISplitViewController* const split = as_controller(platform->controller);
        // Locked (split mode) pins the flyout beside the detail (tiled, always visible); otherwise
        // IsPresented shows it beside / hides it behind the detail.
        if (platform->behavior == flyout_behavior::locked)
        {
            split.preferredSplitBehavior = UISplitViewControllerSplitBehaviorTile;
            split.preferredDisplayMode = UISplitViewControllerDisplayModeOneBesideSecondary;
        }
        else
        {
            split.preferredSplitBehavior = UISplitViewControllerSplitBehaviorAutomatic;
            split.preferredDisplayMode = platform->presented ? UISplitViewControllerDisplayModeOneBesideSecondary
                                                             : UISplitViewControllerDisplayModeSecondaryOnly;
        }
        split.presentsWithGesture = platform->gesture_enabled ? YES : NO;
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
        UIView* const host = as_controller(platform->controller).view;
        [host setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

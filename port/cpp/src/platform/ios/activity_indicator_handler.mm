// activity_indicator_handler — iOS (UIKit) platform recipe. The managed platform view is a
// UIActivityIndicatorView (held, retained, in activity_indicator_platform::native): IsRunning maps to
// startAnimating/stopAnimating with the UpdateIsRunning visibility coupling, Color to the view's
// color. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from ActivityIndicatorHandler.iOS.cs + Platform/iOS/ActivityIndicatorExtensions.cs:
// CreatePlatformView = the Medium style (the port's floor is far above iOS 13); UpdateIsRunning's
// IsRunning && Visible coupling (the Collapse() constraint dance is the shared deferral — Hidden and
// Collapsed both hide, see button_handler's note); UpdateColor (the nullable collapse — non-nullable
// color, the button convention). The C# MauiActivityIndicator SUBCLASS only re-runs UpdateIsRunning
// from LayoutSubviews/Draw — a lifecycle re-application workaround, not mapping behavior; the plain
// UIActivityIndicatorView carries the recipe here (documented simplification).

#import <UIKit/UIKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIActivityIndicatorView* as_spinner(void* native)
    {
        return (__bridge UIActivityIndicatorView*)native;
    }

    using maui::platform::ios::to_ui_color;
} // namespace

namespace maui::core
{
    activity_indicator_platform::~activity_indicator_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    // NOTE: update_visibility is normally shadowed by the mapper's Visibility → map_is_running
    // override; it still pushes faithfully if invoked directly.
    void activity_indicator_platform::update_visibility(maui::core::visibility value)
    {
        as_spinner(native).hidden = value != maui::core::visibility::visible;
    }

    void activity_indicator_platform::update_opacity(double value)
    {
        as_spinner(native).alpha = value;
    }

    void activity_indicator_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_spinner(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        // ActivityIndicatorHandler.iOS.CreatePlatformView: the Medium style (the >= 13 branch).
        UIActivityIndicatorView* const native =
            [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        // ActivityIndicatorExtensions.UpdateIsRunning: animate only while IsRunning && Visible; the
        // visibility half is handled here too (the mapper's Visibility key routes here).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIActivityIndicatorView* const native = as_spinner(platform->native);
        const bool visible = view.visibility() == visibility::visible;
        if (view.is_running() && visible)
        {
            native.hidden = NO;
            [native startAnimating];
        }
        else
        {
            if (native.isAnimating)
            {
                [native stopAnimating];
            }
            native.hidden = !visible;
        }
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // ActivityIndicatorExtensions.UpdateColor (the nullable collapse — see the header).
            as_spinner(platform->native).color = to_ui_color(view.color());
        }
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double width_constraint,
                                                                      double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_spinner(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_spinner(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

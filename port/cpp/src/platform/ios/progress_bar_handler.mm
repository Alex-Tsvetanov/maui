// progress_bar_handler — iOS (UIKit) platform recipe. The managed platform view is a UIProgressView
// (held, retained, in progress_bar_platform::native): Progress maps to the float progress fraction and
// ProgressColor to progressTintColor. Display-only — no inbound events. Compiled as Objective-C++ with
// ARC only for the `ios` backend.
//
// Ported DIRECTLY from ProgressBarHandler.iOS.cs + Platform/iOS/ProgressBarExtensions.cs:
// CreatePlatformView = new UIProgressView(UIProgressViewStyle.Default); UpdateProgress /
// UpdateProgressColor as the map_* bodies (the nullable ProgressColor collapses — non-nullable color,
// the button convention). Not ported (deferred, documented in progress_bar_handler.hpp): the
// FlowDirection mapper override (the iOS-26 RTL subview SemanticContentAttribute walk).
// UIProgressView is not a UIControl: is_enabled keeps the base mirror.

#import <UIKit/UIKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIProgressView* as_bar(void* native)
    {
        return (__bridge UIProgressView*)native;
    }

    using maui::platform::ios::to_ui_color;
} // namespace

namespace maui::core
{
    progress_bar_platform::~progress_bar_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void progress_bar_platform::update_visibility(maui::core::visibility value)
    {
        as_bar(native).hidden = value != maui::core::visibility::visible;
    }

    void progress_bar_platform::update_opacity(double value)
    {
        as_bar(native).alpha = value;
    }

    void progress_bar_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_bar(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<progress_bar_platform>();
        UIProgressView* const native = [[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleDefault];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // ProgressBarExtensions.UpdateProgress: Progress = (float)progress.Progress.
            as_bar(platform->native).progress = static_cast<float>(view.progress());
        }
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // ProgressBarExtensions.UpdateProgressColor (the nullable collapse — see the header).
            as_bar(platform->native).progressTintColor = to_ui_color(view.progress_color());
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_bar(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_bar(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

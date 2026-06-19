// progress_bar_handler — iOS (UIKit) platform recipe. The managed platform view is a UIProgressView
// (held, retained, in progress_bar_platform::native): Progress maps to the float progress fraction and
// ProgressColor to progressTintColor. Display-only — no inbound events. Compiled as Objective-C++ with
// ARC only for the `ios` backend.
//
// Ported DIRECTLY from ProgressBarHandler.iOS.cs + Platform/iOS/ProgressBarExtensions.cs:
// CreatePlatformView = new UIProgressView(UIProgressViewStyle.Default); UpdateProgress /
// UpdateProgressColor as the map_* bodies (the nullable ProgressColor collapses — non-nullable color,
// the button convention). The FlowDirection mapper override IS ported (map_flow_direction): the
// resolved direction (MatchParent → parent-IView fallback) sets the bar's UISemanticContentAttribute +
// is re-applied to each subview (the iOS-26 RTL subview walk). UIProgressView is not a UIControl:
// is_enabled keeps the base mirror.

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

    // ProgressBarHandler.GetSemanticContentAttribute's enum → UISemanticContentAttribute mapping
    // (Force{Left,Right}ToLeft; an unresolved MatchParent stays Unspecified — the C# Unspecified case
    // when the parent has no explicit direction).
    UISemanticContentAttribute to_semantic_content(maui::core::flow_direction fd)
    {
        switch (fd)
        {
            case maui::core::flow_direction::left_to_right:
                return UISemanticContentAttributeForceLeftToRight;
            case maui::core::flow_direction::right_to_left:
                return UISemanticContentAttributeForceRightToLeft;
            case maui::core::flow_direction::match_parent:
                return UISemanticContentAttributeUnspecified;
        }
        return UISemanticContentAttributeUnspecified;
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
            // ProgressBarExtensions.UpdateProgressColor: an unset (default-constructed) ProgressColor must
            // leave the UIProgressView's native default tint (the system blue), NOT a transparent fill —
            // to_ui_color(unset) is a clear color that hides the progress fill. nil restores the default
            // (C# calls ResetProgressTintColor in the null branch).
            const maui::graphics::color color = view.progress_color();
            as_bar(platform->native).progressTintColor = color != maui::graphics::color{} ? to_ui_color(color) : nil;
        }
    }

    void progress_bar_handler::map_flow_direction(progress_bar_handler& handler, i_progress& view)
    {
        // ProgressBarHandler.MapFlowDirection: set the bar's UISemanticContentAttribute from the RESOLVED
        // direction (the MatchParent → parent-IView fallback), then re-apply it to each internal subview —
        // the iOS-26 workaround (UIProgressView stopped propagating the attribute to its subviews). The
        // port's floor is recent iOS, so the subview walk runs unconditionally (it is a harmless no-op on
        // OSes that still propagate). The resolved direction is mirrored for the headless-parity oracle.
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved;
        const UISemanticContentAttribute attribute = to_semantic_content(resolved);
        UIProgressView* const bar = as_bar(platform->native);
        bar.semanticContentAttribute = attribute;
        for (UIView* subview in bar.subviews)
        {
            subview.semanticContentAttribute = attribute;
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

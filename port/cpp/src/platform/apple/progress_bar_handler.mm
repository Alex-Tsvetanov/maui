// progress_bar_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is a
// DETERMINATE bar-style NSProgressIndicator (held, retained, in progress_bar_platform::native):
// Progress maps to doubleValue over a fixed [0, 1] range (UIProgressView.Progress is itself a 0-1
// fraction). Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from ProgressBarHandler.iOS.cs + ProgressBarExtensions.cs (UIKit — MAUI's macOS is Mac
// Catalyst). AppKit DEVIATIONS (documented, not silent):
//  - NSProgressIndicator exposes NO public fill-color API (UIProgressView.ProgressTintColor has no
//    AppKit analog), so map_progress_color records the cross-platform mirror only.
//  - The iOS FlowDirection mapper override is deferred with the handler (see progress_bar_handler.hpp).
// An NSProgressIndicator is an NSView (not NSControl): is_enabled keeps the base mirror.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSProgressIndicator* as_bar(void* native)
    {
        return (__bridge NSProgressIndicator*)native;
    }
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
        as_bar(native).alphaValue = value;
    }

    void progress_bar_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_bar(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void progress_bar_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void progress_bar_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void progress_bar_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void progress_bar_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void progress_bar_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void progress_bar_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void progress_bar_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<progress_bar_platform>();
        // The determinate bar over a fixed 0-1 range — the NSProgressIndicator shape of
        // `new UIProgressView(UIProgressViewStyle.Default)`.
        NSProgressIndicator* const native = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.style = NSProgressIndicatorStyleBar;
        native.indeterminate = NO;
        native.minValue = 0;
        native.maxValue = 1;
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // ProgressBarExtensions.UpdateProgress: the 0-1 fraction lands directly on the bar.
            as_bar(platform->native).doubleValue = view.progress();
        }
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        // AppKit deviation: no public fill-color API — record the mirror (see header note).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->progress_color = view.progress_color();
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_bar(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_bar(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

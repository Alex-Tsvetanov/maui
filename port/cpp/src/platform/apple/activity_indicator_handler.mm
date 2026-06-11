// activity_indicator_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is a
// SPINNING-style NSProgressIndicator (held, retained, in activity_indicator_platform::native):
// IsRunning maps to startAnimation/stopAnimation with the UpdateIsRunning visibility coupling.
// Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from ActivityIndicatorHandler.iOS.cs + ActivityIndicatorExtensions.cs (UIKit — MAUI's
// macOS is Mac Catalyst; the C# MauiActivityIndicator subclass only re-runs UpdateIsRunning from
// LayoutSubviews/Draw, a UIKit lifecycle workaround AppKit does not need). AppKit DEVIATIONS
// (documented, not silent):
//  - NSProgressIndicator exposes NO public spinner color API (UIActivityIndicatorView.Color has no
//    AppKit analog), so map_color records the cross-platform mirror only.
//  - NSProgressIndicator has no isAnimating getter, so the cross-platform `is_running` mirror is kept
//    in sync as the observable animation state (the MauiButtonCell observable-state convention).
//    isDisplayedWhenStopped=NO supplies UIKit's HidesWhenStopped behavior.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSProgressIndicator* as_spinner(void* native)
    {
        return (__bridge NSProgressIndicator*)native;
    }
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
        as_spinner(native).alphaValue = value;
    }

    void activity_indicator_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_spinner(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void activity_indicator_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void activity_indicator_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void activity_indicator_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void activity_indicator_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void activity_indicator_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void activity_indicator_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void activity_indicator_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        // The indeterminate spinner — the NSProgressIndicator shape of UIActivityIndicatorView
        // (Medium style); displayedWhenStopped=NO mirrors HidesWhenStopped.
        NSProgressIndicator* const native = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.style = NSProgressIndicatorStyleSpinning;
        native.indeterminate = YES;
        native.displayedWhenStopped = NO;
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        // ActivityIndicatorExtensions.UpdateIsRunning: animate only while IsRunning && Visible; the
        // visibility half is handled here too (the mapper's Visibility key routes here). The
        // `is_running` mirror is the observable animation state (no NSProgressIndicator getter).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSProgressIndicator* const native = as_spinner(platform->native);
        const bool visible = view.visibility() == visibility::visible;
        const bool running = view.is_running() && visible;
        if (running)
        {
            native.hidden = NO;
            [native startAnimation:nil];
        }
        else
        {
            [native stopAnimation:nil];
            native.hidden = !visible;
        }
        platform->is_running = running;
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        // AppKit deviation: no public spinner color API — record the mirror (see header note).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->color = view.color();
        }
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double /*width_constraint*/,
                                                                      double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_spinner(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_spinner(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

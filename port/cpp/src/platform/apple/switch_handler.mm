// switch_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an NSSwitch
// (held, retained, in switch_platform::native), IsOn maps to NSSwitch.state, and the native toggle
// flows back through a target-action trampoline to i_switch::set_is_on. Compiled as Objective-C++
// with ARC only for the `apple` backend.
//
// Translated from SwitchHandler.iOS.cs + SwitchExtensions.cs (UIKit/UISwitch — MAUI's macOS support is
// Mac Catalyst, so there is no AppKit SwitchHandler to port verbatim): NSSwitch's action fires on a
// state change, matching UISwitch.ValueChanged. AppKit DEVIATIONS (documented, not silent):
//  - NSSwitch exposes NO public track/thumb tint API (UISwitch's OnTintColor / ThumbTintColor /
//    track-subview walk have no AppKit analog), so map_track_color / map_thumb_color record the
//    cross-platform mirrors only — observable native-adjacent state, the MauiButtonCell convention.
//  - The UIKit-26 foreground/trait-change observers (color re-application timing workarounds) are not
//    ported. NeedsContainer IS ported: on_setup_container / on_remove_container wrap the NSSwitch in a
//    plain NSView container (the WrapperView analog), driven by the shared view_mapper's container map.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards NSSwitch's target-action (fired on a state change) to the C++ handler's
// virtual view — the SwitchProxy.OnControlValueChanged port (write back only when the value differs).
@interface MauiSwitchTarget : NSObject
@property(nonatomic) maui::core::switch_handler* handler;
- (void)onValueChanged:(id)sender;
@end

@implementation MauiSwitchTarget
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    auto* const native = (NSSwitch*)sender;
    if (view != nullptr && native != nil)
    {
        const bool native_on = native.state == NSControlStateValueOn;
        if (view->is_on() != native_on)
        {
            view->set_is_on(native_on);
        }
    }
}
@end

namespace
{
    // Key for the associated MauiSwitchTarget kept alive by the NSSwitch (its `target` is weak).
    const char k_target_key = 0;

    NSSwitch* as_switch(void* native)
    {
        return (__bridge NSSwitch*)native;
    }
} // namespace

namespace maui::core
{
    switch_platform::~switch_platform()
    {
        if (container != nullptr)
        {
            CFRelease(container); // balances the __bridge_retained in on_setup_container
            container = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void switch_platform::update_visibility(maui::core::visibility value)
    {
        as_switch(native).hidden = value != maui::core::visibility::visible;
    }

    void switch_platform::update_opacity(double value)
    {
        as_switch(native).alphaValue = value;
    }

    void switch_platform::update_is_enabled(bool value)
    {
        [as_switch(native) setEnabled:static_cast<BOOL>(value)];
    }

    void switch_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_switch(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void switch_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void switch_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void switch_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void switch_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void switch_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void switch_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void switch_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        NSSwitch* const native = [[NSSwitch alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        NSSwitch* const native = as_switch(platform.native);
        MauiSwitchTarget* const target = [[MauiSwitchTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onValueChanged:);
        // ...so keep it alive for the switch's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        NSSwitch* const native = as_switch(platform.native);
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    // C# ViewHandler.SetupContainer (the WrapperView swap): wrap the natural-sized NSSwitch in a plain
    // NSView container so the switch never has to grow (the >101pt accessibility bug) while chrome
    // (background colors, etc.) can still size the container. Re-pointing PlatformView's superview to the
    // wrapper matches the C# `RemoveFromSuperview` → `ContainerView.AddSubview(PlatformView)` dance; the
    // wrapper here is freshly minted (no prior superview on a just-connected handler), so it just adopts
    // the switch and becomes the handler's container_view.
    void switch_handler::on_setup_container()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || platform->container != nullptr)
        {
            return; // C# guard: PlatformView == null || ContainerView != null
        }
        NSSwitch* const native = as_switch(platform->native);
        NSView* const wrapper = [[NSView alloc] initWithFrame:native.bounds];
        [native removeFromSuperview];
        [wrapper addSubview:native];
        platform->container = (__bridge_retained void*)wrapper; // the handler owns one reference
        set_container_view(platform->container);
    }

    // C# ViewHandler.RemoveContainer: unwrap the switch (drop the wrapper, restore the bare PlatformView)
    // and clear container_view. The wrapper has no parent yet in the port (it was never inserted into a
    // superview after setup), so this releases it and re-isolates the switch.
    void switch_handler::on_remove_container()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (platform->container != nullptr)
        {
            NSView* const native = (__bridge NSView*)platform->native;
            [native removeFromSuperview]; // detach from the wrapper before it is released
            CFRelease(platform->container);
            platform->container = nullptr;
        }
        set_container_view(nullptr);
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        // C# MapIsOn: UpdateIsOn(handler) re-runs the TrackColor mapper, then SetState pushes the value.
        handler.update_value("track_color");
        if (auto* platform = handler.typed_platform_view())
        {
            as_switch(platform->native).state = view.is_on() ? NSControlStateValueOn : NSControlStateValueOff;
        }
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        // AppKit deviation: NSSwitch has no public track tint API — record the mirror (see header note).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->track_color = view.track_color();
        }
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        // AppKit deviation: NSSwitch has no public thumb tint API — record the mirror (see header note).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->thumb_color = view.thumb_color();
        }
    }

    maui::graphics::size switch_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_switch(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // NeedsContainer: the layout positions the CONTAINER (the wrapper that native_child handed it), not
        // the bare NSSwitch — so frame the wrapper to the arranged rect and let the NSSwitch fill it. Without
        // this the wrapper kept its setup-time frame and the toggle was mispositioned off-screen.
        if (platform->container != nullptr)
        {
            [(__bridge NSView*)platform->container setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
            [as_switch(platform->native) setFrame:NSMakeRect(0, 0, frame.width, frame.height)];
            return;
        }
        [as_switch(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

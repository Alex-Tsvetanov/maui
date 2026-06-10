// switch_handler — iOS (UIKit) platform recipe. The managed platform view is a UISwitch (held,
// retained, in switch_platform::native); IsOn maps through SetState(animated), the track/thumb colors
// through the SwitchExtensions recipe, and the native toggle flows back through a target-action proxy
// to i_switch::set_is_on. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from SwitchHandler.iOS.cs + Platform/iOS/SwitchExtensions.cs: CreatePlatformView =
// UISwitch(Empty); SwitchProxy's ValueChanged → IsOn write-back (guarded against echo);
// UpdateIsOn/UpdateTrackColor (the track-subview walk incl. the iOS-13 SecondarySystemFill fallback) /
// UpdateThumbColor as the map_* bodies. Not ported (deferred, documented): NeedsContainer (the
// UISwitch >101pt accessibility workaround needs the container infrastructure no backend has yet) and
// the iOS/Catalyst-26 foreground/trait-change observers (color re-application timing workarounds).
// Color collapse: the port's color is non-nullable, so C#'s null-color branches (restore the platform
// default) collapse — the off-track fallback keeps the SecondarySystemFill push for the DEFAULT color.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards UISwitch's ValueChanged target-action to the C++ handler's virtual view —
// the SwitchProxy.OnControlValueChanged port (write back only when the value differs).
@interface MauiSwitchEventProxy : NSObject
@property(nonatomic) maui::core::switch_handler* handler;
- (void)onValueChanged:(id)sender;
@end

@implementation MauiSwitchEventProxy
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    UISwitch* const native = (UISwitch*)sender;
    if (view != nullptr && native != nil && view->is_on() != static_cast<bool>(native.on))
    {
        view->set_is_on(native.on);
    }
}
@end

namespace
{
    // Key for the associated MauiSwitchEventProxy kept alive by the UISwitch (UIControl does not
    // retain its targets — the target-action convention).
    const char k_proxy_key = 0;

    UISwitch* as_switch(void* native)
    {
        return (__bridge UISwitch*)native;
    }

    // SwitchExtensions.GetTrackSubview (iOS 13+ branch — the port's floor is far above 13): the track
    // is the first subview's first subview. Nil-safe at every hop, like the C# `?.` chain.
    UIView* track_subview(UISwitch* native)
    {
        return native.subviews.firstObject.subviews.firstObject;
    }

    using maui::platform::ios::to_ui_color;
} // namespace

namespace maui::core
{
    switch_platform::~switch_platform()
    {
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
        as_switch(native).alpha = value;
    }

    void switch_platform::update_is_enabled(bool value)
    {
        as_switch(native).enabled = static_cast<BOOL>(value);
    }

    void switch_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_switch(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        UISwitch* const native = [[UISwitch alloc] initWithFrame:CGRectZero];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        UISwitch* const native = as_switch(platform.native);
        MauiSwitchEventProxy* const proxy = [[MauiSwitchEventProxy alloc] init];
        proxy.handler = this;
        // SwitchProxy.Connect — the ValueChanged wiring. UIControl holds its targets weakly, so the
        // proxy is kept alive for the switch's lifetime via an associated object (the button pattern).
        [native addTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        objc_setAssociatedObject(native, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        UISwitch* const native = as_switch(platform.native);
        if (auto* const proxy = (MauiSwitchEventProxy*)objc_getAssociatedObject(native, &k_proxy_key))
        {
            [native removeTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        }
        objc_setAssociatedObject(native, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        // C# MapIsOn: UpdateIsOn(handler) re-runs the TrackColor mapper, then
        // SwitchExtensions.UpdateIsOn: SetState(view.IsOn, animated: true).
        handler.update_value("track_color");
        if (auto* platform = handler.typed_platform_view())
        {
            [as_switch(platform->native) setOn:view.is_on() animated:YES];
        }
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UISwitch* const native = as_switch(platform->native);
        // SwitchExtensions.UpdateTrackColor: bail when the track subview cannot be found.
        UIView* const track = track_subview(native);
        if (track == nil)
        {
            return;
        }
        UIColor* const track_color = to_ui_color(view.track_color());
        if (view.is_on())
        {
            // The ON branch drives both the OnTintColor and the live track subview.
            native.onTintColor = track_color;
            track.backgroundColor = track_color;
        }
        else if (view.track_color() == maui::graphics::color{})
        {
            // The DEFAULT (collapsed-null) color keeps C#'s off-state fallback: SecondarySystemFill
            // (the Light/Dark-aware equivalent of the pre-13 RGBA 120,120,128,40).
            track.backgroundColor = UIColor.secondarySystemFillColor;
        }
        else
        {
            track.backgroundColor = track_color;
        }
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        // SwitchExtensions.UpdateThumbColor (the null guard collapses — non-nullable color).
        if (auto* platform = handler.typed_platform_view())
        {
            as_switch(platform->native).thumbTintColor = to_ui_color(view.thumb_color());
        }
    }

    maui::graphics::size switch_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_switch(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_switch(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

// switch_handler — iOS (UIKit) platform recipe. The managed platform view is a UISwitch (held,
// retained, in switch_platform::native); IsOn maps through SetState(animated), the track/thumb colors
// through the SwitchExtensions recipe, and the native toggle flows back through a target-action proxy
// to i_switch::set_is_on. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from SwitchHandler.iOS.cs + Platform/iOS/SwitchExtensions.cs: CreatePlatformView =
// UISwitch(Empty); SwitchProxy's ValueChanged → IsOn write-back (guarded against echo);
// UpdateIsOn/UpdateTrackColor (the track-subview walk incl. the iOS-13 SecondarySystemFill fallback) /
// UpdateThumbColor as the map_* bodies. NeedsContainer IS ported: on_setup_container /
// on_remove_container wrap the UISwitch in a plain UIView container (the >101pt accessibility
// workaround), driven by the shared view_mapper's container map. SwitchProxy's color-re-application
// observers ARE ported (the UIKit-26 theme-reset workarounds): WillEnterForeground re-applies the OFF
// track color and the iOS-26 trait-change registration re-applies the thumb color — both async on the
// main queue with the empirically-required 10ms settle, and both torn down in on_disconnect_handler
// AND the dtor (no dangling observer → no UAF). The MACCATALYST NSWindowDidBecomeKey branch is not
// ported (no macOS backend here yet).
// Color collapse: the port's color is non-nullable, so C#'s null-color branches (restore the platform
// default) collapse — the off-track fallback keeps the SecondarySystemFill push for the DEFAULT color.
// "A custom color is set" (C#'s `is not null`) maps to `!= maui::graphics::color{}` (the default-black
// sentinel the track-color recipe already treats as "no custom color").

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
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
    auto* const native = (UISwitch*)sender;
    if (view != nullptr && native != nil && view->is_on() != static_cast<bool>(native.on))
    {
        view->set_is_on(native.on);
    }
}
@end

// MauiIosSwitch — the UISwitch the handler presents. Its layoutSubviews override re-sizes any gradient/
// image background sublayer apply_background installed (a solid BackgroundColor needs no resize — it is the
// backing layer's backgroundColor), so a Background brush fills the band behind the switch and tracks bounds.
// apply_background runs before arrange, when bounds is zero, so the hook is needed for gradients.
@interface MauiIosSwitch : UISwitch
@end

@implementation MauiIosSwitch
- (void)layoutSubviews
{
    [super layoutSubviews];
    maui::platform::ios::resize_background_layers((__bridge void*)self);
    // Re-frame the clip mask to the new bounds (WrapperView.LayoutSubviews re-runs SetClip): a
    // UIKit-driven resize / rotation that bypasses the handler would otherwise leave the mask sized
    // to the old (or 0×0 map-time) bounds. No-op when no clip is set.
    maui::platform::ios::reapply_clip((__bridge void*)self);
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

    // The empirically-required settle (SwitchProxy's `await Task.Delay(10)`): a 10ms delay on the main
    // queue lets UIKit finish its internal layout/styling before the custom color is re-applied.
    constexpr int64_t k_color_reapply_delay_ms = 10;

    // SwitchProxy.UpdateTrackOffColor: re-apply the OFF track color after a UIKit lifecycle reset.
    // Dispatched onto the main queue; bails unless the switch is still OFF and a custom color is set
    // (C#'s `!platformView.On` + `view.TrackColor is not null` → the default-black sentinel here).
    //
    // `alive` is the handler's reapply_alive flag (a copy of the shared_ptr, so the flag — never the
    // handler — is kept alive by the block). SwitchProxy uses WeakReferences for the same purpose; the
    // port checks `*alive` before dereferencing the raw `handler`, so a block still in flight when the
    // handler is torn down bails instead of touching freed memory (UAF).
    void reapply_track_off_color(maui::core::switch_handler* handler, std::shared_ptr<std::atomic<bool>> alive)
    {
        // The blocks capture `alive` by copy (automatic-storage local), keeping the flag — not the
        // handler — alive; the by-value param is consumed by those captures.
        dispatch_async(dispatch_get_main_queue(), ^{
          if (!*alive)
          {
              return; // the handler was torn down before this block ran
          }
          auto* const platform = handler->typed_platform_view();
          if (platform == nullptr || platform->native == nullptr)
          {
              return;
          }
          UISwitch* const native = as_switch(platform->native);
          if (native.on)
          {
              return; // C#: only re-applies while OFF
          }
          dispatch_after(dispatch_time(DISPATCH_TIME_NOW, k_color_reapply_delay_ms * NSEC_PER_MSEC),
                         dispatch_get_main_queue(), ^{
                           if (!*alive)
                           {
                               return; // torn down during the 10ms settle (SwitchProxy's weak-ref guard)
                           }
                           auto* const view = handler->virtual_view();
                           auto* const settled = handler->typed_platform_view();
                           if (view == nullptr || settled == nullptr || settled->native == nullptr)
                           {
                               return;
                           }
                           if (view->track_color() != maui::graphics::color{}) // a custom color is set
                           {
                               maui::core::switch_handler::map_track_color(*handler, *view);
                           }
                         });
        });
    }

    // SwitchProxy.UpdateThumbColor: re-apply the custom thumb color after a light/dark theme reset.
    // Same async/settle shape + `*alive` liveness guard; bails unless a custom thumb color is set.
    void reapply_thumb_color(maui::core::switch_handler* handler, std::shared_ptr<std::atomic<bool>> alive)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
          if (!*alive)
          {
              return;
          }
          auto* const platform = handler->typed_platform_view();
          if (handler->virtual_view() == nullptr || platform == nullptr || platform->native == nullptr)
          {
              return;
          }
          dispatch_after(dispatch_time(DISPATCH_TIME_NOW, k_color_reapply_delay_ms * NSEC_PER_MSEC),
                         dispatch_get_main_queue(), ^{
                           if (!*alive)
                           {
                               return; // torn down during the 10ms settle (SwitchProxy's weak-ref guard)
                           }
                           auto* const view = handler->virtual_view();
                           auto* const settled = handler->typed_platform_view();
                           if (view == nullptr || settled == nullptr || settled->native == nullptr)
                           {
                               return;
                           }
                           if (view->thumb_color() != maui::graphics::color{}) // a custom color is set
                           {
                               maui::core::switch_handler::map_thumb_color(*handler, *view);
                           }
                         });
        });
    }
} // namespace

namespace maui::core
{
    switch_platform::~switch_platform()
    {
        // The value trampoline first: the UISwitch outlives the handler whenever a superview retains it,
        // and its proxy carries a raw switch_handler*.
        if (native != nullptr)
        {
            UISwitch* const switch_view = as_switch(native);
            if (auto* const proxy = (MauiSwitchEventProxy*)objc_getAssociatedObject(switch_view, &k_proxy_key))
            {
                [switch_view removeTarget:proxy
                                   action:@selector(onValueChanged:)
                         forControlEvents:UIControlEventValueChanged];
                proxy.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(switch_view, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
        // Tear down SwitchProxy's color-re-application observers BEFORE the native UISwitch is released —
        // a surviving NSNotificationCenter observer or trait-change registration would fire into freed
        // memory (UAF). Mirrors on_disconnect_handler so a never-disconnected handler is still safe.
        // Trip the liveness flag first so any deferred re-apply block already in flight bails out.
        *reapply_alive = false;
        if (foreground_observer != nullptr)
        {
            NSObject* const observer = (__bridge_transfer NSObject*)foreground_observer;
            [[NSNotificationCenter defaultCenter] removeObserver:observer];
            foreground_observer = nullptr;
        }
        if (trait_change_registration != nullptr)
        {
            id<UITraitChangeRegistration> const registration =
                (__bridge_transfer id<UITraitChangeRegistration>)trait_change_registration;
            if (native != nullptr)
            {
                [as_switch(native) unregisterForTraitChanges:registration];
            }
            trait_change_registration = nullptr;
        }
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

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void switch_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void switch_platform::update_background(const maui::graphics::paint* value)
    {
        // BackgroundColor / Background brush fills the band behind the UISwitch (it has no bezel): the shared
        // helper paints a solid color onto the backing layer or installs a gradient/image sublayer
        // (MauiIosSwitch.layoutSubviews keeps it sized to bounds). A null paint clears it.
        maui::platform::ios::apply_background(native, value);
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        UISwitch* const native = [[MauiIosSwitch alloc] initWithFrame:CGRectZero];
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

        // SwitchProxy.Connect (iOS branch): on app return-from-background re-apply the OFF track color.
        // object:nil matches any poster — the real poster is the UIApplication singleton (absent in a
        // simctl-spawned test process), exactly as window_handler observes the lifecycle notifications.
        // The blocks capture a copy of reapply_alive so the deferred body can detect a torn-down handler.
        switch_handler* const self = this;
        const std::shared_ptr<std::atomic<bool>> alive = platform.reapply_alive;
        NSObject* const observer =
            [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillEnterForegroundNotification
                                                              object:nil
                                                               queue:nil
                                                          usingBlock:^(NSNotification*) {
                                                            reapply_track_off_color(self, alive);
                                                          }];
        platform.foreground_observer = (__bridge_retained void*)observer; // the void* slot owns the token

        // SwitchProxy.Connect: iOS 26+ resets ThumbTintColor on a light/dark change. The deployment
        // floor IS 26, so the gate is always true; keep it for fidelity with SwitchHandler.iOS.cs.
        if (@available(iOS 26, *))
        {
            id<UITraitChangeRegistration> const registration =
                [native registerForTraitChanges:@[ UITraitUserInterfaceStyle.class ]
                                    withHandler:^(__kindof id<UITraitEnvironment> /*traitEnvironment*/,
                                                  UITraitCollection* /*previousCollection*/) {
                                      reapply_thumb_color(self, alive);
                                    }];
            platform.trait_change_registration = (__bridge_retained void*)registration; // void* owns it

            // iOS 26+ resets ThumbTintColor after the initial layout, so re-apply the custom color now.
            reapply_thumb_color(this, platform.reapply_alive);
        }
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        UISwitch* const native = as_switch(platform.native);
        if (auto* const proxy = (MauiSwitchEventProxy*)objc_getAssociatedObject(native, &k_proxy_key))
        {
            [native removeTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
            proxy.handler = nullptr; // the back-pointer live_view re-reads after user code
        }
        objc_setAssociatedObject(native, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // SwitchProxy.Disconnect: RemoveObserver + UnregisterForTraitChanges; release both. Trip the
        // liveness flag so any deferred re-apply block already in flight bails, then null the void*
        // slots so the dtor does not double-free (no dangling observer → no UAF).
        *platform.reapply_alive = false;
        if (platform.foreground_observer != nullptr)
        {
            NSObject* const observer = (__bridge_transfer NSObject*)platform.foreground_observer;
            [[NSNotificationCenter defaultCenter] removeObserver:observer];
            platform.foreground_observer = nullptr;
        }
        if (platform.trait_change_registration != nullptr)
        {
            id<UITraitChangeRegistration> const registration =
                (__bridge_transfer id<UITraitChangeRegistration>)platform.trait_change_registration;
            if (native != nil)
            {
                [native unregisterForTraitChanges:registration];
            }
            platform.trait_change_registration = nullptr;
        }
    }

    // C# ViewHandler.SetupContainer (ViewHandlerOfT.iOS WrapperView swap): wrap the natural-sized UISwitch
    // in a plain UIView container — the documented UISwitch >101pt accessibility workaround (the switch
    // stays its natural size; the container carries background/chrome). RemoveFromSuperview →
    // ContainerView.AddSubview(PlatformView): the wrapper is freshly minted (no prior superview on a
    // just-connected handler), so it adopts the switch and becomes the handler's container_view.
    void switch_handler::on_setup_container()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || platform->container != nullptr)
        {
            return; // C# guard: PlatformView == null || ContainerView != null
        }
        UISwitch* const native = as_switch(platform->native);
        UIView* const wrapper = [[UIView alloc] initWithFrame:native.bounds];
        [native removeFromSuperview];
        [wrapper addSubview:native];
        platform->container = (__bridge_retained void*)wrapper; // the handler owns one reference
        set_container_view(platform->container);
    }

    // C# ViewHandler.RemoveContainer: unwrap the switch (drop the wrapper, restore the bare PlatformView)
    // and clear container_view. The wrapper has no parent yet in the port, so this releases it and
    // re-isolates the switch.
    void switch_handler::on_remove_container()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (platform->container != nullptr)
        {
            UIView* const native = (__bridge UIView*)platform->native;
            [native removeFromSuperview]; // detach from the wrapper before it is released
            CFRelease(platform->container);
            platform->container = nullptr;
        }
        set_container_view(nullptr);
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
        const bool has_track_color = view.track_color() != maui::graphics::color{};
        if (view.is_on())
        {
            // SwitchExtensions.UpdateTrackColor (ON): an explicit OnColor drives both onTintColor and the
            // live track subview. An UNSET OnColor must restore the system DEFAULT onTintColor (the iOS
            // green) — to_ui_color of the collapsed-null color wrongly paints the on-track opaque black, and
            // leaving the track subview alone lets UISwitch draw its own green track.
            native.onTintColor = has_track_color ? to_ui_color(view.track_color()) : nil;
            if (has_track_color)
            {
                track.backgroundColor = to_ui_color(view.track_color());
            }
        }
        else if (!has_track_color)
        {
            // The DEFAULT (collapsed-null) color keeps C#'s off-state fallback: SecondarySystemFill
            // (the Light/Dark-aware equivalent of the pre-13 RGBA 120,120,128,40).
            track.backgroundColor = UIColor.secondarySystemFillColor;
        }
        else
        {
            track.backgroundColor = to_ui_color(view.track_color());
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
        // NeedsContainer: after on_setup_container the CONTAINER is what the parent adds and the layout
        // positions — the UISwitch is only its subview. Framing the switch alone left the wrapper at its
        // setup-time bounds — the switch's INTRINSIC size at the ORIGIN. (UISwitch sizes itself even from
        // initWithFrame:CGRectZero, so the wrapper is ~51x31 at (0,0), not empty. That plausible size is
        // why this hid: the wrapper looks arranged, it is merely never in the arranged PLACE.)
        //
        // On AppKit that mispositioned the toggle and was caught immediately (see the twin comment in
        // apple/switch_handler.mm). ON UIKIT IT IS INVISIBLE: UIView does not clip to bounds by default,
        // so the switch still DRAWS at the right place — the maui and port at-rest frames are
        // pixel-identical — while hitTest:withEvent: refuses every touch: pointInside: is asked of the
        // wrapper FIRST, and a page row at y=117 is far below the wrapper's ~31pt height at y=0.
        //
        // MEASURED 2026-08-07, maccatalyst, the toggled-on step: MAUI's switch changed 877 px inside
        // x[0..48] y[117..138] (thumb left->right, track dark->light blue) and the port changed ZERO
        // pixels anywhere in the 1024x800 frame. Same asymmetry the iOS lane found via a held idb press,
        // i.e. two backends and two unrelated injectors agreeing.
        if (platform->container != nullptr)
        {
            [(__bridge UIView*)platform->container setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
            [as_switch(platform->native) setFrame:CGRectMake(0, 0, frame.width, frame.height)];
            return;
        }
        [as_switch(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void switch_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

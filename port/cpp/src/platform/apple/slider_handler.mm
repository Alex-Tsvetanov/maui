// slider_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an NSSlider
// (held, retained, in slider_platform::native): Minimum/Maximum/Value map to minValue/maxValue/
// doubleValue, the continuous target-action mirrors UISlider.ValueChanged, and the drag channel is an
// NSSlider subclass whose mouseDown: brackets AppKit's mouse-tracking loop (drag-started before
// [super mouseDown:], drag-completed after it returns on mouse-up) — the AppKit analog of C#
// SliderProxy's TouchDown / TouchUp(Inside|Outside) wiring. Compiled as Objective-C++ with ARC only
// for the `apple` backend.
//
// Translated from SliderHandler.iOS.cs + SliderExtensions.cs (UIKit — MAUI's macOS is Mac Catalyst).
// AppKit DEVIATIONS (documented, not silent):
//  - MinimumTrackColor maps to NSSlider.trackFillColor (the filled side — the AppKit equivalent of
//    MinimumTrackTintColor); MaximumTrackColor and ThumbColor have NO public NSSlider API (the cell
//    draws both), so those two record the cross-platform mirrors only.
//  - ThumbImageSource and the UpdateOnTap platform configuration are deferred (see slider_handler.hpp).

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The drag-aware NSSlider: mouseDown: runs AppKit's whole drag-tracking loop inside [super mouseDown:],
// so bracketing it delivers DragStarted/DragCompleted exactly once per gesture. notifyDragStarted /
// notifyDragCompleted are split out so the seam test can drive the same paths the real gesture takes.
@interface MauiNSSlider : NSSlider
@property(nonatomic) maui::core::slider_handler* handler;
- (void)notifyDragStarted;
- (void)notifyDragCompleted;
@end

@implementation MauiNSSlider
- (void)notifyDragStarted
{
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_drag_started();
        }
    }
}

- (void)notifyDragCompleted
{
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_drag_completed();
        }
    }
}

- (void)mouseDown:(NSEvent*)event
{
    [self notifyDragStarted];
    [super mouseDown:event]; // the AppKit tracking loop (returns on mouse-up)
    [self notifyDragCompleted];
}
@end

// Obj-C trampoline for the continuous value action: forwards the new doubleValue to the virtual view
// (SliderProxy.OnControlValueChanged).
@interface MauiSliderTarget : NSObject
@property(nonatomic) maui::core::slider_handler* handler;
- (void)onValueChanged:(id)sender;
@end

@implementation MauiSliderTarget
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    NSSlider* const native = (NSSlider*)sender;
    if (view != nullptr && native != nil)
    {
        view->set_value(native.doubleValue);
    }
}
@end

namespace
{
    // Key for the associated MauiSliderTarget kept alive by the NSSlider (its `target` is weak).
    const char k_target_key = 0;

    NSSlider* as_slider(void* native)
    {
        return (__bridge NSSlider*)native;
    }

    MauiNSSlider* as_maui_slider(void* native)
    {
        return (__bridge MauiNSSlider*)native;
    }

    using maui::platform::apple::to_ns_color;
} // namespace

namespace maui::core
{
    slider_platform::~slider_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void slider_platform::update_visibility(maui::core::visibility value)
    {
        as_slider(native).hidden = value != maui::core::visibility::visible;
    }

    void slider_platform::update_opacity(double value)
    {
        as_slider(native).alphaValue = value;
    }

    void slider_platform::update_is_enabled(bool value)
    {
        [as_slider(native) setEnabled:static_cast<BOOL>(value)];
    }

    void slider_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_slider(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void slider_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void slider_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void slider_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void slider_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void slider_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void slider_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void slider_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        auto platform = std::make_unique<slider_platform>();
        // CreatePlatformView: a continuous slider (UISlider { Continuous = true } — NSSlider's
        // continuous flag likewise fires the action throughout the drag).
        MauiNSSlider* const native = [[MauiNSSlider alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.continuous = YES;
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        MauiNSSlider* const native = as_maui_slider(platform.native);
        native.handler = this; // the drag channel (mouseDown bracketing)
        MauiSliderTarget* const target = [[MauiSliderTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onValueChanged:);
        // ...so keep it alive for the slider's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        MauiNSSlider* const native = as_maui_slider(platform.native);
        native.handler = nullptr;
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_slider(platform->native).minValue = view.minimum(); // SliderExtensions.UpdateMinimum
        }
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_slider(platform->native).maxValue = view.maximum(); // SliderExtensions.UpdateMaximum
        }
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        // SliderExtensions.UpdateValue: write only when it differs (prevents the action echo).
        if (auto* platform = handler.typed_platform_view())
        {
            NSSlider* const native = as_slider(platform->native);
            if (native.doubleValue != view.value())
            {
                native.doubleValue = view.value();
            }
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        // The filled (minimum) side: NSSlider.trackFillColor (the MinimumTrackTintColor equivalent).
        if (auto* platform = handler.typed_platform_view())
        {
            as_slider(platform->native).trackFillColor = to_ns_color(view.minimum_track_color());
            platform->minimum_track_color = view.minimum_track_color();
        }
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        // AppKit deviation: no public API for the unfilled side — record the mirror (see header note).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum_track_color = view.maximum_track_color();
        }
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        // AppKit deviation: the cell draws the knob; no public tint API — record the mirror.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->thumb_color = view.thumb_color();
        }
    }

    maui::graphics::size slider_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_slider(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void slider_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_slider(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

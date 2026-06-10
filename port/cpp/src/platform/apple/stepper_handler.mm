// stepper_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an NSStepper
// (held, retained, in stepper_platform::native): Minimum/Maximum/Increment/Value map to minValue/
// maxValue/increment/doubleValue, and the native minus/plus tap flows back through a target-action
// trampoline to i_range::set_value. Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from StepperHandler.iOS.cs + StepperExtensions.cs (UIKit — MAUI's macOS is Mac Catalyst):
// NSStepper's action fires per value change, matching UIStepper.ValueChanged. NSStepper clamps at the
// range edges like pre-26 UIStepper, so the iOS-26 AdjustStepValueForBoundaries workaround the ios
// partial ports is NOT needed here; the iOS-26 RTL transform + Liquid Glass measurements are likewise
// N/A (see stepper_handler.hpp). valueWraps is disabled (UIStepper.Wraps defaults to false).

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/stepper_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards NSStepper's target-action (fired on a step) to the C++ handler's virtual
// view — the StepperProxy.OnValueChanged port.
@interface MauiStepperTarget : NSObject
@property(nonatomic) maui::core::stepper_handler* handler;
- (void)onValueChanged:(id)sender;
@end

@implementation MauiStepperTarget
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    NSStepper* const native = (NSStepper*)sender;
    if (view != nullptr && native != nil)
    {
        view->set_value(native.doubleValue);
    }
}
@end

namespace
{
    // Key for the associated MauiStepperTarget kept alive by the NSStepper (its `target` is weak).
    const char k_target_key = 0;

    NSStepper* as_stepper(void* native)
    {
        return (__bridge NSStepper*)native;
    }
} // namespace

namespace maui::core
{
    stepper_platform::~stepper_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void stepper_platform::update_visibility(maui::core::visibility value)
    {
        as_stepper(native).hidden = value != maui::core::visibility::visible;
    }

    void stepper_platform::update_opacity(double value)
    {
        as_stepper(native).alphaValue = value;
    }

    void stepper_platform::update_is_enabled(bool value)
    {
        [as_stepper(native) setEnabled:static_cast<BOOL>(value)];
    }

    void stepper_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_stepper(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void stepper_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void stepper_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void stepper_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void stepper_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void stepper_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void stepper_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void stepper_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<stepper_platform> stepper_handler::create_platform_view()
    {
        auto platform = std::make_unique<stepper_platform>();
        NSStepper* const native = [[NSStepper alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.valueWraps = NO;                             // UIStepper.Wraps defaults to false
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void stepper_handler::on_connect_handler(stepper_platform& platform)
    {
        NSStepper* const native = as_stepper(platform.native);
        MauiStepperTarget* const target = [[MauiStepperTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onValueChanged:);
        // ...so keep it alive for the stepper's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void stepper_handler::on_disconnect_handler(stepper_platform& platform)
    {
        NSStepper* const native = as_stepper(platform.native);
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void stepper_handler::map_increment(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateIncrement: only a positive increment lands on the native step.
        if (auto* platform = handler.typed_platform_view())
        {
            if (view.interval() > 0)
            {
                as_stepper(platform->native).increment = view.interval();
            }
        }
    }

    void stepper_handler::map_minimum(stepper_handler& handler, i_stepper& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_stepper(platform->native).minValue = view.minimum(); // StepperExtensions.UpdateMinimum
        }
    }

    void stepper_handler::map_maximum(stepper_handler& handler, i_stepper& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_stepper(platform->native).maxValue = view.maximum(); // StepperExtensions.UpdateMaximum
        }
    }

    void stepper_handler::map_value(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateValue: refresh the native minimum first (a stale higher minimum
        // would make the native control clamp the incoming value), then write when it differs.
        if (auto* platform = handler.typed_platform_view())
        {
            NSStepper* const native = as_stepper(platform->native);
            if (native.minValue != view.minimum())
            {
                native.minValue = view.minimum();
            }
            if (native.doubleValue != view.value())
            {
                native.doubleValue = view.value();
            }
        }
    }

    maui::graphics::size stepper_handler::get_desired_size(double /*width_constraint*/,
                                                           double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_stepper(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void stepper_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_stepper(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

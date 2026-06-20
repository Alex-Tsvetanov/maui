// stepper_handler — iOS (UIKit) platform recipe. The managed platform view is a UIStepper (held,
// retained, in stepper_platform::native): Minimum/Maximum/Increment/Value map to minimumValue/
// maximumValue/stepValue/value, and the native minus/plus tap flows back through a target-action proxy
// to i_range::set_value. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from StepperHandler.iOS.cs + Platform/iOS/StepperExtensions.cs — INCLUDING the
// iOS-26 boundary handling (the port's deployment floor IS 26, so the version gates collapse to
// always-on): UIStepper 26 no longer clamps a step that would overshoot a bound, it just disables the
// button — AdjustStepValueForBoundaries temporarily shrinks stepValue to the remaining space so the
// exact Minimum/Maximum stays reachable, and the proxy's OnValueChanged corrects an accidental partial
// step back to the full increment when it still fits (apple.com forums thread 802452). The
// FlowDirection mapper override IS ported for its BASE part (map_flow_direction): the resolved
// direction (MatchParent → parent-IView fallback) sets the stepper's UISemanticContentAttribute + is
// re-applied to each subview (the iOS-26 walk), mirroring progress_bar_handler. Not ported (documented
// in stepper_handler.hpp): the iOS-26 RTL FlowDirection CGAffineTransform visual flip and the Liquid
// Glass landscape width compensation (cosmetic, empirically-measured).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_view_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/stepper_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIStepper* as_stepper(void* native)
    {
        return (__bridge UIStepper*)native;
    }

    constexpr double k_epsilon = 1e-10; // the C# floating-point comparison epsilon

    // StepperHandler.NeedsStepValueAdjustment: near a boundary, or a previously shrunk stepValue.
    bool needs_step_value_adjustment(const maui::core::i_stepper& stepper, UIStepper* native)
    {
        return stepper.value() + stepper.interval() > stepper.maximum() ||
               stepper.value() - stepper.interval() < stepper.minimum() ||
               std::fabs(native.stepValue - stepper.interval()) > k_epsilon;
    }

    // StepperHandler.AdjustStepValueForBoundaries: shrink stepValue to the remaining space when a full
    // step would overshoot a bound; restore the original increment away from the bounds.
    void adjust_step_value_for_boundaries(const maui::core::i_stepper& virtual_view, UIStepper* native)
    {
        const double original_increment = virtual_view.interval();
        if (original_increment <= 0)
        {
            return;
        }

        double current_value = virtual_view.value();
        const double minimum = virtual_view.minimum();
        const double maximum = virtual_view.maximum();

        if (maximum <= minimum)
        {
            native.stepValue = original_increment;
            return;
        }

        current_value = std::max(minimum, std::min(maximum, current_value));
        const double space_to_max = maximum - current_value;
        const double space_to_min = current_value - minimum;
        const double current_step_value = native.stepValue;

        if (space_to_max > k_epsilon && space_to_max < original_increment &&
            std::fabs(current_step_value - space_to_max) > k_epsilon)
        {
            native.stepValue = space_to_max;
        }
        else if (space_to_min > k_epsilon && space_to_min < original_increment &&
                 std::fabs(current_step_value - space_to_min) > k_epsilon)
        {
            native.stepValue = space_to_min;
        }
        else if (std::fabs(current_step_value - original_increment) > k_epsilon)
        {
            native.stepValue = original_increment;
        }
    }

    // The shared per-map tail: "iOS 26+ fix: Adjust stepValue for boundary handling".
    void adjust_if_needed(const maui::core::i_stepper& view, UIStepper* native)
    {
        if (native != nil && needs_step_value_adjustment(view, native))
        {
            adjust_step_value_for_boundaries(view, native);
        }
    }
} // namespace

// Obj-C trampoline: forwards UIStepper's ValueChanged to the C++ handler's virtual view — the
// StepperProxy.OnValueChanged port, including the iOS-26 partial-step correction.
@interface MauiStepperEventProxy : NSObject
@property(nonatomic) maui::core::stepper_handler* handler;
- (void)onValueChanged:(id)sender;
@end

@implementation MauiStepperEventProxy
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    auto* const native = (UIStepper*)sender;
    if (view == nullptr || native == nil)
    {
        return;
    }

    const double old_value = view->value();
    double new_value = native.value;

    adjust_if_needed(*view, native);

    // "Correct partial steps caused by boundary adjustment": when the step taken was partial (the
    // shrunk stepValue) but the FULL increment still fits the range, it was not an intentional
    // boundary reach — promote it to the full step.
    const double actual_step = new_value - old_value;
    const double interval = view->interval();
    if (std::fabs(actual_step) > k_epsilon && std::fabs(std::fabs(actual_step) - interval) > k_epsilon)
    {
        const double full_step = old_value + (actual_step > 0 ? interval : -interval);
        if (full_step >= view->minimum() && full_step <= view->maximum())
        {
            native.value = full_step;
            native.stepValue = interval;
            new_value = full_step;
        }
    }

    view->set_value(new_value);
}
@end

// MauiIosStepper — the UIStepper the handler presents. Its layoutSubviews override re-sizes any gradient/
// image background sublayer apply_background installed (a solid BackgroundColor needs no resize — it is the
// backing layer's backgroundColor), so a Background brush fills the band behind the −|+ buttons and tracks
// bounds. apply_background runs before arrange, when bounds is zero, so the hook is needed for gradients.
@interface MauiIosStepper : UIStepper
@end

@implementation MauiIosStepper
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
    // Key for the associated MauiStepperEventProxy kept alive by the UIStepper (UIControl does not
    // retain its targets — the target-action convention).
    const char k_proxy_key = 0;
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
        as_stepper(native).alpha = value;
    }

    void stepper_platform::update_is_enabled(bool value)
    {
        as_stepper(native).enabled = static_cast<BOOL>(value);
    }

    void stepper_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_stepper(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void stepper_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void stepper_platform::update_background(const maui::graphics::paint* value)
    {
        // BackgroundColor / Background brush fills the stepper's frame behind the −|+ buttons (MAUI paints
        // it full-width when the stepper is laid out Fill). The shared helper paints a solid color onto the
        // backing layer or installs a gradient/image sublayer (MauiIosStepper.layoutSubviews keeps it sized);
        // a null paint clears it.
        maui::platform::ios::apply_background(native, value);
    }

    std::unique_ptr<stepper_platform> stepper_handler::create_platform_view()
    {
        auto platform = std::make_unique<stepper_platform>();
        UIStepper* const native = [[MauiIosStepper alloc] initWithFrame:CGRectZero];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void stepper_handler::on_connect_handler(stepper_platform& platform)
    {
        UIStepper* const native = as_stepper(platform.native);
        MauiStepperEventProxy* const proxy = [[MauiStepperEventProxy alloc] init];
        proxy.handler = this;
        // StepperProxy.Connect — the ValueChanged wiring. UIControl holds its targets weakly, so the
        // proxy is kept alive for the stepper's lifetime via an associated object (the button pattern).
        [native addTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        objc_setAssociatedObject(native, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void stepper_handler::on_disconnect_handler(stepper_platform& platform)
    {
        UIStepper* const native = as_stepper(platform.native);
        if (auto* const proxy = (MauiStepperEventProxy*)objc_getAssociatedObject(native, &k_proxy_key))
        {
            [native removeTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        }
        objc_setAssociatedObject(native, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void stepper_handler::map_increment(stepper_handler& handler, i_stepper& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIStepper* const native = as_stepper(platform->native);
        // StepperExtensions.UpdateIncrement: only a positive increment lands on the native step.
        if (view.interval() > 0)
        {
            native.stepValue = view.interval();
        }
        adjust_if_needed(view, native); // MapIncrement's iOS-26 boundary tail
    }

    void stepper_handler::map_minimum(stepper_handler& handler, i_stepper& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIStepper* const native = as_stepper(platform->native);
        native.minimumValue = view.minimum(); // StepperExtensions.UpdateMinimum
        adjust_if_needed(view, native);       // MapMinimum's iOS-26 boundary tail
    }

    void stepper_handler::map_maximum(stepper_handler& handler, i_stepper& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIStepper* const native = as_stepper(platform->native);
        native.maximumValue = view.maximum(); // StepperExtensions.UpdateMaximum
        adjust_if_needed(view, native);       // MapMaximum's iOS-26 boundary tail
    }

    void stepper_handler::map_value(stepper_handler& handler, i_stepper& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIStepper* const native = as_stepper(platform->native);
        // StepperExtensions.UpdateValue: refresh the native minimum first ("If MAUI updates Value
        // before Minimum, a stale higher MinimumValue would cause iOS to clamp Value incorrectly"),
        // then write when it differs.
        if (native.minimumValue != view.minimum())
        {
            native.minimumValue = view.minimum();
        }
        if (native.value != view.value())
        {
            native.value = view.value();
        }
        adjust_if_needed(view, native); // MapValue's iOS-26 boundary tail
    }

    void stepper_handler::map_flow_direction(stepper_handler& handler, i_stepper& view)
    {
        // StepperHandler.MapFlowDirection (base part): set the stepper's UISemanticContentAttribute from
        // the RESOLVED direction (the MatchParent → parent-IView fallback), then re-apply it to each
        // internal subview (the iOS-26 walk — UIStepper stopped propagating the attribute to its
        // subviews). The resolved direction is mirrored for the headless-parity oracle. Mirrors
        // ProgressBarHandler.MapFlowDirection; the iOS-26 RTL CGAffineTransform flip stays deferred.
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved;
        maui::platform::ios::apply_flow_direction(platform->native, resolved);
    }

    maui::graphics::size stepper_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_stepper(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void stepper_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_stepper(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

// date_picker_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an
// NSDatePicker (held, retained, in date_picker_platform::native) in the text-field-and-stepper style
// with year/month/day elements on the UTC time zone (the MauiDatePicker convention): Date pushes to
// dateValue (a null virtual Date falls back to Today, per DatePickerExtensions.UpdateDate),
// Minimum/MaximumDate to minDate/maxDate, and a native edit flows back through a target-action
// trampoline into i_date_picker::set_date (the control's coercion clamps it). Compiled as
// Objective-C++ with ARC only for the `apple` backend.
//
// Idiomatic translation of DatePickerHandler.iOS.cs + DatePickerExtensions.cs (the UITextField whose
// inputView is a UIDatePicker). Documented deviations:
//   - the NSDatePicker commits per value change (there is no Done accessory on a field-style
//     picker) — the UpdateMode.Immediately behavior. `on_done` is still wired (the dialog mirror is
//     read back) for portable drives.
//   - the Format string drives the TEXT rendering on the headless/ios backends (MauiDatePicker.Text);
//     AppKit's NSDatePicker draws its own localized field representation, so format is not pushed.
//   - character_spacing has no NSDatePicker analog (no attributed field text) and is not pushed.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "../apple_shared/date_conversions.hpp"
#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/view_focus_ops.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the NSDatePicker's target-action (fired on a native edit) to the C++
// handler — the DatePickerDelegate.DatePickerValueChanged + SetVirtualViewDate port.
@interface MauiDatePickerTarget : NSObject
@property(nonatomic) maui::core::date_picker_handler* handler;
- (void)onValueChanged:(id)sender;
@end

namespace
{
    // Key for the associated MauiDatePickerTarget kept alive by the NSDatePicker (`target` is weak).
    const char k_target_key = 0;

    NSDatePicker* as_date_picker(void* native)
    {
        return (__bridge NSDatePicker*)native;
    }

    // SetVirtualViewDate: commit the dialog's current value (truncated to the calendar day) to the
    // virtual view, whose own coercion clamps it into [MinimumDate, MaximumDate].
    void commit_date(maui::core::date_picker_handler& handler)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        view->set_date(maui::platform::apple_shared::to_date_time(as_date_picker(platform->native).dateValue).date());
    }
} // namespace

@implementation MauiDatePickerTarget
- (void)onValueChanged:(id)sender
{
    if (self.handler != nullptr)
    {
        commit_date(*self.handler);
    }
}
@end

namespace maui::core
{
    date_picker_platform::~date_picker_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void date_picker_platform::update_visibility(maui::core::visibility value)
    {
        as_date_picker(native).hidden = value != maui::core::visibility::visible;
    }

    void date_picker_platform::update_opacity(double value)
    {
        as_date_picker(native).alphaValue = value;
    }

    void date_picker_platform::update_is_enabled(bool value)
    {
        [as_date_picker(native) setEnabled:static_cast<BOOL>(value)];
    }

    void date_picker_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_date_picker(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void date_picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void date_picker_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void date_picker_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void date_picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void date_picker_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void date_picker_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
        NSDatePicker* const native = [[NSDatePicker alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.datePickerStyle = NSDatePickerStyleTextFieldAndStepper;
        native.datePickerElements = NSDatePickerElementFlagYearMonthDay;
        native.timeZone = [NSTimeZone timeZoneWithName:@"UTC"]; // the MauiDatePicker UTC convention
        platform->native = (__bridge_retained void*)native;     // the void* slot owns one reference
        return platform;
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        NSDatePicker* const native = as_date_picker(platform.native);
        MauiDatePickerTarget* const target = [[MauiDatePickerTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onValueChanged:);
        // ...so keep it alive for the picker's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // The Done-tap analog for portable drives (the field commits per change — see the header).
        platform.on_done = [this] { commit_date(*this); };
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        NSDatePicker* const native = as_date_picker(platform.native);
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        map_date(handler, view); // UpdateFormat routes into UpdateDate (AppKit: the date push only)
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        // DatePickerExtensions.UpdateDate: a null virtual Date falls back to Today; only a real
        // difference lands on the native control (C#'s ToDateTime() != targetDate guard).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSDatePicker* const native = as_date_picker(platform->native);
        const date_time target = view.date().value_or(date_time::today());
        if (maui::platform::apple_shared::to_date_time(native.dateValue) != target)
        {
            native.dateValue = maui::platform::apple_shared::to_ns_date(target);
        }
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            const auto minimum = view.minimum_date();
            as_date_picker(platform->native).minDate =
                minimum ? maui::platform::apple_shared::to_ns_date(*minimum) : nil;
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            const auto maximum = view.maximum_date();
            as_date_picker(platform->native).maxDate =
                maximum ? maui::platform::apple_shared::to_ns_date(*maximum) : nil;
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        // DatePickerExtensions.UpdateTextColor: an unset color keeps the system default.
        if (auto* platform = handler.typed_platform_view())
        {
            const maui::graphics::color color = view.text_color();
            if (color != maui::graphics::color{})
            {
                as_date_picker(platform->native).textColor = maui::platform::apple::to_ns_color(color);
            }
        }
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_date_picker(platform->native).font = maui::platform::apple::to_ns_font(view.font());
        }
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        // No NSDatePicker analog (no attributed field text) — see the header note.
        (void)handler;
        (void)view;
    }

    void date_picker_handler::map_is_open(date_picker_handler& handler, i_date_picker& view)
    {
        // DatePickerHandler.MapIsOpen → become first responder when IsOpen, else resign. AppKit's idiom
        // is window.makeFirstResponder: (the shared view_focus_ops); there is no UITextField editing
        // callback on macOS, so the dialog focus is the observable behavior.
        if (view.is_open())
        {
            focus_native_view(handler.native_view());
        }
        else
        {
            unfocus_native_view(handler.native_view());
        }
    }

    maui::graphics::size date_picker_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_date_picker(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_date_picker(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

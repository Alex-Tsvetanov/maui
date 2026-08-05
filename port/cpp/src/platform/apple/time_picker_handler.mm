// time_picker_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an
// NSDatePicker (held, retained, in time_picker_platform::native) in the text-field-and-stepper style
// with hour/minute elements on the UTC time zone (the MauiTimePicker convention): Time pushes to
// dateValue anchored on the epoch day (a null virtual Time falls back to zero, per
// TimePickerExtensions.UpdateTime), and a native edit flows back through a target-action trampoline
// into i_time_picker::set_time with the SECONDS DROPPED (SetVirtualViewTime builds hour+minute only).
// Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Idiomatic translation of TimePickerHandler.iOS.cs + TimePickerExtensions.cs (the UITextField whose
// inputView is a time-mode UIDatePicker). Documented deviations:
//   - the NSDatePicker commits per value change (no Done accessory on a field-style picker) — the
//     UpdateMode.Immediately behavior. `on_done` is still wired for portable drives.
//   - the Format string drives the TEXT rendering on the headless/ios backends (MauiTimePicker.Text);
//     AppKit's NSDatePicker draws its own localized field representation, so format is not pushed
//     (C#'s per-format culture pick has no field-text to land on).
//   - character_spacing has no NSDatePicker analog (no attributed field text) and is not pushed.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "../apple_shared/date_conversions.hpp"
#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/view_focus_ops.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the NSDatePicker's target-action (fired on a native edit) to the C++
// handler — the MauiTimePickerProxy.OnDateSelected + SetVirtualViewTime port.
@interface MauiTimePickerTarget : NSObject
@property(nonatomic) maui::core::time_picker_handler* handler;
- (void)onValueChanged:(id)sender;
@end

namespace
{
    // Key for the associated MauiTimePickerTarget kept alive by the NSDatePicker (`target` is weak).
    const char k_target_key = 0;

    NSDatePicker* as_time_picker(void* native)
    {
        return (__bridge NSDatePicker*)native;
    }

    // SetVirtualViewTime: commit the wheel's current value with the seconds dropped — C# builds
    // `new TimeSpan(datetime.Hour, datetime.Minute, 0)`.
    void commit_time(maui::core::time_picker_handler& handler)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const maui::core::time_span picked =
            maui::platform::apple_shared::to_time_span(as_time_picker(platform->native).dateValue);
        view->set_time(maui::core::time_span(picked.hours(), picked.minutes(), 0));
    }
} // namespace

@implementation MauiTimePickerTarget
- (void)onValueChanged:(id)sender
{
    if (self.handler != nullptr)
    {
        commit_time(*self.handler);
    }
}
@end

namespace maui::core
{
    // The teardown that must run whether the handler is DISCONNECTED or merely DESTROYED. The native
    // view outlives the handler in any real app (a superview retains it) and the trampolines it keeps
    // in its associated objects carry RAW handler pointers; nothing calls disconnect_handler() when a
    // handler is destroyed (there is no ~view_handler doing it), so the platform dtor has to run this
    // too or the next native callback dereferences freed memory. Idempotent: disconnect_handler()
    // destroys the platform right after calling it, so both paths run on the same object.
    namespace
    {
        void detach_trampolines(time_picker_platform& platform)
        {
            NSDatePicker* const native = as_time_picker(platform.native);
            native.target = nil;
            native.action = nil;
            if (auto* const trampoline = (MauiTimePickerTarget*)objc_getAssociatedObject(native, &k_target_key))
            {
                trampoline.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            platform.on_done = nullptr;
        }
    } // namespace

    time_picker_platform::~time_picker_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void time_picker_platform::update_visibility(maui::core::visibility value)
    {
        as_time_picker(native).hidden = value != maui::core::visibility::visible;
    }

    void time_picker_platform::update_opacity(double value)
    {
        as_time_picker(native).alphaValue = value;
    }

    void time_picker_platform::update_is_enabled(bool value)
    {
        [as_time_picker(native) setEnabled:static_cast<BOOL>(value)];
    }

    void time_picker_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_time_picker(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void time_picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void time_picker_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void time_picker_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void time_picker_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void time_picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void time_picker_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void time_picker_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<time_picker_platform>();
        NSDatePicker* const native = [[NSDatePicker alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        native.datePickerStyle = NSDatePickerStyleTextFieldAndStepper;
        native.datePickerElements = NSDatePickerElementFlagHourMinute; // time-of-day only (no seconds)
        native.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];        // the MauiTimePicker UTC convention
        platform->native = (__bridge_retained void*)native;            // the void* slot owns one reference
        return platform;
    }

    void time_picker_handler::on_connect_handler(time_picker_platform& platform)
    {
        NSDatePicker* const native = as_time_picker(platform.native);
        MauiTimePickerTarget* const target = [[MauiTimePickerTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onValueChanged:);
        // ...so keep it alive for the picker's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // The Done-tap analog for portable drives (the field commits per change — see the header).
        platform.on_done = [this] { commit_time(*this); };
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        detach_trampolines(platform);
    }

    void time_picker_handler::map_format(time_picker_handler& handler, i_time_picker& view)
    {
        map_time(handler, view); // UpdateFormat routes into UpdateTime (AppKit: the time push only)
    }

    void time_picker_handler::map_time(time_picker_handler& handler, i_time_picker& view)
    {
        // TimePickerExtensions.UpdateTime: a null virtual Time falls back to zero, anchored on the
        // epoch day under UTC (the 0001-01-01 anchor's port — see date_conversions.hpp).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_time_picker(platform->native).dateValue =
            maui::platform::apple_shared::to_ns_date(view.time().value_or(time_span{}));
    }

    void time_picker_handler::map_text_color(time_picker_handler& handler, i_time_picker& view)
    {
        // An unset color keeps the system default (the DatePickerExtensions.UpdateTextColor shape).
        if (auto* platform = handler.typed_platform_view())
        {
            const maui::graphics::color color = view.text_color();
            if (color != maui::graphics::color{})
            {
                as_time_picker(platform->native).textColor = maui::platform::apple::to_ns_color(color);
            }
        }
    }

    void time_picker_handler::map_font(time_picker_handler& handler, i_time_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_time_picker(platform->native).font = maui::platform::apple::to_ns_font(view.font());
        }
    }

    void time_picker_handler::map_character_spacing(time_picker_handler& handler, i_time_picker& view)
    {
        // No NSDatePicker analog (no attributed field text) — see the header note.
        (void)handler;
        (void)view;
    }

    void time_picker_handler::map_is_open(time_picker_handler& handler, i_time_picker& view)
    {
        // TimePickerHandler.MapIsOpen → become first responder when IsOpen, else resign. AppKit's idiom
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

    maui::graphics::size time_picker_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_time_picker(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void time_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_time_picker(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

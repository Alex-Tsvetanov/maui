// time_picker_handler — iOS (UIKit) platform recipe. The managed platform view is a real UITextField
// (the MauiTimePicker shape: a rounded-rect field whose inputView is a time-mode UIDatePicker on the
// UTC time zone with the Done accessory), Time/Format map per TimePickerExtensions.UpdateTime, and
// the Done tap commits the wheel value back through i_time_picker::set_time with the SECONDS DROPPED
// (SetVirtualViewTime builds hour+minute only). Compiled as Objective-C++ with ARC only for the `ios`
// backend.
//
// Ported DIRECTLY from TimePickerHandler.iOS.cs + Platform/iOS/MauiTimePicker.cs +
// TimePickerExtensions.cs:
//   CreatePlatformView = new MauiTimePicker(OnDateSelected) (RoundedRect; UIDatePicker { Mode = Time,
//   UTC, Wheels }; Done accessory → SetVirtualViewTime + resign);
//   UpdateTime = wheel date anchored on the epoch day (C# anchors 0001-01-01 — equivalent under UTC,
//   see date_conversions.hpp), field text = Time?.ToFormattedString(Format) — null shows empty (the
//   C# per-format culture pick collapses into the port's invariant/en-US rendering).
// Not ported here (deferred): IsOpen + the focus dance, UpdateImmediately (ValueChanged → live
// commit), the VoiceOver notifications, and the FlowDirection+TextAlignment remap (the shared
// view_mapper carries flow_direction).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "../apple_shared/date_conversions.hpp"
#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline for the Done accessory tap — MauiTimePicker's dateSelected callback feeding
// SetVirtualViewTime.
@interface MauiIosTimePickerDoneTarget : NSObject
@property(nonatomic) maui::core::time_picker_handler* handler;
- (void)onDone:(id)sender;
@end

namespace
{
    // Key for the associated done-target the UITextField keeps alive (bar-button targets are weak).
    const char k_done_key = 0;

    UITextField* as_field(void* native)
    {
        return (__bridge UITextField*)native;
    }

    UIDatePicker* wheel_of(UITextField* field)
    {
        return [field.inputView isKindOfClass:[UIDatePicker class]] ? (UIDatePicker*)field.inputView : nil;
    }

    // SetVirtualViewTime: `new TimeSpan(datetime.Hour, datetime.Minute, 0)` — seconds dropped.
    void commit_time(maui::core::time_picker_handler& handler)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIDatePicker* const wheel = wheel_of(as_field(platform->native));
        if (wheel == nil)
        {
            return;
        }
        const maui::core::time_span picked = maui::platform::apple_shared::to_time_span(wheel.date);
        view->set_time(maui::core::time_span(picked.hours(), picked.minutes(), 0));
    }

    // TimePickerExtensions.UpdateTime(mauiTimePicker, timePicker, picker): the wheel push + the
    // field-text render (null Time shows empty; empty format falls back to "t").
    void update_time(maui::core::time_picker_handler& handler, maui::core::i_time_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        const auto time = view.time();
        if (UIDatePicker* const wheel = wheel_of(field))
        {
            wheel.date = maui::platform::apple_shared::to_ns_date(time.value_or(maui::core::time_span{}));
        }
        if (!time.has_value())
        {
            field.text = @"";
            return;
        }
        const std::string text = maui::core::format_time_span(*time, view.format());
        field.text = [NSString stringWithUTF8String:text.c_str()];
    }
} // namespace

@implementation MauiIosTimePickerDoneTarget
- (void)onDone:(id)sender
{
    if (self.handler != nullptr)
    {
        commit_time(*self.handler); // OnDateSelected → SetVirtualViewTime (+ resign, no session here)
    }
}
@end

namespace maui::core
{
    time_picker_platform::~time_picker_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void time_picker_platform::update_visibility(maui::core::visibility value)
    {
        as_field(native).hidden = value != maui::core::visibility::visible;
    }

    void time_picker_platform::update_opacity(double value)
    {
        as_field(native).alpha = value;
    }

    void time_picker_platform::update_is_enabled(bool value)
    {
        as_field(native).enabled = static_cast<BOOL>(value);
    }

    void time_picker_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_field(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<time_picker_platform>();
        // MauiTimePicker(): RoundedRect field; UIDatePicker { Mode = Time, TimeZone = UTC, Wheels }
        // as the inputView; Button accessibility traits.
        UITextField* const field = [[UITextField alloc] initWithFrame:CGRectZero];
        field.borderStyle = UITextBorderStyleRoundedRect;
        UIDatePicker* const wheel = [[UIDatePicker alloc] initWithFrame:CGRectZero];
        wheel.datePickerMode = UIDatePickerModeTime;
        wheel.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        wheel.preferredDatePickerStyle = UIDatePickerStyleWheels;
        field.inputView = wheel;
        field.inputView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        field.accessibilityTraits = UIAccessibilityTraitButton;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void time_picker_handler::on_connect_handler(time_picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        // The Done accessory (MauiDoneAccessoryView(OnDateSelected)).
        MauiIosTimePickerDoneTarget* const done = [[MauiIosTimePickerDoneTarget alloc] init];
        done.handler = this;
        UIToolbar* const toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
        UIBarButtonItem* const spacer =
            [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                          target:nil
                                                          action:nil];
        UIBarButtonItem* const done_item =
            [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                          target:done
                                                          action:@selector(onDone:)];
        toolbar.items = @[ spacer, done_item ];
        field.inputAccessoryView = toolbar;
        field.inputAccessoryView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        objc_setAssociatedObject(field, &k_done_key, done, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // ConnectHandler's `platformView.UpdateTime(VirtualView.Time)` happens through map_time on
        // attach; on_done is the portable Done channel.
        platform.on_done = [this] { commit_time(*this); };
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        field.inputAccessoryView = nil;
        objc_setAssociatedObject(field, &k_done_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
    }

    void time_picker_handler::map_format(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view); // UpdateFormat routes into UpdateTime
    }

    void time_picker_handler::map_time(time_picker_handler& handler, i_time_picker& view)
    {
        update_time(handler, view);
    }

    void time_picker_handler::map_text_color(time_picker_handler& handler, i_time_picker& view)
    {
        // TimePickerExtensions.UpdateTextColor: an unset color keeps the system default.
        if (auto* platform = handler.typed_platform_view())
        {
            const maui::graphics::color color = view.text_color();
            if (color != maui::graphics::color{})
            {
                as_field(platform->native).textColor = maui::platform::ios::to_ui_color(color);
            }
        }
    }

    void time_picker_handler::map_font(time_picker_handler& handler, i_time_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).font = maui::platform::ios::to_ui_font(view.font(), UIFont.labelFontSize);
        }
    }

    void time_picker_handler::map_character_spacing(time_picker_handler& handler, i_time_picker& view)
    {
        // TextFieldExtensions.UpdateCharacterSpacing: kern the field text (UpdateTime's tail call).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        NSAttributedString* const text_attr =
            maui::platform::ios::with_character_spacing(field.attributedText, view.character_spacing());
        if (text_attr != nil)
        {
            field.attributedText = text_attr;
        }
    }

    maui::graphics::size time_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGSize fitting = [as_field(platform->native)
            sizeThatFits:CGSizeMake(width_constraint > 0 ? width_constraint : CGFLOAT_MAX,
                                    height_constraint > 0 ? height_constraint : CGFLOAT_MAX)];
        return {fitting.width, fitting.height};
    }

    void time_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_field(platform->native).frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
    }
} // namespace maui::core

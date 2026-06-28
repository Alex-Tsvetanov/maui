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
//   C# per-format culture pick collapses into the port's invariant/en-US rendering);
//   MapIsOpen = BecomeFirstResponder when IsOpen, else ResignFirstResponder.
// The IsOpen focus dance is wired through the editing proxy (TimePickerHandler.iOS.cs OnStarted/OnEnded):
// EditingDidBegin sets `IsOpen = IsFocused = true` (IsOpen FIRST), EditingDidEnd sets both false. The
// proxy holds a RAW handler back-ref cleared on disconnect (no retain cycle; C# uses WeakReference).
// Not ported here (deferred): UpdateImmediately (ValueChanged → live commit), the VoiceOver
// notifications, and the FlowDirection+TextAlignment remap (the shared view_mapper carries
// flow_direction).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "../apple_shared/date_conversions.hpp"
#include "ios_conversions.hpp"
#include "ios_done_accessory.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/view_focus_ops.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

// MauiIosTimePicker  <=  Microsoft.Maui.Platform.MauiTimePicker — the UITextField the time picker presents.
// Its layoutSubviews override re-sizes any gradient/image background sublayer apply_background installed to
// the field's current bounds: apply_background runs before arrange, so without this a gradient/image
// BackgroundColor would be left zero-sized and invisible (a solid color needs no resize — it is the UIView
// backgroundColor property). Mirrors MauiTimePicker re-syncing its background on layout.
@interface MauiIosTimePicker : UITextField
@end

@implementation MauiIosTimePicker
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

// Obj-C trampoline for the Done accessory tap — MauiTimePicker's dateSelected callback feeding
// SetVirtualViewTime.
@interface MauiIosTimePickerDoneTarget : NSObject
@property(nonatomic) maui::core::time_picker_handler* handler;
- (void)onDone:(id)sender;
@end

// The IsOpen focus dance (TimePickerHandler.iOS.cs OnStarted/OnEnded): observes EditingDidBegin/DidEnd
// and writes IsOpen + IsFocused back (IsOpen FIRST). Raw handler back-ref cleared on disconnect.
@interface MauiIosTimePickerEditingProxy : NSObject
@property(nonatomic) maui::core::time_picker_handler* handler;
- (void)onStarted:(id)sender;
- (void)onEnded:(id)sender;
@end

namespace
{
    // Keys for the associated done-target / editing-proxy the UITextField keeps alive (weak otherwise).
    const char k_done_key = 0;
    const char k_editing_proxy_key = 0;

    UITextField* as_field(void* native)
    {
        return (__bridge UITextField*)native;
    }

    // Real MAUI (net10.0-iOS) renders the STANDARD time specifiers in the DEVICE locale via NSDateFormatter:
    // the default/empty/"t" short time → 24h "14:00" on a 24h-locale device (NOT the en-US "2:00 PM" the
    // invariant format_time_span would produce), and "T" → the medium (with-seconds) style. Verified against
    // the maui-compare reference (ios_time_picker shows "14:00", the main time_picker "Default" shows "0:00").
    // Custom patterns ("hh:mm", "HH:mm", …) keep format_time_span — their letters are locale-agnostic. The
    // formatter's locale defaults to currentLocale; TimeZone is pinned to GMT0 to match how the wheel maps the
    // time-of-day (UTC) — the analogue of the date handler's localized_default_date.
    std::string localized_default_time(const maui::core::time_span& value, NSDateFormatterStyle style)
    {
        NSDateFormatter* const formatter = [[NSDateFormatter alloc] init];
        formatter.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
        formatter.dateStyle = NSDateFormatterNoStyle;
        formatter.timeStyle = style;
        NSString* const text = [formatter stringFromDate:maui::platform::apple_shared::to_ns_date(value)];
        return text != nil ? std::string(text.UTF8String) : std::string{};
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
        // Standard specifiers (empty/"t"/"T") → device-locale via NSDateFormatter, matching real MAUI; custom
        // patterns keep the invariant format_time_span (their letters are locale-agnostic).
        const std::string_view format = view.format();
        std::string text;
        if (format.empty() || format == "t")
        {
            text = localized_default_time(*time, NSDateFormatterShortStyle);
        }
        else if (format == "T")
        {
            text = localized_default_time(*time, NSDateFormatterMediumStyle);
        }
        else
        {
            text = maui::core::format_time_span(*time, format);
        }
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

@implementation MauiIosTimePickerEditingProxy
- (void)onStarted:(id)sender
{
    // TimePickerHandler.iOS.cs OnStarted: `virtualView.IsFocused = virtualView.IsOpen = true` (IsOpen FIRST).
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view != nullptr)
    {
        view->set_is_open(true);
        view->set_is_focused(true);
    }
}

- (void)onEnded:(id)sender
{
    // TimePickerHandler.iOS.cs OnEnded: `virtualView.IsFocused = virtualView.IsOpen = false` (IsOpen FIRST).
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view != nullptr)
    {
        view->set_is_open(false);
        view->set_is_focused(false);
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

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void time_picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void time_picker_platform::update_background(const maui::graphics::paint* value)
    {
        // The MauiTimePicker is a RoundedRect UITextField. A solid BackgroundColor must go to the UIView
        // backgroundColor PROPERTY (not the backing layer): the rounded-rect bezel is drawn over a layer
        // fill (peeking only at the corners), whereas the view property suppresses the bezel and fills the
        // field flat — matching MAUI's solid fill. Gradient/image paints use the shared layer machinery;
        // a null paint clears the override back to the system default.
        UITextField* const field = as_field(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            field.borderStyle = UITextBorderStyleRoundedRect;
            field.backgroundColor = maui::platform::ios::to_ui_color(solid->color());
        }
        else if (value != nullptr)
        {
            // Gradient/image: the brush fill sits at the BOTTOM of the layer (zPosition -1, behind the
            // content per InsertBackgroundLayer). The RoundedRect bezel is drawn ABOVE it and would hide all
            // but the field's edge, so drop the bezel to None — the gradient then fills the field flat (text
            // on top), matching MAUI's gradient TimePicker fill.
            field.borderStyle = UITextBorderStyleNone;
            field.backgroundColor = nil;
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            field.borderStyle = UITextBorderStyleRoundedRect;
            field.backgroundColor = nil;
        }
    }

    std::unique_ptr<time_picker_platform> time_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<time_picker_platform>();
        // MauiTimePicker(): RoundedRect field; UIDatePicker { Mode = Time, TimeZone = UTC, Wheels }
        // as the inputView; Button accessibility traits.
        UITextField* const field = [[MauiIosTimePicker alloc] initWithFrame:CGRectZero];
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
        field.inputAccessoryView = maui::platform::ios::make_done_accessory(done, @selector(onDone:));
        field.inputAccessoryView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        objc_setAssociatedObject(field, &k_done_key, done, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // The IsOpen focus dance: EditingDidBegin/DidEnd drive the editing proxy (raw back-ref).
        MauiIosTimePickerEditingProxy* const editing = [[MauiIosTimePickerEditingProxy alloc] init];
        editing.handler = this;
        [field addTarget:editing action:@selector(onStarted:) forControlEvents:UIControlEventEditingDidBegin];
        [field addTarget:editing action:@selector(onEnded:) forControlEvents:UIControlEventEditingDidEnd];
        objc_setAssociatedObject(field, &k_editing_proxy_key, editing, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // ConnectHandler's `platformView.UpdateTime(VirtualView.Time)` happens through map_time on
        // attach; on_done is the portable Done channel.
        platform.on_done = [this] { commit_time(*this); };
    }

    void time_picker_handler::on_disconnect_handler(time_picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        field.inputAccessoryView = nil;
        if (MauiIosTimePickerEditingProxy* const editing =
                (MauiIosTimePickerEditingProxy*)objc_getAssociatedObject(field, &k_editing_proxy_key))
        {
            [field removeTarget:editing action:@selector(onStarted:) forControlEvents:UIControlEventEditingDidBegin];
            [field removeTarget:editing action:@selector(onEnded:) forControlEvents:UIControlEventEditingDidEnd];
            editing.handler = nullptr;
        }
        objc_setAssociatedObject(field, &k_done_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(field, &k_editing_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
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
            as_field(platform->native).font =
                maui::platform::ios::to_ui_font(view.font(), maui::platform::ios::default_text_font_size());
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

    void time_picker_handler::map_is_open(time_picker_handler& handler, i_time_picker& view)
    {
        // TimePickerHandler.MapIsOpen: BecomeFirstResponder when IsOpen (presenting the wheel), else
        // ResignFirstResponder. On a real device this fires EditingDidBegin/DidEnd, which the editing
        // proxy turns into the IsOpen + IsFocused write-back.
        if (view.is_open())
        {
            focus_native_view(handler.native_view());
        }
        else
        {
            unfocus_native_view(handler.native_view());
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

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void time_picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

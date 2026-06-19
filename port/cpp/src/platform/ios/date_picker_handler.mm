// date_picker_handler — iOS (UIKit) platform recipe. The managed platform view is a real UITextField
// (the MauiDatePicker shape: a rounded-rect field whose inputView is a date-mode UIDatePicker on the
// UTC time zone with the Done accessory), Date/Minimum/Maximum/Format map per
// DatePickerExtensions.UpdateDate/UpdateMinimumDate/UpdateMaximumDate, and the Done tap commits the
// dialog value back through i_date_picker::set_date (the control's coercion clamps it). Compiled as
// Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from DatePickerHandler.iOS.cs + Platform/iOS/MauiDatePicker.cs +
// DatePickerExtensions.cs:
//   CreatePlatformView = new MauiDatePicker() (RoundedRect; UIDatePicker { Mode = Date, UTC, Wheels };
//   the Done accessory → OnDoneClicked → SetVirtualViewDate + resign);
//   UpdateDate = set the dialog date only on a real difference, then render the field text — null
//   Date shows empty; "d"/"D"/empty route through the standard patterns, anything else is a custom
//   DateTime.ToString pattern (the port renders in the invariant/en-US culture — see date_time.hpp);
//   UpdateMinimumDate/UpdateMaximumDate land on the dialog's MinimumDate/MaximumDate;
//   MapIsOpen = BecomeFirstResponder when IsOpen, else ResignFirstResponder.
// The IsOpen focus dance is wired through the editing proxy (DatePickerHandler.iOS.cs OnStarted/OnEnded):
// EditingDidBegin sets `IsOpen = IsFocused = true` (IsOpen FIRST), EditingDidEnd sets both false. The
// proxy holds a RAW handler back-ref cleared on disconnect (no retain cycle; C# uses WeakReference).
// Not ported here (deferred): UpdateImmediately (ValueChanged → live commit, an iOS platform-specific),
// the VoiceOver notifications, and the FlowDirection+TextAlignment remap (the shared view_mapper carries
// flow_direction).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "../apple_shared/date_conversions.hpp"
#include "ios_conversions.hpp"
#include "ios_done_accessory.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/view_focus_ops.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

// Obj-C trampoline for the Done accessory tap — MauiDatePicker's DoneClicked feeding
// DatePickerHandler.OnDoneClicked → SetVirtualViewDate.
@interface MauiIosDatePickerDoneTarget : NSObject
@property(nonatomic) maui::core::date_picker_handler* handler;
- (void)onDone:(id)sender;
@end

// The IsOpen focus dance (DatePickerHandler.iOS.cs OnStarted/OnEnded): observes EditingDidBegin/DidEnd
// and writes IsOpen + IsFocused back (IsOpen FIRST). Raw handler back-ref cleared on disconnect.
@interface MauiIosDatePickerEditingProxy : NSObject
@property(nonatomic) maui::core::date_picker_handler* handler;
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

    UIDatePicker* dialog_of(UITextField* field)
    {
        return [field.inputView isKindOfClass:[UIDatePicker class]] ? (UIDatePicker*)field.inputView : nil;
    }

    // SetVirtualViewDate: VirtualView.Date = DatePickerDialog.Date.ToDateTime() (truncated to the
    // calendar day; the control's coercion clamps into [MinimumDate, MaximumDate]).
    void commit_date(maui::core::date_picker_handler& handler)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIDatePicker* const dialog = dialog_of(as_field(platform->native));
        if (dialog == nil)
        {
            return;
        }
        view->set_date(maui::platform::apple_shared::to_date_time(dialog.date).date());
    }

    // DatePickerExtensions.UpdateDate(platformDatePicker, datePicker, picker): the dialog push (only
    // on a real difference) + the field-text render.
    void update_date(maui::core::date_picker_handler& handler, maui::core::i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        const auto date = view.date();
        const maui::core::date_time target = date.value_or(maui::core::date_time::today());
        UIDatePicker* const dialog = dialog_of(field);
        if (dialog != nil && maui::platform::apple_shared::to_date_time(dialog.date) != target)
        {
            [dialog setDate:maui::platform::apple_shared::to_ns_date(target) animated:NO];
        }

        if (!date.has_value())
        {
            field.text = @"";
            return;
        }
        const std::string_view format = view.format();
        std::string text;
        if (format.empty() || format == "d" || format == "D")
        {
            text = maui::core::format_date_time(*date, format == "D" ? "D" : "d");
        }
        else
        {
            text = maui::core::format_date_time(*date, format);
        }
        field.text = [NSString stringWithUTF8String:text.c_str()];
    }
} // namespace

@implementation MauiIosDatePickerDoneTarget
- (void)onDone:(id)sender
{
    if (self.handler != nullptr)
    {
        commit_date(*self.handler); // OnDoneClicked → SetVirtualViewDate (+ resign, no session here)
    }
}
@end

@implementation MauiIosDatePickerEditingProxy
- (void)onStarted:(id)sender
{
    // DatePickerHandler.iOS.cs OnStarted: `virtualView.IsFocused = virtualView.IsOpen = true` (IsOpen FIRST).
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view != nullptr)
    {
        view->set_is_open(true);
        view->set_is_focused(true);
    }
}

- (void)onEnded:(id)sender
{
    // DatePickerHandler.iOS.cs OnEnded: `virtualView.IsFocused = virtualView.IsOpen = false` (IsOpen FIRST).
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
        as_field(native).hidden = value != maui::core::visibility::visible;
    }

    void date_picker_platform::update_opacity(double value)
    {
        as_field(native).alpha = value;
    }

    void date_picker_platform::update_is_enabled(bool value)
    {
        as_field(native).enabled = static_cast<BOOL>(value);
    }

    void date_picker_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_field(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
        // The MauiDatePicker is a RoundedRect UITextField. A solid BackgroundColor must go to the UIView
        // backgroundColor PROPERTY (not the backing layer): the rounded-rect bezel is drawn over a layer
        // fill (so it peeks only at the corners), whereas the view property suppresses the bezel and fills
        // the field flat — matching MAUI's solid DatePicker fill. Gradient/image paints use the shared
        // layer machinery; a null paint clears the override back to the system default.
        UITextField* const field = as_field(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            field.backgroundColor = maui::platform::ios::to_ui_color(solid->color());
        }
        else if (value != nullptr)
        {
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            field.backgroundColor = nil;
        }
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
        // MauiDatePicker(): RoundedRect field; UIDatePicker { Mode = Date, TimeZone = UTC, Wheels }
        // as the inputView; Button accessibility traits.
        UITextField* const field = [[UITextField alloc] initWithFrame:CGRectZero];
        field.borderStyle = UITextBorderStyleRoundedRect;
        UIDatePicker* const dialog = [[UIDatePicker alloc] initWithFrame:CGRectZero];
        dialog.datePickerMode = UIDatePickerModeDate;
        dialog.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        dialog.preferredDatePickerStyle = UIDatePickerStyleWheels;
        field.inputView = dialog;
        field.inputView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        field.accessibilityTraits = UIAccessibilityTraitButton;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        // The Done accessory (MauiDoneAccessoryView with DoneClicked → OnDoneClicked).
        MauiIosDatePickerDoneTarget* const done = [[MauiIosDatePickerDoneTarget alloc] init];
        done.handler = this;
        field.inputAccessoryView = maui::platform::ios::make_done_accessory(done, @selector(onDone:));
        field.inputAccessoryView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        objc_setAssociatedObject(field, &k_done_key, done, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // The IsOpen focus dance: EditingDidBegin/DidEnd drive the editing proxy (raw back-ref).
        MauiIosDatePickerEditingProxy* const editing = [[MauiIosDatePickerEditingProxy alloc] init];
        editing.handler = this;
        [field addTarget:editing action:@selector(onStarted:) forControlEvents:UIControlEventEditingDidBegin];
        [field addTarget:editing action:@selector(onEnded:) forControlEvents:UIControlEventEditingDidEnd];
        objc_setAssociatedObject(field, &k_editing_proxy_key, editing, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // ConnectHandler's initial dialog seed (the C# `picker.Date = dt.ToNSDate()` block) happens
        // through map_date on attach; on_done is the portable Done channel.
        platform.on_done = [this] { commit_date(*this); };
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        field.inputAccessoryView = nil;
        if (MauiIosDatePickerEditingProxy* const editing =
                (MauiIosDatePickerEditingProxy*)objc_getAssociatedObject(field, &k_editing_proxy_key))
        {
            [field removeTarget:editing action:@selector(onStarted:) forControlEvents:UIControlEventEditingDidBegin];
            [field removeTarget:editing action:@selector(onEnded:) forControlEvents:UIControlEventEditingDidEnd];
            editing.handler = nullptr;
        }
        objc_setAssociatedObject(field, &k_done_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(field, &k_editing_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view); // UpdateFormat routes into UpdateDate
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
        update_date(handler, view);
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            if (UIDatePicker* const dialog = dialog_of(as_field(platform->native)))
            {
                const auto minimum = view.minimum_date();
                dialog.minimumDate = minimum ? maui::platform::apple_shared::to_ns_date(*minimum) : nil;
            }
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            if (UIDatePicker* const dialog = dialog_of(as_field(platform->native)))
            {
                const auto maximum = view.maximum_date();
                dialog.maximumDate = maximum ? maui::platform::apple_shared::to_ns_date(*maximum) : nil;
            }
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
        // DatePickerExtensions.UpdateTextColor: an unset color keeps the system default; C# then
        // forces a re-render through UpdateDate ("HACK This forces the color to update").
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const maui::graphics::color color = view.text_color();
        if (color != maui::graphics::color{})
        {
            as_field(platform->native).textColor = maui::platform::ios::to_ui_color(color);
        }
        update_date(handler, view);
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).font = maui::platform::ios::to_ui_font(view.font(), UIFont.labelFontSize);
        }
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
        // TextFieldExtensions.UpdateCharacterSpacing: kern the field text (UpdateDate's tail call).
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

    void date_picker_handler::map_is_open(date_picker_handler& handler, i_date_picker& view)
    {
        // DatePickerHandler.MapIsOpen: BecomeFirstResponder when IsOpen (presenting the dialog), else
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

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
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

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_field(platform->native).frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
    }
} // namespace maui::core

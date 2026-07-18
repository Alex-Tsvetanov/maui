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

#include <TargetConditionals.h>

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

// MauiIosDatePicker  <=  Microsoft.Maui.Platform.MauiDatePicker — the UITextField the date picker presents.
// Its layoutSubviews override re-sizes any gradient/image background sublayer apply_background installed to
// the field's current bounds: apply_background runs before arrange, so without this a gradient/image
// BackgroundColor would be left zero-sized and invisible (a solid color needs no resize — it is the UIView
// backgroundColor property). Mirrors MauiDatePicker re-syncing its background on layout.
@interface MauiIosDatePicker : UITextField
@end

@implementation MauiIosDatePicker
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

#if TARGET_OS_MACCATALYST
// MauiMacDatePicker — the bare UIDatePicker DatePickerHandler.MacCatalyst.cs presents (there is no wrapping
// MauiDatePicker UITextField on Catalyst). It carries the SAME layoutSubviews hook the iOS MauiIosDatePicker
// does: apply_background installs a gradient/image background sublayer before arrange (bounds 0×0), so
// without a resize on each layout the brush fill would stay zero-/content-sized (rendering only text-width)
// instead of filling the picker's full frame — matching MAUI's full-width gradient DatePicker fill. A solid
// BackgroundColor needs no resize (it is the UIView backgroundColor property, which already fills bounds).
@interface MauiMacDatePicker : UIDatePicker
@end

@implementation MauiMacDatePicker
- (void)layoutSubviews
{
    [super layoutSubviews];
    maui::platform::ios::resize_background_layers((__bridge void*)self);
    maui::platform::ios::reapply_clip((__bridge void*)self);
}
@end
#endif // TARGET_OS_MACCATALYST

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

#if TARGET_OS_MACCATALYST
// Mac Catalyst uses a distinct handler shape (DatePickerHandler.MacCatalyst.cs): the platform view IS a
// bare compact UIDatePicker (Mode=Date, UTC) used directly — no MauiDatePicker UITextField, no bordered
// RoundedRect box. UpdateImmediately=true, so the wheel/segment value commits LIVE through
// UIControlEventValueChanged (the UIDatePickerProxy.OnValueChanged → SetVirtualViewDate; it also sets
// IsFocused=true). Raw handler back-ref cleared on disconnect (C# holds a WeakReference).
@interface MauiMacDatePickerValueProxy : NSObject
@property(nonatomic) maui::core::date_picker_handler* handler;
- (void)onValueChanged:(id)sender;
@end
#endif

namespace
{
    // Keys for the associated done-target / editing-proxy (iOS) or value-proxy (Catalyst) the native view
    // keeps alive (weak otherwise). k_editing_proxy_key doubles as the value-proxy key on Catalyst.
#if !TARGET_OS_MACCATALYST
    const char k_done_key = 0; // the Done accessory target — iOS only (the compact Catalyst picker has none)
#endif
    const char k_editing_proxy_key = 0;

#if TARGET_OS_MACCATALYST
    // On Mac Catalyst the platform view IS the UIDatePicker (no wrapping UITextField).
    UIDatePicker* as_date_picker(void* native)
    {
        return (__bridge UIDatePicker*)native;
    }

    // SetVirtualViewDate (DatePickerHandler.MacCatalyst.cs): VirtualView.Date = PlatformView.Date.ToDateTime()
    // — the control's coercion clamps into [MinimumDate, MaximumDate]. The compact UIDatePicker holds the
    // value directly; there is no inputView dialog to read.
    void commit_date_catalyst(maui::core::date_picker_handler& handler)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        view->set_date(maui::platform::apple_shared::to_date_time(as_date_picker(platform->native).date).date());
    }

    // MapDate on Catalyst (DatePickerHandler.MacCatalyst.cs → UpdateDate): push only the date onto the
    // UIDatePicker (its compact segments render the device-locale format). Format/TextColor/Font are no-ops
    // on Catalyst, so there is no field text to render.
    void update_date_catalyst(maui::core::date_picker_handler& handler, maui::core::i_date_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIDatePicker* const picker = as_date_picker(platform->native);
        const auto date = view.date();
        const maui::core::date_time target = date.value_or(maui::core::date_time::today());
        if (maui::platform::apple_shared::to_date_time(picker.date) != target)
        {
            [picker setDate:maui::platform::apple_shared::to_ns_date(target) animated:NO];
        }
    }
#else
    UITextField* as_field(void* native)
    {
        return (__bridge UITextField*)native;
    }

    UIDatePicker* dialog_of(UITextField* field)
    {
        return [field.inputView isKindOfClass:[UIDatePicker class]] ? (UIDatePicker*)field.inputView : nil;
    }

    // DatePickerExtensions.iOS renders the default / "d" / "D" date through NSDateFormatter in the DEVICE
    // locale (NOT the invariant en-US pattern format_date_time produces): "D" → NSDateFormatterFullStyle,
    // empty/"d" → SetLocalizedDateFormatFromTemplate("yMd") (the locale's short date forced to a 4-digit
    // year). TimeZone is pinned to GMT0 so the stored-UTC calendar day maps without a zone shift. This is
    // the device-locale parity fix — custom patterns (the else branches in update_date) still go through
    // the invariant format_date_time, matching C#'s ToString(format, InvariantCulture) for '/'-patterns.
    std::string localized_default_date(const maui::core::date_time& value, bool full_style)
    {
        NSDateFormatter* const formatter = [[NSDateFormatter alloc] init];
        formatter.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
        if (full_style)
        {
            formatter.dateStyle = NSDateFormatterFullStyle;
        }
        else
        {
            [formatter setLocalizedDateFormatFromTemplate:@"yMd"];
        }
        NSString* const text = [formatter stringFromDate:maui::platform::apple_shared::to_ns_date(value)];
        return text != nil ? std::string(text.UTF8String) : std::string{};
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
            // Device-locale render (DatePickerExtensions.iOS): empty/"d" → localized "yMd", "D" → Full style.
            text = localized_default_date(*date, format == "D");
        }
        else
        {
            // Custom pattern: the invariant/en-US formatter (C#: ToString(format, InvariantCulture) for the
            // '/'-patterns; locale-agnostic letters otherwise).
            text = maui::core::format_date_time(*date, format);
        }
        field.text = [NSString stringWithUTF8String:text.c_str()];
    }
#endif // TARGET_OS_MACCATALYST
} // namespace

#if !TARGET_OS_MACCATALYST
@implementation MauiIosDatePickerDoneTarget
- (void)onDone:(id)sender
{
    if (self.handler != nullptr)
    {
        commit_date(*self.handler); // OnDoneClicked → SetVirtualViewDate (+ resign, no session here)
    }
}
@end
#endif

#if TARGET_OS_MACCATALYST
@implementation MauiMacDatePickerValueProxy
- (void)onValueChanged:(id)sender
{
    // UIDatePickerProxy.OnValueChanged (DatePickerHandler.MacCatalyst.cs): UpdateImmediately=true, so a live
    // segment/wheel change commits straight back (SetVirtualViewDate) and marks the view focused.
    auto* const handler = self.handler;
    if (handler == nullptr)
    {
        return;
    }
    commit_date_catalyst(*handler);
    if (auto* const view = handler->virtual_view())
    {
        view->set_is_focused(true);
    }
}
@end
#endif

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

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). These
    // touch only UIView/UIControl properties, so they cast `native` to the common base — on iOS it is a
    // MauiIosDatePicker (UITextField), on Mac Catalyst a bare UIDatePicker (UIControl); both are UIViews.
    void date_picker_platform::update_visibility(maui::core::visibility value)
    {
        ((__bridge UIView*)native).hidden = value != maui::core::visibility::visible;
    }

    void date_picker_platform::update_opacity(double value)
    {
        ((__bridge UIView*)native).alpha = value;
    }

    void date_picker_platform::update_is_enabled(bool value)
    {
        ((__bridge UIControl*)native).enabled = static_cast<BOOL>(value);
    }

    void date_picker_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        ((__bridge UIView*)native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void date_picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void date_picker_platform::update_background(const maui::graphics::paint* value)
    {
#if TARGET_OS_MACCATALYST
        // On Catalyst the platform view is a bare UIDatePicker (no RoundedRect bezel). A solid fill goes to
        // the UIView backgroundColor property; gradient/image paints ride the shared backing-layer machinery;
        // a null paint clears the override back to the system default.
        UIView* const picker = (__bridge UIView*)native;
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            picker.backgroundColor = maui::platform::ios::to_ui_color(solid->color());
        }
        else if (value != nullptr)
        {
            picker.backgroundColor = nil;
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            picker.backgroundColor = nil;
        }
        return;
#else
        // The MauiDatePicker is a RoundedRect UITextField. A solid BackgroundColor must go to the UIView
        // backgroundColor PROPERTY (not the backing layer): the rounded-rect bezel is drawn over a layer
        // fill (so it peeks only at the corners), whereas the view property suppresses the bezel and fills
        // the field flat — matching MAUI's solid DatePicker fill. Gradient/image paints use the shared
        // layer machinery; a null paint clears the override back to the system default.
        UITextField* const field = as_field(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            field.borderStyle = UITextBorderStyleRoundedRect;
            field.backgroundColor = maui::platform::ios::to_ui_color(solid->color());
        }
        else if (value != nullptr)
        {
            // Gradient/image: the brush fill sits at the BOTTOM of the layer (zPosition -1, behind the
            // thumb/content per InsertBackgroundLayer). The RoundedRect bezel is drawn ABOVE it and would
            // hide all but the field's edge, so drop the bezel to None — the gradient then fills the field
            // flat (text on top), matching MAUI's gradient DatePicker fill.
            field.borderStyle = UITextBorderStyleNone;
            field.backgroundColor = nil;
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            field.borderStyle = UITextBorderStyleRoundedRect;
            field.backgroundColor = nil;
        }
#endif // TARGET_OS_MACCATALYST
    }

    std::unique_ptr<date_picker_platform> date_picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<date_picker_platform>();
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs CreatePlatformView: the platform view IS a bare, compact
        // UIDatePicker { Mode = Date, TimeZone = UTC } used directly — no wrapping MauiDatePicker UITextField
        // and no RoundedRect box. On Catalyst UIKit renders this as an inline segmented field with bare,
        // left-aligned, device-locale text (removing the bordered field boxes the reused iOS recipe drew).
        UIDatePicker* const picker = [[MauiMacDatePicker alloc] initWithFrame:CGRectZero];
        picker.datePickerMode = UIDatePickerModeDate;
        picker.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        platform->native = (__bridge_retained void*)picker; // the void* slot owns one reference
        return platform;
#else
        // MauiDatePicker(): RoundedRect field; UIDatePicker { Mode = Date, TimeZone = UTC, Wheels }
        // as the inputView; Button accessibility traits.
        UITextField* const field = [[MauiIosDatePicker alloc] initWithFrame:CGRectZero];
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
#endif // TARGET_OS_MACCATALYST
    }

    void date_picker_handler::on_connect_handler(date_picker_platform& platform)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs ConnectHandler: no Done accessory (the compact UIDatePicker has
        // none). The UIDatePickerProxy wires UIControlEventValueChanged → live SetVirtualViewDate. The full
        // C# EditingDidBegin/window-close IsOpen dance (over the picker's internal segment text fields) is a
        // focus nicety not visible in a static capture and is not replicated here; the value-commit + focus
        // is. on_done stays wired to the same commit for the portable Done channel (headless parity).
        UIDatePicker* const picker = as_date_picker(platform.native);
        MauiMacDatePickerValueProxy* const value_proxy = [[MauiMacDatePickerValueProxy alloc] init];
        value_proxy.handler = this;
        [picker addTarget:value_proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        objc_setAssociatedObject(picker, &k_editing_proxy_key, value_proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = [this] { commit_date_catalyst(*this); };
#else
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
#endif // TARGET_OS_MACCATALYST
    }

    void date_picker_handler::on_disconnect_handler(date_picker_platform& platform)
    {
#if TARGET_OS_MACCATALYST
        UIDatePicker* const picker = as_date_picker(platform.native);
        if (MauiMacDatePickerValueProxy* const value_proxy =
                (MauiMacDatePickerValueProxy*)objc_getAssociatedObject(picker, &k_editing_proxy_key))
        {
            [picker removeTarget:value_proxy
                          action:@selector(onValueChanged:)
                forControlEvents:UIControlEventValueChanged];
            value_proxy.handler = nullptr;
        }
        objc_setAssociatedObject(picker, &k_editing_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
#else
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
#endif // TARGET_OS_MACCATALYST
    }

    void date_picker_handler::map_format(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs MapFormat still routes into UpdateDate, but on Catalyst UpdateDate
        // only pushes the value onto the UIDatePicker — the custom Format string is NOT rendered (the compact
        // segments always show the device-locale format), so the Format row reads e.g. "2.07.2026".
        update_date_catalyst(handler, view);
#else
        update_date(handler, view); // UpdateFormat routes into UpdateDate
#endif
    }

    void date_picker_handler::map_date(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        update_date_catalyst(handler, view); // MapDate → UpdateDate: push the value onto the UIDatePicker
#else
        update_date(handler, view);
#endif
    }

    void date_picker_handler::map_minimum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
#if TARGET_OS_MACCATALYST
            UIDatePicker* const dialog = as_date_picker(platform->native);
#else
            UIDatePicker* const dialog = dialog_of(as_field(platform->native));
            if (dialog == nil)
            {
                return;
            }
#endif
            const auto minimum = view.minimum_date();
            dialog.minimumDate = minimum ? maui::platform::apple_shared::to_ns_date(*minimum) : nil;
        }
    }

    void date_picker_handler::map_maximum_date(date_picker_handler& handler, i_date_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
#if TARGET_OS_MACCATALYST
            UIDatePicker* const dialog = as_date_picker(platform->native);
#else
            UIDatePicker* const dialog = dialog_of(as_field(platform->native));
            if (dialog == nil)
            {
                return;
            }
#endif
            const auto maximum = view.maximum_date();
            dialog.maximumDate = maximum ? maui::platform::apple_shared::to_ns_date(*maximum) : nil;
        }
    }

    void date_picker_handler::map_text_color(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs MapTextColor is EMPTY — the compact UIDatePicker owns its text
        // color, so the TextColor row stays the system default (black) even when the demo sets Red.
        (void)handler;
        (void)view;
#else
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
#endif
    }

    void date_picker_handler::map_font(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs MapFont is EMPTY — the compact UIDatePicker owns its font.
        (void)handler;
        (void)view;
#else
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).font =
                maui::platform::ios::to_ui_font(view.font(), maui::platform::ios::default_text_font_size());
        }
#endif
    }

    void date_picker_handler::map_character_spacing(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs MapCharacterSpacing is EMPTY (no field text to kern).
        (void)handler;
        (void)view;
#else
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
#endif
    }

    void date_picker_handler::map_is_open(date_picker_handler& handler, i_date_picker& view)
    {
#if TARGET_OS_MACCATALYST
        // DatePickerHandler.MacCatalyst.cs MapIsOpen is EMPTY — the compact UIDatePicker presents its own
        // popover on tap; there is no BecomeFirstResponder dance to drive.
        (void)handler;
        (void)view;
#else
        // DatePickerHandler.MapIsOpen: BecomeFirstResponder when IsOpen (presenting the dialog), else
        // ResignFirstResponder. On a real device this fires EditingDidBegin/DidEnd, which the editing
        // proxy turns into the IsOpen + IsFocused write-back.
        if (view.is_open())
        {
            (void)focus_native_view(handler.native_view());
        }
        else
        {
            unfocus_native_view(handler.native_view());
        }
#endif
    }

    maui::graphics::size date_picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // sizeThatFits: / .frame are UIView members — on Catalyst `native` is the UIDatePicker, on iOS the
        // MauiIosDatePicker (UITextField); cast to the common base so the measure/arrange path targets the
        // real platform view on both.
        const CGSize constraint = CGSizeMake(width_constraint > 0 ? width_constraint : CGFLOAT_MAX,
                                             height_constraint > 0 ? height_constraint : CGFLOAT_MAX);
#if !TARGET_OS_MACCATALYST
        // A gradient/image background dropped the field's borderStyle to None (update_background — so the
        // fill renders flat), but borderStyle also drives sizeThatFits's HEIGHT: RoundedRect adds ~8pt of
        // vertical padding, None gives the bare text height. MAUI keeps the gradient DatePicker at the full
        // RoundedRect height (the gradient fills it) — the port's collapsed to text-height, so every gradient
        // field below the first rendered half-tall and drifted the whole date_picker page (measured 34pt vs
        // MAUI 34pt for the first field, 18.7pt for the gradient ones). Measure at the ROUNDEDRECT height
        // regardless of the visible border: temporarily restore it around sizeThatFits (synchronous — no
        // intermediate render), so the flat-fill render is unchanged.
        UITextField* const field = as_field(platform->native);
        const UITextBorderStyle visible_border = field.borderStyle;
        const bool restore_border = visible_border == UITextBorderStyleNone;
        if (restore_border)
        {
            field.borderStyle = UITextBorderStyleRoundedRect;
        }
        const CGSize fitting = [field sizeThatFits:constraint];
        if (restore_border)
        {
            field.borderStyle = visible_border;
        }
#else
        const CGSize fitting = [((__bridge UIView*)platform->native) sizeThatFits:constraint];
#endif
        return {fitting.width, fitting.height};
    }

    void date_picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        ((__bridge UIView*)platform->native).frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
        // Re-size gradient/image background sublayers to the arranged bounds — the Mac Catalyst bare
        // UIDatePicker has no layoutSubviews override (unlike the iOS MauiIosDatePicker subclass), so a
        // gradient Background otherwise stays at the tiny initial size. Idempotent on iOS.
        maui::platform::ios::resize_background_layers(platform->native);
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void date_picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

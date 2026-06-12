// picker_handler — iOS (UIKit) platform recipe. The managed platform view is a real UITextField (the
// MauiPicker shape: a rounded-rect field whose inputView is a UIPickerView and whose
// inputAccessoryView is the Done toolbar), the items reload the wheel through a UIPickerViewModel
// port reading the i_item_delegate face, and the Done tap commits the pending row back through
// i_picker::set_selected_index. Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from PickerHandler.iOS.cs + Platform/iOS/MauiPicker.cs + PickerExtensions.cs:
//   CreatePlatformView = new MauiPicker(pickerView) { BorderStyle = RoundedRect, InputView,
//   InputAccessoryView = MauiDoneAccessoryView(OnDone), AccessibilityTraits = Button };
//   PickerSource (UIPickerViewModel) = MauiPickerSource below (GetRowsInComponent → get_count,
//   GetTitle → get_item, Selected → tracks SelectedIndex; UpdateImmediately is a platform-specific
//   not ported); OnDone = FinishSelectItem (an unset row with items present commits row 0, then
//   UpdatePickerFromPickerSource writes text + the virtual selection and resigns first responder);
//   map bodies = PickerExtensions.UpdatePicker / UpdatePickerTitle (attributed placeholder in
//   TitleColor) / UpdateTextColor / UpdateFont / UpdateCharacterSpacing / alignment updates.
// Not ported here (deferred): IsOpen + the focus/first-responder dance (EditingDidBegin/DidEnd set
// IsFocused/IsOpen — focus subsystem), the touch-dismiss window gesture (needs a UIWindow), the
// VoiceOver focus notifications, and the EditingChanged keyboard-typing reset (no editing sessions in
// the spawned test process — see button_ios_tests.mm).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_done_accessory.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// MauiPickerSource  <=  Microsoft.Maui.Handlers.PickerSource (UIPickerViewModel): the wheel's data
// source + delegate, reading rows through the virtual view's i_item_delegate face and tracking the
// pending (not yet committed) row.
@interface MauiPickerSource : NSObject <UIPickerViewDataSource, UIPickerViewDelegate>
@property(nonatomic) maui::core::picker_handler* handler;
@property(nonatomic) NSInteger selectedIndex;
@end

@implementation MauiPickerSource
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView*)pickerView
{
    return 1; // GetComponentCount
}

- (NSInteger)pickerView:(UIPickerView*)pickerView numberOfRowsInComponent:(NSInteger)component
{
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    return view != nullptr ? view->get_count() : 0; // GetRowsInComponent
}

- (NSString*)pickerView:(UIPickerView*)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view == nullptr)
    {
        return @"";
    }
    const std::string title = view->get_item(static_cast<int>(row)); // GetTitle
    NSString* const result = [NSString stringWithUTF8String:title.c_str()];
    return result != nil ? result : @"";
}

- (void)pickerView:(UIPickerView*)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    self.selectedIndex = row; // Selected (UpdateImmediately is a platform-specific, not ported)
}
@end

// Obj-C trampoline for the Done accessory tap — the MauiDoneAccessoryView callback feeding
// PickerHandler.OnDone → FinishSelectItem.
@interface MauiPickerDoneTarget : NSObject
@property(nonatomic) maui::core::picker_handler* handler;
- (void)onDone:(id)sender;
@end

namespace
{
    // Keys for the associated objects the UITextField keeps alive (targets/delegates are weak).
    const char k_source_key = 0;
    const char k_done_key = 0;

    UITextField* as_field(void* native)
    {
        return (__bridge UITextField*)native;
    }

    UIPickerView* wheel_of(UITextField* field)
    {
        return [field.inputView isKindOfClass:[UIPickerView class]] ? (UIPickerView*)field.inputView : nil;
    }

    MauiPickerSource* source_of(UITextField* field)
    {
        return (MauiPickerSource*)objc_getAssociatedObject(field, &k_source_key);
    }

    NSString* to_ns_string(std::string_view value)
    {
        const std::string utf8(value);
        NSString* const result = [NSString stringWithUTF8String:utf8.c_str()];
        return result != nil ? result : @"";
    }

    // PickerExtensions.UpdatePickerTitle: the Title (in TitleColor) as the attributed placeholder.
    void update_picker_title(UITextField* field, const maui::core::i_picker& view)
    {
        const maui::graphics::color title_color = view.title_color();
        UIColor* const foreground =
            title_color != maui::graphics::color{} ? maui::platform::ios::to_ui_color(title_color) : nil;
        NSDictionary* const attributes = foreground != nil ? @{NSForegroundColorAttributeName : foreground} : nil;
        field.attributedPlaceholder = [[NSAttributedString alloc] initWithString:to_ns_string(view.title())
                                                                      attributes:attributes];
    }

    // PickerExtensions.UpdatePicker(platformPicker, picker, newSelectedIndex): refresh the display
    // text (the Title placeholder shows at -1), reload the wheel, then write the selection back to
    // the virtual view + the source (skipped while empty) and spin the wheel to the row.
    void update_picker(maui::core::picker_handler& handler, maui::core::i_picker& view, int selected_index)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        if (selected_index != -1)
        {
            const std::string item = view.get_item(selected_index);
            field.text = [NSString stringWithUTF8String:item.c_str()];
        }
        else
        {
            field.text = nil;
            update_picker_title(field, view);
        }

        UIPickerView* const wheel = wheel_of(field);
        [wheel reloadAllComponents];

        if (view.get_count() == 0)
        {
            return;
        }

        view.set_selected_index(selected_index); // picker.SelectedIndex = selectedIndex
        if (MauiPickerSource* const source = source_of(field))
        {
            source.selectedIndex = selected_index;
        }
        [wheel selectRow:std::max(selected_index, 0) inComponent:0 animated:YES];
    }

    // PickerHandler.FinishSelectItem: an unset (-1) pending row with items present commits row 0,
    // then UpdatePickerFromPickerSource writes the text + the virtual selection. (The C# tail also
    // resigns first responder — no editing sessions in the spawned test process.)
    void finish_select_item(maui::core::picker_handler& handler, int row)
    {
        auto* view = handler.virtual_view();
        auto* platform = handler.typed_platform_view();
        if (view == nullptr || platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        MauiPickerSource* const source = source_of(field);
        if (row == -1 && view->get_count() > 0)
        {
            // UpdatePickerSelectedIndex(0).
            row = 0;
            if (source != nil)
            {
                source.selectedIndex = 0;
            }
            [wheel_of(field) selectRow:0 inComponent:0 animated:YES];
        }
        // UpdatePickerFromPickerSource: Text = GetItem(selectedIndex); VirtualView.SelectedIndex = it.
        const std::string item = view->get_item(row);
        field.text = [NSString stringWithUTF8String:item.c_str()];
        view->set_selected_index(row);
    }
} // namespace

@implementation MauiPickerDoneTarget
- (void)onDone:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* const platform = self.handler->typed_platform_view();
    UITextField* const field = platform != nullptr ? as_field(platform->native) : nil;
    MauiPickerSource* const source = field != nil ? source_of(field) : nil;
    finish_select_item(*self.handler, source != nil ? static_cast<int>(source.selectedIndex) : -1);
}
@end

namespace maui::core
{
    picker_platform::~picker_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void picker_platform::update_visibility(maui::core::visibility value)
    {
        as_field(native).hidden = value != maui::core::visibility::visible;
    }

    void picker_platform::update_opacity(double value)
    {
        as_field(native).alpha = value;
    }

    void picker_platform::update_is_enabled(bool value)
    {
        as_field(native).enabled = static_cast<BOOL>(value);
    }

    void picker_platform::update_automation_id(std::string_view value)
    {
        as_field(native).accessibilityIdentifier = to_ns_string(value);
    }

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<picker_platform>();
        // CreatePlatformView: new MauiPicker(new UIPickerView()) { BorderStyle = RoundedRect,
        // InputView = pickerView, InputAccessoryView = MauiDoneAccessoryView, Button traits }.
        UITextField* const field = [[UITextField alloc] initWithFrame:CGRectZero];
        field.borderStyle = UITextBorderStyleRoundedRect;
        UIPickerView* const wheel = [[UIPickerView alloc] initWithFrame:CGRectZero];
        field.inputView = wheel;
        field.inputView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        field.accessibilityTraits = UIAccessibilityTraitButton;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        // _pickerView.Model = new PickerSource(this).
        MauiPickerSource* const source = [[MauiPickerSource alloc] init];
        source.handler = this;
        source.selectedIndex = -1;
        UIPickerView* const wheel = wheel_of(field);
        wheel.dataSource = source;
        wheel.delegate = source;
        objc_setAssociatedObject(field, &k_source_key, source, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // InputAccessoryView = new MauiDoneAccessoryView(OnDone): a toolbar whose Done bar button
        // taps through the trampoline into FinishSelectItem.
        MauiPickerDoneTarget* const done = [[MauiPickerDoneTarget alloc] init];
        done.handler = this;
        field.inputAccessoryView = maui::platform::ios::make_done_accessory(done, @selector(onDone:));
        field.inputAccessoryView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
        objc_setAssociatedObject(field, &k_done_key, done, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // The portable Done channel (the headless twin's seam): commits the given row (or the
        // source's pending one at -1 → row 0, FinishSelectItem).
        platform.on_done = [this](int row) {
            auto* typed = typed_platform_view();
            if (typed == nullptr || typed->native == nullptr)
            {
                return;
            }
            MauiPickerSource* const pending_source = source_of(as_field(typed->native));
            if (row != -1 && pending_source != nil)
            {
                pending_source.selectedIndex = row;
            }
            finish_select_item(*this, row);
        };
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        UIPickerView* const wheel = wheel_of(field);
        wheel.dataSource = nil;
        wheel.delegate = nil;
        field.inputAccessoryView = nil;
        objc_setAssociatedObject(field, &k_source_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(field, &k_done_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // Reload -> UpdatePicker(picker)
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // UpdateSelectedIndex
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            update_picker_title(as_field(platform->native), view); // UpdateTitle
        }
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            update_picker_title(as_field(platform->native), view); // UpdateTitleColor
        }
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
    {
        // PickerExtensions.UpdateTextColor: TextColor?.ToPlatform() (an unset color keeps the system
        // default).
        if (auto* platform = handler.typed_platform_view())
        {
            const maui::graphics::color color = view.text_color();
            if (color != maui::graphics::color{})
            {
                as_field(platform->native).textColor = maui::platform::ios::to_ui_color(color);
            }
        }
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).font = maui::platform::ios::to_ui_font(view.font(), UIFont.labelFontSize);
        }
    }

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
    {
        // TextFieldExtensions.UpdateCharacterSpacing: kern the text AND the placeholder (each only
        // when WithCharacterSpacing returns a value).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        const double spacing = view.character_spacing();
        NSAttributedString* const text_attr =
            maui::platform::ios::with_character_spacing(field.attributedText, spacing);
        if (text_attr != nil)
        {
            field.attributedText = text_attr;
        }
        NSAttributedString* const placeholder_attr =
            maui::platform::ios::with_character_spacing(field.attributedPlaceholder, spacing);
        if (placeholder_attr != nil)
        {
            field.attributedPlaceholder = placeholder_attr;
        }
    }

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).textAlignment =
                maui::platform::ios::to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_field(platform->native).contentVerticalAlignment =
                maui::platform::ios::to_ui_control_content_vertical_alignment(view.vertical_text_alignment());
        }
    }

    maui::graphics::size picker_handler::get_desired_size(double width_constraint, double height_constraint) const
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

    void picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_field(platform->native).frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
    }
} // namespace maui::core

// entry_handler — iOS (UIKit) platform recipe. The managed platform view is a real UITextField (a
// MauiIosTextField subclass for the selection-changed channel), the value properties map to it, and
// native edits flow back through a target-action + UITextFieldDelegate proxy to
// i_entry::send_text_changed(old, new) / send_completed(). Compiled as Objective-C++ with ARC only for
// the `ios` backend.
//
// Ported DIRECTLY from EntryHandler.iOS.cs + Platform/iOS/TextFieldExtensions.cs +
// TextInputExtensions.cs + MauiTextField.cs (the same oracles the AppKit twin in
// src/platform/apple/entry_handler.mm was adapted from — UIKit needs no adaptation):
//   CreatePlatformView = MauiTextField { BorderStyle = RoundedRect, ClipsToBounds = true };
//   MauiTextFieldProxy: EditingChanged → cursor write-back + UpdateText (send_text_changed),
//   EditingDidBegin → re-apply the virtual cursor/selection, EditingDidEnd → UpdateText,
//   ShouldReturn → resign + Completed (send_completed) + false, ShouldChangeCharacters →
//   TextWithinMaxLength (the REAL native max-length gate), and the MauiTextField SelectedTextRange
//   override → OnSelectionChanged (CursorPosition/SelectionLength write-back).
//   Map bodies below = UpdateText/UpdateTextColor/UpdateIsPassword (first-responder state dance)/
//   UpdatePlaceholder/UpdateIsReadOnly/UpdateMaxLength/UpdateFont/UpdateCharacterSpacing/
//   UpdateHorizontal+VerticalTextAlignment/UpdateIsTextPredictionEnabled/UpdateIsSpellCheckEnabled/
//   UpdateReturnType (returnKeyType — REAL on iOS)/UpdateClearButtonVisibility (clearButtonMode — REAL
//   on iOS)/UpdateCursorPosition/UpdateSelectionLength (selectedTextRange, both directions).
// Not ported here (deferred): MapKeyboard + AddMauiDoneAccessoryView (the Keyboard subsystem is out of
// the contract; ShouldReturn's KeyboardAutoManager.GoToNextResponderOrResign collapses to its resign
// arm), UpdateClearButtonColor (tints UIKit's private clearButton subview via KVC), TextPropertySet
// (the port has no native-programmatic-text channel; map_text is the only programmatic writer), the
// iOS-26 ShouldChangeCharactersInRanges variant (this SDK's delegate channel is the classic single
// range), MapBackground, and IsFocused (focus subsystem).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the UITextField's editing events + delegate callbacks to the C++ handler's
// virtual view. Ports EntryHandler.MauiTextFieldProxy — and, like the AppKit twin's delegate, tracks the
// previous string so an edit can report the *old* value (C# reads the virtual view's Text; here the
// field is the source of truth and the trampoline diffs it).
@interface MauiIosEntryProxy : NSObject <UITextFieldDelegate>
@property(nonatomic) maui::core::entry_handler* handler;
@property(nonatomic, copy) NSString* previousText;
- (void)onEditingChanged:(id)sender;
- (void)onEditingDidBegin:(id)sender;
- (void)onEditingDidEnd:(id)sender;
- (void)mauiSelectionChangedFrom:(UITextField*)field;
@end

// MauiIosTextField  <=  Microsoft.Maui.Platform.MauiTextField — the UITextField subclass whose
// SelectedTextRange override raises the selection-changed channel (UIKit has no public notification for
// it). The proxy reference is weak: the field RETAINS the proxy via an associated object (set in
// on_connect_handler), so this back-reference must not create a cycle.
@interface MauiIosTextField : UITextField
@property(nonatomic, weak) MauiIosEntryProxy* mauiProxy;
@end

@implementation MauiIosTextField
- (void)setSelectedTextRange:(UITextRange*)selectedTextRange
{
    UITextRange* const old = [super selectedTextRange];
    [super setSelectedTextRange:selectedTextRange];
    // MauiTextField.SelectedTextRange: raise SelectionChanged when the range changed (C# compares the
    // UITextPosition references — UIKit mints fresh position objects, so this fires liberally and the
    // proxy's write-back guards on actual value differences).
    if (old.start != selectedTextRange.start || old.end != selectedTextRange.end)
    {
        [self.mauiProxy mauiSelectionChangedFrom:self];
    }
}
@end

namespace
{
    // Key for the associated MauiIosEntryProxy kept alive by the UITextField (both UIControl targets and
    // the `delegate` are held weakly — the target-action convention).
    const char k_proxy_key = 0;

    UITextField* as_field(void* native)
    {
        return (__bridge UITextField*)native;
    }

    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_control_content_vertical_alignment;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::to_ui_return_key_type;
    using maui::platform::ios::with_character_spacing;

    // TextInputExtensions.GetCursorPosition (cursorOffset = 0): the caret offset from the start of the
    // document, floored at 0 (0 when there is no selection — e.g. the field is not being edited).
    int get_cursor_position(UITextField* field)
    {
        UITextPosition* const zero = [field positionFromPosition:field.beginningOfDocument offset:0];
        UITextPosition* const current = field.selectedTextRange != nil ? field.selectedTextRange.start : zero;
        if (current == nil)
        {
            return 0;
        }
        const NSInteger offset = [field offsetFromPosition:field.beginningOfDocument toPosition:current];
        return offset > 0 ? static_cast<int>(offset) : 0;
    }

    // TextInputExtensions.GetSelectedTextLength: the selected span's length (0 with no selection).
    int get_selected_text_length(UITextField* field)
    {
        UITextRange* const range = field.selectedTextRange;
        if (range == nil)
        {
            return 0;
        }
        return static_cast<int>([field offsetFromPosition:range.start toPosition:range.end]);
    }

    // TextFieldExtensions.GetSelectionStart: resolve the virtual CursorPosition to a text position
    // (clamped into the document — out of range falls to the end), writing the clamped offset back when
    // it differs.
    UITextPosition* get_selection_start(UITextField* field, maui::core::i_entry& view, int& start_offset)
    {
        const int cursor_position = view.cursor_position();
        UITextPosition* start = [field positionFromPosition:field.beginningOfDocument offset:cursor_position];
        if (start == nil)
        {
            start = field.endOfDocument;
        }
        const auto offset = static_cast<int>([field offsetFromPosition:field.beginningOfDocument toPosition:start]);
        start_offset = std::max(0, offset);
        if (start_offset != cursor_position)
        {
            view.set_cursor_position(start_offset);
        }
        return start;
    }

    // TextFieldExtensions.GetSelectionEnd: resolve the virtual SelectionLength from the (possibly
    // clamped) start, writing the achievable length back when it differs.
    UITextPosition* get_selection_end(UITextField* field, maui::core::i_entry& view, UITextPosition* start,
                                      int start_offset)
    {
        const int selection_length = view.selection_length();
        const int text_length = static_cast<int>(field.text != nil ? field.text.length : 0);
        // "Get the desired range in respect to the actual length of the text we are working with."
        const int span = std::min(text_length - view.cursor_position(), selection_length);
        UITextPosition* end = [field positionFromPosition:start offset:span];
        if (end == nil)
        {
            end = start;
        }
        const auto raw_end = static_cast<int>([field offsetFromPosition:field.beginningOfDocument toPosition:end]);
        const int end_offset = std::max(start_offset, raw_end);
        const int new_selection_length = std::max(0, end_offset - start_offset);
        if (new_selection_length != selection_length)
        {
            view.set_selection_length(new_selection_length);
        }
        return end;
    }

    // TextFieldExtensions.UpdateCursorSelection: "Updates both the IEntry.CursorPosition and
    // IEntry.SelectionLength properties", then moves the native selection to match.
    void update_cursor_selection(UITextField* field, maui::core::i_entry& view)
    {
        if (view.is_read_only())
        {
            return;
        }
        int start_offset = 0;
        UITextPosition* const start = get_selection_start(field, view, start_offset);
        UITextPosition* const end = get_selection_end(field, view, start, start_offset);
        field.selectedTextRange = [field textRangeFromPosition:start toPosition:end];
    }

    // TextFieldExtensions.UpdatePlaceholder: a null placeholder clears the attributed placeholder; a
    // PlaceholderColor builds it with the foreground attribute, otherwise the plain attributed string
    // keeps the system's muted placeholder rendering. The port collapses C#'s nullables the same way the
    // AppKit twin does: an EMPTY placeholder string clears (string_view cannot be null), and the
    // default-constructed color (opaque black) counts as "no explicit color". C#'s trailing
    // `AttributedPlaceholder.WithCharacterSpacing(...)` DISCARDS its result, so — bug-faithfully — no
    // kerning is applied here; UpdateCharacterSpacing's placeholder branch is the kerning writer.
    void update_placeholder(UITextField* field, const maui::core::i_entry& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        if (text == nil || text.length == 0)
        {
            field.attributedPlaceholder = nil;
            return;
        }
        const maui::graphics::color color = view.placeholder_color();
        const bool explicit_color = color != maui::graphics::color{}; // != the default (opaque black)
        if (!explicit_color)
        {
            field.attributedPlaceholder = [[NSAttributedString alloc] initWithString:text];
            return;
        }
        field.attributedPlaceholder =
            [[NSAttributedString alloc] initWithString:text
                                            attributes:@{NSForegroundColorAttributeName : to_ui_color(color)}];
    }

    // TextFieldExtensions.UpdateMaxLength: trim the attributed text to MaxLength
    // (AttributedStringExtensions.TrimToMaxLength — a no-op for a negative cap or no overflow).
    void apply_max_length(UITextField* field, const maui::core::i_entry& view)
    {
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSAttributedString* const current = field.attributedText;
        if (current != nil && current.length > static_cast<NSUInteger>(max_length))
        {
            field.attributedText =
                [current attributedSubstringFromRange:NSMakeRange(0, static_cast<NSUInteger>(max_length))];
        }
    }
} // namespace

@implementation MauiIosEntryProxy
// The shared old/new diff: report the field's text to the virtual view when it actually changed (the
// C# UpdateText path is a property set, so an equal value is a no-op — this guard is its analog; it
// also keeps the IsPassword remove-and-reinsert dance silent, like C#'s SuppressTextPropertySet).
- (void)mauiSyncTextFrom:(UITextField*)field
{
    // Snapshot the nullable getters once, then coalesce (the analyzer must see one stable value).
    NSString* const previous = self.previousText;
    NSString* const current = field.text;
    NSString* const old_value = previous != nil ? previous : @"";
    NSString* const new_value = current != nil ? current : @"";
    if ([old_value isEqualToString:new_value])
    {
        return;
    }
    self.previousText = new_value;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            // UTF8String is _Nullable; guard before binding to the std::string_view parameters.
            const char* const old_utf8 = old_value.UTF8String;
            const char* const new_utf8 = new_value.UTF8String;
            view->send_text_changed(old_utf8 != nullptr ? old_utf8 : "", new_utf8 != nullptr ? new_utf8 : "");
        }
    }
}

- (void)onEditingChanged:(id)sender
{
    auto* const field = (UITextField*)sender;
    if (self.handler == nullptr || field == nil)
    {
        return;
    }
    if (auto* view = self.handler->virtual_view())
    {
        // "Update cursor position before updating text so that when TextChanged event fires, the
        // CursorPosition property reflects the current native cursor position" (OnEditingChanged).
        const int cursor = get_cursor_position(field);
        if (view->cursor_position() != cursor)
        {
            view->set_cursor_position(cursor);
        }
        [self mauiSyncTextFrom:field];
    }
}

- (void)onEditingDidBegin:(id)sender
{
    auto* const field = (UITextField*)sender;
    if (self.handler == nullptr || field == nil)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    if (view == nullptr)
    {
        return;
    }
    // OnEditingBegan: re-apply the virtual selection now that the field has an editing session
    // (IsFocused = true has no port surface yet — the focus subsystem is deferred).
    if (view->selection_length() > 0)
    {
        maui::core::entry_handler::map_selection_length(*self.handler, *view);
    }
    else
    {
        maui::core::entry_handler::map_cursor_position(*self.handler, *view);
    }
}

- (void)onEditingDidEnd:(id)sender
{
    auto* const field = (UITextField*)sender;
    if (self.handler == nullptr || field == nil)
    {
        return;
    }
    // OnEditingEnded: one final text sync (Completed is ShouldReturn's job, not end-of-edit's).
    [self mauiSyncTextFrom:field];
}

- (BOOL)textFieldShouldReturn:(UITextField*)textField
{
    // OnShouldReturn: KeyboardAutoManager.GoToNextResponderOrResign collapses to its resign arm (the
    // next-responder walk belongs to the deferred Keyboard subsystem), then Completed, then false so
    // UIKit inserts no newline.
    [textField resignFirstResponder];
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_completed();
        }
    }
    return NO;
}

- (BOOL)textField:(UITextField*)textField
    shouldChangeCharactersInRange:(NSRange)range
                replacementString:(NSString*)string
{
    // OnShouldChangeCharacters → ITextInputExtensions.TextWithinMaxLength: the REAL native max-length
    // gate (typing/pasting beyond MaxLength is rejected at the keystroke).
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view == nullptr)
    {
        return YES;
    }
    NSString* const current = textField.text != nil ? textField.text : @"";
    const NSUInteger current_length = current.length;
    // "fix a crash on undo"
    if (range.location + range.length > current_length)
    {
        return NO;
    }
    const int max_length = view->max_length();
    if (max_length < 0)
    {
        return YES;
    }
    const NSUInteger add_length = string != nil ? string.length : 0;
    const NSUInteger new_length = current_length + add_length - range.length;
    const bool should_change = std::cmp_less_equal(new_length, max_length);
    // "cut text when user is pasting a text longer that maxlength": C# writes the truncated paste onto
    // the virtual Text (which maps back to the field); the port writes the field — its source of truth —
    // and reports the change through the same diff channel.
    const bool replacement_blank =
        string == nil ||
        [string stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet].length == 0;
    if (!should_change && !replacement_blank && string.length >= static_cast<NSUInteger>(max_length))
    {
        textField.text = [string substringToIndex:static_cast<NSUInteger>(max_length)];
        [self mauiSyncTextFrom:textField];
    }
    return should_change ? YES : NO;
}

- (void)mauiSelectionChangedFrom:(UITextField*)field
{
    // OnSelectionChanged: push the native caret/selection back onto the virtual view (guarded on actual
    // differences, exactly like C#).
    if (self.handler == nullptr || field == nil)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    if (view == nullptr)
    {
        return;
    }
    const int cursor = get_cursor_position(field);
    const int selected = get_selected_text_length(field);
    if (view->cursor_position() != cursor)
    {
        view->set_cursor_position(cursor);
    }
    if (view->selection_length() != selected)
    {
        view->set_selection_length(selected);
    }
}
@end

namespace maui::core
{
    entry_platform::~entry_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void entry_platform::update_visibility(maui::core::visibility value)
    {
        as_field(native).hidden = value != maui::core::visibility::visible;
    }

    void entry_platform::update_opacity(double value)
    {
        as_field(native).alpha = value;
    }

    void entry_platform::update_is_enabled(bool value)
    {
        // ViewExtensions.UpdateIsEnabled's UIControl branch (a UITextField is a UIControl).
        as_field(native).enabled = static_cast<BOOL>(value);
    }

    void entry_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_field(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<entry_platform> entry_handler::create_platform_view()
    {
        auto platform = std::make_unique<entry_platform>();
        // CreatePlatformView: new MauiTextField { BorderStyle = RoundedRect, ClipsToBounds = true }.
        // AddMauiDoneAccessoryView (the keyboard Done bar) is deferred with the Keyboard subsystem.
        MauiIosTextField* const field = [[MauiIosTextField alloc] initWithFrame:CGRectZero];
        field.borderStyle = UITextBorderStyleRoundedRect;
        field.clipsToBounds = YES;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void entry_handler::on_connect_handler(entry_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        MauiIosEntryProxy* const proxy = [[MauiIosEntryProxy alloc] init];
        proxy.handler = this;
        proxy.previousText = field.text != nil ? field.text : @"";
        // MauiTextFieldProxy.Connect: the three editing control events + the delegate callbacks
        // (ShouldReturn / ShouldChangeCharacters) + the SelectedTextRange channel (SetVirtualView's
        // SelectionChanged hookup). UIControl targets and `delegate` are weak, so the proxy is kept
        // alive for the field's lifetime via an associated object (the button/AppKit pattern).
        [field addTarget:proxy action:@selector(onEditingChanged:) forControlEvents:UIControlEventEditingChanged];
        [field addTarget:proxy action:@selector(onEditingDidBegin:) forControlEvents:UIControlEventEditingDidBegin];
        [field addTarget:proxy action:@selector(onEditingDidEnd:) forControlEvents:UIControlEventEditingDidEnd];
        field.delegate = proxy;
        if ([field isKindOfClass:[MauiIosTextField class]])
        {
            ((MauiIosTextField*)field).mauiProxy = proxy;
        }
        objc_setAssociatedObject(field, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        UITextField* const field = as_field(platform.native);
        if (auto* const proxy = (MauiIosEntryProxy*)objc_getAssociatedObject(field, &k_proxy_key))
        {
            // MauiTextFieldProxy.Disconnect — unhook the same wirings.
            [field removeTarget:proxy
                          action:@selector(onEditingChanged:)
                forControlEvents:UIControlEventEditingChanged];
            [field removeTarget:proxy
                          action:@selector(onEditingDidBegin:)
                forControlEvents:UIControlEventEditingDidBegin];
            [field removeTarget:proxy action:@selector(onEditingDidEnd:) forControlEvents:UIControlEventEditingDidEnd];
        }
        field.delegate = nil;
        if ([field isKindOfClass:[MauiIosTextField class]])
        {
            ((MauiIosTextField*)field).mauiProxy = nil;
        }
        objc_setAssociatedObject(field, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void entry_handler::map_text(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); a nil field text simply clears it.
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        UITextField* const field = as_field(platform->native);
        field.text = value; // TextFieldExtensions.UpdateText
        // MapText → MapFormatting: UpdateMaxLength + UpdateCharacterSpacing + UpdateHorizontalTextAlignment.
        apply_max_length(field, view);
        map_character_spacing(handler, view);
        field.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        // Keep the proxy's previous-value tracker in sync with programmatic text changes.
        if (auto* const proxy = (MauiIosEntryProxy*)objc_getAssociatedObject(field, &k_proxy_key))
        {
            proxy.previousText = field.text != nil ? field.text : @"";
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            update_placeholder(as_field(platform->native), view);
        }
    }

    void entry_handler::map_placeholder_color(entry_handler& handler, i_entry& view)
    {
        // MapPlaceholderColor routes through the same UpdatePlaceholder as MapPlaceholder.
        if (auto* platform = handler.typed_platform_view())
        {
            update_placeholder(as_field(platform->native), view);
        }
    }

    void entry_handler::map_is_password(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UITextField* const field = as_field(platform->native);
        // TextFieldExtensions.UpdateIsPassword: while the field is being edited, flipping SecureTextEntry
        // detaches the keyboard state — the enable/disable + becomeFirstResponder dance re-establishes
        // it, and the text is re-inserted so the secure field does not clear on the next keystroke. The
        // proxy diff guard keeps the remove-and-reinsert silent (C#'s SuppressTextPropertySet analog).
        if (view.is_password() && field.isFirstResponder)
        {
            NSString* const current = field.text;
            field.enabled = NO;
            field.secureTextEntry = YES;
            field.enabled = static_cast<BOOL>(view.is_enabled());
            [field becomeFirstResponder];
            if (current != nil && current.length > 0)
            {
                field.text = @"";
                [field insertText:current];
            }
        }
        else
        {
            field.secureTextEntry = static_cast<BOOL>(view.is_password());
        }
    }

    void entry_handler::map_is_read_only(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateIsReadOnly: read-only (or input-transparent) disables interaction.
            as_field(platform->native).userInteractionEnabled =
                (view.is_read_only() || view.input_transparent()) ? NO : YES;
        }
    }

    void entry_handler::map_max_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            apply_max_length(as_field(platform->native), view); // TextFieldExtensions.UpdateMaxLength
        }
    }

    void entry_handler::map_text_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateTextColor's non-null branch; the TextColor == null fallback to
            // the theme label color has no analog (non-nullable color value type, the same collapse as
            // the AppKit twin). UpdateClearButtonColor (private clearButton KVC tint) is not ported.
            as_field(platform->native).textColor = to_ui_color(view.text_color());
        }
    }

    void entry_handler::map_font(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateFont with UIFont.LabelFontSize as the control default.
            as_field(platform->native).font = to_ui_font(view.font(), static_cast<double>(UIFont.labelFontSize));
        }
    }

    void entry_handler::map_character_spacing(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // TextFieldExtensions.UpdateCharacterSpacing: kern the text AND the placeholder (each only when
        // WithCharacterSpacing returns a value — the empty / nothing-to-unset cases are no-ops).
        UITextField* const field = as_field(platform->native);
        const double spacing = view.character_spacing();
        NSAttributedString* const text_attr = with_character_spacing(field.attributedText, spacing);
        if (text_attr != nil)
        {
            field.attributedText = text_attr;
        }
        NSAttributedString* const placeholder_attr = with_character_spacing(field.attributedPlaceholder, spacing);
        if (placeholder_attr != nil)
        {
            field.attributedPlaceholder = placeholder_attr;
        }
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateHorizontalTextAlignment (the RTL flip rides with FlowDirection).
            as_field(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateVerticalTextAlignment → UIControl.contentVerticalAlignment (a
            // REAL UIKit property — no custom cell needed, unlike AppKit).
            as_field(platform->native).contentVerticalAlignment =
                to_ui_control_content_vertical_alignment(view.vertical_text_alignment());
        }
    }

    void entry_handler::map_is_text_prediction_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateIsTextPredictionEnabled → autocorrectionType (REAL on iOS; the
            // AppKit twin could only mirror it without a field editor).
            as_field(platform->native).autocorrectionType =
                view.is_text_prediction_enabled() ? UITextAutocorrectionTypeYes : UITextAutocorrectionTypeNo;
        }
    }

    void entry_handler::map_is_spell_check_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateIsSpellCheckEnabled → spellCheckingType (REAL on iOS).
            as_field(platform->native).spellCheckingType =
                view.is_spell_check_enabled() ? UITextSpellCheckingTypeYes : UITextSpellCheckingTypeNo;
        }
    }

    void entry_handler::map_return_type(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextFieldExtensions.UpdateReturnType → returnKeyType (REAL on iOS — the software keyboard
            // styles its return key; the AppKit twin recorded a mirror).
            as_field(platform->native).returnKeyType = to_ui_return_key_type(view.return_type());
        }
    }

    void entry_handler::map_clear_button_visibility(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // TextFieldExtensions.UpdateClearButtonVisibility → clearButtonMode (REAL on iOS — the in-field
        // clear affordance the AppKit twin lacked). UpdateClearButtonColor (tinting the private
        // clearButton subview via KVC) is not ported.
        as_field(platform->native).clearButtonMode =
            view.clear_button_visibility() == maui::core::clear_button_visibility::while_editing
                ? UITextFieldViewModeWhileEditing
                : UITextFieldViewModeNever;
    }

    void entry_handler::map_cursor_position(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position(); // observable outside an editing session
        // TextFieldExtensions.UpdateCursorPosition: without an editing session there is no
        // SelectedTextRange (nil → return — C#'s own early-out); during one, re-establish the whole
        // selection from the virtual pair when the native caret differs.
        UITextField* const field = as_field(platform->native);
        UITextRange* const selected = field.selectedTextRange;
        if (selected == nil)
        {
            return;
        }
        const auto native_cursor = static_cast<int>([field offsetFromPosition:field.beginningOfDocument
                                                                   toPosition:selected.start]);
        if (native_cursor != view.cursor_position())
        {
            update_cursor_selection(field, view);
        }
    }

    void entry_handler::map_selection_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length(); // observable outside an editing session
        // TextFieldExtensions.UpdateSelectionLength (same shape as map_cursor_position).
        UITextField* const field = as_field(platform->native);
        UITextRange* const selected = field.selectedTextRange;
        if (selected == nil)
        {
            return;
        }
        const auto native_length = static_cast<int>([field offsetFromPosition:selected.start toPosition:selected.end]);
        if (native_length != view.selection_length())
        {
            update_cursor_selection(field, view);
        }
    }

    maui::graphics::size entry_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform
        // maximum, then the native view measures itself (UIView.SizeThatFits).
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_field(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void entry_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_field(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

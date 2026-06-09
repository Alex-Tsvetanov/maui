// entry_handler — Apple (AppKit / macOS) platform recipe. The real-native twin of the headless partial:
// the managed platform view is an editable NSTextField (held, retained, in entry_platform::native), the
// value properties map to it, and native edits flow back through an NSTextFieldDelegate trampoline to
// i_entry::send_text_changed(old, new) / send_completed(). Compiled as Objective-C++ with ARC only for the
// `apple` backend.
//
// Translated from EntryHandler.iOS.cs (UIKit's MauiTextField): MAUI's macOS support is Mac Catalyst
// (UIKit), so there is no AppKit EntryHandler in the read-only C# source to port verbatim — the
// cross-platform contract (i_entry, the mapper) is faithful, and the AppKit specifics are the standard
// editable-NSTextField equivalents of the UITextField recipe:
//   - EditingChanged  → controlTextDidChange:    → send_text_changed(old, new)
//   - EditingDidEnd   → controlTextDidEndEditing: → send_completed()
// Secure entry has no NSTextField property (unlike UITextField.SecureTextEntry); the AppKit mechanism is
// the field's cell — an NSSecureTextFieldCell vs a plain NSTextFieldCell — so map_is_password swaps the
// cell (preserving stringValue/placeholder).

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_text_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Two cells for the editable field — a plain and a secure variant — both honoring a maui vertical text
// alignment by offsetting the text rect within the cell bounds (NSTextField has no vertical alignment;
// MauiLabel.VerticalAlignment is custom on iOS). map_is_password swaps between them (preserving
// alignment) and map_vertical_text_alignment sets it on whichever is current. A macro keeps the two cell
// bodies identical without multiple inheritance (Obj-C has none) — the offset logic is shared.
#define MAUI_ENTRY_VERTICAL_ALIGNMENT_BODY                                                                             \
    -(NSRect)maui_offsetRect : (NSRect)rect inBounds : (NSRect)bounds                                                  \
    {                                                                                                                  \
        const CGFloat full = bounds.size.height;                                                                       \
        const CGFloat text_height = rect.size.height;                                                                  \
        switch (self.verticalAlignment)                                                                                \
        {                                                                                                              \
            case maui::core::text_alignment::center:                                                                   \
            case maui::core::text_alignment::justify:                                                                  \
                rect.origin.y = bounds.origin.y + ((full - text_height) / 2);                                          \
                break;                                                                                                 \
            case maui::core::text_alignment::end:                                                                      \
                rect.origin.y = bounds.origin.y + (full - text_height);                                                \
                break;                                                                                                 \
            case maui::core::text_alignment::start:                                                                    \
                break;                                                                                                 \
        }                                                                                                              \
        return rect;                                                                                                   \
    }                                                                                                                  \
    -(NSRect)titleRectForBounds : (NSRect)bounds                                                                       \
    {                                                                                                                  \
        return [self maui_offsetRect:[super titleRectForBounds:bounds] inBounds:bounds];                               \
    }                                                                                                                  \
    -(NSRect)drawingRectForBounds : (NSRect)bounds                                                                     \
    {                                                                                                                  \
        return [self maui_offsetRect:[super drawingRectForBounds:bounds] inBounds:bounds];                             \
    }

@interface MauiEntryTextFieldCell : NSTextFieldCell
@property(nonatomic) maui::core::text_alignment verticalAlignment;
- (NSRect)maui_offsetRect:(NSRect)rect inBounds:(NSRect)bounds;
@end

@implementation MauiEntryTextFieldCell
MAUI_ENTRY_VERTICAL_ALIGNMENT_BODY
@end

@interface MauiEntrySecureTextFieldCell : NSSecureTextFieldCell
@property(nonatomic) maui::core::text_alignment verticalAlignment;
- (NSRect)maui_offsetRect:(NSRect)rect inBounds:(NSRect)bounds;
@end

@implementation MauiEntrySecureTextFieldCell
MAUI_ENTRY_VERTICAL_ALIGNMENT_BODY
@end

#undef MAUI_ENTRY_VERTICAL_ALIGNMENT_BODY

// Obj-C trampoline: forwards the NSTextField's editing notifications to the C++ handler's virtual view.
// Tracks the previous string so an edit can report the *old* value (the C# proxy keeps a WeakReference
// to the IEntry and reads its Text; here the field is the source of truth and the trampoline diffs it).
@interface MauiEntryDelegate : NSObject <NSTextFieldDelegate>
@property(nonatomic) maui::core::entry_handler* handler;
@property(nonatomic, copy) NSString* previousText;
@end

@implementation MauiEntryDelegate
- (void)controlTextDidChange:(NSNotification*)notification
{
    NSTextField* const field = (NSTextField*)notification.object;
    NSString* const old_value = self.previousText != nil ? self.previousText : @"";
    NSString* const new_value = field.stringValue != nil ? field.stringValue : @"";
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

// AppKit posts NSTextViewDidChangeSelectionNotification from the field editor; the handler subscribes to
// it (in on_connect_handler) and routes here so the user moving the caret writes CursorPosition /
// SelectionLength back onto the virtual view — mirroring EntryHandler.iOS's OnSelectionChanged.
- (void)mauiSelectionChanged:(NSNotification*)notification
{
    (void)notification;
    if (self.handler == nullptr)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    NSText* const editor = (self.handler->typed_platform_view() != nullptr)
                               ? ((__bridge NSTextField*)self.handler->typed_platform_view()->native).currentEditor
                               : nil;
    if (view == nullptr || editor == nil)
    {
        return;
    }
    const NSRange selection = editor.selectedRange;
    const auto cursor = static_cast<int>(selection.location);
    const auto length = static_cast<int>(selection.length);
    if (view->cursor_position() != cursor)
    {
        view->set_cursor_position(cursor);
    }
    if (view->selection_length() != length)
    {
        view->set_selection_length(length);
    }
}

- (void)controlTextDidEndEditing:(NSNotification*)notification
{
    (void)notification;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_completed();
        }
    }
}
@end

namespace
{
    // Key for the associated MauiEntryDelegate kept alive by the NSTextField (its `delegate` is weak).
    const char k_delegate_key = 0;

    NSTextField* as_field(void* native)
    {
        return (__bridge NSTextField*)native;
    }

    NSTextAlignment to_ns_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return NSTextAlignmentCenter;
            case maui::core::text_alignment::end:
                return NSTextAlignmentRight;
            case maui::core::text_alignment::justify:
                return NSTextAlignmentJustified;
            case maui::core::text_alignment::start:
                return NSTextAlignmentLeft;
        }
        return NSTextAlignmentLeft;
    }

    using maui::platform::apple::to_ns_color;
    using maui::platform::apple::to_ns_font;

    // Move the field editor's caret/selection to (cursor_position, selection_length), clamped to the
    // current text length — the AppKit analog of TextFieldExtensions.UpdateCursorSelection. Shared by
    // map_cursor_position / map_selection_length (which both re-establish the whole range from the pair).
    void apply_editor_selection(NSText* editor, int cursor_position, int selection_length)
    {
        const NSUInteger length = editor.string.length;
        const auto requested_start = static_cast<NSUInteger>(cursor_position < 0 ? 0 : cursor_position);
        const NSUInteger location = requested_start <= length ? requested_start : length;
        const auto requested_span = static_cast<NSUInteger>(selection_length < 0 ? 0 : selection_length);
        const NSUInteger span = (location + requested_span <= length) ? requested_span : (length - location);
        editor.selectedRange = NSMakeRange(location, span);
    }

    // Set the editable field's vertical alignment on whichever custom cell is current (plain or secure).
    void set_field_vertical_alignment(NSTextField* field, maui::core::text_alignment value)
    {
        if ([field.cell isKindOfClass:[MauiEntryTextFieldCell class]])
        {
            ((MauiEntryTextFieldCell*)field.cell).verticalAlignment = value;
        }
        else if ([field.cell isKindOfClass:[MauiEntrySecureTextFieldCell class]])
        {
            ((MauiEntrySecureTextFieldCell*)field.cell).verticalAlignment = value;
        }
    }

    // Rebuild the field's attributed text from the current plain stringValue, applying kerning when
    // character_spacing != 0 (TextFieldExtensions.UpdateCharacterSpacing's text branch). At spacing 0 the
    // value is reset to the plain string so any prior kerning is removed (C#'s WithCharacterSpacing
    // un-sets a previously-applied KerningAdjustment); setting stringValue keeps the readable getter as
    // the source of truth.
    void refresh_entry_text_formatting(NSTextField* field, const maui::core::i_entry& view)
    {
        const double spacing = view.character_spacing();
        NSString* const plain = field.stringValue != nil ? field.stringValue : @"";
        if (spacing == 0)
        {
            field.stringValue = plain; // drop any prior kerned attributedStringValue
            return;
        }
        NSAttributedString* const attributed = maui::platform::apple::kern_attributed(plain, spacing, nil);
        if (attributed != nil)
        {
            field.attributedStringValue = attributed;
        }
    }

    // Rebuild the placeholder from the entry's placeholder text + (optional) color + kerning, mirroring
    // TextFieldExtensions.UpdatePlaceholder. With neither an explicit color nor kerning the plain
    // placeholderString is used (so its getter stays readable — the system draws it in its default muted
    // color); a non-default color or any kerning switches to the attributed placeholder that carries them.
    //
    // C# distinguishes "no placeholder color" via a nullable Color (null => plain). The port models color
    // as a non-null value defaulting to opaque black, so it treats the default-black value as "unset" (the
    // same way map_text_color applies the default black via the plain color path): only a placeholder_color
    // the developer changed away from the default takes the attributed-foreground branch.
    void refresh_entry_placeholder(NSTextField* field, const maui::core::i_entry& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        const maui::graphics::color color = view.placeholder_color();
        const bool explicit_color = color != maui::graphics::color{}; // != the default (opaque black)
        NSColor* const foreground = explicit_color ? to_ns_color(color) : nil;
        const double spacing = view.character_spacing();
        if (foreground == nil && spacing == 0)
        {
            field.placeholderString = text != nil ? text : @"";
            return;
        }
        NSAttributedString* const attributed = maui::platform::apple::placeholder_attributed(text, foreground, spacing);
        field.placeholderAttributedString = attributed;
    }
} // namespace

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
        as_field(native).alphaValue = value;
    }

    void entry_platform::update_is_enabled(bool value)
    {
        [as_field(native) setEnabled:static_cast<BOOL>(value)];
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
        NSTextField* const field = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        // Install the plain vertical-alignment-aware cell (map_is_password swaps to the secure variant).
        MauiEntryTextFieldCell* const cell = [[MauiEntryTextFieldCell alloc] initTextCell:@""];
        cell.editable = YES;
        cell.selectable = YES;
        cell.bezeled = YES;
        cell.bezelStyle = NSTextFieldRoundedBezel;
        cell.drawsBackground = YES;
        cell.verticalAlignment = maui::core::text_alignment::center; // C# Entry default
        field.cell = cell;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void entry_handler::on_connect_handler(entry_platform& platform)
    {
        NSTextField* const field = as_field(platform.native);
        MauiEntryDelegate* const delegate = [[MauiEntryDelegate alloc] init];
        delegate.handler = this;
        delegate.previousText = field.stringValue != nil ? field.stringValue : @"";
        field.delegate = delegate; // NSTextField holds delegate weakly...
        // ...so keep it alive for the field's lifetime via an associated object.
        objc_setAssociatedObject(field, &k_delegate_key, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // The field editor (an NSTextView) posts selection changes; route them to the delegate so the
        // user moving the caret writes CursorPosition / SelectionLength back (EntryHandler OnSelectionChanged).
        [[NSNotificationCenter defaultCenter] addObserver:delegate
                                                 selector:@selector(mauiSelectionChanged:)
                                                     name:NSTextViewDidChangeSelectionNotification
                                                   object:nil];
    }

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        NSTextField* const field = as_field(platform.native);
        if (auto* const delegate = (MauiEntryDelegate*)objc_getAssociatedObject(field, &k_delegate_key))
        {
            [[NSNotificationCenter defaultCenter] removeObserver:delegate
                                                            name:NSTextViewDidChangeSelectionNotification
                                                          object:nil];
        }
        field.delegate = nil;
        objc_setAssociatedObject(field, &k_delegate_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void entry_handler::map_text(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); setStringValue: wants non-null.
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        NSTextField* const field = as_field(platform->native);
        field.stringValue = value != nil ? value : @"";
        // Any text update re-applies the attributed formatting (C# MapText -> MapFormatting).
        refresh_entry_text_formatting(field, view);
        // Keep the delegate's previous-value tracker in sync with programmatic text changes.
        if (auto* const delegate = (MauiEntryDelegate*)objc_getAssociatedObject(field, &k_delegate_key))
        {
            delegate.previousText = field.stringValue;
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_entry_placeholder(as_field(platform->native), view);
        }
    }

    void entry_handler::map_placeholder_color(entry_handler& handler, i_entry& view)
    {
        // The placeholder foreground color requires an attributed placeholder (placeholderAttributedString);
        // refresh_entry_placeholder builds it with the color + kerning, keeping the plain placeholderString
        // path when neither is set. Ports TextFieldExtensions.UpdatePlaceholder (which MapPlaceholderColor
        // also routes through).
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_entry_placeholder(as_field(platform->native), view);
        }
    }

    void entry_handler::map_is_password(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSTextField* const field = as_field(platform->native);
        const bool secure = view.is_password();
        const bool already_secure = [field.cell isKindOfClass:[NSSecureTextFieldCell class]];
        if (secure == already_secure)
        {
            return;
        }
        // AppKit has no SecureTextEntry toggle; swap the cell between the secure and plain custom variants,
        // preserving state. Swapping the cell resets cell-backed appearance (notably the font + the
        // vertical alignment), so capture and re-apply it afterwards — otherwise toggling is_password at
        // runtime (after map_font / map_vertical_text_alignment ran) would drop them.
        NSString* const current = field.stringValue != nil ? field.stringValue : @"";
        NSFont* const font = field.font;
        NSColor* const text_color = field.textColor;
        const maui::core::text_alignment vertical = view.vertical_text_alignment();
        // A lone `if` (not a clone-shaped ternary) picks the cell class — the secure and plain cells are
        // genuinely different classes, but a conditional-operator over two `[X class]` sends trips
        // bugprone-branch-clone (it can't distinguish the receivers).
        Class cell_class = [MauiEntryTextFieldCell class];
        if (secure)
        {
            cell_class = [MauiEntrySecureTextFieldCell class];
        }
        NSTextFieldCell* const replacement = [[cell_class alloc] initTextCell:@""];
        replacement.editable = field.editable;
        replacement.selectable = field.selectable;
        replacement.bezeled = field.bezeled;
        replacement.bezelStyle = field.bezelStyle;
        replacement.drawsBackground = YES;
        replacement.alignment = field.alignment;
        field.cell = replacement;
        set_field_vertical_alignment(field, vertical);
        field.stringValue = current;
        if (font != nil)
        {
            field.font = font;
        }
        field.textColor = text_color;
        // Re-apply the attributed text + placeholder formatting onto the fresh cell.
        refresh_entry_text_formatting(field, view);
        refresh_entry_placeholder(field, view);
    }

    void entry_handler::map_is_read_only(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_field(platform->native).editable = view.is_read_only() ? NO : YES;
        }
    }

    void entry_handler::map_max_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The control truncates in set_text (C# Entry semantics ported there); the native field has no
        // length cap of its own. Re-apply truncation defensively in case a longer value already sits in
        // the field (e.g. set before max_length).
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSTextField* const field = as_field(platform->native);
        NSString* const current = field.stringValue != nil ? field.stringValue : @"";
        // max_length is >= 0 here (guarded above), so compare in the unsigned domain to match NSUInteger.
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            field.stringValue = [current substringToIndex:static_cast<NSUInteger>(max_length)];
        }
    }

    void entry_handler::map_text_color(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_field(platform->native).textColor = to_ns_color(view.text_color());
        }
    }

    void entry_handler::map_font(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_field(platform->native).font = to_ns_font(view.font());
        }
    }

    void entry_handler::map_character_spacing(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSTextField* const field = as_field(platform->native);
        // Apply kerning to both the text and the placeholder (TextFieldExtensions.UpdateCharacterSpacing),
        // then re-apply the horizontal alignment — rebuilding the attributed value can drop the paragraph
        // alignment (C# MapFormatting re-runs UpdateHorizontalTextAlignment for exactly this reason).
        refresh_entry_text_formatting(field, view);
        refresh_entry_placeholder(field, view);
        field.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_field(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            NSTextField* const field = as_field(platform->native);
            set_field_vertical_alignment(field, view.vertical_text_alignment());
            [field setNeedsDisplay:YES];
        }
    }

    void entry_handler::map_is_text_prediction_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The field editor (an NSTextView, available only while editing) carries autocorrection; when it
        // is attached, toggle its automaticTextReplacement (the AppKit analog of UITextField's
        // AutocorrectionType). The mirror records the value either way (so it is observable when no editor
        // exists — e.g. before first responder).
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        if (NSText* const editor = as_field(platform->native).currentEditor; [editor isKindOfClass:[NSTextView class]])
        {
            ((NSTextView*)editor).automaticTextReplacementEnabled = view.is_text_prediction_enabled() ? YES : NO;
        }
    }

    void entry_handler::map_is_spell_check_enabled(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Spell checking likewise lives on the field editor (continuousSpellCheckingEnabled); apply it when
        // editing, and always record the mirror (UpdateIsSpellCheckEnabled's SpellCheckingType analog).
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        if (NSText* const editor = as_field(platform->native).currentEditor; [editor isKindOfClass:[NSTextView class]])
        {
            ((NSTextView*)editor).continuousSpellCheckingEnabled = view.is_spell_check_enabled() ? YES : NO;
        }
    }

    void entry_handler::map_return_type(entry_handler& handler, i_entry& view)
    {
        // macOS uses a hardware keyboard with no software return-key styling (the iOS ReturnKeyType has no
        // AppKit equivalent), so this records the mirror for observability; the value carries no native
        // effect on desktop (documented in STATUS). Completed still fires on Return via the field delegate.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->entry_return_type = view.return_type();
        }
    }

    void entry_handler::map_clear_button_visibility(entry_handler& handler, i_entry& view)
    {
        // A plain NSTextField has no built-in clear button (that is an NSSearchField affordance), so this
        // records the mirror; wiring a real clear button would mean hosting an NSSearchField-style overlay
        // (deferred, documented in STATUS). The headless backend mirrors it too.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->clear_button = view.clear_button_visibility();
        }
    }

    void entry_handler::map_cursor_position(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        // When the field is being edited, move the field editor's caret/selection to match (the AppKit
        // analog of TextFieldExtensions.UpdateCursorPosition). Outside editing there is no field editor,
        // so the mirror alone records the intent.
        if (NSText* const editor = as_field(platform->native).currentEditor; [editor isKindOfClass:[NSTextView class]])
        {
            apply_editor_selection(editor, view.cursor_position(), view.selection_length());
        }
    }

    void entry_handler::map_selection_length(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
        if (NSText* const editor = as_field(platform->native).currentEditor; [editor isKindOfClass:[NSTextView class]])
        {
            apply_editor_selection(editor, view.cursor_position(), view.selection_length());
        }
    }

    maui::graphics::size entry_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_field(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void entry_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_field(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void entry_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void entry_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers
    // (M4d ViewMapper visuals). `native` is this struct's NSView handle.
    void entry_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void entry_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void entry_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the native field via the shared
    // apple_semantics_ops helpers (M5d native a11y / hit-test). `native` is this struct's NSView handle.
    // (Read-only is independent — map_is_read_only drives NSTextField.editable.)
    void entry_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void entry_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core

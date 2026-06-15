// editor_handler — Apple (AppKit / macOS) platform recipe. The real-native twin of the headless
// partial: the managed platform view is an NSScrollView hosting a MauiEditorTextView (an NSTextView
// subclass carrying the placeholder label — the AppKit translation of MauiTextView.cs's
// PlaceholderLabel), the value properties map to it, and native edits flow back through an
// NSTextViewDelegate trampoline to i_editor::send_text_changed(old, new) /
// i_editor::send_completed(). Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from EditorHandler.iOS.cs + Platform/iOS/TextViewExtensions.cs + MauiTextView.cs: MAUI's
// macOS support is Mac Catalyst (UIKit), so there is no AppKit EditorHandler in the read-only C# source
// to port verbatim — the cross-platform contract (i_editor, the mapper) is faithful, and the AppKit
// specifics are the standard scrollable-NSTextView equivalents of the UITextView recipe:
//   - MauiTextView.TextSetOrChanged / Changed → textDidChange:           → send_text_changed(old, new)
//   - MauiTextViewEventProxy.OnEnded          → textDidEndEditing:       → send_completed()
//   - MauiTextView.SelectionChanged           → textViewDidChangeSelection: → cursor/selection write-back
//   - MauiTextView's PlaceholderLabel (a UILabel hidden while text is present) → a non-editable
//     NSTextField label subview, hidden the same way.
// Documented AppKit deviations: vertical_text_alignment is mirror-only (MauiTextView centers via UIKit
// content insets; an NSTextView is inherently top-aligned), and the max-length keystroke gate rides the
// delegate's shouldChangeTextInRange (the same TextWithinMaxLength rule).

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
#include "maui/core/editor_handler.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The NSTextView subclass carrying the placeholder label — the AppKit translation of
// Microsoft.Maui.Platform.MauiTextView (whose PlaceholderLabel is a UILabel hidden once text is
// present). The label is a non-editable, non-bezeled NSTextField pinned to the top-left text inset.
@interface MauiEditorTextView : NSTextView
@property(nonatomic, strong) NSTextField* placeholderLabel;
- (void)mauiUpdatePlaceholderVisibility;
@end

@implementation MauiEditorTextView
- (void)mauiUpdatePlaceholderVisibility
{
    // MauiTextView.HidePlaceholderIfTextIsPresent.
    self.placeholderLabel.hidden = self.string.length > 0;
}
@end

// Obj-C trampoline: forwards the NSTextView's editing notifications to the C++ handler's virtual view.
// Tracks the previous string so an edit can report the *old* value (the C# proxy reads the virtual
// view's Text; here the text view is the source of truth and the trampoline diffs it).
@interface MauiEditorDelegate : NSObject <NSTextViewDelegate>
@property(nonatomic) maui::core::editor_handler* handler;
@property(nonatomic, copy) NSString* previousText;
@end

namespace
{
    // Key for the associated MauiEditorDelegate kept alive by the NSScrollView (NSTextView's `delegate`
    // is weak).
    const char k_delegate_key = 0;

    NSScrollView* as_scroll_view(void* native)
    {
        return (__bridge NSScrollView*)native;
    }

    MauiEditorTextView* as_text_view(void* native)
    {
        return (MauiEditorTextView*)as_scroll_view(native).documentView;
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

    // Re-apply kerning across the stored text + the typing attributes (the NSTextView analog of
    // TextViewExtensions.UpdateCharacterSpacing, which kerns the attributed text; typingAttributes keep
    // newly-typed characters kerned too).
    void apply_editor_character_spacing(MauiEditorTextView* text_view, double spacing)
    {
        NSTextStorage* const storage = text_view.textStorage;
        const NSRange range = NSMakeRange(0, storage.length);
        if (spacing == 0)
        {
            [storage removeAttribute:NSKernAttributeName range:range];
        }
        else
        {
            [storage addAttribute:NSKernAttributeName value:[NSNumber numberWithDouble:spacing] range:range];
        }
        NSMutableDictionary* const typing = [text_view.typingAttributes mutableCopy];
        if (spacing == 0)
        {
            [typing removeObjectForKey:NSKernAttributeName];
        }
        else
        {
            typing[NSKernAttributeName] = [NSNumber numberWithDouble:spacing];
        }
        text_view.typingAttributes = typing;
    }

    // Rebuild the placeholder label from the editor's placeholder text + (optional) color, mirroring
    // TextViewExtensions.UpdatePlaceholder onto MauiTextView's PlaceholderText/PlaceholderTextColor. The
    // default-constructed color (opaque black) counts as "no explicit color" (the same collapse as the
    // entry recipe), keeping the system's muted placeholder rendering.
    void refresh_editor_placeholder(MauiEditorTextView* text_view, const maui::core::i_editor& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        text_view.placeholderLabel.stringValue = text != nil ? text : @"";
        const maui::graphics::color color = view.placeholder_color();
        const bool explicit_color = color != maui::graphics::color{}; // != the default (opaque black)
        text_view.placeholderLabel.textColor = explicit_color ? to_ns_color(color) : NSColor.secondaryLabelColor;
        [text_view.placeholderLabel sizeToFit];
        [text_view mauiUpdatePlaceholderVisibility];
    }

    // Move the caret/selection to (cursor_position, selection_length), clamped to the current text
    // length — TextViewExtensions.UpdateCursorPosition/UpdateSelectionLength collapse to one range write.
    void apply_editor_selection(MauiEditorTextView* text_view, int cursor_position, int selection_length)
    {
        const NSUInteger length = text_view.string.length;
        const auto requested_start = static_cast<NSUInteger>(cursor_position < 0 ? 0 : cursor_position);
        const NSUInteger location = requested_start <= length ? requested_start : length;
        const auto requested_span = static_cast<NSUInteger>(selection_length < 0 ? 0 : selection_length);
        const NSUInteger span = (location + requested_span <= length) ? requested_span : (length - location);
        text_view.selectedRange = NSMakeRange(location, span);
    }
} // namespace

@implementation MauiEditorDelegate
- (void)textDidChange:(NSNotification*)notification
{
    auto* const text_view = (MauiEditorTextView*)notification.object;
    [text_view mauiUpdatePlaceholderVisibility];
    NSString* const old_value = self.previousText != nil ? self.previousText : @"";
    NSString* const new_value = text_view.string != nil ? text_view.string : @"";
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

- (void)textDidEndEditing:(NSNotification*)notification
{
    (void)notification;
    // MauiTextViewEventProxy.OnEnded → Completed (the editor signals completion on end-of-edit, not on
    // Return — a multi-line editor inserts newlines on Return).
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_completed();
        }
    }
}

- (void)textViewDidChangeSelection:(NSNotification*)notification
{
    // MauiTextViewEventProxy.OnSelectionChanged: push the native caret/selection back onto the virtual
    // view (guarded on actual value differences, exactly like C#).
    auto* const text_view = (MauiEditorTextView*)notification.object;
    if (self.handler == nullptr || text_view == nil)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    if (view == nullptr)
    {
        return;
    }
    const NSRange selection = text_view.selectedRange;
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

- (BOOL)textView:(NSTextView*)textView
    shouldChangeTextInRange:(NSRange)affectedCharRange
          replacementString:(NSString*)replacementString
{
    // ITextInputExtensions.TextWithinMaxLength — the REAL native max-length gate (typing/pasting beyond
    // MaxLength is rejected at the keystroke), the same rule the iOS proxy's OnShouldChangeText applies.
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view == nullptr)
    {
        return YES;
    }
    const int max_length = view->max_length();
    if (max_length < 0)
    {
        return YES;
    }
    const NSUInteger current_length = textView.string != nil ? textView.string.length : 0;
    if (affectedCharRange.location + affectedCharRange.length > current_length)
    {
        return NO;
    }
    const NSUInteger add_length = replacementString != nil ? replacementString.length : 0;
    const NSUInteger new_length = current_length + add_length - affectedCharRange.length;
    return new_length <= static_cast<NSUInteger>(max_length) ? YES : NO;
}
@end

namespace maui::core
{
    editor_platform::~editor_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes target the OUTER scroll view (the view a superview hosts).
    void editor_platform::update_visibility(maui::core::visibility value)
    {
        as_scroll_view(native).hidden = value != maui::core::visibility::visible;
    }

    void editor_platform::update_opacity(double value)
    {
        as_scroll_view(native).alphaValue = value;
    }

    void editor_platform::update_is_enabled(bool value)
    {
        // NSScrollView has no `enabled`; the editable text view carries the interaction state (a
        // disabled editor stops accepting edits but stays selectable, like UITextView.Editable).
        as_text_view(native).editable = value && !is_read_only ? YES : NO;
    }

    void editor_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_scroll_view(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<editor_platform> editor_handler::create_platform_view()
    {
        auto platform = std::make_unique<editor_platform>();
        // The standard AppKit scrollable text view recipe (the NSTextView analog of CreatePlatformView's
        // `new MauiTextView()` — a UITextView scrolls by itself; AppKit splits the scroller out).
        NSScrollView* const scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        scroll.hasVerticalScroller = YES;
        scroll.borderType = NSBezelBorder;
        scroll.drawsBackground = YES;
        MauiEditorTextView* const text_view = [[MauiEditorTextView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        text_view.editable = YES;
        text_view.selectable = YES;
        text_view.richText = NO;
        text_view.verticallyResizable = YES;
        text_view.horizontallyResizable = NO;
        text_view.autoresizingMask = NSViewWidthSizable;
        // The placeholder label (MauiTextView.InitPlaceholderLabel): non-editable, transparent, pinned
        // near the text origin, hidden once text is present.
        NSTextField* const placeholder = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 0, 0, 0)];
        placeholder.editable = NO;
        placeholder.selectable = NO;
        placeholder.bezeled = NO;
        placeholder.drawsBackground = NO;
        placeholder.textColor = NSColor.secondaryLabelColor;
        text_view.placeholderLabel = placeholder;
        [text_view addSubview:placeholder];
        [text_view mauiUpdatePlaceholderVisibility];
        scroll.documentView = text_view;
        platform->native = (__bridge_retained void*)scroll; // the void* slot owns one reference
        return platform;
    }

    void editor_handler::on_connect_handler(editor_platform& platform)
    {
        MauiEditorTextView* const text_view = as_text_view(platform.native);
        MauiEditorDelegate* const delegate = [[MauiEditorDelegate alloc] init];
        delegate.handler = this;
        delegate.previousText = text_view.string != nil ? text_view.string : @"";
        text_view.delegate = delegate; // NSTextView holds its delegate weakly...
        // ...so keep it alive for the view's lifetime via an associated object on the scroll view.
        objc_setAssociatedObject(as_scroll_view(platform.native), &k_delegate_key, delegate,
                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void editor_handler::on_disconnect_handler(editor_platform& platform)
    {
        as_text_view(platform.native).delegate = nil;
        objc_setAssociatedObject(as_scroll_view(platform.native), &k_delegate_key, nil,
                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void editor_handler::map_text(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        MauiEditorTextView* const text_view = as_text_view(platform->native);
        [text_view setString:value != nil ? value : @""];
        [text_view mauiUpdatePlaceholderVisibility];
        // Any text update re-applies the attributed formatting (C# MapText → MapFormatting).
        apply_editor_character_spacing(text_view, view.character_spacing());
        text_view.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        // Keep the delegate's previous-value tracker in sync with programmatic text changes.
        if (auto* const delegate =
                (MauiEditorDelegate*)objc_getAssociatedObject(as_scroll_view(platform->native), &k_delegate_key))
        {
            delegate.previousText = text_view.string != nil ? text_view.string : @"";
        }
    }

    void editor_handler::map_placeholder(editor_handler& handler, i_editor& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_editor_placeholder(as_text_view(platform->native), view);
        }
    }

    void editor_handler::map_placeholder_color(editor_handler& handler, i_editor& view)
    {
        // MapPlaceholderColor routes through the same placeholder rebuild as MapPlaceholder.
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_editor_placeholder(as_text_view(platform->native), view);
        }
    }

    void editor_handler::map_is_read_only(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_read_only = view.is_read_only();
            as_text_view(platform->native).editable = view.is_read_only() ? NO : YES;
        }
    }

    void editor_handler::map_max_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The keystroke gate lives in the delegate (shouldChangeTextInRange); re-apply truncation
        // defensively in case a longer value already sits in the view (TextViewExtensions.UpdateMaxLength).
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        MauiEditorTextView* const text_view = as_text_view(platform->native);
        NSString* const current = text_view.string != nil ? text_view.string : @"";
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            [text_view setString:[current substringToIndex:static_cast<NSUInteger>(max_length)]];
            [text_view mauiUpdatePlaceholderVisibility];
        }
    }

    void editor_handler::map_text_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_text_view(platform->native).textColor = to_ns_color(view.text_color());
        }
    }

    void editor_handler::map_font(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_text_view(platform->native).font = to_ns_font(view.font());
        }
    }

    void editor_handler::map_character_spacing(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        MauiEditorTextView* const text_view = as_text_view(platform->native);
        apply_editor_character_spacing(text_view, view.character_spacing());
        // Rebuilding attributes can drop the paragraph alignment; re-apply it (C# MapFormatting re-runs
        // UpdateHorizontalTextAlignment for the same reason).
        text_view.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
    }

    void editor_handler::map_horizontal_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_text_view(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void editor_handler::map_vertical_text_alignment(editor_handler& handler, i_editor& view)
    {
        // An NSTextView is inherently top-aligned; MauiTextView centers via UIKit content insets, which
        // have no AppKit analog. The mirror records the value for observability (documented deviation).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void editor_handler::map_is_text_prediction_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // REAL on an NSTextView (unlike the entry's field editor dance): automaticTextReplacement is
            // the AppKit analog of UITextView's AutocorrectionType.
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
            as_text_view(platform->native).automaticTextReplacementEnabled =
                view.is_text_prediction_enabled() ? YES : NO;
        }
    }

    void editor_handler::map_is_spell_check_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
            as_text_view(platform->native).continuousSpellCheckingEnabled = view.is_spell_check_enabled() ? YES : NO;
        }
    }

    void editor_handler::map_keyboard(editor_handler& handler, i_editor& view)
    {
        // AppKit has NO soft keyboard — no UIKeyboardType / Done input accessory analog on macOS
        // (DEVIATION, documented in STATUS): the macOS twin records the cross-platform mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->keyboard = view.keyboard();
        }
    }

    void editor_handler::map_cursor_position(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        apply_editor_selection(as_text_view(platform->native), view.cursor_position(), view.selection_length());
    }

    void editor_handler::map_selection_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
        apply_editor_selection(as_text_view(platform->native), view.cursor_position(), view.selection_length());
    }

    maui::graphics::size editor_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // Size to the laid-out text (the NSTextView analog of UITextView.SizeThatFits): ensure layout,
        // then report the used rect, clamped to a finite width constraint.
        MauiEditorTextView* const text_view = as_text_view(platform->native);
        [text_view.layoutManager ensureLayoutForTextContainer:text_view.textContainer];
        const NSRect used = [text_view.layoutManager usedRectForTextContainer:text_view.textContainer];
        double width = used.size.width;
        if (width_constraint > 0 && width_constraint < width)
        {
            width = width_constraint;
        }
        return {width, used.size.height};
    }

    void editor_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_scroll_view(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers.
    void editor_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void editor_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers.
    void editor_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void editor_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void editor_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed via the shared apple_semantics_ops
    // helpers (semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate).
    void editor_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void editor_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core

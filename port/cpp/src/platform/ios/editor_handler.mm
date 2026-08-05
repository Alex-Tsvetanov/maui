// editor_handler — iOS (UIKit) platform recipe. The managed platform view is a MauiIosEditorTextView
// (the port of Microsoft.Maui.Platform.MauiTextView — a UITextView with a placeholder UILabel hidden
// while text is present), the value properties map to it, and native edits flow back through a
// UITextViewDelegate proxy to i_editor::send_text_changed(old, new) / i_editor::send_completed().
// Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from EditorHandler.iOS.cs + Platform/iOS/TextViewExtensions.cs + MauiTextView.cs:
//   CreatePlatformView = new MauiTextView(); MauiTextViewEventProxy: TextSetOrChanged/Changed →
//   UpdateText (send_text_changed), Began → IsFocused=true, Ended → Completed + IsFocused=false,
//   ShouldChangeText → TextWithinMaxLength (the REAL native max-length gate), SelectionChanged →
//   CursorPosition/SelectionLength write-back.
//   Map bodies below = TextViewExtensions.UpdateText/UpdateTextColor/UpdatePlaceholder(+Color)/
//   UpdateIsReadOnly/UpdateMaxLength/UpdateFont/UpdateCharacterSpacing/UpdateHorizontal+Vertical
//   TextAlignment/UpdateKeyboard/UpdateIsTextPredictionEnabled/UpdateIsSpellCheckEnabled/
//   UpdateCursorPosition/UpdateSelectionLength.
// Keyboard subsystem (W8-53): MapKeyboard pushes UIKeyboardType + the autocapitalization/spellcheck/
// autocorrection traits (ios_keyboard_ops.hpp), plus the Done input accessory toolbar
// (AddMauiDoneAccessoryView → ios_done_accessory.hpp). Focus (W8-53): becomeFirstResponder /
// resignFirstResponder via the shared view_command_mapper (view_focus_ops.mm), reflected onto IsFocused.
// MapBackground's ImageSourcePaint branch (W8-55) is handled by the shared view_mapper → ios_visual_ops
// apply_background (an image_source_paint installs the source's image as a named backing CALayer), so an
// image-backed Background renders on the editor like any other view.
// Placeholder parity (W8-54): MauiTextView's placeholder dance is ported — InitPlaceholderLabel (a
// wrapping UILabel), UpdatePlaceholderLabelFrame (pinned to the text container's LineFragmentPadding /
// TextContainerInset in layoutSubviews), UpdatePlaceholderFont (the placeholder tracks the editor font),
// and UpdateHorizontalTextAlignment (the placeholder follows the editor's alignment).
// Keyboard auto-manager (W7 keyboard-automanager): the editor participates in KeyboardAutoManagerScroll's
// scroll-avoidance (the UITextViewTextDidBeginEditingNotification observer, connected once via the entry
// handler's on_connect_handler — the engine is a global notification subscriber, not per-control wired).
// There is NO editor return-key / next-responder entry point: EditorHandler.iOS.cs does not call
// KeyboardAutoManager.GoToNextResponderOrResign anywhere (a UITextView's return key inserts a newline by
// design — only EntryHandler.iOS.cs's OnShouldReturn invokes the walk). Per the derive-not-invent rule,
// no editor return-key wiring is added here (see the unit's deviation note).
// Vertical centering (U05): MauiTextView.ShouldCenterVertically is ported as apply_vertical_text_alignment
// — when the view is taller than its content, the contentOffset is nudged so the text sits Center / End /
// Start within the slack (Center → -max(1, available/2); End → -max(1, available); Start → CGPointZero).
// It runs from map_vertical_text_alignment (the VerticalTextAlignment setter) and from map_text (a
// content-height change re-centers, as MauiTextView.LayoutSubviews does). The keyboard-aware cursor
// adjustment that follows it in C# is deferred (it is not the gap core; see the keyboard-auto-manager note
// above).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "ios_conversions.hpp"
#include "ios_done_accessory.hpp"
#include "ios_keyboard_ops.hpp"
#include "ios_text_ops.hpp"
#include "ios_view_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Re-runs MauiTextView.ShouldCenterVertically against the view's current bounds/content size (defined
// below). Declared up here so the subclass's layoutSubviews can re-center on layout.
namespace
{
    void apply_vertical_text_alignment_for(UITextView* text_view, maui::core::text_alignment alignment);
} // namespace

// MauiIosEditorTextView  <=  Microsoft.Maui.Platform.MauiTextView — the UITextView subclass carrying
// the placeholder label (a UILabel hidden once text is present) and the vertical alignment that drives
// ShouldCenterVertically. `mauiVerticalTextAlignment` stores the enum as the raw underlying value so the
// header (text_alignment.hpp) need not be imported into the @interface.
@interface MauiIosEditorTextView : UITextView
@property(nonatomic, strong) UILabel* placeholderLabel;
@property(nonatomic) NSInteger mauiVerticalTextAlignment; // text_alignment, raw value (Start = 0)
- (void)mauiUpdatePlaceholderVisibility;
@end

@implementation MauiIosEditorTextView
- (void)mauiUpdatePlaceholderVisibility
{
    // MauiTextView.HidePlaceholderIfTextIsPresent.
    self.placeholderLabel.hidden = self.text.length > 0;
}

// MauiTextView.UpdatePlaceholderLabelFrame: pin the placeholder to the text origin — x at the text
// container's LineFragmentPadding, y at TextContainerInset.Top — sized to the content area inside the
// horizontal padding and vertical insets. A no-op until the view has bounds (matches the C# guard).
- (void)mauiUpdatePlaceholderFrame
{
    if (self.placeholderLabel == nil || CGRectEqualToRect(self.bounds, CGRectZero))
    {
        return;
    }
    const CGFloat x = self.textContainer.lineFragmentPadding;
    const CGFloat y = self.textContainerInset.top;
    const CGFloat width = self.bounds.size.width - (x * 2);
    const CGFloat height = self.frame.size.height - (self.textContainerInset.top + self.textContainerInset.bottom);
    self.placeholderLabel.frame = CGRectMake(x, y, width > 0 ? width : 0, height > 0 ? height : 0);
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self mauiUpdatePlaceholderFrame]; // MauiTextView.LayoutSubviews → UpdatePlaceholderLabelFrame
    // ...then ShouldCenterVertically, so the content re-centers on every layout (a content-height or
    // bounds change re-runs the centering math — MauiTextView.LayoutSubviews:146).
    apply_vertical_text_alignment_for(self, static_cast<maui::core::text_alignment>(self.mauiVerticalTextAlignment));
    // Re-frame the clip mask to the new bounds (WrapperView.LayoutSubviews re-runs SetClip): the mask is
    // sized at map time, before the first layout, when bounds is 0×0. No-op when no clip is set.
    maui::platform::ios::reapply_clip((__bridge void*)self);
}
@end

// MauiTextView.ShouldCenterVertically (MauiTextView.cs:196-207): when the view is taller than its content
// there is vertical slack, and the contentOffset is nudged so the text sits Center / End / Start within
// that slack. availableSpace = Bounds.Height − ContentSize.Height*ZoomScale; a value <= 0 means the
// content already fills (or overflows) the view, so the offset is left untouched. Center →
// -Math.Max(1, availableSpace/2); End → -Math.Max(1, availableSpace); Start (and Justify) → CGPointZero.
// (The C# keyboard-aware cursor adjustment that follows is deferred — see the file header.)
namespace
{
    void apply_vertical_text_alignment_for(UITextView* text_view, maui::core::text_alignment alignment)
    {
        const CGFloat content_height = text_view.contentSize.height;
        const CGFloat available_space = text_view.bounds.size.height - (content_height * text_view.zoomScale);
        if (available_space <= 0)
        {
            return;
        }
        switch (alignment)
        {
            case maui::core::text_alignment::center:
                text_view.contentOffset = CGPointMake(0, -std::max<CGFloat>(1, available_space / 2));
                break;
            case maui::core::text_alignment::end:
                text_view.contentOffset = CGPointMake(0, -std::max<CGFloat>(1, available_space));
                break;
            default: // Start (the default) and Justify: top-aligned, no offset.
                text_view.contentOffset = CGPointZero;
                break;
        }
    }
} // namespace

// Obj-C trampoline: forwards the UITextView's delegate callbacks to the C++ handler's virtual view.
// Ports EditorHandler.MauiTextViewEventProxy — and, like the entry proxy, tracks the previous string so
// an edit can report the *old* value.
@interface MauiIosEditorProxy : NSObject <UITextViewDelegate>
@property(nonatomic) maui::core::editor_handler* handler;
@property(nonatomic, copy) NSString* previousText;
@end

namespace
{
    // Key for the associated MauiIosEditorProxy kept alive by the UITextView (`delegate` is weak).
    const char k_proxy_key = 0;

    MauiIosEditorTextView* as_text_view(void* native)
    {
        return (__bridge MauiIosEditorTextView*)native;
    }

    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::with_character_spacing;

    // TextViewExtensions.UpdatePlaceholder(+Color) onto MauiTextView's PlaceholderText/Color. The
    // default-constructed color (opaque black) counts as "no explicit color" (the entry collapse),
    // keeping UIKit's muted placeholderText gray. After setting the text/color, re-pin the placeholder
    // frame against the text container insets (MauiTextView.LayoutSubviews → UpdatePlaceholderLabelFrame).
    void refresh_editor_placeholder(MauiIosEditorTextView* text_view, const maui::core::i_editor& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        text_view.placeholderLabel.text = text != nil ? text : @"";
        const maui::graphics::color color = view.placeholder_color();
        // is-set discriminator: an explicit PlaceholderColor=Black equals the default-constructed
        // sentinel by value, so key off BindableObject.IsSet (else it falls to the muted system gray).
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool explicit_color = bindable != nullptr && bindable->is_property_set("placeholder_color");
        text_view.placeholderLabel.textColor = explicit_color ? to_ui_color(color) : UIColor.placeholderTextColor;
        [text_view.placeholderLabel sizeToFit]; // MauiTextView.PlaceholderText setter's SizeToFit
        [text_view mauiUpdatePlaceholderFrame]; // ...then the layout pass overrides the frame for wrapping
        [text_view mauiUpdatePlaceholderVisibility];
    }

    // MauiTextView.UpdatePlaceholderFont + UpdateHorizontalTextAlignment: the placeholder label tracks the
    // editor's font and text alignment so the hint renders in the same style as the typed text.
    void sync_placeholder_style(MauiIosEditorTextView* text_view, const maui::core::i_editor& view)
    {
        text_view.placeholderLabel.font = text_view.font; // base.Font setter → UpdatePlaceholderFont(value)
        text_view.placeholderLabel.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        [text_view mauiUpdatePlaceholderFrame];
    }

    // Move the caret/selection to (cursor_position, selection_length), clamped to the current text
    // length (TextViewExtensions.UpdateCursorPosition/UpdateSelectionLength — a UITextView exposes the
    // plain selectedRange, so no UITextPosition arithmetic is needed).
    void apply_editor_selection(UITextView* text_view, int cursor_position, int selection_length)
    {
        const NSUInteger length = text_view.text != nil ? text_view.text.length : 0;
        const auto requested_start = static_cast<NSUInteger>(cursor_position < 0 ? 0 : cursor_position);
        const NSUInteger location = requested_start <= length ? requested_start : length;
        const auto requested_span = static_cast<NSUInteger>(selection_length < 0 ? 0 : selection_length);
        const NSUInteger span = (location + requested_span <= length) ? requested_span : (length - location);
        text_view.selectedRange = NSMakeRange(location, span);
    }

    // TextViewExtensions.UpdateMaxLength: trim the text to MaxLength (a no-op for a negative cap or no
    // overflow).
    void apply_max_length(MauiIosEditorTextView* text_view, const maui::core::i_editor& view)
    {
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSString* const current = text_view.text != nil ? text_view.text : @"";
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            text_view.text = [current substringToIndex:static_cast<NSUInteger>(max_length)];
            [text_view mauiUpdatePlaceholderVisibility];
        }
    }
} // namespace

@implementation MauiIosEditorProxy
// The shared old/new diff: report the view's text to the virtual view when it actually changed (the C#
// UpdateText path is a property set, so an equal value is a no-op — this guard is its analog).
- (void)mauiSyncTextFrom:(MauiIosEditorTextView*)textView
{
    [textView mauiUpdatePlaceholderVisibility];
    NSString* const previous = self.previousText;
    NSString* const current = textView.text;
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
            const char* const old_utf8 = old_value.UTF8String;
            const char* const new_utf8 = new_value.UTF8String;
            view->send_text_changed(old_utf8 != nullptr ? old_utf8 : "", new_utf8 != nullptr ? new_utf8 : "");
        }
    }
}

- (void)textViewDidChange:(UITextView*)textView
{
    [self mauiSyncTextFrom:(MauiIosEditorTextView*)textView];
}

- (void)textViewDidBeginEditing:(UITextView*)textView
{
    // The text view took first responder: reflect IsFocused = true onto the virtual view (fires Focused
    // + ChangeVisualState through set_is_focused) — the native focus callback's analog.
    (void)textView;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->set_is_focused(true);
        }
    }
}

- (void)textViewDidEndEditing:(UITextView*)textView
{
    // MauiTextViewEventProxy.OnEnded: one final text sync, then IsFocused = false (it resigned first
    // responder), then Completed — matching EditorHandler.iOS.cs OnEnded (IsFocused=false before
    // Completed()), so a Completed handler already observes the unfocused state.
    MauiIosEditorProxy* const keep = self; // each raise below may destroy the editor -> frees us
    [keep mauiSyncTextFrom:(MauiIosEditorTextView*)textView];
    if (auto* const view = maui::platform::ios::live_view(keep.handler))
    {
        view->set_is_focused(false);
    }
    if (auto* const view = maui::platform::ios::live_view(keep.handler))
    {
        view->send_completed();
    }
}

- (void)onDoneClicked:(id)sender
{
    // MauiDoneAccessoryView's OnDoneClicked: resign first responder (which fires OnEnded → Completed +
    // IsFocused=false). The Editor's Done bar dismisses the keyboard for a multi-line field.
    (void)sender;
    if (self.handler == nullptr)
    {
        return;
    }
    if (auto* const platform = self.handler->typed_platform_view())
    {
        [(__bridge UITextView*)platform->native resignFirstResponder];
    }
}

- (void)textViewDidChangeSelection:(UITextView*)textView
{
    // OnSelectionChanged: push the native caret/selection back onto the virtual view (guarded on actual
    // value differences, exactly like C#).
    if (self.handler == nullptr || textView == nil)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    if (view == nullptr)
    {
        return;
    }
    const NSRange selection = textView.selectedRange;
    const auto cursor = static_cast<int>(selection.location);
    const auto length = static_cast<int>(selection.length);
    if (view->cursor_position() != cursor)
    {
        view->set_cursor_position(cursor);
    }
    // set_cursor_position raised a property change: re-read before touching the view again.
    auto* const still = maui::platform::ios::live_view(self.handler);
    if (still != nullptr && still->selection_length() != length)
    {
        still->set_selection_length(length);
    }
}

- (BOOL)textView:(UITextView*)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString*)text
{
    // OnShouldChangeText → ITextInputExtensions.TextWithinMaxLength: the REAL native max-length gate.
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view == nullptr)
    {
        return YES;
    }
    NSString* const current = textView.text != nil ? textView.text : @"";
    const NSUInteger current_length = current.length;
    if (range.location + range.length > current_length)
    {
        return NO;
    }
    const int max_length = view->max_length();
    if (max_length < 0)
    {
        return YES;
    }
    const NSUInteger add_length = text != nil ? text.length : 0;
    const NSUInteger new_length = current_length + add_length - range.length;
    return std::cmp_less_equal(new_length, max_length) ? YES : NO;
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
        void detach_trampolines(editor_platform& platform)
        {
            MauiIosEditorTextView* const text_view = as_text_view(platform.native);
            text_view.delegate = nil;
            if (auto* const trampoline = (MauiIosEditorProxy*)objc_getAssociatedObject(text_view, &k_proxy_key))
            {
                trampoline.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(text_view, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
    } // namespace

    editor_platform::~editor_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void editor_platform::update_visibility(maui::core::visibility value)
    {
        as_text_view(native).hidden = value != maui::core::visibility::visible;
    }

    void editor_platform::update_opacity(double value)
    {
        as_text_view(native).alpha = value;
    }

    void editor_platform::update_is_enabled(bool value)
    {
        // A UITextView is not a UIControl; ViewExtensions.UpdateIsEnabled drives userInteractionEnabled.
        as_text_view(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void editor_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_text_view(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapBackground → the UITextView layer's backgroundColor (solid) / gradient or image
    // sublayer, mirroring the apple backend. clip_views' red editor fills under the clip mask.
    void editor_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void editor_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    std::unique_ptr<editor_platform> editor_handler::create_platform_view()
    {
        auto platform = std::make_unique<editor_platform>();
        // CreatePlatformView: new MauiTextView(); the placeholder label is created with it
        // (MauiTextView.InitPlaceholderLabel). The Done input accessory is attached in on_connect_handler
        // (the proxy is its target — created there).
        MauiIosEditorTextView* const text_view = [[MauiIosEditorTextView alloc] initWithFrame:CGRectZero];
        UILabel* const placeholder = [[UILabel alloc] initWithFrame:CGRectMake(5, 5, 0, 0)];
        placeholder.textColor = UIColor.placeholderTextColor;
        placeholder.numberOfLines = 0;
        text_view.placeholderLabel = placeholder;
        [text_view addSubview:placeholder];
        [text_view mauiUpdatePlaceholderVisibility];
        platform->native = (__bridge_retained void*)text_view; // the void* slot owns one reference
        return platform;
    }

    void editor_handler::on_connect_handler(editor_platform& platform)
    {
        MauiIosEditorTextView* const text_view = as_text_view(platform.native);
        MauiIosEditorProxy* const proxy = [[MauiIosEditorProxy alloc] init];
        proxy.handler = this;
        proxy.previousText = text_view.text != nil ? text_view.text : @"";
        text_view.delegate = proxy; // weak, so the proxy is retained via an associated object
        objc_setAssociatedObject(text_view, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // AddMauiDoneAccessoryView: the Done toolbar above the soft keyboard (the proxy is its target).
        text_view.inputAccessoryView = maui::platform::ios::make_done_accessory(proxy, @selector(onDoneClicked:));
    }

    void editor_handler::on_disconnect_handler(editor_platform& platform)
    {
        detach_trampolines(platform);
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
        MauiIosEditorTextView* const text_view = as_text_view(platform->native);
        text_view.text = value; // TextViewExtensions.UpdateText (nil simply clears)
        [text_view mauiUpdatePlaceholderVisibility];
        // MapText → MapFormatting: UpdateMaxLength + UpdateCharacterSpacing (+ alignment re-apply).
        apply_max_length(text_view, view);
        map_character_spacing(handler, view);
        text_view.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        // The content height just changed, so re-run ShouldCenterVertically (MauiTextView.LayoutSubviews
        // re-centers on every layout) to keep a Center/End editor aligned against the new content size.
        text_view.mauiVerticalTextAlignment = static_cast<NSInteger>(view.vertical_text_alignment());
        apply_vertical_text_alignment_for(text_view, view.vertical_text_alignment());
        // Keep the proxy's previous-value tracker in sync with programmatic text changes.
        if (auto* const proxy = (MauiIosEditorProxy*)objc_getAssociatedObject(text_view, &k_proxy_key))
        {
            proxy.previousText = text_view.text != nil ? text_view.text : @"";
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
            // TextViewExtensions.UpdateIsReadOnly → UITextView.Editable.
            platform->is_read_only = view.is_read_only();
            as_text_view(platform->native).editable = view.is_read_only() ? NO : YES;
        }
    }

    void editor_handler::map_max_length(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            apply_max_length(as_text_view(platform->native), view);
        }
    }

    void editor_handler::map_text_color(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // An unset TextColor (default-constructed sentinel) must resolve to the dynamic system
            // label color so text stays legible in dark mode — mirrors EditorHandler.iOS's reliance on
            // the platform's default label color rather than forcing opaque black. Explicit colors win.
            // is-set discriminator (see label_handler.mm): an explicit TextColor=Black equals the
            // default-constructed sentinel by value, so key off BindableObject.IsSet, not `!= color{}`.
            const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            as_text_view(platform->native).textColor =
                color_is_set ? to_ui_color(view.text_color()) : UIColor.labelColor;
        }
    }

    void editor_handler::map_font(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // TextViewExtensions.UpdateFont's effective control default is MAUI's FontSize creator value
            // (SystemFontSize), not the LabelFontSize fallback — see default_text_font_size().
            MauiIosEditorTextView* const text_view = as_text_view(platform->native);
            text_view.font = to_ui_font(view.font(), maui::platform::ios::default_text_font_size());
            // MauiTextView.Font setter → UpdatePlaceholderFont: the placeholder tracks the editor's font.
            sync_placeholder_style(text_view, view);
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
        // TextViewExtensions.UpdateCharacterSpacing: kern the attributed text (only when
        // WithCharacterSpacing returns a value — the empty / nothing-to-unset cases are no-ops).
        MauiIosEditorTextView* const text_view = as_text_view(platform->native);
        NSAttributedString* const kerned = with_character_spacing(text_view.attributedText, view.character_spacing());
        if (kerned != nil)
        {
            text_view.attributedText = kerned;
        }
    }

    void editor_handler::map_horizontal_text_alignment(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            MauiIosEditorTextView* const text_view = as_text_view(platform->native);
            text_view.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
            // MauiTextView.TextAlignment setter → UpdateHorizontalTextAlignment: the placeholder follows.
            sync_placeholder_style(text_view, view);
        }
    }

    void editor_handler::map_vertical_text_alignment(editor_handler& handler, i_editor& view)
    {
        // TextViewExtensions.UpdateVerticalTextAlignment → MauiTextView.VerticalTextAlignment setter, which
        // fires ShouldCenterVertically: store the alignment (so layoutSubviews keeps re-centering against
        // it), then re-center the content within the view now.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
            MauiIosEditorTextView* const text_view = as_text_view(platform->native);
            text_view.mauiVerticalTextAlignment = static_cast<NSInteger>(platform->vertical_alignment);
            apply_vertical_text_alignment_for(text_view, platform->vertical_alignment);
        }
    }

    void editor_handler::map_is_text_prediction_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
            as_text_view(platform->native).autocorrectionType =
                view.is_text_prediction_enabled() ? UITextAutocorrectionTypeYes : UITextAutocorrectionTypeNo;
        }
    }

    void editor_handler::map_is_spell_check_enabled(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
            as_text_view(platform->native).spellCheckingType =
                view.is_spell_check_enabled() ? UITextSpellCheckingTypeYes : UITextSpellCheckingTypeNo;
        }
    }

    void editor_handler::map_keyboard(editor_handler& handler, i_editor& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        // TextViewExtensions.UpdateKeyboard: ApplyKeyboard, then (for non-custom keyboards) re-apply the
        // prediction/spellcheck pushes, then ReloadInputViews so a live keyboard re-styles.
        MauiIosEditorTextView* const text_view = as_text_view(platform->native);
        maui::platform::ios::apply_keyboard(text_view, view.keyboard());
        if (!maui::platform::ios::is_custom_keyboard(view.keyboard()))
        {
            map_is_text_prediction_enabled(handler, view);
            map_is_spell_check_enabled(handler, view);
        }
        [text_view reloadInputViews];
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

    maui::graphics::size editor_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // EditorHandler.iOS.GetDesiredSize: infinite constraints collapse to SizeThatFits.
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_text_view(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void editor_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_text_view(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void editor_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

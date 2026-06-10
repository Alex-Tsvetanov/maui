// search_bar_handler — Apple (AppKit / macOS) platform recipe. The real-native twin of the headless
// partial: the managed platform view is an NSSearchField (held, retained, in
// search_bar_platform::native), the value properties map to it, and native events flow back through an
// NSSearchFieldDelegate trampoline (text edits) + the field's target-action (the search press, fired by
// AppKit when the user commits the search with Return) to i_search_bar::send_text_changed(old, new) /
// send_search_button_pressed(). Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from SearchBarHandler.iOS.cs + MauiSearchBar.cs (UIKit's UISearchBar): MAUI's macOS
// support is Mac Catalyst, so there is no AppKit SearchBarHandler in the read-only C# source — the
// cross-platform contract (i_search_bar, the mapper) is faithful, and the AppKit specifics are the
// standard NSSearchField equivalents of the UISearchBar recipe:
//   - UISearchBar.TextSetOrChanged / EditingChanged → controlTextDidChange: → send_text_changed
//   - UISearchBar.SearchButtonClicked (Return)      → the search field's action → send_search_button_pressed
//   - the QueryEditor (the inner UITextField) collapses onto the NSSearchField itself — an NSSearchField
//     IS an editable NSTextField, so the font/color/alignment maps go straight to it.
// Documented AppKit deviations: cancel_button_color / search_icon_color are recorded as mirrors only
// (the NSSearchFieldCell's cancel/search NSButtonCells render template images; C# tints the private
// UIKit subviews — no public AppKit tint exists); return_type is mirror-only (hardware keyboard);
// vertical_text_alignment is mirror-only (the search cell centers its single line natively).

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
#include "maui/core/i_search_bar.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the NSSearchField's editing notifications + search action to the C++
// handler's virtual view. Tracks the previous string so an edit can report the *old* value.
@interface MauiSearchBarDelegate : NSObject <NSSearchFieldDelegate>
@property(nonatomic) maui::core::search_bar_handler* handler;
@property(nonatomic, copy) NSString* previousText;
- (void)onSearch:(id)sender;
@end

namespace
{
    // Key for the associated MauiSearchBarDelegate kept alive by the NSSearchField (its `delegate` and
    // `target` are both weak).
    const char k_delegate_key = 0;

    NSSearchField* as_search_field(void* native)
    {
        return (__bridge NSSearchField*)native;
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

    // Rebuild the placeholder (plain when no explicit color/kerning, attributed otherwise) — the same
    // collapse as the entry recipe (the default-constructed opaque-black color counts as "unset").
    void refresh_search_placeholder(NSSearchField* field, const maui::core::i_search_bar& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        const maui::graphics::color color = view.placeholder_color();
        const bool explicit_color = color != maui::graphics::color{};
        NSColor* const foreground = explicit_color ? to_ns_color(color) : nil;
        const double spacing = view.character_spacing();
        if (foreground == nil && spacing == 0)
        {
            field.placeholderString = text != nil ? text : @"";
            return;
        }
        field.placeholderAttributedString = maui::platform::apple::placeholder_attributed(text, foreground, spacing);
    }

    // Re-apply kerning onto the field's text (the entry's refresh_entry_text_formatting shape).
    void refresh_search_text_formatting(NSSearchField* field, const maui::core::i_search_bar& view)
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
} // namespace

@implementation MauiSearchBarDelegate
- (void)controlTextDidChange:(NSNotification*)notification
{
    NSSearchField* const field = (NSSearchField*)notification.object;
    NSString* const old_value = self.previousText != nil ? self.previousText : @"";
    NSString* const new_value = field.stringValue != nil ? field.stringValue : @"";
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

- (void)onSearch:(id)sender
{
    // The NSSearchField action — with sendsWholeSearchString it fires when the user commits the search
    // (Return), the AppKit analog of UISearchBar.SearchButtonClicked. AppKit also fires it with an
    // EMPTY string when the user clears via the cancel button; C#'s OnSearchButtonClicked has no such
    // empty-fire, so the cancel-clear is forwarded as a text change only (the field already cleared).
    auto* const field = (NSSearchField*)sender;
    if (self.handler == nullptr || field == nil)
    {
        return;
    }
    auto* const view = self.handler->virtual_view();
    if (view == nullptr)
    {
        return;
    }
    view->send_search_button_pressed();
}
@end

namespace maui::core
{
    search_bar_platform::~search_bar_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void search_bar_platform::update_visibility(maui::core::visibility value)
    {
        as_search_field(native).hidden = value != maui::core::visibility::visible;
    }

    void search_bar_platform::update_opacity(double value)
    {
        as_search_field(native).alphaValue = value;
    }

    void search_bar_platform::update_is_enabled(bool value)
    {
        [as_search_field(native) setEnabled:static_cast<BOOL>(value)];
    }

    void search_bar_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_search_field(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
        NSSearchField* const field = [[NSSearchField alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        // Fire the action only when the user commits the search (Return / cancel), not per keystroke —
        // matching UISearchBar's SearchButtonClicked semantics (text changes ride the delegate).
        field.sendsWholeSearchString = YES;
        field.sendsSearchStringImmediately = NO;
        platform->native = (__bridge_retained void*)field; // the void* slot owns one reference
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        NSSearchField* const field = as_search_field(platform.native);
        MauiSearchBarDelegate* const delegate = [[MauiSearchBarDelegate alloc] init];
        delegate.handler = this;
        delegate.previousText = field.stringValue != nil ? field.stringValue : @"";
        field.delegate = delegate; // weak...
        field.target = delegate;   // ...and the target-action convention is weak too,
        field.action = @selector(onSearch:);
        // ...so keep the delegate alive for the field's lifetime via an associated object.
        objc_setAssociatedObject(field, &k_delegate_key, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        NSSearchField* const field = as_search_field(platform.native);
        field.delegate = nil;
        field.target = nil;
        field.action = nil;
        objc_setAssociatedObject(field, &k_delegate_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void search_bar_handler::map_text(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        NSSearchField* const field = as_search_field(platform->native);
        field.stringValue = value != nil ? value : @"";
        refresh_search_text_formatting(field, view); // MapText → MapFormatting
        if (auto* const delegate = (MauiSearchBarDelegate*)objc_getAssociatedObject(field, &k_delegate_key))
        {
            delegate.previousText = field.stringValue;
        }
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_field(platform->native), view);
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_field(platform->native), view);
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_read_only = view.is_read_only();
            as_search_field(platform->native).editable = view.is_read_only() ? NO : YES;
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Defensive trim, mirroring SearchBarExtensions.UpdateMaxLength (the keystroke gate is the
        // delegate's ShouldChangeText on UIKit; AppKit's field editor path keeps the trim authoritative).
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSSearchField* const field = as_search_field(platform->native);
        NSString* const current = field.stringValue != nil ? field.stringValue : @"";
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            field.stringValue = [current substringToIndex:static_cast<NSUInteger>(max_length)];
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_search_field(platform->native).textColor = to_ns_color(view.text_color());
        }
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_search_field(platform->native).font = to_ns_font(view.font());
        }
    }

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        NSSearchField* const field = as_search_field(platform->native);
        refresh_search_text_formatting(field, view);
        refresh_search_placeholder(field, view);
        field.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_search_field(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        // The NSSearchFieldCell centers its single line natively; the mirror records the value
        // (documented deviation — UIKit drives the inner text field's contentVerticalAlignment).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Like the entry: the field editor (when attached) carries autocorrection; the mirror records
        // the value either way.
        platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
        if (NSText* const editor = as_search_field(platform->native).currentEditor;
            [editor isKindOfClass:[NSTextView class]])
        {
            ((NSTextView*)editor).automaticTextReplacementEnabled = view.is_text_prediction_enabled() ? YES : NO;
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_spell_check_enabled = view.is_spell_check_enabled();
        if (NSText* const editor = as_search_field(platform->native).currentEditor;
            [editor isKindOfClass:[NSTextView class]])
        {
            ((NSTextView*)editor).continuousSpellCheckingEnabled = view.is_spell_check_enabled() ? YES : NO;
        }
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        // During an editing session, move the field editor's caret (the entry recipe); otherwise the
        // mirror records the intent.
        if (NSText* const editor = as_search_field(platform->native).currentEditor;
            [editor isKindOfClass:[NSTextView class]])
        {
            const NSUInteger length = editor.string.length;
            const auto requested = static_cast<NSUInteger>(view.cursor_position() < 0 ? 0 : view.cursor_position());
            const NSUInteger location = requested <= length ? requested : length;
            const auto span_req = static_cast<NSUInteger>(view.selection_length() < 0 ? 0 : view.selection_length());
            const NSUInteger span = (location + span_req <= length) ? span_req : (length - location);
            editor.selectedRange = NSMakeRange(location, span);
        }
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
        map_cursor_position(handler, view); // both re-establish the whole range from the pair
    }

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        // The NSSearchFieldCell's cancelButtonCell renders a template image with no public tint; C#
        // tints UIKit's private cancel-button subview. The mirror records the value (documented).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->cancel_button_color = view.cancel_button_color();
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        // Same as the cancel button: the search-loupe button cell has no public tint. Mirror only.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->search_icon_color = view.search_icon_color();
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        // macOS has no software return-key styling; the mirror records the value (the entry collapse).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->bar_return_type = view.return_type();
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double /*width_constraint*/,
                                                              double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_search_field(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_search_field(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed via the shared apple_view_ops helpers.
    void search_bar_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void search_bar_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed via the shared apple_visual_ops helpers.
    void search_bar_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void search_bar_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void search_bar_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag (shared apple_semantics_ops helpers).
    void search_bar_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void search_bar_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core

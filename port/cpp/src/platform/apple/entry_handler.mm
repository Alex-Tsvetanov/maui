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
#include "apple_view_ops.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

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
        field.editable = YES;
        field.selectable = YES;
        field.bezeled = YES;
        field.bezelStyle = NSTextFieldRoundedBezel;
        field.drawsBackground = YES;
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
    }

    void entry_handler::on_disconnect_handler(entry_platform& platform)
    {
        NSTextField* const field = as_field(platform.native);
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
        // Keep the delegate's previous-value tracker in sync with programmatic text changes.
        if (auto* const delegate = (MauiEntryDelegate*)objc_getAssociatedObject(field, &k_delegate_key))
        {
            delegate.previousText = field.stringValue;
        }
    }

    void entry_handler::map_placeholder(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string placeholder(view.placeholder());
        NSString* const value = [NSString stringWithUTF8String:placeholder.c_str()];
        as_field(platform->native).placeholderString = value != nil ? value : @"";
    }

    void entry_handler::map_placeholder_color(entry_handler& /*handler*/, i_entry& /*view*/)
    {
        // TODO: AppKit colors a placeholder only via an attributed placeholderAttributedString, and
        // setting that nulls out the plain `placeholderString` getter (they are separate backing stores).
        // Coloring the placeholder therefore conflicts with keeping `placeholderString` as the readable
        // source of truth; an attributed-placeholder path is deferred (cf. the deferred character_spacing
        // kerning). The headless backend mirrors placeholder_color.
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
        // AppKit has no SecureTextEntry toggle; swap the cell between secure and plain, preserving state.
        // Swapping the cell resets cell-backed appearance (notably the font), so capture and re-apply it
        // afterwards — otherwise toggling is_password at runtime (after map_font ran) would drop the font.
        NSString* const current = field.stringValue != nil ? field.stringValue : @"";
        NSString* const placeholder = field.placeholderString;
        NSFont* const font = field.font;
        NSColor* const text_color = field.textColor;
        NSTextFieldCell* const replacement =
            secure ? [[NSSecureTextFieldCell alloc] initTextCell:@""] : [[NSTextFieldCell alloc] initTextCell:@""];
        replacement.editable = field.editable;
        replacement.selectable = field.selectable;
        replacement.bezeled = field.bezeled;
        replacement.bezelStyle = field.bezelStyle;
        replacement.alignment = field.alignment;
        field.cell = replacement;
        field.stringValue = current;
        if (placeholder != nil)
        {
            field.placeholderString = placeholder;
        }
        if (font != nil)
        {
            field.font = font;
        }
        field.textColor = text_color;
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

    void entry_handler::map_character_spacing(entry_handler& /*handler*/, i_entry& /*view*/)
    {
        // TODO: AppKit needs an attributed string (NSKernAttributeName) for per-character spacing — a
        // larger change (it overrides the plain stringValue path). Deferred; the headless backend maps it.
    }

    void entry_handler::map_horizontal_text_alignment(entry_handler& handler, i_entry& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_field(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void entry_handler::map_vertical_text_alignment(entry_handler& /*handler*/, i_entry& /*view*/)
    {
        // TODO: NSTextField has no vertical text alignment without a custom field editor / cell. Deferred;
        // the headless backend mirrors it.
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
} // namespace maui::core

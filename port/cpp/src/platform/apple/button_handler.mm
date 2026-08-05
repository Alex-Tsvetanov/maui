// button_handler — Apple (AppKit / macOS) platform recipe. The real-native twin of the headless
// partial: the managed platform view is an NSButton (held, retained, in button_platform::native), Text
// maps to NSButton.title, and the native click flows back through a target-action trampoline to
// i_button::send_clicked(). Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Translated from ButtonHandler.iOS.cs (UIKit): MAUI's macOS support is Mac Catalyst (UIKit), so there
// is no AppKit ButtonHandler in the read-only C# source to port verbatim — the cross-platform contract
// (i_button, the mapper) is faithful, and the AppKit specifics are the standard NSButton equivalents of
// the UIButton recipe. NSButton's action fires on a completed click (mouse-up inside), matching
// TouchUpInside → Clicked.

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
#include "maui/core/bindable_object.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards NSButton's target-action to the C++ handler's virtual view.
@interface MauiButtonTarget : NSObject
@property(nonatomic) maui::core::button_handler* handler;
- (void)onClick:(id)sender;
@end

@implementation MauiButtonTarget
- (void)onClick:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_clicked();
        }
    }
}
@end

// AppKit has no NSButton.contentEdgeInsets (the UIButton mechanism C#'s UpdatePadding uses), so a custom
// cell carries the maui padding and reserves it around the content. It grows the cell's reported size by
// the padding (so fittingSize / sizeToFit account for it, mirroring how ContentEdgeInsets enlarges a
// UIButton) and insets the interior drawing rect by the same amount. The insets are public so the seam
// test can read back what map_padding pushed (the observable-native-state convention the other handlers
// follow).
@interface MauiButtonCell : NSButtonCell
@property(nonatomic) NSEdgeInsets contentInsets;
@end

@implementation MauiButtonCell
- (NSSize)cellSizeForBounds:(NSRect)rect
{
    NSSize size = [super cellSizeForBounds:rect];
    size.width += self.contentInsets.left + self.contentInsets.right;
    size.height += self.contentInsets.top + self.contentInsets.bottom;
    return size;
}

- (NSRect)drawingRectForBounds:(NSRect)rect
{
    const NSRect base = [super drawingRectForBounds:rect];
    return NSMakeRect(base.origin.x + self.contentInsets.left, base.origin.y + self.contentInsets.top,
                      base.size.width - (self.contentInsets.left + self.contentInsets.right),
                      base.size.height - (self.contentInsets.top + self.contentInsets.bottom));
}
@end

namespace
{
    // Key for the associated MauiButtonTarget kept alive by the NSButton (its `target` is weak).
    const char k_target_key = 0;

    // ButtonHandler.DefaultPadding — "the padding that Xcode has when 'Default' content insets are
    // used" (left/right 12, top/bottom 7); ButtonHandler.MapPadding always passes it to UpdatePadding,
    // which substitutes it when the cross-platform Padding is NaN (Button's default). Mirrors the iOS
    // backend's k_default_padding_* so an unstyled NSButton reserves the native-default content insets
    // rather than collapsing to bare glyph width (the `clipping` digit-row regression).
    constexpr double k_default_padding_horizontal = 12;
    constexpr double k_default_padding_vertical = 7;

    NSButton* as_button(void* native)
    {
        return (__bridge NSButton*)native;
    }

    using maui::platform::apple::to_ns_color;
    using maui::platform::apple::to_ns_font;

    NSImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const raw = [NSString stringWithUTF8String:file.c_str()];
        if (raw == nil)
        {
            return nil;
        }
        return [[NSImage alloc] initWithContentsOfFile:raw];
    }

    // NSCachesDirectory (the image_handler convention) for the loader's on-disk uri cache.
    std::string platform_cache_directory()
    {
        NSArray<NSString*>* const paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        if (paths.count == 0)
        {
            return {};
        }
        NSString* const dir = [paths objectAtIndex:0];
        const char* const utf8 = dir.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }
} // namespace

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
        void detach_trampolines(button_platform& platform)
        {
            NSButton* const button = as_button(platform.native);
            button.target = nil;
            button.action = nil;
            if (auto* const trampoline = (MauiButtonTarget*)objc_getAssociatedObject(button, &k_target_key))
            {
                trampoline.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(button, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
    } // namespace

    button_platform::~button_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void button_platform::update_visibility(maui::core::visibility value)
    {
        as_button(native).hidden = value != maui::core::visibility::visible;
    }

    void button_platform::update_opacity(double value)
    {
        as_button(native).alphaValue = value;
    }

    void button_platform::update_is_enabled(bool value)
    {
        [as_button(native) setEnabled:static_cast<BOOL>(value)];
    }

    void button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        NSButton* const button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        // Install the padding-aware cell, copying the default cell's appearance so the bezel/title style
        // is preserved (map_padding then drives its insets).
        MauiButtonCell* const cell = [[MauiButtonCell alloc] initTextCell:@""];
        cell.bezelStyle = NSBezelStyleRounded;
        button.cell = cell;
        [button setButtonType:NSButtonTypeMomentaryPushIn];
        [button setBezelStyle:NSBezelStyleRounded];
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        NSButton* const button = as_button(platform.native);
        MauiButtonTarget* const target = [[MauiButtonTarget alloc] init];
        target.handler = this;
        button.target = target; // NSButton holds target weakly (target-action convention)...
        button.action = @selector(onClick:);
        // ...so keep it alive for the button's lifetime via an associated object.
        objc_setAssociatedObject(button, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        detach_trampolines(platform);
    }

    namespace
    {
        // Rebuild the NSButton's attributed title from its current plain title, mirroring
        // ButtonExtensions.UpdateCharacterSpacing (TitleLabel.AttributedText + WithCharacterSpacing +
        // WithTextColor). Kerning needs an attributed title, and a kerned title must carry its color
        // explicitly (the plain contentTintColor does not apply to attributed text), so when spacing != 0
        // the title is rebuilt attributed WITH the text color. When spacing == 0 the attributed title is
        // cleared so the plain `title` (colored by map_text_color's contentTintColor) is shown — matching
        // C#, where WithCharacterSpacing returns null at spacing 0 and no attributed title is set.
        void refresh_button_title_formatting(NSButton* button, const i_text_button& view)
        {
            const double spacing = view.character_spacing();
            if (spacing == 0)
            {
                // Drop any prior attributed title so the plain `title` shows (with contentTintColor).
                [button setAttributedTitle:[[NSAttributedString alloc] initWithString:button.title]];
                return;
            }
            // Same unset-color discrimination as map_text_color: an explicit TextColor wins, otherwise
            // fall back to the system default title color instead of the black default-constructed sentinel.
            const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            NSColor* const foreground = color_is_set ? to_ns_color(view.text_color()) : NSColor.controlTextColor;
            NSAttributedString* const attributed =
                maui::platform::apple::kern_attributed(button.title, spacing, foreground);
            [button setAttributedTitle:attributed != nil ? attributed
                                                         : [[NSAttributedString alloc] initWithString:button.title]];
        }
    } // namespace

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); setTitle: wants non-null.
        NSString* const raw = [NSString stringWithUTF8String:text.c_str()];
        NSButton* const button = as_button(platform->native);
        button.title = raw != nil ? raw : @"";
        // Any text update re-applies the attributed formatting (C# MapText -> MapFormatting).
        refresh_button_title_formatting(button, view);
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            NSButton* const button = as_button(platform->native);
            // TextColor defaults to null in C# (left at the native control tint); the port's
            // default-constructed color{} is opaque BLACK, not a "leave it alone" sentinel, so this must
            // discriminate on whether the property was explicitly SET (BindableObject.IsSet) rather than
            // trust the getter — the same unset-color fix already applied elsewhere (see "Unset-color
            // sentinel collision" in the port's fix history). Unset: leave contentTintColor at its NSButton
            // system default instead of stomping it to black.
            const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            button.contentTintColor = color_is_set ? to_ns_color(view.text_color()) : nil;
            // A kerned (attributed) title carries its own color, so re-apply it (C# MapTextColor's
            // UpdateTextColor sets the plain title color; the attributed title is rebuilt to match).
            refresh_button_title_formatting(button, view);
        }
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).font = to_ns_font(view.font());
        }
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // Rebuild the attributed title with the new kerning (ButtonExtensions.UpdateCharacterSpacing).
            refresh_button_title_formatting(as_button(platform->native), view);
        }
    }

    void button_handler::map_padding(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSButton* const button = as_button(platform->native);
        // The custom cell reserves the padding around the content (the AppKit analog of UIButton's
        // ContentEdgeInsets; UpdatePadding pushes button.Padding straight onto the insets).
        if (auto* const cell = [button.cell isKindOfClass:[MauiButtonCell class]] ? (MauiButtonCell*)button.cell : nil)
        {
            // ButtonExtensions.UpdatePadding(button, DefaultPadding): a NaN Padding (Button's default)
            // falls back to DefaultPadding(12,7) so the cell reserves the native-default insets instead
            // of the NaN that Button.padding_property now defaults to. (iOS/Android do the same.)
            maui::core::thickness padding = view.padding();
            if (padding.is_nan())
            {
                padding = maui::core::thickness(k_default_padding_horizontal, k_default_padding_vertical);
            }
            cell.contentInsets = NSEdgeInsetsMake(padding.top, padding.left, padding.bottom, padding.right);
            [button setNeedsDisplay:YES];
            [button invalidateIntrinsicContentSize];
        }
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSButton* const button = as_button(platform->native);
        button.wantsLayer = YES;
        button.layer.borderColor = to_ns_color(view.stroke_color()).CGColor;
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSButton* const button = as_button(platform->native);
        button.wantsLayer = YES;
        button.layer.borderWidth = view.stroke_thickness();
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSButton* const button = as_button(platform->native);
        button.wantsLayer = YES;
        button.layer.cornerRadius = static_cast<CGFloat>(view.corner_radius());
    }

    maui::graphics::size button_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_button(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    // An NSButton cannot render smaller than its intrinsic content (title + insets) — the AppKit twin of the
    // UIKit floor. MAUI surfaces it through ButtonHandler.NeedsContainer/WrapperView; with no container in the
    // port, view<>::measure honors this flag so an under-sized WidthRequest/HeightRequest grows to the content
    // instead of clipping the title.
    bool button_handler::content_is_minimum_size() const
    {
        return true;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers
    // (M4d ViewMapper visuals). `native` is this struct's NSView handle.
    void button_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void button_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the native view via the shared
    // apple_semantics_ops helpers (M5d native a11y / hit-test). `native` is this struct's NSView handle.
    void button_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void button_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----
    // The AppKit twin of the iOS recipe: NSButton.image carries the picture; AppKit has no
    // UIImageRenderingMode (the AlwaysOriginal step is iOS-only) and applies the image immediately, so no
    // explicit layout-refresh is needed. The on-disk uri cache mirrors the image_button_handler wiring.
    void button_handler::configure_loader(maui::core::image_source_loader& loader)
    {
        loader.set_disk_cache_directory(platform_cache_directory());
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = load_image_from_file(file_src.file());
    }

    void button_handler::apply_loaded_result(button_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = result.loaded() ? (__bridge NSImage*)result.image() : nil;
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = nil;
    }
} // namespace maui::core

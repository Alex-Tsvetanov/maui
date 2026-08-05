// radio_button_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is a real
// radio-style NSButton (NSButtonTypeRadio: the round indicator + a title label), the string content
// rides `title` (kerned/colored through the shared apple_text_ops attributed recipe — the button
// handler's exact convention), the stroke/corner ride the layer, and a native click flows back through
// a target-action trampoline to send_is_checked(true) (a radio click SELECTS — the cross-platform
// RadioButtonGroup unchecks the others; sibling NSButton radio auto-grouping by shared action is NOT
// used, keeping the exclusion in the Controls layer exactly like C#). Compiled as Objective-C++ with
// ARC only for the `apple` backend.
//
// MAUI's macOS support is Mac Catalyst, so there is no AppKit RadioButtonHandler in the read-only C#
// source — this partial is the AppKit translation of the contract: NSButtonTypeRadio stands in for the
// DefaultTemplate's Ellipse indicator pair, and state follows IsChecked.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_text_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the radio NSButton's target-action to the C++ handler's virtual view.
@interface MauiRadioButtonTarget : NSObject
@property(nonatomic) maui::core::radio_button_handler* handler;
- (void)onSelect:(id)sender;
@end

@implementation MauiRadioButtonTarget
- (void)onSelect:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            // RadioButton.SelectRadioButton: a tap checks the button (the from-handler write).
            view->send_is_checked(true);
        }
    }
}
@end

namespace
{
    // Key for the associated MauiRadioButtonTarget kept alive by the NSButton (its `target` is weak).
    const char k_target_key = 0;

    NSButton* as_button(void* native)
    {
        return (__bridge NSButton*)native;
    }

    using maui::platform::apple::to_ns_color;
    using maui::platform::apple::to_ns_font;

    // Rebuild the radio button's attributed title from its plain title — the button handler's
    // refresh_button_title_formatting twin over the i_text_style face: kerning needs an attributed
    // title (carrying its color explicitly); at spacing 0 the attributed title is reset so the plain
    // `title` (colored by contentTintColor) shows.
    void refresh_radio_title_formatting(NSButton* button, const maui::core::i_text_style& view)
    {
        const double spacing = view.character_spacing();
        if (spacing == 0)
        {
            [button setAttributedTitle:[[NSAttributedString alloc] initWithString:button.title]];
            return;
        }
        // Same unset-color discrimination as map_text_color below (and button_handler's): an explicit
        // TextColor wins, else the system title color rather than the port's opaque-black default.
        NSColor* const explicit_color = maui::platform::apple::explicit_text_color_or_nil(view);
        NSColor* const foreground = explicit_color != nil ? explicit_color : NSColor.controlTextColor;
        NSAttributedString* const attributed =
            maui::platform::apple::kern_attributed(button.title, spacing, foreground);
        [button setAttributedTitle:attributed != nil ? attributed
                                                     : [[NSAttributedString alloc] initWithString:button.title]];
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
        void detach_trampolines(radio_button_platform& platform)
        {
            NSButton* const button = as_button(platform.native);
            button.target = nil;
            button.action = nil;
            if (auto* const trampoline = (MauiRadioButtonTarget*)objc_getAssociatedObject(button, &k_target_key))
            {
                trampoline.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(button, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
    } // namespace

    radio_button_platform::~radio_button_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void radio_button_platform::update_visibility(maui::core::visibility value)
    {
        as_button(native).hidden = value != maui::core::visibility::visible;
    }

    void radio_button_platform::update_opacity(double value)
    {
        as_button(native).alphaValue = value;
    }

    void radio_button_platform::update_is_enabled(bool value)
    {
        [as_button(native) setEnabled:static_cast<BOOL>(value)];
    }

    void radio_button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void radio_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void radio_button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void radio_button_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void radio_button_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void radio_button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void radio_button_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void radio_button_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        NSButton* const button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        [button setButtonType:NSButtonTypeRadio];
        button.title = @"";
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        NSButton* const button = as_button(platform.native);
        MauiRadioButtonTarget* const target = [[MauiRadioButtonTarget alloc] init];
        target.handler = this;
        button.target = target; // weak (target-action convention)...
        button.action = @selector(onSelect:);
        // ...so keep it alive for the button's lifetime via an associated object.
        objc_setAssociatedObject(button, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        detach_trampolines(platform);
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked();
        as_button(platform->native).state = view.is_checked() ? NSControlStateValueOn : NSControlStateValueOff;
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string content(view.content_as_string());
        platform->content = content;
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); setTitle: wants non-null.
        NSString* const raw = [NSString stringWithUTF8String:content.c_str()];
        NSButton* const button = as_button(platform->native);
        button.title = raw != nil ? raw : @"";
        // Any content update re-applies the attributed formatting (the button MapText convention).
        refresh_radio_title_formatting(button, view);
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            NSButton* const button = as_button(platform->native);
            // nil contentTintColor = the system default, which follows the appearance (button_handler does
            // the same); the port's default-constructed TextColor is opaque black and must not be painted.
            button.contentTintColor = maui::platform::apple::explicit_text_color_or_nil(view);
            // A kerned (attributed) title carries its own color, so re-apply it.
            refresh_radio_title_formatting(button, view);
        }
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).font = to_ns_font(view.font());
        }
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            refresh_radio_title_formatting(as_button(platform->native), view);
        }
    }

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
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

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
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

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
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

    maui::graphics::size radio_button_handler::get_desired_size(double /*width_constraint*/,
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

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

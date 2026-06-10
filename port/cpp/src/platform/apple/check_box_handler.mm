// check_box_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an NSButton
// with the native CHECKBOX style (NSButtonTypeSwitch — AppKit's built-in check box; held, retained, in
// check_box_platform::native): IsChecked maps to the button state, and the native toggle flows back
// through a target-action trampoline to i_check_box::send_is_checked. Compiled as Objective-C++ with
// ARC only for the `apple` backend.
//
// Translated from CheckBoxHandler.iOS.cs (whose platform view is the DRAWN MauiCheckBox — UIKit has no
// native check box; AppKit does, so the drawn-control port lives in the ios .mm and this backend uses
// the genuine NSButton checkbox). AppKit notes:
//  - Foreground maps to contentTintColor (macOS tints the checkbox accent with it); a null foreground
//    restores nil (the system accent), mirroring `Color?.AsPaint()`'s null = platform default.
//  - The MinimumSize-44 floor is the iOS handler's touch-target rule; AppKit keeps fittingSize (a
//    pointer target needs no 44pt floor) — documented deviation.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/i_check_box.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the checkbox NSButton's target-action (fired on a state flip) to the C++
// handler's virtual view — the CheckBoxHandler.OnCheckedChanged port.
@interface MauiCheckBoxTarget : NSObject
@property(nonatomic) maui::core::check_box_handler* handler;
- (void)onCheckedChanged:(id)sender;
@end

@implementation MauiCheckBoxTarget
- (void)onCheckedChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    NSButton* const native = (NSButton*)sender;
    if (view != nullptr && native != nil)
    {
        const bool native_checked = native.state == NSControlStateValueOn;
        if (view->is_checked() != native_checked)
        {
            view->send_is_checked(native_checked);
        }
    }
}
@end

namespace
{
    // Key for the associated MauiCheckBoxTarget kept alive by the NSButton (its `target` is weak).
    const char k_target_key = 0;

    NSButton* as_check_box(void* native)
    {
        return (__bridge NSButton*)native;
    }

    using maui::platform::apple::to_ns_color;
} // namespace

namespace maui::core
{
    check_box_platform::~check_box_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void check_box_platform::update_visibility(maui::core::visibility value)
    {
        as_check_box(native).hidden = value != maui::core::visibility::visible;
    }

    void check_box_platform::update_opacity(double value)
    {
        as_check_box(native).alphaValue = value;
    }

    void check_box_platform::update_is_enabled(bool value)
    {
        [as_check_box(native) setEnabled:static_cast<BOOL>(value)];
    }

    void check_box_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_check_box(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void check_box_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void check_box_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void check_box_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void check_box_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void check_box_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void check_box_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void check_box_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        auto platform = std::make_unique<check_box_platform>();
        NSButton* const native = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        [native setButtonType:NSButtonTypeSwitch];          // the native AppKit check box
        native.title = @"";                                 // a bare box (MAUI's CheckBox carries no label)
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        NSButton* const native = as_check_box(platform.native);
        MauiCheckBoxTarget* const target = [[MauiCheckBoxTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onCheckedChanged:);
        // ...so keep it alive for the checkbox's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        NSButton* const native = as_check_box(platform.native);
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_check_box(platform->native).state = view.is_checked() ? NSControlStateValueOn : NSControlStateValueOff;
        }
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // MauiCheckBox.UpdateForeground's tint, translated: the paint's color tints the checkbox accent
        // (contentTintColor); a null foreground restores the system default (nil).
        const maui::graphics::paint* foreground = view.foreground();
        as_check_box(platform->native).contentTintColor =
            foreground != nullptr ? to_ns_color(foreground->background_color()) : nil;
    }

    maui::graphics::size check_box_handler::get_desired_size(double /*width_constraint*/,
                                                             double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_check_box(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void check_box_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_check_box(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

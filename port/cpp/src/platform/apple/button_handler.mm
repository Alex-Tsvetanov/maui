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

#include "apple_conversions.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
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

namespace
{
    // Key for the associated MauiButtonTarget kept alive by the NSButton (its `target` is weak).
    const char k_target_key = 0;

    NSButton* as_button(void* native)
    {
        return (__bridge NSButton*)native;
    }

    using maui::platform::apple::to_ns_color;
    using maui::platform::apple::to_ns_font;
} // namespace

namespace maui::core
{
    button_platform::~button_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        NSButton* const button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
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
        NSButton* const button = as_button(platform.native);
        button.target = nil;
        button.action = nil;
        objc_setAssociatedObject(button, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

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
        as_button(platform->native).title = raw != nil ? raw : @"";
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).contentTintColor = to_ns_color(view.text_color());
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

    void button_handler::map_character_spacing(button_handler& /*handler*/, i_text_button& /*view*/)
    {
        // TODO: AppKit needs an attributed title (NSKernAttributeName) for per-character spacing — a
        // larger change (it also overrides title color/font). Deferred; the headless backend maps it.
    }

    void button_handler::map_padding(button_handler& /*handler*/, i_button& /*view*/)
    {
        // TODO: NSButton has no direct padding; it needs a custom NSButtonCell / content insets. The
        // headless backend maps it. Deferred for AppKit.
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

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

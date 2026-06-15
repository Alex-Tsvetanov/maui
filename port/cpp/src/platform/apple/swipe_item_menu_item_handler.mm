// swipe_item_menu_item_handler — Apple (AppKit / macOS) platform recipe: the NSButton twin of the iOS
// UIButton recipe. MAUI's macOS support is Mac Catalyst (UIKit), so there is no AppKit
// SwipeItemMenuItemHandler in the read-only C# source — the cross-platform contract + mapper are
// faithful, and the AppKit specifics are the standard NSButton equivalents of the UIButton recipe
// (SwipeItemMenuItemHandler.iOS.cs): an NSButton with no user interaction (the SwipeView's pan drives
// activation), whose title/title-colour/font/background/icon the mapper pushes, that re-runs MapSource
// when its frame changes so the icon re-resizes. Compiled as Objective-C++ with ARC for the `apple`
// backend.
//
// DEVIATION (parent notify): identical to the iOS twin — the port's swipe host is a plain NSView and the
// shared swipe_machine reads each item's visibility live, so MapVisibility records the value rather than
// poking a parent MauiSwipeView (which AppKit does not have).

#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "apple_conversions.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/swipe_item_menu_item_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

// Obj-C: the SwipeItemButton twin — an NSButton that fires a C-callback when its frame changes (the C#
// SwipeItemButton overrides the Frame setter to raise FrameChanged).
@interface MauiSwipeItemNSButton : NSButton
@property(nonatomic) void (*frameChanged)(void*);
@property(nonatomic) void* frameContext;
@end

@implementation MauiSwipeItemNSButton
- (void)setFrame:(NSRect)frame
{
    [super setFrame:frame];
    if (self.frameChanged != nullptr)
    {
        self.frameChanged(self.frameContext);
    }
}
@end

namespace
{
    MauiSwipeItemNSButton* as_button(void* native)
    {
        return (__bridge MauiSwipeItemNSButton*)native;
    }

    void on_frame_changed(void* context)
    {
        auto* handler = static_cast<maui::core::swipe_item_menu_item_handler*>(context);
        if (handler != nullptr)
        {
            handler->update_value("source");
        }
    }
} // namespace

namespace maui::core
{
    swipe_item_menu_item_platform::~swipe_item_menu_item_platform()
    {
        if (native != nullptr)
        {
            MauiSwipeItemNSButton* const button = as_button(native);
            button.frameChanged = nullptr;
            button.frameContext = nullptr;
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<swipe_item_menu_item_platform> swipe_item_menu_item_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_item_menu_item_platform>();
        MauiSwipeItemNSButton* const button = [[MauiSwipeItemNSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        button.bezelStyle = NSBezelStyleRounded;
        [button setButtonType:NSButtonTypeMomentaryPushIn];
        button.enabled = NO; // the AppKit analog of UserInteractionEnabled=false (the pan drives activation)
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void swipe_item_menu_item_handler::connect()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        MauiSwipeItemNSButton* const button = as_button(platform->native);
        button.frameContext = this;
        button.frameChanged = &on_frame_changed;
    }

    void swipe_item_menu_item_handler::disconnect() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        MauiSwipeItemNSButton* const button = as_button(platform->native);
        button.frameChanged = nullptr;
        button.frameContext = nullptr;
    }

    void swipe_item_menu_item_handler::apply_text() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const std::string text(item_view_->text());
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        NSString* const safe = value != nil ? value : @"";
        MauiSwipeItemNSButton* const button = as_button(platform->native);
        button.identifier = safe; // the AppKit analog of RestorationIdentifier = Text
        button.title = safe;
        platform->title = text;
    }

    void swipe_item_menu_item_handler::apply_text_color() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto color = get_text_color(*item_view_);
        platform->has_title_color = color.has_value();
        if (color.has_value())
        {
            platform->title_color_argb = color->to_uint();
            MauiSwipeItemNSButton* const button = as_button(platform->native);
            NSMutableAttributedString* const attributed =
                [[NSMutableAttributedString alloc] initWithAttributedString:button.attributedTitle];
            [attributed addAttribute:NSForegroundColorAttributeName
                               value:maui::platform::apple::to_ns_color(*color)
                               range:NSMakeRange(0, attributed.length)];
            button.attributedTitle = attributed;
        }
    }

    void swipe_item_menu_item_handler::apply_character_spacing() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->character_spacing = item_view_->character_spacing();
    }

    void swipe_item_menu_item_handler::apply_font() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::core::font item_font = item_view_->font();
        platform->item_font = item_font;
        as_button(platform->native).font = maui::platform::apple::to_ns_font(item_font);
    }

    void swipe_item_menu_item_handler::apply_background() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::graphics::paint* const paint = item_view_->background();
        platform->has_background = paint != nullptr;
        if (paint != nullptr)
        {
            platform->background_argb = paint->background_color().to_uint();
        }
        as_button(platform->native).wantsLayer = YES;
        maui::platform::apple::apply_background(platform->native, paint);
    }

    void swipe_item_menu_item_handler::apply_source() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto source = item_view_->source();
        platform->has_source = source != nullptr && !source->is_empty();
        if (!platform->has_source)
        {
            as_button(platform->native).image = nil;
        }
    }

    void swipe_item_menu_item_handler::apply_visibility() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::core::visibility value = item_view_->visibility();
        platform->item_visibility = value;
        as_button(platform->native).hidden = value != maui::core::visibility::visible;
    }
} // namespace maui::core

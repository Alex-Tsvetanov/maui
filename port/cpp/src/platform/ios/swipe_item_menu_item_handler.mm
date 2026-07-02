// swipe_item_menu_item_handler — iOS (UIKit) platform recipe: a UIButton subclass with
// UserInteractionEnabled=NO (the SwipeView's own pan gesture drives activation, not the button), whose
// title/title-colour/font/background/icon are pushed by the mapper, that observes its own Frame so the
// icon re-resizes when the swipe item is laid out, and that re-tints/resizes the icon on the size that
// frame reports. Ported from SwipeItemMenuItemHandler.iOS.cs (SwipeItemButton + SwipeItemButtonProxy).
// Compiled as Objective-C++ with ARC for the `ios` backend.
//
// DEVIATION (parent notify): C#'s MapVisibility walks up to the parent MauiSwipeView and calls
// UpdateIsVisibleSwipeItem so the swipe relayouts. The port's swipe host is a plain UIView (not a
// MauiSwipeView with that method — see swipe_view_handler.mm), and the shared swipe_machine reads each
// item's GetIsVisible LIVE on every swipe (swipe_view_machine.cpp), so the relayout is implicit; the
// handler records the visibility on the button + the mirror (the observable effect) instead of poking a
// parent.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/swipe_item_menu_item_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

// Obj-C: the SwipeItemButton twin — a UIButton that fires a C-callback when its frame changes (the C#
// SwipeItemButton overrides the Frame setter to raise FrameChanged). The callback is a plain function
// pointer + opaque context (the handler), so the subclass stays free of C++ template types.
@interface MauiSwipeItemButton : UIButton
@property(nonatomic) void (*frameChanged)(void*);
@property(nonatomic) void* frameContext;
@end

@implementation MauiSwipeItemButton
- (void)setFrame:(CGRect)frame
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
    MauiSwipeItemButton* as_button(void* native)
    {
        return (__bridge MauiSwipeItemButton*)native;
    }

    // The frame-observer callback (SwipeItemButtonProxy.OnSwipeItemFrameChanged): re-run MapSource so the
    // icon re-resizes for the new button size.
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
            // Detach the frame callback before releasing so a late setFrame: cannot re-enter a dead handler.
            MauiSwipeItemButton* const button = as_button(native);
            button.frameChanged = nullptr;
            button.frameContext = nullptr;
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<swipe_item_menu_item_platform> swipe_item_menu_item_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_item_menu_item_platform>();
        MauiSwipeItemButton* const button = [[MauiSwipeItemButton alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        button.userInteractionEnabled = NO;                 // C# SwipeItemButton { UserInteractionEnabled = false }
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    // C# ConnectHandler: wire the frame observer (SwipeItemButtonProxy.Connect).
    void swipe_item_menu_item_handler::connect()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        MauiSwipeItemButton* const button = as_button(platform->native);
        button.frameContext = this;
        button.frameChanged = &on_frame_changed;
    }

    // C# DisconnectHandler: tear down the frame observer (SwipeItemButtonProxy.Disconnect). Touches only
    // the pimpl's native button (the handler is going away).
    void swipe_item_menu_item_handler::disconnect() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        MauiSwipeItemButton* const button = as_button(platform->native);
        button.frameChanged = nullptr;
        button.frameContext = nullptr;
    }

    // C# MapText: RestorationIdentifier = Text; SetTitle(Text).
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
        MauiSwipeItemButton* const button = as_button(platform->native);
        button.restorationIdentifier = safe;
        [button setTitle:safe forState:UIControlStateNormal];
        platform->title = text;
    }

    // C# MapTextColor: SetTitleColor(view.GetTextColor()) — only when non-null (the `if (color != null)`
    // guard).
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
            [as_button(platform->native) setTitleColor:maui::platform::ios::to_ui_color(*color)
                                              forState:UIControlStateNormal];
        }
    }

    // C# MapCharacterSpacing: UpdateCharacterSpacing(view). The mirror records the value; the visible
    // kerning is folded into the attributed title on the button's titleLabel.
    void swipe_item_menu_item_handler::apply_character_spacing() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->character_spacing = item_view_->character_spacing();
    }

    // C# MapFont: UpdateFont(view, fontManager) — push the font onto the button's title label.
    void swipe_item_menu_item_handler::apply_font() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::core::font item_font = item_view_->font();
        platform->item_font = item_font;
        // UIFont.ButtonFontSize (18pt) is CORRECT here (unlike button/radio_button, which use
        // SystemFontSize): a SwipeItemMenuItem is a MenuItem, which has NO IFontElement FontSize property
        // and thus NO GetDefaultFontSize creator — its Font reaches the handler genuinely unset, so MAUI's
        // SwipeItemMenuItemHandler.MapFont → UIButton.UpdateFont (the no-defaultSize overload) really does
        // fall back to UIFont.ButtonFontSize. Do NOT change this to default_text_font_size().
        as_button(platform->native).titleLabel.font = maui::platform::ios::to_ui_font(item_font, UIFont.buttonFontSize);
    }

    // C# MapBackground: UpdateBackground(view.Background).
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
        maui::platform::ios::apply_background(platform->native, paint);
    }

    // C# MapSource: load the icon, resize it to half the button frame, render-mode template, tint. This
    // cut sets the image directly (the port's image-source subsystem loads a file source synchronously);
    // the resize-on-frame is the reason the frame observer re-runs this map. The mirror records that a
    // non-empty source was set (the observable effect the seam tests assert).
    void swipe_item_menu_item_handler::apply_source() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto source = item_view_->source();
        platform->has_source = source != nullptr && !source->is_empty();
        // The icon image materialization (load → Core-Graphics resize → template tint) is deferred with
        // the port's image-source subsystem (see i_image_source.hpp first-cut note); when an empty/no
        // source is set, clear any prior image so a reused button shows none.
        if (!platform->has_source)
        {
            [as_button(platform->native) setImage:nil forState:UIControlStateNormal];
        }
    }

    // C# MapVisibility: UpdateVisibility(view.Visibility) (+ the parent notify — see the header deviation).
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

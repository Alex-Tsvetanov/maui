// search_bar_handler — iOS (UIKit) platform recipe. The managed platform view is a real UISearchBar,
// the value properties map to it (most through its inner UISearchTextField — C#'s QueryEditor), and
// native events flow back through a UISearchBarDelegate proxy to i_search_bar::send_text_changed(old,
// new) / send_search_button_pressed(). Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from SearchBarHandler.iOS.cs + MauiSearchBar.cs + Platform/iOS/SearchBarExtensions.cs:
//   CreatePlatformView = new MauiSearchBar { BarStyle = Default }; QueryEditor = GetSearchTextField()
//   (the public searchTextField on this SDK). MauiSearchBarProxy: TextSetOrChanged/EditingChanged →
//   UpdateText (send_text_changed), SearchButtonClicked → SearchButtonPressed
//   (send_search_button_pressed), CancelButtonClicked → virtualView.Text = string.Empty,
//   ShouldChangeTextInRange → the MaxLength gate, plus UpdateCancelButtonVisibility on text changes.
//   Map bodies below = SearchBarExtensions.UpdateText/UpdatePlaceholder/UpdateIsReadOnly/
//   UpdateMaxLength/UpdateCancelButton/UpdateSearchIcon(leftView tint)/UpdateReturnType +
//   TextFieldish font/color/alignment/prediction/spellcheck pushes onto the QueryEditor.
// Keyboard subsystem (W8-53): MapKeyboard pushes UIKeyboardType + the autocapitalization/spellcheck/
// autocorrection traits onto the search field (ios_keyboard_ops.hpp). The Done input accessory is NOT
// added (C# adds it only on Entry/Editor, not SearchBar). Focus (W8-53): the begin/end editing delegate
// callbacks reflect IsFocused; the shared view_command_mapper drives becomeFirstResponder on the bar.
// MovedToWindow re-fire (U22): MauiSearchBar : UISearchBar overrides the moved-to-window lifecycle and
// the proxy re-fires UpdateValue(CancelButtonColor) — the descendant cancel UIButton doesn't exist until
// UIKit builds the bar's internal hierarchy once it joins the window, so a color set earlier is lost and
// must be re-applied. The port mirrors this with MauiIosSearchBar (a UISearchBar subclass overriding
// -didMoveToWindow) firing a block into the handler, which calls update_value("cancel_button_color").
// Not ported here (deferred): the QueryEditor UITextPosition cursor arithmetic beyond the clamped-range
// write (the entry carries the full port).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_keyboard_ops.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

// Obj-C trampoline: forwards the UISearchBar's delegate callbacks to the C++ handler's virtual view.
// Ports SearchBarHandler.MauiSearchBarProxy — tracking the previous string for the (old, new) pair.
@interface MauiIosSearchBarProxy : NSObject <UISearchBarDelegate>
@property(nonatomic) maui::core::search_bar_handler* handler;
@property(nonatomic, copy) NSString* previousText;
@end

// MauiSearchBar.cs: a UISearchBar subclass that fires when added to the window hierarchy. C# raises an
// internal OnMovedToWindow event from MovedToWindow()/WillMoveToWindow(window != null); the port exposes
// the moved-to-window signal as a block the handler installs (the analog of the C# event subscription).
// Only the moved-to-window arm is ported — the descendant cancel button isn't built until UIKit places
// the bar in the window, so this is the hook that re-fires the cancel-button color.
@interface MauiIosSearchBar : UISearchBar
// Fires once the bar has been added to a window (didMoveToWindow with a non-nil window).
@property(nonatomic, copy) void (^onMovedToWindow)(void);
@end

namespace
{
    // Key for the associated MauiIosSearchBarProxy kept alive by the UISearchBar (`delegate` is weak).
    const char k_proxy_key = 0;

    UISearchBar* as_search_bar(void* native)
    {
        return (__bridge UISearchBar*)native;
    }

    UISearchTextField* query_editor(void* native)
    {
        return as_search_bar(native).searchTextField; // C# GetSearchTextField / QueryEditor
    }

    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_control_content_vertical_alignment;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::to_ui_return_key_type;
    using maui::platform::ios::with_character_spacing;

    // SearchBarExtensions.UpdatePlaceholder: plain placeholder unless an explicit color is set (the
    // default-constructed opaque-black color counts as "unset" — the entry collapse).
    void refresh_search_placeholder(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        const maui::graphics::color color = view.placeholder_color();
        // is-set discriminator: an explicit PlaceholderColor=Black equals the default-constructed
        // sentinel by value, so key off BindableObject.IsSet (else it falls to the plain muted gray).
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool explicit_color = bindable != nullptr && bindable->is_property_set("placeholder_color");
        if (!explicit_color)
        {
            bar.placeholder = text;
            return;
        }
        if (text == nil || text.length == 0)
        {
            bar.placeholder = text;
            return;
        }
        bar.searchTextField.attributedPlaceholder =
            [[NSAttributedString alloc] initWithString:text
                                            attributes:@{NSForegroundColorAttributeName : to_ui_color(color)}];
    }

    // SearchBarExtensions.UpdateCancelButton's predicate: exclude any UIButton that descends from a
    // UITextField — those are the clear ("x") button INSIDE the search field, not the cancel button
    // OUTSIDE it. C# does `btn => btn.FindParent(v => v is UITextField) == null`.
    bool has_text_field_ancestor(UIView* view)
    {
        for (UIView* parent = view.superview; parent != nil; parent = parent.superview)
        {
            if ([parent isKindOfClass:[UITextField class]])
            {
                return true;
            }
        }
        return false;
    }

    // SearchBarHandler.UpdateCancelButtonVisibility + SearchBarExtensions.UpdateCancelButton: the
    // cancel button shows while the bar has text; when visible and an explicit color is set, tint the
    // cancel UIButton. Mirror C#'s FindDescendantView<UIButton> with the UITextField-exclusion predicate
    // so the depth-first walk skips the search field's clear button and returns the genuine cancel button.
    UIButton* find_cancel_button(UIView* root)
    {
        for (UIView* subview in root.subviews)
        {
            if ([subview isKindOfClass:[UIButton class]] && !has_text_field_ancestor(subview))
            {
                return (UIButton*)subview;
            }
            if (UIButton* const nested = find_cancel_button(subview))
            {
                return nested;
            }
        }
        return nil;
    }

    // SearchBarExtensions overlay tag: a distinctive UISearchBar viewWithTag: key identifying the colored
    // cancel-button overlay added on iOS 26+ (C#'s CancelButtonColorOverlayTag). A fixed sentinel avoids
    // collisions with system-assigned tags. 'CBOV' = Cancel-Button-Overlay.
    constexpr NSInteger k_cancel_button_color_overlay_tag = 0x43424F56; // 'CBOV'

    // SearchBarExtensions.RemoveCancelButtonOverlay: ViewWithTag searches the whole subtree, so it finds
    // the overlay wherever it was parented. No-op when none was added.
    void remove_cancel_button_overlay(UISearchBar* bar)
    {
        [[bar viewWithTag:k_cancel_button_color_overlay_tag] removeFromSuperview];
    }

    // SearchBarExtensions.ApplyCancelButtonOverlay's FindDescendantView<UIImageView>(): the UIImageView the
    // cancel UIButton uses to render its "X" icon (needed for its rendered size). C#'s FindDescendantView is
    // a BFS that also tests the root; here the root is the cancel UIButton (never a UIImageView) carrying a
    // single icon image view, so this recursive first-match walk (matching find_cancel_button's shape)
    // returns the same view.
    UIImageView* find_descendant_image_view(UIView* root)
    {
        for (UIView* subview in root.subviews)
        {
            if ([subview isKindOfClass:[UIImageView class]])
            {
                return (UIImageView*)subview;
            }
            if (UIImageView* const nested = find_descendant_image_view(subview))
            {
                return nested;
            }
        }
        return nil;
    }

    // ApplyCancelButtonOverlay and ScheduleOverlayRetry are mutually recursive (retry re-invokes apply).
    void apply_cancel_button_overlay(UISearchBar* bar, UIButton* cancel_button, UIColor* color, int retry_count);

    // SearchBarExtensions.ScheduleOverlayRetry: re-dispatch ApplyCancelButtonOverlay on the main queue once
    // the cancel button is ready (it is detached or zero-frame otherwise). C# holds a WeakReference<UISearchBar>;
    // the port captures the bar __weak (its lifetime is the handler's, not the block's) plus the color, and
    // re-finds the live cancel button when the block runs.
    void schedule_overlay_retry(UISearchBar* bar, UIColor* color, int retry_count)
    {
        __weak UISearchBar* const weak_bar = bar;
        dispatch_async(dispatch_get_main_queue(), ^{
          UISearchBar* const strong_bar = weak_bar;
          if (strong_bar == nil)
          {
              return;
          }
          if (UIButton* const btn = find_cancel_button(strong_bar))
          {
              apply_cancel_button_overlay(strong_bar, btn, color, retry_count + 1);
          }
        });
    }

    // SearchBarExtensions.ApplyCancelButtonOverlay: on iOS 26+ UIKit re-tints the cancel UIButton icon on
    // every layout pass, so titleColor/tintColor cannot color it; instead render the "xmark" SF Symbol in the
    // wanted color and pin a non-interactive UIImageView over the button's icon. Retries (up to twice) when
    // the button is detached or not yet laid out (zero frame) — e.g. an AppThemeBinding color set pre-appear.
    void apply_cancel_button_overlay(UISearchBar* bar, UIButton* cancel_button, UIColor* color, int retry_count)
    {
        UIView* const parent = cancel_button.superview;
        if (parent == nil)
        {
            // Detached by UIKit (e.g. mid theme transition): retry with a fresh cancel-button lookup.
            if (retry_count < 2)
            {
                schedule_overlay_retry(bar, color, retry_count);
            }
            return;
        }
        // Remove any overlay from a previous call (color change or re-focus).
        [[bar viewWithTag:k_cancel_button_color_overlay_tag] removeFromSuperview];
        UIImageView* const icon = find_descendant_image_view(cancel_button);
        if (icon == nil)
        {
            return;
        }
        // Icon frame converted into the parent's coordinate space; drives the rendered overlay size.
        const CGRect icon_frame = [parent convertRect:icon.frame fromView:icon.superview];
        if (icon_frame.size.width <= 0 || icon_frame.size.height <= 0)
        {
            // Not laid out yet: retry after the next layout pass so we get a valid frame.
            if (retry_count < 2)
            {
                schedule_overlay_retry(bar, color, retry_count);
            }
            return;
        }
        UIImage* const xmark = [UIImage systemImageNamed:@"xmark"];
        if (xmark == nil)
        {
            return;
        }
        const CGSize image_size = icon_frame.size;
        // new UIGraphicsImageRendererFormat { Opaque = false, Scale = 0 }: preferredFormat is the
        // non-deprecated way to get a mutable format; opaque=NO keeps the transparent surround and scale=0
        // adopts the main-screen scale.
        UIGraphicsImageRendererFormat* const format = [UIGraphicsImageRendererFormat preferredFormat];
        format.opaque = NO;
        format.scale = 0;
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:image_size
                                                                                         format:format];
        // AlwaysOriginal (applied below) stops UIKit re-tinting the baked image; SourceIn + fill re-colors
        // the drawn glyph to `color`.
        UIImage* const colored = [renderer imageWithActions:^(UIGraphicsImageRendererContext*) {
          [xmark drawInRect:CGRectMake(0, 0, image_size.width, image_size.height)];
          CGContextRef const context = UIGraphicsGetCurrentContext();
          if (context != nullptr)
          {
              CGContextSetBlendMode(context, kCGBlendModeSourceIn);
              CGContextSetFillColorWithColor(context, color.CGColor);
              CGContextFillRect(context, CGRectMake(0, 0, image_size.width, image_size.height));
          }
        }];
        UIImageView* const overlay = [[UIImageView alloc] init];
        overlay.image = [colored imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
        overlay.contentMode = UIViewContentModeScaleAspectFit;
        overlay.tag = k_cancel_button_color_overlay_tag;
        overlay.userInteractionEnabled = NO;
        overlay.translatesAutoresizingMaskIntoConstraints = NO;
        // Topmost sibling of the cancel button; autolayout keeps it centered over the icon across rotation,
        // split view, and dynamic-type changes.
        [parent addSubview:overlay];
        [NSLayoutConstraint activateConstraints:@[
            [overlay.centerXAnchor constraintEqualToAnchor:cancel_button.centerXAnchor],
            [overlay.centerYAnchor constraintEqualToAnchor:cancel_button.centerYAnchor],
            [overlay.widthAnchor constraintEqualToConstant:image_size.width],
            [overlay.heightAnchor constraintEqualToConstant:image_size.height],
        ]];
    }

    void refresh_cancel_button(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const bool should_show = bar.text != nil && bar.text.length > 0; // ShouldShowCancelButton()
        [bar setShowsCancelButton:should_show ? YES : NO animated:NO];

        // Can't cache the cancel button — iOS drops it when hidden and builds a fresh one — so re-find it
        // each time, excluding the search field's inner clear button (find_cancel_button's UITextField
        // exclusion mirrors C#'s FindParent(v => v is UITextField) == null predicate).
        UIButton* const cancel = find_cancel_button(bar);
        bool ios26 = false;
        if (@available(iOS 26.0, *)) // OperatingSystem.IsIOSVersionAtLeast(26)
        {
            ios26 = true;
        }
        if (cancel == nil)
        {
            // Cancel button hidden: drop any overlay we added earlier (iOS 26+ only).
            if (ios26)
            {
                remove_cancel_button_overlay(bar);
            }
            return;
        }

        const maui::graphics::color color = view.cancel_button_color();
        // is-set discriminator: an explicit CancelButtonColor=Black equals the default sentinel by value,
        // so key off BindableObject.IsSet (else an explicit-black cancel tint is dropped).
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool explicit_color = bindable != nullptr && bindable->is_property_set("cancel_button_color");
        if (explicit_color)
        {
            // SearchBarExtensions.UpdateCancelButton: title color on Normal/Highlighted/Disabled, plus
            // TintColor for the Mac idiom (the cancel button renders an icon there, so tint colors it).
            UIColor* const tint = to_ui_color(color);
            [cancel setTitleColor:tint forState:UIControlStateNormal];
            [cancel setTitleColor:tint forState:UIControlStateHighlighted];
            [cancel setTitleColor:tint forState:UIControlStateDisabled];
            if (cancel.traitCollection.userInterfaceIdiom == UIUserInterfaceIdiomMac)
            {
                cancel.tintColor = tint;
            }
            // iOS 26+ re-tints the cancel icon every layout pass, defeating titleColor/tintColor — apply a
            // colored UIImageView overlay after the pending layout (DispatchAsync so the button frame is
            // valid). C# re-reads the view's CancelButtonColor in the block; the C++ view can't be captured
            // safely across the async gap, so snapshot the apply/remove decision (explicit_color) + the
            // resolved UIColor now, capture the bar __weak, and re-find the live cancel button in the block.
            if (ios26)
            {
                __weak UISearchBar* const weak_bar = bar;
                const bool wants_overlay = explicit_color;
                dispatch_async(dispatch_get_main_queue(), ^{
                  UISearchBar* const strong_bar = weak_bar;
                  if (strong_bar == nil)
                  {
                      return;
                  }
                  if (!wants_overlay)
                  {
                      remove_cancel_button_overlay(strong_bar);
                      return;
                  }
                  if (UIButton* const current = find_cancel_button(strong_bar))
                  {
                      apply_cancel_button_overlay(strong_bar, current, tint, 0);
                  }
                });
            }
        }
        else if (ios26)
        {
            // CancelButtonColor cleared: remove any overlay we added earlier.
            remove_cancel_button_overlay(bar);
        }
    }

    // SearchBarExtensions.UpdateMaxLength's trim arm.
    void apply_max_length(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSString* const current = bar.text != nil ? bar.text : @"";
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            bar.text = [current substringToIndex:static_cast<NSUInteger>(max_length)];
        }
    }
} // namespace

@implementation MauiIosSearchBarProxy
- (void)mauiSyncTextFrom:(UISearchBar*)bar
{
    NSString* const previous = self.previousText;
    NSString* const current = bar.text;
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
            // OnTextPropertySet → UpdateCancelButtonVisibility.
            refresh_cancel_button(bar, *view);
        }
    }
}

- (void)searchBar:(UISearchBar*)searchBar textDidChange:(NSString*)searchText
{
    (void)searchText;
    [self mauiSyncTextFrom:searchBar];
}

- (void)searchBarSearchButtonClicked:(UISearchBar*)searchBar
{
    (void)searchBar;
    // OnSearchButtonClicked → VirtualView.SearchButtonPressed().
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_search_button_pressed();
        }
    }
}

- (void)searchBarCancelButtonClicked:(UISearchBar*)searchBar
{
    // OnCancelClicked → virtualView.Text = string.Empty: clear the native bar (its source of truth) and
    // report the change through the same diff channel.
    searchBar.text = @"";
    [self mauiSyncTextFrom:searchBar];
}

- (void)searchBarTextDidBeginEditing:(UISearchBar*)searchBar
{
    // The search field took first responder: reflect IsFocused = true onto the virtual view (fires
    // Focused + ChangeVisualState through set_is_focused) — the native focus callback's analog.
    (void)searchBar;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->set_is_focused(true);
        }
    }
}

- (void)searchBarTextDidEndEditing:(UISearchBar*)searchBar
{
    // The search field resigned first responder: reflect IsFocused = false (fires Unfocused).
    (void)searchBar;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->set_is_focused(false);
        }
    }
}

- (BOOL)searchBar:(UISearchBar*)searchBar shouldChangeTextInRange:(NSRange)range replacementText:(NSString*)text
{
    // ShouldChangeText: newLength <= MaxLength.
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
    NSString* const current = searchBar.text != nil ? searchBar.text : @"";
    if (range.location + range.length > current.length)
    {
        return NO;
    }
    const NSUInteger add_length = text != nil ? text.length : 0;
    const NSUInteger new_length = current.length + add_length - range.length;
    return new_length <= static_cast<NSUInteger>(max_length) ? YES : NO;
}
@end

@implementation MauiIosSearchBar
// MauiSearchBar.MovedToWindow()/WillMoveToWindow(window != null): once the bar joins a window UIKit has
// built its internal hierarchy (the descendant cancel UIButton now exists). -didMoveToWindow runs during
// that layout cycle, so firing here guarantees the re-fire walks a realized tree.
- (void)didMoveToWindow
{
    [super didMoveToWindow];
    if (self.window != nil && self.onMovedToWindow != nil)
    {
        self.onMovedToWindow();
    }
}

// Re-frame the clip mask to the new bounds (WrapperView.LayoutSubviews re-runs SetClip): apply_clip sizes
// the mask at map time, before the first layout, when bounds is 0×0 — and a UIKit-driven resize / rotation
// never routes through the handler. No-op when no clip is set.
- (void)layoutSubviews
{
    [super layoutSubviews];
    maui::platform::ios::reapply_clip((__bridge void*)self);
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
        as_search_bar(native).hidden = value != maui::core::visibility::visible;
    }

    void search_bar_platform::update_opacity(double value)
    {
        as_search_bar(native).alpha = value;
    }

    void search_bar_platform::update_is_enabled(bool value)
    {
        // SearchBarExtensions.UpdateIsEnabled → UserInteractionEnabled (a UISearchBar is not a UIControl).
        as_search_bar(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void search_bar_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_search_bar(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapBackground. A UISearchBar draws its own bar chrome over layer.backgroundColor, so a
    // SOLID Background tints the UIView's backgroundColor (the C# ViewExtensions.UpdateBackground path) to
    // fill the bar — clip_views' red search bar then fills under the clip mask. Gradient/image paints route
    // through the shared layer applier.
    void search_bar_platform::update_background(const maui::graphics::paint* value)
    {
        UISearchBar* const bar = as_search_bar(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            // SearchBarExtensions.UpdateBackground: a solid Background tints the whole bar chrome via
            // BarTintColor — NOT layer.backgroundColor, which UISearchBar draws its own chrome OVER (leaving
            // the red fill partial, the clip_views diff). Transparent clears the bar to a blank image.
            if (solid->color().alpha <= 0.0F)
            {
                bar.backgroundImage = [[UIImage alloc] init];
                bar.barTintColor = UIColor.clearColor;
            }
            else
            {
                bar.backgroundImage = nil;
                bar.barTintColor = maui::platform::ios::to_ui_color(solid->color());
            }
        }
        else if (value == nullptr)
        {
            bar.barTintColor = UISearchBar.appearance.barTintColor; // C# null case
        }
        else
        {
            // GradientPaint / image → the shared layer applier (ViewExtensions.UpdateBackground path).
            maui::platform::ios::apply_background(native, value);
        }
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void search_bar_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
        // CreatePlatformView: new MauiSearchBar() { BarStyle = UIBarStyle.Default } — the moved-to-window
        // subclass so the cancel-button color can re-fire once the bar joins the window hierarchy.
        MauiIosSearchBar* const bar = [[MauiIosSearchBar alloc] initWithFrame:CGRectZero];
        bar.barStyle = UIBarStyleDefault;
        platform->native = (__bridge_retained void*)bar; // the void* slot owns one reference
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        UISearchBar* const bar = as_search_bar(platform.native);
        MauiIosSearchBarProxy* const proxy = [[MauiIosSearchBarProxy alloc] init];
        proxy.handler = this;
        proxy.previousText = bar.text != nil ? bar.text : @"";
        bar.delegate = proxy; // weak, so the proxy is retained via an associated object
        objc_setAssociatedObject(bar, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // MauiSearchBarProxy.OnMovedToWindow: re-fire MapCancelButtonColor once the bar is in the window
        // and the descendant cancel button exists. The handler owns the bar (handler → platform → native),
        // so the raw `this` capture outlives the block; on_disconnect clears it before teardown.
        if ([bar isKindOfClass:[MauiIosSearchBar class]])
        {
            search_bar_handler* const self = this;
            ((MauiIosSearchBar*)bar).onMovedToWindow = ^{
              self->update_value("cancel_button_color");
            };
        }
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        UISearchBar* const bar = as_search_bar(platform.native);
        if ([bar isKindOfClass:[MauiIosSearchBar class]])
        {
            ((MauiIosSearchBar*)bar).onMovedToWindow = nil;
        }
        bar.delegate = nil;
        objc_setAssociatedObject(bar, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
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
        UISearchBar* const bar = as_search_bar(platform->native);
        bar.text = value; // SearchBarExtensions.UpdateText
        // MapText → MapFormatting: character spacing + alignment + max length.
        map_character_spacing(handler, view);
        query_editor(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        apply_max_length(bar, view);
        refresh_cancel_button(bar, view);
        if (auto* const proxy = (MauiIosSearchBarProxy*)objc_getAssociatedObject(bar, &k_proxy_key))
        {
            proxy.previousText = bar.text != nil ? bar.text : @"";
        }
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // SearchBarExtensions.UpdateIsReadOnly: read-only (or input-transparent) disables interaction.
            platform->is_read_only = view.is_read_only();
            as_search_bar(platform->native).userInteractionEnabled =
                (view.is_read_only() || view.input_transparent()) ? NO : YES;
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            apply_max_length(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // An unset TextColor (default-constructed sentinel) must resolve to the dynamic system label
            // color so the query text stays legible in dark mode — SearchBarHandler.iOS relies on the
            // search field's default label color rather than forcing opaque black. Explicit colors win.
            // is-set discriminator (see label_handler.mm): an explicit TextColor=Black equals the
            // default-constructed sentinel by value, so key off BindableObject.IsSet, not `!= color{}`.
            const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            query_editor(platform->native).textColor =
                color_is_set ? to_ui_color(view.text_color()) : UIColor.labelColor;
        }
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            query_editor(platform->native).font = to_ui_font(view.font(), static_cast<double>(UIFont.systemFontSize));
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
        // QueryEditor.UpdateCharacterSpacing: kern the text and the placeholder when present.
        UISearchTextField* const editor = query_editor(platform->native);
        const double spacing = view.character_spacing();
        NSAttributedString* const text_attr = with_character_spacing(editor.attributedText, spacing);
        if (text_attr != nil)
        {
            editor.attributedText = text_attr;
        }
        NSAttributedString* const placeholder_attr = with_character_spacing(editor.attributedPlaceholder, spacing);
        if (placeholder_attr != nil)
        {
            editor.attributedPlaceholder = placeholder_attr;
        }
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            query_editor(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // UpdateVerticalTextAlignment → the QueryEditor's contentVerticalAlignment (a UIControl).
            platform->vertical_alignment = view.vertical_text_alignment();
            query_editor(platform->native).contentVerticalAlignment =
                to_ui_control_content_vertical_alignment(view.vertical_text_alignment());
        }
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
            query_editor(platform->native).autocorrectionType =
                view.is_text_prediction_enabled() ? UITextAutocorrectionTypeYes : UITextAutocorrectionTypeNo;
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
            query_editor(platform->native).spellCheckingType =
                view.is_spell_check_enabled() ? UITextSpellCheckingTypeYes : UITextSpellCheckingTypeNo;
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        // SearchBarExtensions.UpdateKeyboard: ApplyKeyboard onto the search field (UISearchTextField
        // conforms to UITextInputTraits), then (for non-custom keyboards) re-apply prediction/spellcheck,
        // then ReloadInputViews so a live keyboard re-styles.
        UISearchBar* const bar = as_search_bar(platform->native);
        maui::platform::ios::apply_keyboard(query_editor(platform->native), view.keyboard());
        if (!maui::platform::ios::is_custom_keyboard(view.keyboard()))
        {
            map_is_text_prediction_enabled(handler, view);
            map_is_spell_check_enabled(handler, view);
        }
        [bar reloadInputViews];
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        // Clamped-range write onto the QueryEditor (the entry recipe's UpdateCursorSelection, collapsed:
        // without an editing session there is no selectedTextRange and the mirror records the intent).
        UISearchTextField* const editor = query_editor(platform->native);
        if (editor.selectedTextRange == nil)
        {
            return;
        }
        const int text_length = static_cast<int>(editor.text != nil ? editor.text.length : 0);
        const int start = view.cursor_position() < text_length ? view.cursor_position() : text_length;
        const int span_max = text_length - start;
        const int span = view.selection_length() < span_max ? view.selection_length() : span_max;
        UITextPosition* const from = [editor positionFromPosition:editor.beginningOfDocument offset:start];
        UITextPosition* const to = [editor positionFromPosition:editor.beginningOfDocument offset:start + span];
        if (from != nil && to != nil)
        {
            editor.selectedTextRange = [editor textRangeFromPosition:from toPosition:to];
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
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->cancel_button_color = view.cancel_button_color();
            refresh_cancel_button(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateSearchIcon: tint the loupe (the QueryEditor's leftView image).
        platform->search_icon_color = view.search_icon_color();
        const maui::graphics::color color = view.search_icon_color();
        // is-set discriminator: an explicit SearchIconColor=Black equals the default sentinel by value,
        // so key off BindableObject.IsSet (else an explicit-black loupe tint is dropped).
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool explicit_color = bindable != nullptr && bindable->is_property_set("search_icon_color");
        UIView* const left = query_editor(platform->native).leftView;
        if (explicit_color && [left isKindOfClass:[UIImageView class]])
        {
            auto* const icon = (UIImageView*)left;
            icon.image = [icon.image imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
            icon.tintColor = to_ui_color(color);
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // SearchBarExtensions.UpdateReturnType → the QueryEditor's returnKeyType (REAL on iOS).
            platform->bar_return_type = view.return_type();
            query_editor(platform->native).returnKeyType = to_ui_return_key_type(view.return_type());
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // SearchBarHandler.iOS.GetDesiredSize: infinite constraints collapse to the bar's fitted size.
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_search_bar(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_search_bar(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void search_bar_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

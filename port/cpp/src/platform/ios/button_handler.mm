// button_handler — iOS (UIKit) platform recipe, the M6 Rosetta Stone. The managed platform view is a
// UIButton (held, retained, in button_platform::native), Text maps to the Normal-state title, and the
// native touch events flow back through a target-action proxy to i_button's send_* channel. Compiled
// as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from ButtonHandler.iOS.cs + Platform/iOS/ButtonExtensions.cs (the same oracles the
// AppKit twin in src/platform/apple/button_handler.mm was adapted from — UIKit needs no adaptation):
// CreatePlatformView = UIButton(UIButtonType.System) + SetControlPropertiesFromProxy; ButtonEventProxy's
// TouchDown → Pressed, TouchUpInside → Released + Clicked, TouchUpOutside/TouchCancel → Released;
// UpdateText/UpdateTextColor/UpdateCharacterSpacing/UpdateFont/UpdatePadding/UpdateStroke* as the map_*
// bodies below. Not ported here (deferred to the M6 fan-out): NeedsContainer/WrapperView (the port has
// no container infrastructure on any backend yet), MapImageSource (the button image part), and the
// Mac-Catalyst-only UIButtonConfiguration branches of MapBackground/MapTextColor.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the UIButton's touch target-actions to the C++ handler's virtual view.
// Ports ButtonHandler.ButtonEventProxy — including the Released-before-Clicked order on TouchUpInside.
@interface MauiButtonEventProxy : NSObject
@property(nonatomic) maui::core::button_handler* handler;
- (void)onTouchUpInside:(id)sender;
- (void)onTouchUpOutside:(id)sender;
- (void)onTouchDown:(id)sender;
- (void)onTouchCancel:(id)sender;
@end

@implementation MauiButtonEventProxy
- (void)onTouchUpInside:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_released();
            view->send_clicked();
        }
    }
}

- (void)onTouchUpOutside:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_released();
        }
    }
}

- (void)onTouchDown:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_pressed();
        }
    }
}

- (void)onTouchCancel:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_released();
        }
    }
}
@end

namespace
{
    // Key for the associated MauiButtonEventProxy kept alive by the UIButton (UIControl does not
    // retain its targets — the target-action convention).
    const char k_proxy_key = 0;

    // ButtonHandler.ControlStates — the three states SetControlPropertiesFromProxy seeds.
    constexpr std::array<UIControlState, 3> k_control_states{UIControlStateNormal, UIControlStateHighlighted,
                                                             UIControlStateDisabled};

    // ButtonHandler.DefaultPadding — "the padding that Xcode has when 'Default' content insets are
    // used" (left/right 12, top/bottom 7); substituted when the cross-platform Padding is NaN.
    // Kept as two scalars (thickness's ctor is not constexpr, and a dynamically-initialized static
    // would be a bugprone-throwing-static-initialization finding).
    constexpr double k_default_padding_horizontal = 12;
    constexpr double k_default_padding_vertical = 7;

    // ButtonExtensions.AlmostZero — top/bottom insets of exactly 0 are floored back to a UIKit
    // default, so a literal 0 is nudged to an invisible epsilon.
    constexpr double k_almost_zero = 0.00001;

    UIButton* as_button(void* native)
    {
        return (__bridge UIButton*)native;
    }

    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_font;
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

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    // Visibility is the same simplification as the AppKit twin: Hidden and Collapsed both hide the
    // view (ViewExtensions.iOS's Collapse() constraint dance is deferred with the shared view ops).
    void button_platform::update_visibility(maui::core::visibility value)
    {
        as_button(native).hidden = value != maui::core::visibility::visible;
    }

    void button_platform::update_opacity(double value)
    {
        as_button(native).alpha = value;
    }

    void button_platform::update_is_enabled(bool value)
    {
        as_button(native).enabled = static_cast<BOOL>(value);
    }

    void button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    namespace
    {
        // ButtonHandler.SetControlPropertiesFromProxy: adopt any UIAppearance-proxy values for the
        // three control states up front ("If new values are null, old values are preserved" — a nil
        // proxy value leaves the UIButton's own default in place).
        void set_control_properties_from_proxy(UIButton* button)
        {
            UIButton* const proxy = [UIButton appearance];
            for (const UIControlState state : k_control_states)
            {
                [button setTitleColor:[proxy titleColorForState:state] forState:state];
                [button setTitleShadowColor:[proxy titleShadowColorForState:state] forState:state];
                [button setBackgroundImage:[proxy backgroundImageForState:state] forState:state];
            }
        }

        // Rebuild the UIButton's attributed title from its current plain title, mirroring
        // ButtonExtensions.UpdateCharacterSpacing (TitleLabel.AttributedText.WithCharacterSpacing +
        // WithTextColor → SetAttributedTitle(…, Normal)). Kerning needs an attributed title, and a
        // kerned title must carry its color explicitly (setTitleColor:/tintColor do not apply to an
        // attributed title), so when spacing != 0 the title is rebuilt attributed WITH the text color.
        // When spacing == 0 the attributed title is reset to nil so the plain title (colored by
        // map_text_color) shows — matching C#, where WithCharacterSpacing returns null on a fresh,
        // un-kerned title and SetAttributedTitle(null) keeps the plain-title path.
        void refresh_button_title_formatting(UIButton* button, const i_text_button& view)
        {
            const double spacing = view.character_spacing();
            if (spacing == 0)
            {
                [button setAttributedTitle:nil forState:UIControlStateNormal];
                return;
            }
            UIColor* const foreground = to_ui_color(view.text_color());
            // Rebuild from the PLAIN title storage (what map_text sets) — unlike currentTitle, it is
            // well-defined even while a previous attributed title is still installed (the re-kern path).
            NSString* const plain_title = [button titleForState:UIControlStateNormal];
            [button setAttributedTitle:maui::platform::ios::kern_attributed(plain_title, spacing, foreground)
                              forState:UIControlStateNormal];
        }
    } // namespace

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        UIButton* const button = [UIButton buttonWithType:UIButtonTypeSystem];
        set_control_properties_from_proxy(button);
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        MauiButtonEventProxy* const proxy = [[MauiButtonEventProxy alloc] init];
        proxy.handler = this;
        // ButtonEventProxy.Connect — the four touch events. UIControl holds its targets weakly, so the
        // proxy is kept alive for the button's lifetime via an associated object (the AppKit pattern).
        [button addTarget:proxy action:@selector(onTouchUpInside:) forControlEvents:UIControlEventTouchUpInside];
        [button addTarget:proxy action:@selector(onTouchUpOutside:) forControlEvents:UIControlEventTouchUpOutside];
        [button addTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
        [button addTarget:proxy action:@selector(onTouchCancel:) forControlEvents:UIControlEventTouchCancel];
        objc_setAssociatedObject(button, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        if (auto* const proxy = (MauiButtonEventProxy*)objc_getAssociatedObject(button, &k_proxy_key))
        {
            // ButtonEventProxy.Disconnect — unhook the same four (target, action, event) wirings.
            [button removeTarget:proxy action:@selector(onTouchUpInside:) forControlEvents:UIControlEventTouchUpInside];
            [button removeTarget:proxy
                          action:@selector(onTouchUpOutside:)
                forControlEvents:UIControlEventTouchUpOutside];
            [button removeTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
            [button removeTarget:proxy action:@selector(onTouchCancel:) forControlEvents:UIControlEventTouchCancel];
        }
        objc_setAssociatedObject(button, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); a nil title simply clears it.
        NSString* const raw = [NSString stringWithUTF8String:text.c_str()];
        UIButton* const button = as_button(platform->native);
        [button setTitle:raw forState:UIControlStateNormal]; // ButtonExtensions.UpdateText
        // "Any text update requires that we update any attributed string formatting" (MapText →
        // MapFormatting → UpdateCharacterSpacing).
        refresh_button_title_formatting(button, view);
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UIButton* const button = as_button(platform->native);
        // ButtonExtensions.UpdateTextColor's non-null branch: the three seeded states + TintColor. The
        // TextColor == null branch (clear the per-state overrides, adopt the window's tint) has no
        // analog — the port's color is a non-nullable value type (same collapse as the AppKit twin).
        UIColor* const color = to_ui_color(view.text_color());
        [button setTitleColor:color forState:UIControlStateNormal];
        [button setTitleColor:color forState:UIControlStateHighlighted];
        [button setTitleColor:color forState:UIControlStateDisabled];
        button.tintColor = color;
        // A kerned (attributed) title carries its own color, so re-apply it (the C# MapTextColor relies
        // on the next MapFormatting pass; the port refreshes eagerly, like its map_text).
        refresh_button_title_formatting(button, view);
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // ButtonExtensions.UpdateFont targets TitleLabel with UIFont.ButtonFontSize as the default.
            as_button(platform->native).titleLabel.font =
                to_ui_font(view.font(), static_cast<double>(UIFont.buttonFontSize));
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
        UIButton* const button = as_button(platform->native);
        // ButtonExtensions.UpdatePadding(button, DefaultPadding): NaN padding falls back to the default,
        // the current border width is added on every side (int-truncated, as in C#), and top/bottom of
        // exactly 0 become AlmostZero so UIKit does not floor them back to its own default.
        maui::core::thickness padding = view.padding();
        if (padding.is_nan())
        {
            padding = maui::core::thickness(k_default_padding_horizontal, k_default_padding_vertical);
        }
        const int additional_padding = static_cast<int>(button.layer.borderWidth);
        padding = maui::core::thickness(padding.left + additional_padding, padding.top + additional_padding,
                                        padding.right + additional_padding, padding.bottom + additional_padding);
        double top = padding.top;
        if (top == 0.0)
        {
            top = k_almost_zero;
        }
        double bottom = padding.bottom;
        if (bottom == 0.0)
        {
            bottom = k_almost_zero;
        }
        // ContentEdgeInsets is deprecated on iOS 15+ but still functional for non-UIButtonConfiguration
        // buttons; C#'s UpdatePadding uses it under the very same suppression (#pragma warning disable
        // CA1416/CA1422), so the port mirrors that — including the suppression.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        button.contentEdgeInsets = UIEdgeInsetsMake(top, padding.left, bottom, padding.right);
#pragma clang diagnostic pop
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // ButtonExtensions.UpdateStrokeColor; its `is not null` guard has no analog (non-nullable
            // color value type, the same collapse as map_text_color).
            as_button(platform->native).layer.borderColor = to_ui_color(view.stroke_color()).CGColor;
        }
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr && view.stroke_thickness() >= 0) // ButtonExtensions.UpdateStrokeThickness
        {
            as_button(platform->native).layer.borderWidth = static_cast<CGFloat>(view.stroke_thickness());
        }
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr && view.corner_radius() >= 0) // ButtonExtensions.UpdateCornerRadius
        {
            as_button(platform->native).layer.cornerRadius = static_cast<CGFloat>(view.corner_radius());
        }
    }

    maui::graphics::size button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform
        // maximum, then the native view measures itself (UIView.SizeThatFits).
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_button(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

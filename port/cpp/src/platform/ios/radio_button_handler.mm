// radio_button_handler — iOS (UIKit) platform recipe: the NATIVE DEFAULT FALLBACK (documented
// deviation). C# renders RadioButton on iOS through a ControlTemplate (RadioButtonHandler.iOS.cs hosts
// a plain ContentView and leaves the text/stroke maps [MissingMapper]); the port's templated content
// is deferred, so this partial renders the DefaultTemplate's essentials natively instead: a
// UIButton(System) whose Normal/Selected images are the circle / filled-circle SF-symbol pair (the
// template's Ellipse indicator + CheckedIndicator), the string content on the title, IsChecked riding
// UIButton.selected — plus C#'s MapIsChecked AccessibilityValue "1"/"0" push, kept verbatim. A tap
// flows back through a target-action proxy to send_is_checked(true) (RadioButton.SelectRadioButton —
// a radio tap SELECTS; the cross-platform RadioButtonGroup unchecks the others). The text style uses
// the ButtonExtensions recipe (title colors + kerned attributed title + TitleLabel font); the
// stroke/corner ride the layer. Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the UIButton's touch-up-inside to the C++ handler's virtual view.
@interface MauiIosRadioButtonProxy : NSObject
@property(nonatomic) maui::core::radio_button_handler* handler;
- (void)onSelect:(id)sender;
@end

@implementation MauiIosRadioButtonProxy
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
    // Key for the associated MauiIosRadioButtonProxy kept alive by the UIButton (targets are weak).
    const char k_proxy_key = 0;

    UIButton* as_button(void* native)
    {
        return (__bridge UIButton*)native;
    }

    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_font;

    // The kerned-title rebuild — the button partial's refresh_button_title_formatting twin over the
    // i_text_style face (see button_handler.mm for the plain-vs-attributed title rules).
    void refresh_radio_title_formatting(UIButton* button, const maui::core::i_text_style& view)
    {
        const double spacing = view.character_spacing();
        if (spacing == 0)
        {
            [button setAttributedTitle:nil forState:UIControlStateNormal];
            return;
        }
        const maui::graphics::color text_color = view.text_color();
        UIColor* const foreground =
            text_color != maui::graphics::color{} ? to_ui_color(text_color) : UIColor.labelColor;
        NSString* const plain_title = [button titleForState:UIControlStateNormal];
        [button setAttributedTitle:maui::platform::ios::kern_attributed(plain_title, spacing, foreground)
                          forState:UIControlStateNormal];
    }
} // namespace

namespace maui::core
{
    radio_button_platform::~radio_button_platform()
    {
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
        as_button(native).alpha = value;
    }

    void radio_button_platform::update_is_enabled(bool value)
    {
        as_button(native).enabled = static_cast<BOOL>(value);
    }

    void radio_button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        // Custom (not System): the iOS-26 system button paints a tint-colored fill behind the SELECTED
        // state (a white box once the tint is the dynamic label color), which MAUI's templated radio has
        // no analog for. A custom button still tints the template SF-symbol indicators via tintColor.
        UIButton* const button = [UIButton buttonWithType:UIButtonTypeCustom];
        // The DefaultTemplate's indicator pair, as SF symbols riding UIButton's state machinery: the
        // empty ring while unselected, the filled ring while selected (the CheckedIndicator's opacity
        // flip, collapsed onto UIButton.selected).
        [button setImage:[UIImage systemImageNamed:@"circle"] forState:UIControlStateNormal];
        [button setImage:[UIImage systemImageNamed:@"smallcircle.filled.circle"] forState:UIControlStateSelected];
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        MauiIosRadioButtonProxy* const proxy = [[MauiIosRadioButtonProxy alloc] init];
        proxy.handler = this;
        // UIControl targets are weak, so the proxy is kept alive via an associated object.
        [button addTarget:proxy action:@selector(onSelect:) forControlEvents:UIControlEventTouchUpInside];
        objc_setAssociatedObject(button, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        if (auto* const proxy = (MauiIosRadioButtonProxy*)objc_getAssociatedObject(button, &k_proxy_key))
        {
            [button removeTarget:proxy action:@selector(onSelect:) forControlEvents:UIControlEventTouchUpInside];
        }
        objc_setAssociatedObject(button, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked();
        UIButton* const button = as_button(platform->native);
        button.selected = static_cast<BOOL>(view.is_checked());
        // RadioButtonHandler.iOS MapIsChecked: AccessibilityValue mirrors the checked state.
        button.accessibilityValue = view.is_checked() ? @"1" : @"0";
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
        NSString* const raw = [NSString stringWithUTF8String:content.c_str()];
        UIButton* const button = as_button(platform->native);
        [button setTitle:(raw != nil ? raw : @"") forState:UIControlStateNormal];
        refresh_radio_title_formatting(button, view);
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The ButtonExtensions.UpdateTextColor recipe; the tintColor also tints the SF-symbol
        // indicator, the fallback's stand-in for the template's themed Ellipse strokes. An unset
        // (default-constructed) TextColor must resolve to the dynamic system label color, NOT a clear/
        // opaque-black tint that hides BOTH the title AND the circle on dark backgrounds. Explicit wins.
        UIButton* const button = as_button(platform->native);
        const maui::graphics::color text_color = view.text_color();
        UIColor* const color = text_color != maui::graphics::color{} ? to_ui_color(text_color) : UIColor.labelColor;
        [button setTitleColor:color forState:UIControlStateNormal];
        [button setTitleColor:color forState:UIControlStateHighlighted];
        [button setTitleColor:color forState:UIControlStateDisabled];
        button.tintColor = color;
        refresh_radio_title_formatting(button, view);
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).titleLabel.font =
                to_ui_font(view.font(), static_cast<double>(UIFont.buttonFontSize));
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
        if (platform != nullptr)
        {
            as_button(platform->native).layer.borderColor = to_ui_color(view.stroke_color()).CGColor;
        }
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.borderWidth = view.stroke_thickness();
        }
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.cornerRadius = static_cast<CGFloat>(view.corner_radius());
        }
    }

    maui::graphics::size radio_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_button(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

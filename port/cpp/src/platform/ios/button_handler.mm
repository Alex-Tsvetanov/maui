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
// bodies below; MapImageSource (the button image part) loads through the handler-owned source loader and
// applies via ButtonImageSourcePartSetter.SetImageSource = AlwaysOriginal + SetImage(Normal) +
// LayoutIfNeeded (ButtonHandler.iOS.cs:216-231). Not ported here (deferred to the M6 fan-out):
// NeedsContainer/WrapperView (the port has no container infrastructure on any backend yet), the
// ContentLayout text+image composition (UpdateContentLayout — needs the container), and the
// Mac-Catalyst-only UIButtonConfiguration branches of MapBackground/MapTextColor.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
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
#include "maui/graphics/solid_paint.hpp"

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

    UIImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const raw = [NSString stringWithUTF8String:file.c_str()];
        if (raw == nil)
        {
            return nil;
        }
        // ImageSourceExtensions.GetPlatformImage: UIImage.FromBundle(name) ?? UIImage.FromFile(file).
        UIImage* const bundled = [UIImage imageNamed:raw];
        return bundled != nil ? bundled : [UIImage imageWithContentsOfFile:raw];
    }

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

    // ButtonHandler.iOS.cs ButtonImageSourcePartSetter.SetImageSource: force AlwaysOriginal rendering,
    // SetImage(Normal), then LayoutIfNeeded — "UIButton.SetImage does not immediately assign to
    // ImageView.Image; it is applied only at render, so force a layout to keep SizeThatFits correct."
    void set_button_image(UIButton* button, UIImage* image)
    {
        UIImage* const resolved = [image imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
        [button setImage:resolved forState:UIControlStateNormal];
        [button layoutIfNeeded];
    }

    // Port of UIImageExtensions.ResizeImageSource (Graphics/Platforms/iOS/UIImageExtensions.cs): scale the
    // button image to fit (max_width, max_height), clamped so it never exceeds the ORIGINAL image size, and
    // skip scaling UP unless asked. Returns the source unchanged when no resize is needed, else a new
    // UIImage built at CurrentScale/factor (which shrinks its point .Size) preserving the rendering mode.
    UIImage* resize_image_source(UIImage* source_image, CGFloat max_width, CGFloat max_height,
                                 CGSize original_image_size, bool should_scale_up)
    {
        if (source_image == nil || source_image.CGImage == nullptr)
        {
            return nil;
        }
        max_width = static_cast<CGFloat>(std::min<double>(max_width, original_image_size.width));
        max_height = static_cast<CGFloat>(std::min<double>(max_height, original_image_size.height));
        const CGSize source_size = source_image.size;
        if (source_size.width <= 0 || source_size.height <= 0)
        {
            return source_image;
        }
        const auto max_resize_factor =
            static_cast<float>(std::min(max_width / source_size.width, max_height / source_size.height));
        if (max_resize_factor > 1.0F && !should_scale_up)
        {
            return source_image;
        }
        // C#'s UIImage.CurrentScale is the Obj-C `scale` property; a higher scale → smaller point .Size, so
        // dividing by the (<1) factor scales the image DOWN in points.
        UIImage* resized = [UIImage imageWithCGImage:source_image.CGImage
                                               scale:source_image.scale / max_resize_factor
                                         orientation:source_image.imageOrientation];
        // Preserve the rendering mode to maintain color behavior (matches C#).
        resized = [resized imageWithRenderingMode:source_image.renderingMode];
        return resized;
    }
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
        // A 1×1 image of a solid color. A UIButton(UIButtonType.System) ignores backgroundColor for its
        // fill, so MAUI's ButtonHandler.MapBackground draws the BackgroundColor as a per-state
        // backgroundImage — this mints that image.
        UIImage* solid_color_image(UIColor* color)
        {
            UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(1, 1)];
            return [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
              CGContextSetFillColorWithColor(context.CGContext, color.CGColor);
              CGContextFillRect(context.CGContext, CGRectMake(0, 0, 1, 1));
            }];
        }
    } // namespace

    // ButtonExtensions.UpdateBackground: a system UIButton draws its BackgroundColor as a per-state
    // backgroundImage (plain backgroundColor is ignored by the button's own drawing). A solid paint becomes
    // a 1×1 colored image for every control state; a gradient/image paint defers to the shared layer-based
    // apply_background; a null paint clears the override.
    void button_platform::update_background(const maui::graphics::paint* value)
    {
        UIButton* const button = as_button(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            UIImage* const image = solid_color_image(maui::platform::ios::to_ui_color(solid->color()));
            for (const UIControlState state : k_control_states)
            {
                [button setBackgroundImage:image forState:state];
            }
        }
        else if (value != nullptr)
        {
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            for (const UIControlState state : k_control_states)
            {
                [button setBackgroundImage:nil forState:state];
            }
        }
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the UIButton's layer to the clip geometry, sized to
    // its CURRENT bounds (0×0 before the first layout — platform_arrange re-frames it). The UIButton has no
    // MauiIos* subclass, so the re-frame on a UIKit-driven resize rides the handler's platform_arrange;
    // apply_and_store_clip stashes the borrow for that re-frame.
    void button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_button(native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
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
            // MAUI's iOS Button does NOT visibly apply CharacterSpacing on the rendered title — a runtime
            // mapper-order quirk: ButtonExtensions.UpdateCharacterSpacing kerns the EXISTING
            // TitleLabel.AttributedText, but on the live mapping path that title is the plain one MapText
            // installs (SetTitle, not an attributed title), so the kerning never visibly takes (the
            // maui-compare reference renders "Button", never "B u t t o n", for a CharacterSpacing button).
            // Per the 2026-06-23 user ruling the maui-compare RUNTIME is ground truth here, so the port
            // matches it: a button's CharacterSpacing is a visual no-op — the plain title (UpdateText) +
            // title color (UpdateTextColor) carry the text. Clearing any attributed title keeps the plain
            // one showing. (Deliberately diverges from MAUI's CharacterSpacingInitializesCorrectly unit test,
            // which the runtime contradicts; tried mirroring titleLabel.attributedText, but UIButton
            // synthesizes a non-nil attributed title synchronously after SetTitle, so kerning still applied.)
            (void)view;
            [button setAttributedTitle:nil forState:UIControlStateNormal];
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
        // ButtonExtensions.UpdateTextColor. Non-null branch: seed the three states + TintColor with the
        // explicit color. Null branch (MAUI default): clear the per-state overrides and adopt the window's
        // tint (the system accent, which adapts to light/dark) — the port treats the default-constructed
        // (unset) color as that null, so an unstyled button shows the iOS tint, not black-on-black in dark.
        const maui::graphics::color text_color = view.text_color();
        if (text_color != maui::graphics::color{})
        {
            UIColor* const color = to_ui_color(text_color);
            [button setTitleColor:color forState:UIControlStateNormal];
            [button setTitleColor:color forState:UIControlStateHighlighted];
            [button setTitleColor:color forState:UIControlStateDisabled];
            button.tintColor = color;
        }
        else
        {
            [button setTitleColor:nil forState:UIControlStateNormal];
            [button setTitleColor:nil forState:UIControlStateHighlighted];
            [button setTitleColor:nil forState:UIControlStateDisabled];
            button.tintColor = nil; // inherit the system/window tint (adapts light/dark)
        }
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
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform
        // maximum, then the native view measures itself (UIView.SizeThatFits).
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        UIButton* const button = as_button(platform->native);

        UIImage* image = [button imageForState:UIControlStateNormal];
        if (image == nil)
        {
            // No image: a text-only (or empty) button. Keep the native measure — SizeThatFits is correct for
            // a title + content insets — and drop any captured original image (Button.iOS.cs's else branch
            // and OnHandlerChangingCore both reset _originalImageSize / _originalCGImage).
            platform->original_image_valid = false;
            platform->original_cg_image = nullptr;
            const CGSize fitting = [button sizeThatFits:CGSizeMake(width, height)];
            return {fitting.width, fitting.height};
        }

        // ---- Port of Button.iOS.cs ICrossPlatformLayout.CrossPlatformMeasure (the image path) ----
        //
        // Why this exists: the port has no WrapperView/container, so a Button with an ImageSource never ran
        // MAUI's CrossPlatformMeasure — get_desired_size fell through to a raw -[UIButton sizeThatFits:],
        // which returns the button's intrinsic size grown to the FULL image pixel size. Combined with
        // content_is_minimum_size()==true (below), view::measure treats that as a hard floor, so a big icon
        // (settings.png = 128×128 @1x) made the button render at the full ~128pt+ image height (Top position
        // stacks image + title, larger still) and shoved the following controls off-screen. This replicates
        // CrossPlatformMeasure so the image is resized DOWN to the available content area and the reported
        // size follows the resized image, not the raw pixels. NOTE: this is NOT a scale/loading bug —
        // settings.png is a 128×128 @1x asset (UIImage.size = 128pt), so the height came from measure
        // ignoring the constraint, not from loading a @2x/@3x source at the wrong scale.
        //
        // ContentLayout (image position + spacing) is read through the i_text_button::content_layout_spec()
        // seam (a narrow projection of Button.ContentLayout, like image_source()), so the measure composes
        // the image + title along the correct axis: Left/Right → WIDTH, Top/Bottom → HEIGHT (C#'s
        // layout.Position branches). The gallery's settings buttons use Top, so this axis choice matters.
        const i_button* const vv = virtual_view();
        const auto* const tvv = dynamic_cast<const i_text_button*>(vv);
        const maui::core::button_content_spec content_layout =
            tvv != nullptr ? tvv->content_layout_spec() : maui::core::button_content_spec{};
        const double spacing = content_layout.spacing;
        using image_position = maui::core::button_content_spec::image_position;
        const bool position_is_vertical =
            content_layout.position == image_position::top || content_layout.position == image_position::bottom;

        // C#: borderWidth = button.BorderWidth < 0 ? 0 : button.BorderWidth (clamp negatives to 0).
        const double border_width = vv != nullptr && vv->stroke_thickness() > 0 ? vv->stroke_thickness() : 0.0;

        maui::core::thickness padding = vv != nullptr ? vv->padding() : maui::core::thickness{};
        if (padding.is_nan())
        {
            padding = maui::core::thickness(k_default_padding_horizontal, k_default_padding_vertical);
        }

        NSString* const title = [button titleForState:UIControlStateNormal];
        const bool has_title = title != nil && title.length > 0;

        // ResizeImageIfNecessary: shrink CurrentImage to fit the content area, resizing from the ORIGINAL
        // source each pass so repeated measures don't ratchet it ever smaller (Button.iOS.cs's
        // _originalImageSize / _originalCGImage). Capture the original the first time or when a new image is
        // loaded (identity recheck against the stored CGImageRef).
        void* const cg_image = (__bridge void*)(id)image.CGImage;
        if (!platform->original_image_valid || platform->original_cg_image != cg_image)
        {
            platform->original_cg_image = cg_image;
            platform->original_image_width = image.size.width;
            platform->original_image_height = image.size.height;
            platform->original_image_valid = true;
        }
        const CGSize original_size = CGSizeMake(static_cast<CGFloat>(platform->original_image_width),
                                                static_cast<CGFloat>(platform->original_image_height));

        // additionalHorizontalSpace / additionalVerticalSpace: base padding + border, plus (when a title is
        // present) the spacing on the image-position axis — Left/Right add it to width, Top/Bottom to height
        // (C#'s ResizeImageIfNecessary layout.Position branch).
        double additional_horizontal_space = padding.left + padding.right + (border_width * 2);
        double additional_vertical_space = padding.top + padding.bottom + (border_width * 2);
        if (has_title)
        {
            if (position_is_vertical)
            {
                additional_vertical_space += spacing;
            }
            else
            {
                additional_horizontal_space += spacing;
            }
        }

        const double current_image_width = image.size.width;
        const double current_image_height = image.size.height;
        constexpr double k_buffer = 0.1; // C#'s off-by-a-fraction tolerance

        // C# guards on !double.IsNaN(constraint); the port receives CGFLOAT_MAX for an unconstrained axis, so
        // an available* stays at the (huge) constraint there and the resize is a no-op on that axis — an
        // unconstrained button keeps its natural image size, exactly as MAUI does with infinite space.
        double available_width = width;
        double available_height = height;
        const double content_width = width - additional_horizontal_space;
        if (current_image_width - content_width > k_buffer)
        {
            available_width = content_width;
        }
        const double content_height = height - additional_vertical_space;
        if (current_image_height - content_height > k_buffer)
        {
            available_height = content_height;
        }
        available_width = std::max(available_width, 0.1);
        available_height = std::max(available_height, 0.1);

        if (current_image_height - available_height > k_buffer || current_image_width - available_width > k_buffer)
        {
            UIImage* const resized = resize_image_source(image, static_cast<CGFloat>(available_width),
                                                         static_cast<CGFloat>(available_height), original_size, false);
            if (resized != nil && resized != image)
            {
                set_button_image(button, resized);
                image = [button imageForState:UIControlStateNormal];
            }
        }

        // Title rect (ComputeTitleRect): measure the title bounded by the space left after the image +
        // spacing + padding. Faithfully mirrors the C# THREE subtraction blocks (Button.iOS.cs:287-306),
        // including its `&&`-before-`||` precedence quirks — the guards are literal ports so a Top/Bottom
        // title gets the full width (C# subtracts the image/spacing only for Left/Right), which is why the
        // gallery's Top settings button measures correctly. An empty title contributes a zero rect.
        const bool position_is_left = content_layout.position == image_position::left;
        const bool position_is_right = content_layout.position == image_position::right;
        CGSize title_rect = CGSizeZero;
        if (has_title)
        {
            const CGFloat img_w = image != nil ? image.size.width : 0;
            double title_width_constraint = width - (border_width * 2);
            // Block 1 (C#:290-296): image present, finite width, Left/Right → subtract the image width.
            if (image != nil && std::isfinite(title_width_constraint) && (position_is_left || position_is_right))
            {
                title_width_constraint -= img_w;
            }
            // Block 2 (C#:298-301): `image!=null && Left || Right` == (image && Left) || Right → subtract
            // spacing + horizontal padding. Block 3 else-if (image==null) subtracts just the padding.
            if ((image != nil && position_is_left) || position_is_right)
            {
                title_width_constraint -= spacing + padding.left + padding.right;
            }
            else if (image == nil)
            {
                title_width_constraint -= padding.left + padding.right;
            }
            const double title_height_constraint = height - (border_width * 2);
            UIFont* const font = button.titleLabel.font;
            NSDictionary* const attrs = font != nil ? @{NSFontAttributeName : font} : @{};
            const CGSize bound =
                std::isfinite(title_width_constraint)
                    ? [title
                          boundingRectWithSize:CGSizeMake(static_cast<CGFloat>(std::max(title_width_constraint, 0.0)),
                                                          static_cast<CGFloat>(std::isfinite(title_height_constraint)
                                                                                   ? title_height_constraint
                                                                                   : CGFLOAT_MAX))
                                       options:NSStringDrawingUsesLineFragmentOrigin
                                    attributes:attrs
                                       context:nil]
                          .size
                    : [title sizeWithAttributes:attrs];
            title_rect = bound;
        }

        const double image_w = image != nil ? image.size.width : 0.0;
        const double image_h = image != nil ? image.size.height : 0.0;

        double button_content_width =
            std::max<double>(title_rect.width, image_w) + padding.left + padding.right + (border_width * 2);
        double button_content_height =
            std::max<double>(title_rect.height, image_h) + padding.top + padding.bottom + (border_width * 2);

        // Both image and title present → add spacing + the smaller of the two on the image-position axis
        // (Left/Right → width; Top/Bottom → height, where image and title stack), matching C#.
        if (has_title && image != nil)
        {
            if (position_is_vertical)
            {
                button_content_height += spacing;
                button_content_height += std::min<double>(title_rect.height, image_h);
            }
            else
            {
                button_content_width += spacing;
                button_content_width += std::min<double>(title_rect.width, image_w);
            }
        }

        const double return_width = std::min<double>(button_content_width, width);
        const double return_height = std::min<double>(button_content_height, height);
        // Ceil to match UIView.SizeThatFits (C# casts to int after Math.Ceiling).
        return {std::ceil(return_width), std::ceil(return_height)};
    }

    // A UIButton cannot render smaller than its intrinsic content (title + content insets). MAUI surfaces
    // this through ButtonHandler.NeedsContainer/WrapperView: the wrapped button's measured size is a floor,
    // so a WidthRequest/HeightRequest narrower than the content grows to the content instead of truncating
    // the title to "…". The port has no WrapperView, so view<>::measure honors this flag directly — matching
    // the maui-compare reference (ClippingPage Layout2's WidthRequest=50 "Hey" buttons render full-width).
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
        [as_button(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
        // Re-frame the clip mask to the just-set bounds (WrapperView.LayoutSubviews re-runs SetClip): the
        // mask was sized at map time, before any layout, when bounds was 0×0. No-op when no clip is set.
        maui::platform::ios::reapply_clip(platform->native);
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----

    // iOS loader wiring: the on-disk uri cache (the async http(s) fetch stays with image_handler —
    // matching the image_button_handler.mm deviation).
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
        set_button_image(as_button(platform.native), load_image_from_file(file_src.file()));
    }

    void button_handler::apply_loaded_result(button_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        set_button_image(as_button(platform.native), result.loaded() ? (__bridge UIImage*)result.image() : nil);
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        [as_button(platform.native) setImage:nil forState:UIControlStateNormal];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

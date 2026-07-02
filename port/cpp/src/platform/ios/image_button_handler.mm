// image_button_handler — iOS (UIKit) platform recipe. The managed platform view is a real UIButton
// (UIButtonType.System, ClipsToBounds = true), the image surface loads through SetImage(..., Normal)
// with AlwaysOriginal rendering, the stroke/corner ride the layer (ButtonExtensions.UpdateStrokeColor/
// Thickness/CornerRadius's layer recipe), and the touch events flow back through a target-action proxy
// to send_pressed/send_released/send_clicked. Compiled as Objective-C++ with ARC only for the `ios`
// backend.
//
// Ported DIRECTLY from ImageButtonHandler.iOS.cs: CreatePlatformView = new UIButton(System) {
// ClipsToBounds = true }; ImageButtonProxy: TouchDown → Pressed, TouchUpInside → Released + Clicked,
// TouchUpOutside → Released; ImageButtonImageSourcePartSetter.SetImageSource = first-frame +
// AlwaysOriginal + SetImage(Normal) + Fill alignments + LayoutIfNeeded; MapPadding → UpdatePadding
// (contentEdgeInsets); MapStrokeColor/Thickness/CornerRadius → the ButtonExtensions layer pushes.
// NeedsContainer/WrapperView and the ICrossPlatformLayout measure override are not ported (the port
// has no container subsystem; sizeThatFits + the padding fold stand in).
//
// Loader wiring (documented deviation, same as the apple twin): configure_loader installs the on-disk
// cache only — the NSURLSession async http(s) fetch currently lives privately in image_handler.mm.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

// Obj-C trampoline: forwards the UIButton's touch events to the C++ handler's virtual view. Ports
// ImageButtonHandler.ImageButtonProxy — including the Released-before-Clicked order on TouchUpInside.
@interface MauiIosImageButtonProxy : NSObject
@property(nonatomic) maui::core::image_button_handler* handler;
- (void)onTouchUpInside:(id)sender;
- (void)onTouchUpOutside:(id)sender;
- (void)onTouchDown:(id)sender;
@end

@implementation MauiIosImageButtonProxy
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
@end

namespace
{
    // Key for the associated MauiIosImageButtonProxy kept alive by the UIButton (targets are weak).
    const char k_proxy_key = 0;

    UIButton* as_button(void* native)
    {
        return (__bridge UIButton*)native;
    }

    using maui::platform::ios::to_ui_color;

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

    // ImageButtonImageSourcePartSetter.SetImageSource: take the first frame of a multi-frame image,
    // force AlwaysOriginal rendering, SetImage(Normal), stretch the content alignments, and force a
    // layout so SizeThatFits is immediately correct.
    void set_button_image(UIButton* button, UIImage* image)
    {
        UIImage* resolved = image;
        if (resolved != nil && resolved.images != nil && resolved.images.count > 0)
        {
            resolved = resolved.images[0];
        }
        resolved = [resolved imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
        [button setImage:resolved forState:UIControlStateNormal];
        button.contentHorizontalAlignment = UIControlContentHorizontalAlignmentFill;
        button.contentVerticalAlignment = UIControlContentVerticalAlignmentFill;
        [button layoutIfNeeded];
    }

    // The maui aspect → the image view's contentMode (AspectExtensions.ToUIViewContentMode).
    UIViewContentMode to_ui_content_mode(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
                return UIViewContentModeScaleAspectFit;
            case maui::core::aspect::aspect_fill:
                return UIViewContentModeScaleAspectFill;
            case maui::core::aspect::fill:
                return UIViewContentModeScaleToFill;
            case maui::core::aspect::center:
                return UIViewContentModeCenter;
        }
        return UIViewContentModeScaleAspectFit;
    }

    // The control states whose backgroundImage carries the BackgroundColor fill (mirrors button_handler).
    constexpr std::array<UIControlState, 3> k_control_states{UIControlStateNormal, UIControlStateHighlighted,
                                                             UIControlStateDisabled};

    // A 1×1 image of a solid color. A UIButton(System) ignores backgroundColor for its fill, so MAUI draws
    // the BackgroundColor as a per-state backgroundImage (ImageButtonHandler shares ButtonHandler's recipe).
    UIImage* solid_color_image(UIColor* color)
    {
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(1, 1)];
        return [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
          CGContextSetFillColorWithColor(context.CGContext, color.CGColor);
          CGContextFillRect(context.CGContext, CGRectMake(0, 0, 1, 1));
        }];
    }
} // namespace

namespace maui::core
{
    image_button_platform::~image_button_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void image_button_platform::update_visibility(maui::core::visibility value)
    {
        as_button(native).hidden = value != maui::core::visibility::visible;
    }

    void image_button_platform::update_opacity(double value)
    {
        as_button(native).alpha = value;
    }

    void image_button_platform::update_is_enabled(bool value)
    {
        as_button(native).enabled = static_cast<BOOL>(value);
    }

    void image_button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void image_button_platform::update_background(const maui::graphics::paint* value)
    {
        // ImageButton is a system UIButton: it ignores backgroundColor for its fill, so a solid
        // BackgroundColor must be drawn as a per-state backgroundImage (the ButtonHandler recipe). A
        // gradient/image paint defers to the shared layer-based apply_background; a null paint clears it.
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
    void image_button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = as_button(native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_button_platform>();
        // CreatePlatformView: new UIButton(UIButtonType.System) { ClipsToBounds = true }.
        UIButton* const button = [UIButton buttonWithType:UIButtonTypeSystem];
        button.clipsToBounds = YES;
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    // iOS loader wiring: the on-disk uri cache (the async http(s) fetch stays with image_handler —
    // documented in the file header).
    void image_button_handler::configure_loader(image_source_loader& loader)
    {
        loader.set_disk_cache_directory(platform_cache_directory());
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        MauiIosImageButtonProxy* const proxy = [[MauiIosImageButtonProxy alloc] init];
        proxy.handler = this;
        // ImageButtonProxy.Connect: the three touch events. UIControl targets are weak, so the proxy is
        // kept alive for the button's lifetime via an associated object.
        [button addTarget:proxy action:@selector(onTouchUpInside:) forControlEvents:UIControlEventTouchUpInside];
        [button addTarget:proxy action:@selector(onTouchUpOutside:) forControlEvents:UIControlEventTouchUpOutside];
        [button addTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
        objc_setAssociatedObject(button, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        if (auto* const proxy = (MauiIosImageButtonProxy*)objc_getAssociatedObject(button, &k_proxy_key))
        {
            [button removeTarget:proxy action:@selector(onTouchUpInside:) forControlEvents:UIControlEventTouchUpInside];
            [button removeTarget:proxy
                          action:@selector(onTouchUpOutside:)
                forControlEvents:UIControlEventTouchUpOutside];
            [button removeTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
        }
        objc_setAssociatedObject(button, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The image rides the button's imageView (IImageHandler.PlatformView => PlatformView.ImageView).
        platform->image_aspect = view.aspect();
        as_button(platform->native).imageView.contentMode = to_ui_content_mode(view.aspect());
    }

    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // ImageHandler.MapIsOpaque → UIImageView.Opaque.
            platform->opaque = view.is_opaque();
            as_button(platform->native).imageView.opaque = static_cast<BOOL>(view.is_opaque());
        }
    }

    // IsAnimationPlaying — re-asserted by map_source after a load (the inherited ImageHandler.MapSource →
    // UpdateValue(IsAnimationPlaying)). C# routes MapIsAnimationPlaying through handler.PlatformView, which
    // for an ImageButton is the UIButton itself (not its UIImageView), and IImageSourcePart.IsAnimationPlaying
    // is pinned false on ImageButton — so ImageViewExtensions.UpdateIsAnimationPlaying never fires/animates.
    // The iOS twin mirrors the (always-false) flag and drives no native animation, matching C#.
    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // UIButtonExtensions.UpdatePadding → contentEdgeInsets (this SDK keeps the classic property;
        // the configuration-based path is iOS 15+ sugar over the same insets).
        platform->padding = view.padding();
        const thickness padding = view.padding();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        as_button(platform->native).contentEdgeInsets =
            UIEdgeInsetsMake(padding.top, padding.left, padding.bottom, padding.right);
#pragma clang diagnostic pop
    }

    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // ButtonExtensions.UpdateStrokeColor → layer.borderColor.
            as_button(platform->native).layer.borderColor = to_ui_color(view.stroke_color()).CGColor;
        }
    }

    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.borderWidth = view.stroke_thickness();
        }
    }

    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.cornerRadius = static_cast<CGFloat>(view.corner_radius());
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        set_button_image(as_button(platform.native), load_image_from_file(file_src.file()));
    }

    void image_button_handler::apply_loaded_result(image_button_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        set_button_image(as_button(platform.native), result.loaded() ? (__bridge UIImage*)result.image() : nil);
    }

    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        [as_button(platform.native) setImage:nil forState:UIControlStateNormal];
    }

    maui::graphics::size image_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        UIButton* const button = as_button(platform->native);
        // Port of ImageButton.iOS.cs ICrossPlatformLayout.CrossPlatformMeasure: with an image loaded, a
        // plain -[UIButton sizeThatFits:] (and -[UIImageView sizeThatFits:]) returns the raw image PIXEL
        // size, ignoring the constraint — so a large icon (e.g. a 128px @2x/@3x source) measured the button
        // to the full image height and blew past a normal button in a constrained stack. C# instead calls
        // ImageView.SizeThatFitsImage(constraint, Padding), the aspect-aware fit already ported as
        // maui::platform::ios::size_that_fits_image; the imageView contentMode (map_aspect) drives it. No
        // image → keep the SizeThatFits fallback (C#'s else branch), correct for a source-less button.
        UIImageView* const image_view = button.imageView;
        if (image_view != nil && image_view.image != nil)
        {
            const CGSize fitting =
                maui::platform::ios::size_that_fits_image(image_view, CGSizeMake(width, height), platform->padding);
            return {fitting.width, fitting.height};
        }
        const CGSize fitting = [button sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void image_button_handler::platform_arrange(const maui::graphics::rect& frame)
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

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void image_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core

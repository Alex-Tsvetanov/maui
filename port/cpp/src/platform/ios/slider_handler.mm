// slider_handler — iOS (UIKit) platform recipe. The managed platform view is a UISlider (held,
// retained, in slider_platform::native): Minimum/Maximum/Value map to MinValue/MaxValue/Value (floats,
// as in C#), the colors to the three tint properties, and the native events flow back through a
// target-action proxy — ValueChanged → set_value, TouchDown → send_drag_started,
// TouchUpInside|TouchUpOutside → send_drag_completed. Compiled as Objective-C++ with ARC only for the
// `ios` backend.
//
// Ported DIRECTLY from SliderHandler.iOS.cs + Platform/iOS/SliderExtensions.cs: CreatePlatformView =
// UISlider { Continuous = true }; SliderProxy's three wirings; UpdateMinimum/UpdateMaximum/UpdateValue
// (the differs-only guard) / UpdateMinimumTrackColor / UpdateMaximumTrackColor / UpdateThumbColor as
// the map_* bodies (the null-color guards collapse — non-nullable color, the button convention).
// ThumbImageSource IS ported (map_thumb_image_source → SetThumbImage via the handler-owned loader, with
// the thumb-tint cleared while an image is shown) and MapUpdateOnTap IS ported (a real
// UITapGestureRecognizer that sets Value from the tap location, via the i_ios_slider_specifics knob). Not
// ported (deferred, documented): the MacCatalyst PreferredBehavioralStyle branch (Catalyst-only).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_ios_slider_specifics.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// MauiIosSlider — the UISlider the handler presents. Its layoutSubviews override re-sizes any gradient/
// image background sublayer apply_background installed (a solid BackgroundColor needs no resize — it is the
// backing layer's backgroundColor), so a Background brush fills the band behind the track and tracks bounds.
// apply_background runs before arrange, when bounds is zero, so without this hook a gradient stays invisible.
@interface MauiIosSlider : UISlider
@end

@implementation MauiIosSlider
- (void)layoutSubviews
{
    [super layoutSubviews];
    maui::platform::ios::resize_background_layers((__bridge void*)self);
}
@end

// Obj-C trampoline: forwards the UISlider's control events to the C++ handler's virtual view — the
// SliderProxy port (ValueChanged / TouchDown / TouchUp).
@interface MauiSliderEventProxy : NSObject
@property(nonatomic) maui::core::slider_handler* handler;
- (void)onValueChanged:(id)sender;
- (void)onTouchDown:(id)sender;
- (void)onTouchUp:(id)sender;
@end

@implementation MauiSliderEventProxy
- (void)onValueChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    auto* const native = (UISlider*)sender;
    if (view != nullptr && native != nil)
    {
        view->set_value(native.value); // OnControlValueChanged: VirtualView.Value = PlatformView.Value
    }
}

- (void)onTouchDown:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_drag_started(); // OnTouchDownControlEvent → DragStarted
        }
    }
}

- (void)onTouchUp:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_drag_completed(); // OnTouchUpControlEvent → DragCompleted
        }
    }
}
@end

// The UpdateOnTap recognizer's target (SliderHandler.MapUpdateOnTap's UITapGestureRecognizer body): on a
// tap, set Value from the tapped X relative to the track width — Value = tappedX * MaxValue / Width.
@interface MauiSliderTapProxy : NSObject
@property(nonatomic) maui::core::slider_handler* handler;
- (void)onTap:(UITapGestureRecognizer*)recognizer;
@end

@implementation MauiSliderTapProxy
- (void)onTap:(UITapGestureRecognizer*)recognizer
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* view = self.handler->virtual_view();
    auto* const platform = self.handler->typed_platform_view();
    if (view == nullptr || platform == nullptr || platform->native == nullptr)
    {
        return;
    }
    UISlider* const slider = (__bridge UISlider*)platform->native;
    const CGPoint location = [recognizer locationInView:slider];
    const CGFloat width = slider.frame.size.width;
    // C# guards `tappedLocation != default` — CGPoint has no operator!=, so compare components.
    const bool non_default = location.x != 0 || location.y != 0;
    if (non_default && width > 0)
    {
        view->set_value(static_cast<double>(location.x) * static_cast<double>(slider.maximumValue) /
                        static_cast<double>(width));
    }
}
@end

namespace
{
    // Key for the associated MauiSliderEventProxy kept alive by the UISlider (UIControl does not
    // retain its targets — the target-action convention).
    const char k_proxy_key = 0;
    // Key for the UpdateOnTap recognizer (attached to the slider) + its target proxy (attached to the
    // recognizer) — both kept alive via associated objects (distinct keys, distinct host objects).
    const char k_tap_recognizer_key = 0;
    const char k_tap_target_key = 0;

    UISlider* as_slider(void* native)
    {
        return (__bridge UISlider*)native;
    }

    using maui::platform::ios::to_ui_color;

    // C# SliderExtensions: thumbImage.ApplyTintColor(ThumbColor.ToPlatform()) — recolor the thumb image
    // with the requested tint before SetThumbImage, so a tinted knob image renders in the tint (the bug
    // was the iOS recipe ignoring ThumbColor entirely). Render the image into a graphics context with the
    // color as the fill, preserving the alpha mask (the standard template-tint recolor).
    UIImage* tint_image(UIImage* image, UIColor* color)
    {
        if (image == nil || color == nil)
        {
            return image;
        }
        const CGRect bounds = CGRectMake(0, 0, image.size.width, image.size.height);
        UIGraphicsImageRendererFormat* const format = [UIGraphicsImageRendererFormat preferredFormat];
        format.opaque = NO;
        format.scale = image.scale;
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithBounds:bounds format:format];
        return [renderer imageWithActions:^(UIGraphicsImageRendererContext* ctx) {
          // Draw the image as a template (alpha mask), then fill the mask with the tint color.
          [image drawInRect:bounds];
          [color set];
          UIRectFillUsingBlendMode(bounds, kCGBlendModeSourceIn);
        }];
    }
} // namespace

namespace maui::core
{
    slider_platform::~slider_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void slider_platform::update_visibility(maui::core::visibility value)
    {
        as_slider(native).hidden = value != maui::core::visibility::visible;
    }

    void slider_platform::update_opacity(double value)
    {
        as_slider(native).alpha = value;
    }

    void slider_platform::update_is_enabled(bool value)
    {
        as_slider(native).enabled = static_cast<BOOL>(value);
    }

    void slider_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_slider(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    void slider_platform::update_background(const maui::graphics::paint* value)
    {
        // A UISlider has no bezel, so its BackgroundColor / Background brush shows as a band behind the
        // track: the shared helper paints a solid color onto the backing layer or installs a gradient/image
        // sublayer (MauiIosSlider.layoutSubviews re-sizes the gradient to bounds). A null paint clears it.
        maui::platform::ios::apply_background(native, value);
    }

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        auto platform = std::make_unique<slider_platform>();
        UISlider* const native = [[MauiIosSlider alloc] initWithFrame:CGRectZero];
        native.continuous = YES;                            // CreatePlatformView: new UISlider { Continuous = true }
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        UISlider* const native = as_slider(platform.native);
        MauiSliderEventProxy* const proxy = [[MauiSliderEventProxy alloc] init];
        proxy.handler = this;
        // SliderProxy.Connect — the three wirings. UIControl holds its targets weakly, so the proxy is
        // kept alive for the slider's lifetime via an associated object (the button pattern).
        [native addTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
        [native addTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
        [native addTarget:proxy
                      action:@selector(onTouchUp:)
            forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
        objc_setAssociatedObject(native, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        UISlider* const native = as_slider(platform.native);
        if (auto* const proxy = (MauiSliderEventProxy*)objc_getAssociatedObject(native, &k_proxy_key))
        {
            // SliderProxy.Disconnect — unhook the same three wirings.
            [native removeTarget:proxy action:@selector(onValueChanged:) forControlEvents:UIControlEventValueChanged];
            [native removeTarget:proxy action:@selector(onTouchDown:) forControlEvents:UIControlEventTouchDown];
            [native removeTarget:proxy
                          action:@selector(onTouchUp:)
                forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
        }
        objc_setAssociatedObject(native, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // SliderExtensions.UpdateMinimum: MinValue = (float)slider.Minimum.
            as_slider(platform->native).minimumValue = static_cast<float>(view.minimum());
        }
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // SliderExtensions.UpdateMaximum: MaxValue = (float)slider.Maximum.
            as_slider(platform->native).maximumValue = static_cast<float>(view.maximum());
        }
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        // SliderExtensions.UpdateValue: write only when (float)Value differs (prevents the echo).
        if (auto* platform = handler.typed_platform_view())
        {
            UISlider* const native = as_slider(platform->native);
            if (static_cast<float>(view.value()) != native.value)
            {
                native.value = static_cast<float>(view.value());
            }
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // SliderExtensions.UpdateMinimumTrackColor: an unset (default-constructed) color must leave the
            // UISlider's native default tint, NOT force a transparent fill — to_ui_color(unset) is a clear
            // color that hides the track. nil restores the system default (the C# null branch is a no-op).
            const maui::graphics::color color = view.minimum_track_color();
            as_slider(platform->native).minimumTrackTintColor =
                color != maui::graphics::color{} ? to_ui_color(color) : nil;
        }
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // SliderExtensions.UpdateMaximumTrackColor: unset leaves the native default gray track (nil),
            // not a transparent fill (to_ui_color(unset) would hide it).
            const maui::graphics::color color = view.maximum_track_color();
            as_slider(platform->native).maximumTrackTintColor =
                color != maui::graphics::color{} ? to_ui_color(color) : nil;
        }
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        // SliderExtensions.UpdateThumbColor: when a ThumbImageSource is set, the image wins (re-run the
        // image map so the thumb image — not the tint — is applied); otherwise push the color tint. An unset
        // ThumbColor must leave the native default thumb (the white pill), so map it to nil rather than a
        // transparent tint (to_ui_color(unset) would make the thumb invisible).
        if (view.thumb_image_source() != nullptr && !view.thumb_image_source()->is_empty())
        {
            map_thumb_image_source(handler, view);
            return;
        }
        if (auto* platform = handler.typed_platform_view())
        {
            const maui::graphics::color color = view.thumb_color();
            as_slider(platform->native).thumbTintColor = color != maui::graphics::color{} ? to_ui_color(color) : nil;
        }
    }

    maui::graphics::size slider_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_slider(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void slider_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_slider(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // ThumbImageSource (SliderExtensions.UpdateThumbImageSourceAsync), iOS recipe. The cross-platform
    // map routes the source through the handler-owned loader; these per-backend primitives apply/clear the
    // decoded UIImage on the real UISlider.
    void slider_handler::configure_thumb_loader(image_source_loader& /*loader*/)
    {
        // Leave the loader on its synchronous read_uri_bytes default: the common ThumbImageSource is a
        // file/local source, resolved inline. (A remote-URI thumb would want the NSURLSession fetch the
        // image_handler installs; the knob image is virtually always a bundled asset, so the default
        // suffices and keeps the slider handler free of the network dependency.)
    }

    void slider_handler::apply_thumb_image(slider_platform& platform, i_slider& view, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        UISlider* const slider = as_slider(platform.native);
        // SliderExtensions: "Clear the thumb color if we have a thumb image, so the slider doesn't clear
        // the image while sliding", then SetThumbImage for the Normal state.
        slider.thumbTintColor = nil;
        UIImage* image = result.loaded() ? (__bridge UIImage*)result.image() : nil;
        // SliderExtensions.UpdateThumbImageSourceAsync: when BOTH a thumb image and a ThumbColor are set,
        // tint the image with the color (ApplyTintColor) before SetThumbImage. The default (collapsed-null)
        // color leaves the image untinted — the `color{}` sentinel mirrors switch_handler's track_color.
        if (image != nil && view.thumb_color() != maui::graphics::color{})
        {
            image = tint_image(image, maui::platform::ios::to_ui_color(view.thumb_color()));
        }
        [slider setThumbImage:image forState:UIControlStateNormal];
        platform.thumb_image_set = image != nil;
        // iOS 26+: SetThumbImage no longer triggers the layout pass that repositions the thumb. Call it
        // unconditionally (harmless pre-26), matching this unit's other unconditional iOS-26 workarounds.
        [slider setNeedsLayout];
    }

    void slider_handler::clear_thumb_image(slider_platform& platform, i_slider& view)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        // SliderExtensions.UpdateThumbImageSourceAsync else branch: drop the image, then re-apply the
        // thumb color (UpdateThumbColor).
        UISlider* const slider = as_slider(platform.native);
        [slider setThumbImage:nil forState:UIControlStateNormal];
        platform.thumb_image_set = false;
        // Restore the thumb color (UpdateThumbColor): an unset color leaves the native default white thumb
        // (nil), not a transparent tint that would make it invisible.
        const maui::graphics::color thumb = view.thumb_color();
        slider.thumbTintColor = thumb != maui::graphics::color{} ? maui::platform::ios::to_ui_color(thumb) : nil;
        // iOS 26+: SetThumbImage(nil) likewise needs an explicit layout pass (unconditional, harmless pre-26).
        [slider setNeedsLayout];
    }

    // SliderHandler.MapUpdateOnTap: install (or remove) the tap-to-set UITapGestureRecognizer based on the
    // iOSSpecific UpdateOnTap knob. Re-running is idempotent: a recognizer is attached only once, and a
    // false value removes it. The recognizer + its target proxy are kept alive via associated objects.
    void slider_handler::map_update_on_tap(slider_handler& handler, i_slider& view)
    {
        auto* platform = handler.typed_platform_view();
        const auto* specifics = dynamic_cast<const i_ios_slider_specifics*>(&view);
        if (platform == nullptr || platform->native == nullptr || specifics == nullptr)
        {
            return;
        }
        UISlider* const slider = as_slider(platform->native);
        auto* existing = (UITapGestureRecognizer*)objc_getAssociatedObject(slider, &k_tap_recognizer_key);
        if (specifics->update_on_tap())
        {
            if (existing == nil)
            {
                MauiSliderTapProxy* const tap_proxy = [[MauiSliderTapProxy alloc] init];
                tap_proxy.handler = &handler;
                UITapGestureRecognizer* const recognizer =
                    [[UITapGestureRecognizer alloc] initWithTarget:tap_proxy action:@selector(onTap:)];
                recognizer.cancelsTouchesInView = NO; // C# CancelsTouchesInView = false
                [slider addGestureRecognizer:recognizer];
                // The recognizer retains its target only weakly; keep the proxy alive next to it.
                objc_setAssociatedObject(recognizer, &k_tap_target_key, tap_proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                objc_setAssociatedObject(slider, &k_tap_recognizer_key, recognizer, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            }
        }
        else if (existing != nil)
        {
            [slider removeGestureRecognizer:existing];
            objc_setAssociatedObject(slider, &k_tap_recognizer_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
        platform->update_on_tap = specifics->update_on_tap();
    }
} // namespace maui::core

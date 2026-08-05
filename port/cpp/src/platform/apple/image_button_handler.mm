// image_button_handler — Apple (AppKit / macOS) platform recipe. The real-native twin of the headless
// partial: the managed platform view is an image-bearing NSButton (held, retained, in
// image_button_platform::native), the image surface loads into NSButton.image, the stroke/corner ride
// the layer (the button recipe), and the native click flows back through a target-action trampoline to
// send_released() + send_clicked() (TouchUpInside order). Compiled as Objective-C++ with ARC only for
// the `apple` backend.
//
// Translated from ImageButtonHandler.iOS.cs (UIButton(UIButtonType.System) { ClipsToBounds = true } +
// SetImage(..., UIControlState.Normal) + the ImageButtonProxy touch events): MAUI's macOS support is
// Mac Catalyst, so there is no AppKit ImageButtonHandler in the read-only C# source — the AppKit
// specifics are the standard NSButton-with-image equivalents (a borderless NSButton showing
// imageOnly, scaling per the maui aspect). NSButton's action fires on a completed click (mouse-up
// inside), so press/release granularity collapses to the click (the button control's documented AppKit
// shape); send_released still precedes send_clicked, preserving the C# order.
//
// Loader wiring (documented deviation): configure_loader installs the on-disk cache only — the
// NSURLSession async http(s) fetch currently lives privately in image_handler.mm, so the image button's
// uri loads use the loader's synchronous default (file:// + local paths); hoisting the shared fetch is
// noted in STATUS.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
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

// Obj-C trampoline: forwards NSButton's target-action to the C++ handler's virtual view, preserving
// the Released-before-Clicked order of TouchUpInside.
@interface MauiImageButtonTarget : NSObject
@property(nonatomic) maui::core::image_button_handler* handler;
- (void)onClick:(id)sender;
@end

@implementation MauiImageButtonTarget
- (void)onClick:(id)sender
{
    (void)sender;
    // `keep` pins us: the raise below is user code and may destroy the view, which runs
    // ~image_button_platform and drops the association that holds this trampoline. Every deref that
    // FOLLOWS a raise goes through live_view (apple_view_ops.hpp), which re-reads the
    // back-pointer that same dtor nulls.
    MauiImageButtonTarget* const keep = self;
    if (auto* const view = maui::platform::apple::live_view(keep.handler))
    {
        view->send_released();
    }
    if (auto* const view = maui::platform::apple::live_view(keep.handler))
    {
        view->send_clicked();
    }
}
@end

namespace
{
    // Key for the associated MauiImageButtonTarget kept alive by the NSButton (its `target` is weak).
    const char k_target_key = 0;

    NSButton* as_button(void* native)
    {
        return (__bridge NSButton*)native;
    }

    using maui::platform::apple::to_ns_color;

    // The maui aspect → NSImageScaling for the button's image (the image_handler mapping table).
    NSImageScaling to_ns_image_scaling(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
            case maui::core::aspect::aspect_fill: // closest built-in (no clipping fill on AppKit)
                return NSImageScaleProportionallyUpOrDown;
            case maui::core::aspect::fill:
                return NSImageScaleAxesIndependently;
            case maui::core::aspect::center:
                return NSImageScaleNone;
        }
        return NSImageScaleProportionallyUpOrDown;
    }

    NSImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const raw = [NSString stringWithUTF8String:file.c_str()];
        if (raw == nil)
        {
            return nil;
        }
        return [[NSImage alloc] initWithContentsOfFile:raw];
    }

    // NSCachesDirectory (the image_handler convention) for the loader's on-disk uri cache.
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
} // namespace

namespace maui::core
{
    // The teardown that must run whether the handler is DISCONNECTED or merely DESTROYED. The native
    // view outlives the handler in any real app (a superview retains it) and the trampolines it keeps
    // in its associated objects carry RAW handler pointers; nothing calls disconnect_handler() when a
    // handler is destroyed (there is no ~view_handler doing it), so the platform dtor has to run this
    // too or the next native callback dereferences freed memory. Idempotent: disconnect_handler()
    // destroys the platform right after calling it, so both paths run on the same object.
    namespace
    {
        void detach_trampolines(image_button_platform& platform)
        {
            NSButton* const button = as_button(platform.native);
            button.target = nil;
            button.action = nil;
            if (auto* const trampoline = (MauiImageButtonTarget*)objc_getAssociatedObject(button, &k_target_key))
            {
                trampoline.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(button, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
    } // namespace

    image_button_platform::~image_button_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
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
        as_button(native).alphaValue = value;
    }

    void image_button_platform::update_is_enabled(bool value)
    {
        [as_button(native) setEnabled:static_cast<BOOL>(value)];
    }

    void image_button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_button_platform>();
        // The image-only NSButton (the AppKit shape of UIButton(System) { ClipsToBounds = true }):
        // borderless, image-positioned, clipping to its layer bounds.
        NSButton* const button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        button.title = @"";
        button.imagePosition = NSImageOnly;
        button.bordered = NO;
        [button setButtonType:NSButtonTypeMomentaryChange];
        button.wantsLayer = YES;
        button.layer.masksToBounds = YES;                   // ClipsToBounds = true
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    // Apple loader wiring: the on-disk uri cache (the async http(s) fetch stays with image_handler —
    // documented in the file header).
    void image_button_handler::configure_loader(image_source_loader& loader)
    {
        loader.set_disk_cache_directory(platform_cache_directory());
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        NSButton* const button = as_button(platform.native);
        MauiImageButtonTarget* const target = [[MauiImageButtonTarget alloc] init];
        target.handler = this;
        button.target = target; // weak (target-action convention)...
        button.action = @selector(onClick:);
        // ...so keep it alive for the button's lifetime via an associated object.
        objc_setAssociatedObject(button, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
        detach_trampolines(platform);
    }

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->image_aspect = view.aspect();
        as_button(platform->native).imageScaling = to_ns_image_scaling(view.aspect());
    }

    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSButton* const button = as_button(platform->native);
        button.wantsLayer = YES;
        button.layer.opaque = static_cast<BOOL>(view.is_opaque());
    }

    // IsAnimationPlaying — re-asserted by map_source after a load (the inherited ImageHandler.MapSource →
    // UpdateValue(IsAnimationPlaying)). C# routes MapIsAnimationPlaying through handler.PlatformView, which
    // for an ImageButton is the UIButton (not a UIImageView), and IImageSourcePart.IsAnimationPlaying is
    // pinned false on ImageButton — so the C# call never starts native animation. An NSButton has no
    // multi-frame .animates surface, so the AppKit twin mirrors the (always-false) flag and drives nothing.
    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
    {
        // AppKit has no contentEdgeInsets; the maui padding is recorded on the mirror and folded into
        // the measure below (UpdatePadding enlarges the UIButton's fitted size the same way).
        if (auto* platform = handler.typed_platform_view())
        {
            platform->padding = view.padding();
        }
    }

    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
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

    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
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

    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
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

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = load_image_from_file(file_src.file());
    }

    void image_button_handler::apply_loaded_result(image_button_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = result.loaded() ? (__bridge NSImage*)result.image() : nil;
    }

    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_button(platform.native).image = nil;
    }

    maui::graphics::size image_button_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // The fitted size plus the maui padding (the ContentEdgeInsets enlargement UpdatePadding gives a
        // UIButton; the borderless NSButton carries the padding on the mirror).
        const NSSize fitting = [as_button(platform->native) fittingSize];
        const thickness padding = platform->padding;
        return {fitting.width + padding.left + padding.right, fitting.height + padding.top + padding.bottom};
    }

    void image_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed via the shared apple_view_ops helpers.
    void image_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void image_button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed via the shared apple_visual_ops helpers.
    void image_button_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void image_button_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void image_button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag (shared apple_semantics_ops helpers).
    void image_button_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void image_button_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core

// image_handler — Apple (AppKit / macOS) platform recipe (aspect + file source). The real-native twin of
// the headless partial: the managed platform view is an NSImageView (held, retained, in
// image_platform::native); the cross-platform aspect maps to the view's imageScaling (+ imageAlignment),
// and a file source loads SYNCHRONOUSLY into the view's image. Compiled as Objective-C++ with ARC only for
// the `apple` backend.
//
// aspect — translated from ImageHandler.iOS.cs + AspectExtensions.cs (UIKit's UIViewContentMode):
//   AspectFit  → ScaleAspectFit   → NSImageScaleProportionallyUpOrDown
//   AspectFill → ScaleAspectFill   → NSImageScaleProportionallyUpOrDown + centered  [*]
//   Fill       → ScaleToFill       → NSImageScaleAxesIndependently
//   Center     → Center            → NSImageScaleNone + centered
// [*] AppKit's NSImageScaling has no exact aspect-fill-with-clipping mode (NSImageView does not clip an
//     up-scaled image the way UIView ScaleAspectFill does); proportional + centered is the closest
//     built-in approximation. This imperfect mapping is noted rather than worked around (a clipping
//     container would be a larger change, out of scope for this minimal cut).
//
// source — translated from ImageHandler.iOS.cs MapSource + ImageSourceExtensions.cs GetPlatformImage
// (UIImage.FromBundle(name) ?? UIImage.FromFile(file)). A FILE source loads SYNCHRONOUSLY via
// [[NSImage alloc] initWithContentsOfFile:] (AppKit's NSImage has no FromBundle split — the file path is
// loaded directly); a uri/stream source routes through the handler-owned image_source_loader, whose apply
// sets imageView.image from the decoded NSImage in the result (the loader's services produce it — see
// src/platform/apple/image_source_services.mm). A null/empty source clears imageView.image. The
// cross-platform routing lives in image_handler.cpp::map_source; only the three primitives below touch the
// NSImageView. DEFERRED: font image sources, disk caching, resolution reload, the full DI service-provider.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSImageView* as_image_view(void* native)
    {
        return (__bridge NSImageView*)native;
    }

    NSImageScaling to_ns_image_scaling(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
            case maui::core::aspect::aspect_fill:
                return NSImageScaleProportionallyUpOrDown;
            case maui::core::aspect::fill:
                return NSImageScaleAxesIndependently;
            case maui::core::aspect::center:
                return NSImageScaleNone;
        }
        return NSImageScaleProportionallyUpOrDown;
    }

    // Synchronous file load: AppKit's [[NSImage alloc] initWithContentsOfFile:] (returns nil if the file is
    // missing or not a decodable image). The C# original tries FromBundle first; AppKit has no equivalent
    // path split, so the file path is loaded directly.
    NSImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const ns_path = [NSString stringWithUTF8String:file.c_str()];
        if (ns_path == nil)
        {
            return nil;
        }
        return [[NSImage alloc] initWithContentsOfFile:ns_path];
    }
} // namespace

namespace maui::core
{
    image_platform::~image_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void image_platform::update_visibility(maui::core::visibility value)
    {
        as_image_view(native).hidden = value != maui::core::visibility::visible;
    }

    void image_platform::update_opacity(double value)
    {
        as_image_view(native).alphaValue = value;
    }

    void image_platform::update_is_enabled(bool value)
    {
        [as_image_view(native) setEnabled:static_cast<BOOL>(value)];
    }

    void image_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_image_view(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
        NSImageView* const view = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)view; // the void* slot owns one reference
        return platform;
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSImageView* const image_view = as_image_view(platform->native);
        image_view.imageScaling = to_ns_image_scaling(view.aspect());
        // Keep the image centered for every mode (NSImageView's own default), so Center and the
        // aspect-* approximations sit in the middle of the view rather than a corner.
        image_view.imageAlignment = NSImageAlignCenter;
    }

    // IsOpaque → the backing layer's `opaque` hint (the rendering optimization C# IsOpaque expresses). The
    // view is given a layer so the flag has somewhere to live.
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSImageView* const image_view = as_image_view(platform->native);
        image_view.wantsLayer = YES;
        image_view.layer.opaque = static_cast<BOOL>(view.is_opaque());
    }

    // IsAnimationPlaying → NSImageView.animates (the AppKit analog of UIImageView's start/stopAnimating).
    // DEVIATION: the multi-frame GIF decode (UIImageView.AnimationImages) is not ported — only an animated
    // NSImage already set on the view would animate; the flag itself is faithful + mapped.
    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        as_image_view(platform->native).animates = static_cast<BOOL>(view.is_animation_playing());
    }

    // ---- per-backend source primitives (the cross-platform map_source in image_handler.cpp routes here) ----

    // File fast-path: load the path via NSImage straight into the view (nil on a failed load → cleared).
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_image_view(platform.native).image = load_image_from_file(file_src.file());
    }

    // The async loader's apply: set the view to the decoded NSImage the result carries (the result retains
    // it; the NSImageView takes its own retain). A !loaded() result clears the view.
    void image_handler::apply_loaded_result(image_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_image_view(platform.native).image = result.loaded() ? (__bridge NSImage*)result.image() : nil;
    }

    // Clear the loaded image.
    void image_handler::clear_source_native(image_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        as_image_view(platform.native).image = nil;
    }

    maui::graphics::size image_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // No image bytes are loaded this cut, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_image_view(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void image_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void image_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers
    // (M4d ViewMapper visuals). `native` is this struct's NSView handle.
    void image_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void image_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void image_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }
} // namespace maui::core

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
#import <Foundation/Foundation.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "apple_semantics_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp"
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

    // The platform cache root (C# FileSystem.CacheDirectory → NSCachesDirectory). Empty on the rare failure
    // to resolve it, which simply leaves the disk layer off. The "com.microsoft.maui/MauiUriImages" tail is
    // appended by uri_image_disk_cache, matching C# UriImageSourceService.CacheDirectory.
    std::string platform_cache_directory()
    {
        NSArray<NSString*>* const dirs = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString* const base = dirs.firstObject;
        if (base == nil)
        {
            return {};
        }
        const char* const utf8 = base.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    // Copy an NSData's bytes into image_bytes (empty for nil/zero-length).
    maui::core::image_bytes to_image_bytes(NSData* data)
    {
        if (data == nil || data.length == 0)
        {
            return {};
        }
        maui::core::image_bytes bytes(static_cast<std::size_t>(data.length));
        std::memcpy(bytes.data(), data.bytes, static_cast<std::size_t>(data.length));
        return bytes;
    }

    // The async uri fetch (C# UriImageSourceService.DownloadImageAsync, translated to a real NSURLSession
    // dataTask). A `file://` / local path is read synchronously via the cross-platform reader (so it matches
    // the loader's fast-path and stays off the network); an http(s) URI starts an async dataTask whose
    // completion — running on the URLSession delegate queue — hops the bytes back onto the MAIN queue (the
    // dispatcher hand-off) before invoking `sink`. The token is checked at completion so a cancelled load
    // drops its bytes. The only cross-thread elements are the cancellation atomic + this main-queue hop
    // (the loader's apply then runs inline on the main thread, where the NSImageView lives). The signature
    // binds to image_source_loader::uri_fetch (its by-value uri/token args bind to these const refs).
    // The owned per-fetch state the completion blocks capture (by shared_ptr): the move-only sink + an OWNED
    // copy of the cancellation token. Bundled so the blocks (which must be copyable + outlive fetch_uri_async)
    // hold their own token lifetime rather than a dangling reference to the const-ref parameter.
    struct uri_fetch_state
    {
        maui::core::image_source_loader::uri_bytes_sink sink;
        maui::core::cancellation_token token;
    };

    void fetch_uri_async(const std::string& uri, const maui::core::cancellation_token& token,
                         maui::core::image_source_loader::uri_bytes_sink sink)
    {
        auto state = std::make_shared<uri_fetch_state>(uri_fetch_state{.sink = std::move(sink), .token = token});

        // Local files / non-http schemes: read synchronously (the loader's file:// fast-path equivalent).
        if (!uri.starts_with("http://") && !uri.starts_with("https://"))
        {
            state->sink(maui::core::read_uri_bytes(uri));
            return;
        }

        NSString* const ns_uri = [NSString stringWithUTF8String:uri.c_str()];
        NSURL* const url = ns_uri != nil ? [NSURL URLWithString:ns_uri] : nil;
        if (url == nil)
        {
            state->sink(maui::core::image_bytes{}); // a malformed uri → nothing fetched
            return;
        }

        NSURLSessionDataTask* const task = [[NSURLSession sharedSession]
              dataTaskWithURL:url
            completionHandler:^(NSData* data, NSURLResponse* /*response*/, NSError* /*error*/) {
              // On the URLSession queue: snapshot the bytes (__block so they can be MOVED into the main-queue
              // block), then marshal the sink onto the main queue.
              __block maui::core::image_bytes bytes =
                  state->token.is_cancelled() ? maui::core::image_bytes{} : to_image_bytes(data);
              dispatch_async(dispatch_get_main_queue(), ^{
                state->sink(std::move(bytes));
              });
            }];
        [task resume];
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

    // Apple loader wiring: the real NSURLSession async uri fetch + the NSCachesDirectory on-disk cache
    // (UriImageSourceService DownloadAndCacheImageAsync). No i_dispatcher is set: fetch_uri_async already
    // hops its completion onto the MAIN queue, so the loader's apply runs inline on the main thread (where
    // the NSImageView lives) — the main-queue hop IS the dispatcher hand-off.
    void image_handler::configure_loader(image_source_loader& loader)
    {
        loader.set_disk_cache_directory(platform_cache_directory());
        loader.set_uri_fetch(&fetch_uri_async);
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

    // IsAnimationPlaying → NSImageView.animates — NATIVE multi-frame GIF playback. This is the AppKit analog
    // of UIKit's StartAnimating()/StopAnimating() (ImageViewExtensions.UpdateIsAnimationPlaying): an NSImage
    // decoded from GIF data (decode_image_bytes' [[NSImage alloc] initWithData:]) keeps ALL frames in its
    // NSBitmapImageRep (NSImageFrameCount > 1), and `animates = YES` cycles them using the GIF's own per-frame
    // delays. So the real frame cycle is driven by AppKit's built-in animator — no hand-built frame timer or
    // AnimationImages array is needed (that UIKit machinery, ImageAnimationHelper, exists only because
    // UIImageView has no auto-animated multi-frame image; NSImageView does). Setting the flag false stops on
    // the current frame (StopAnimating). map_source re-applies this after a load so a freshly-decoded animated
    // image starts playing when IsAnimationPlaying is already set.
    //
    // REDUCTION (documented): we rely on AppKit's built-in GIF frame cycling rather than re-implementing the
    // FFImageLoading GCD-based frame array; for a non-GIF (single-frame) image `animates` is a harmless no-op.
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

    // Accessibility metadata + the input-transparent flag pushed to the native view via the shared
    // apple_semantics_ops helpers (M5d native a11y / hit-test). `native` is this struct's NSView handle.
    void image_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void image_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core

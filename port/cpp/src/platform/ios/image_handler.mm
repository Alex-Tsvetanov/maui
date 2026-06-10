// image_handler — iOS (UIKit) platform recipe (aspect + sources + native GIF playback). The managed
// platform view is a real UIImageView (held, retained, in image_platform::native); the cross-platform
// aspect maps to contentMode (+ the ClipsToBounds rule), and the sources flow through the SAME
// cross-platform routing as every backend (image_handler.cpp::map_source): a FILE source loads
// synchronously here, everything else (uri / stream / font) goes through the handler-owned
// image_source_loader whose services decode real UIImages (src/platform/ios/image_source_services.mm).
// Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from ImageHandler.iOS.cs + Platform/iOS/ImageViewExtensions.cs + AspectExtensions.cs
// + ImageSources/iOS/ImageSourceExtensions.cs (the same oracles the AppKit twin in
// src/platform/apple/image_handler.mm was adapted from):
//   - UpdateAspect: Aspect → UIViewContentMode, ClipsToBounds for ScaleAspectFill/Center — UIKit's
//     REAL aspect-fill-with-clipping (the AppKit twin could only approximate it).
//   - the ImageImageSourcePartSetter.SetImageSource shape: an ANIMATED UIImage (non-nil Images) lands
//     as Image = Images[0] + AnimationImages/AnimationDuration; a still image clears the animation
//     pair; then IsAnimationPlaying is re-asserted so a freshly-loaded GIF starts cycling.
//   - UpdateIsAnimationPlaying: StartAnimating / StopAnimating, guarded by IsAnimating.
//   - the file path: CGImageSource decode of the file bytes (animated-capable, the
//     FileImageSourceService.iOS route) ?? UIImage.FromBundle(name-without-extension) — the
//     GetPlatformImage fallback chain ("UIImage.FromBundle(bundleName) ?? UIImage.FromFile(file)";
//     FromFile is subsumed by the byte decode). The @2x/@3x scaled-file probe is deferred (needs the
//     display-density plumbing).
// IsOpaque → UIView.opaque (the same rendering hint the AppKit twin pushes to layer.opaque — UIView's
// property IS the layer flag). MauiImageView's window-change reload (OnWindowChanged /
// RequiresReload) is deferred with the resolution-reload machinery, as on the AppKit twin.

#import <UIKit/UIKit.h>

#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_decode.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UIImageView* as_image_view(void* native)
    {
        return (__bridge UIImageView*)native;
    }

    // AspectExtensions.ToUIViewContentMode.
    UIViewContentMode to_ui_view_content_mode(maui::core::aspect value)
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

    // ImageViewExtensions.UpdateIsAnimationPlaying: start/stop native frame cycling, guarded by
    // IsAnimating so a re-push is idempotent.
    void apply_is_animation_playing(UIImageView* image_view, bool playing)
    {
        if (playing)
        {
            if (!image_view.isAnimating)
            {
                [image_view startAnimating];
            }
        }
        else
        {
            if (image_view.isAnimating)
            {
                [image_view stopAnimating];
            }
        }
    }

    // ImageImageSourcePartSetter.SetImageSource: an animated UIImage (its `images` frame array is
    // non-nil — the CGImageSource decode produced [UIImage animatedImageWithImages:duration:]) lands as
    // the first frame + the AnimationImages/AnimationDuration pair; a still image clears them. Then the
    // IsAnimationPlaying flag is re-asserted (C#'s trailing UpdateValue(IsAnimationPlaying)) so a GIF
    // that arrives while the flag is already set starts playing.
    //
    // Writing AnimationImages makes UIKit RECOMPUTE the view's opaqueness (verified on the iOS 26
    // simulator: `animationImages = nil` flips `opaque` back to YES), silently undoing map_is_opaque —
    // so the last-pushed IsOpaque mirror is re-asserted afterwards. (C# never trips this: its handler
    // does not map IsOpaque natively; the port's is_opaque push is the documented extension.)
    void set_image_on_view(UIImageView* image_view, UIImage* image, const maui::core::image_platform& platform)
    {
        if (image != nil && image.images != nil)
        {
            image_view.image = image.images.firstObject;
            image_view.animationImages = image.images;
            image_view.animationDuration = image.duration;
        }
        else
        {
            image_view.animationImages = nil;
            image_view.animationDuration = 0.0;
            image_view.image = image;
        }
        image_view.opaque = static_cast<BOOL>(platform.opaque);
        apply_is_animation_playing(image_view, platform.animation_playing);
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

    // The synchronous file load (FileImageSourceService.iOS GetImageAsync's chain): decode the file's
    // bytes through the shared CGImageSource decode (animated-capable; covers UIImage.FromFile), then
    // fall back to the bundle lookup with the extension-less base name (UIImage.FromBundle — "MauiImage
    // assets are flattened into the app bundle root").
    UIImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const ns_path = [NSString stringWithUTF8String:file.c_str()];
        if (ns_path == nil)
        {
            return nil;
        }
        NSData* const data = [NSData dataWithContentsOfFile:ns_path];
        if (data != nil)
        {
            // The ARC-strong local takes its own retain BEFORE the result (and its CFRelease disposer)
            // goes out of scope, so the image outlives the temporary decode result.
            UIImage* decoded = nil;
            const maui::core::image_source_result result =
                maui::core::decode_image_bytes(to_image_bytes(data), "file", file);
            if (result.loaded())
            {
                decoded = (__bridge UIImage*)result.image();
            }
            if (decoded != nil)
            {
                return decoded;
            }
        }
        NSString* const bundle_name = ns_path.lastPathComponent.stringByDeletingPathExtension;
        return bundle_name.length > 0 ? [UIImage imageNamed:bundle_name] : nil;
    }

    // The platform cache root (C# FileSystem.CacheDirectory → NSCachesDirectory). Empty on the rare
    // failure to resolve it, which simply leaves the disk layer off. The "com.microsoft.maui/
    // MauiUriImages" tail is appended by uri_image_disk_cache, matching C# UriImageSourceService.
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

    // The async uri fetch (C# UriImageSourceService.DownloadImageAsync, translated to a real
    // NSURLSession dataTask) — the same recipe as the AppKit twin: a file:// / local path is read
    // synchronously via the cross-platform reader; an http(s) URI starts an async dataTask whose
    // completion — running on the URLSession delegate queue — hops the bytes back onto the MAIN queue
    // (the dispatcher hand-off) before invoking `sink`. The token is checked at completion so a
    // cancelled load drops its bytes. The owned per-fetch state the blocks capture (by shared_ptr): the
    // move-only sink + an OWNED copy of the cancellation token (the blocks must be copyable and outlive
    // fetch_uri_async, so they hold their own token lifetime rather than a dangling const-ref).
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
              // On the URLSession queue: snapshot the bytes (__block so they can be MOVED into the
              // main-queue block), then marshal the sink onto the main queue.
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
        as_image_view(native).alpha = value;
    }

    void image_platform::update_is_enabled(bool value)
    {
        // ViewExtensions.UpdateIsEnabled's non-UIControl branch: a UIImageView only gets the
        // interaction toggle.
        as_image_view(native).userInteractionEnabled = static_cast<BOOL>(value);
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
        // CreatePlatformView returns a MauiImageView whose only addition is the window-change reload
        // channel (deferred with the resolution-reload machinery) — a plain UIImageView carries the cut.
        UIImageView* const view = [[UIImageView alloc] initWithFrame:CGRectZero];
        platform->native = (__bridge_retained void*)view; // the void* slot owns one reference
        return platform;
    }

    // iOS loader wiring: the real NSURLSession async uri fetch + the NSCachesDirectory on-disk cache
    // (UriImageSourceService DownloadAndCacheImageAsync). No i_dispatcher is set: fetch_uri_async
    // already hops its completion onto the MAIN queue, so the loader's apply runs inline on the main
    // thread (where the UIImageView lives) — the main-queue hop IS the dispatcher hand-off.
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
        // ImageViewExtensions.UpdateAspect: the content mode plus the clipping rule — an up-scaled
        // ScaleAspectFill (and an oversized Center) clips to the view bounds.
        UIImageView* const image_view = as_image_view(platform->native);
        const UIViewContentMode mode = to_ui_view_content_mode(view.aspect());
        image_view.contentMode = mode;
        image_view.clipsToBounds = mode == UIViewContentModeScaleAspectFill || mode == UIViewContentModeCenter;
    }

    // IsOpaque → UIView.opaque, the renderer's may-skip-blending hint (the UIKit face of the same layer
    // flag the AppKit twin sets via layer.opaque). The mirror records the last-pushed flag so the source
    // applies can re-assert it after an AnimationImages write resets UIKit's computed opaqueness.
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->opaque = view.is_opaque();
            as_image_view(platform->native).opaque = static_cast<BOOL>(view.is_opaque());
        }
    }

    // IsAnimationPlaying → StartAnimating/StopAnimating over the AnimationImages frame array the
    // decoded animated UIImage supplied (ImageViewExtensions.UpdateIsAnimationPlaying). The mirror
    // records the last-pushed flag so the file fast-path's apply can re-assert it (the async path
    // re-runs this map from the cross-platform map_source instead).
    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->animation_playing = view.is_animation_playing();
        apply_is_animation_playing(as_image_view(platform->native), view.is_animation_playing());
    }

    // ---- per-backend source primitives (the cross-platform map_source in image_handler.cpp routes here) ----

    // File fast-path: decode the path synchronously (animated-capable) straight into the view (nil on a
    // failed load → cleared).
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        set_image_on_view(as_image_view(platform.native), load_image_from_file(file_src.file()), platform);
    }

    // The async loader's apply: set the view to the decoded UIImage the result carries (the result
    // retains it; the UIImageView takes its own retain). A !loaded() result clears the view.
    void image_handler::apply_loaded_result(image_platform& platform, const image_source_result& result)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        set_image_on_view(as_image_view(platform.native), result.loaded() ? (__bridge UIImage*)result.image() : nil,
                          platform);
    }

    // Clear the loaded image (ImageViewExtensions.Clear: stop the animation, drop the frames, then the
    // still image). The AnimationImages write resets UIKit's computed opaqueness, so the last-pushed
    // IsOpaque mirror is re-asserted (see set_image_on_view).
    void image_handler::clear_source_native(image_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        UIImageView* const image_view = as_image_view(platform.native);
        [image_view stopAnimating];
        image_view.animationImages = nil;
        image_view.image = nil;
        image_view.opaque = static_cast<BOOL>(platform.opaque);
    }

    maui::graphics::size image_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform
        // maximum, then the native view measures itself (UIImageView's SizeThatFits reports the image's
        // dimensions; the constraint-aware SizeThatFitsImage refinement is layout-layer work).
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_image_view(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_image_view(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core

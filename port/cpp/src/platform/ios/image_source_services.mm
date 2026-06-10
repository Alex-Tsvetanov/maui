// Per-source image services + decode_image_bytes — iOS (UIKit) backend. The UIImage twin of
// src/platform/apple/image_source_services.mm (whose loads produce NSImage): each load produces a
// UIImage, retained into the image_source_result (with a CFRelease disposer — RAII, the loader drops the
// previous result on the next load). Compiled as Objective-C++ with ARC for the `ios` backend.
//
// Unlike AppKit's NSImage — which keeps every GIF frame inside its bitmap rep and animates natively —
// UIKit's [UIImage imageWithData:] decodes only the FIRST frame. Every decode therefore goes through
// ImageIO (CGImageSource), exactly like the C# original:
//
// decode — ImageSourceExtensions.iOS.cs GetPlatformImage(CGImageSource, scale): a single-frame source
//          decodes CGImageSourceCreateImageAtIndex(0, ShouldCache=false) into a UIImage carrying the
//          EXIF orientation; a multi-frame source (an animated GIF) expands its frames by the GCD of
//          the per-frame delays into [UIImage animatedImageWithImages:duration:] — the port of
//          ImageAnimationHelper.cs (FFImageLoading's GifDecoder lineage).
// file   — FileImageSourceService.iOS.cs GetImageAsync: probe the @2x/@3x sibling for the current
//          screen scale (ImageSourceExtensions.GetScaledFile → ios_image_ops.hpp), decode through
//          CGImageSource at the loaded scale, then fall back to [UIImage imageWithContentsOfFile:]
//          (C#'s `?? imageSource.GetPlatformImage()`). DEVIATION: no app-bundle probe (UIImage.FromBundle
//          / PlatformGetFullAppPackageFilePath) — the port's process has no MauiImage bundle, so paths
//          are used as given (the apple twin documents the same reduction).
// stream — StreamImageSourceService.iOS.cs: the source's bytes → NSData → the CGImageSource decode.
// uri    — UriImageSourceService.iOS.cs: file:// bytes are read cross-platform (read_uri_bytes) then
//          decoded; http(s) is fetched via NSData(contentsOfURL:) (synchronous, mirroring the apple
//          twin's standalone-service cut). The image control's REAL uri path instead runs through the
//          loader's disk/TTL cache + its injectable async fetch — this service stays registered for
//          resolution/DI and is the non-cached equivalent.
// font   — FontImageSourceService.iOS.cs GetImageAsync + ImageSourceExtensions.GetPlatformImage
//          (IFontImageSource): the glyph is drawn (an NSAttributedString in the source's font + color,
//          centered on its bounding rect) through a UIGraphicsImageRenderer. The result is
//          RESOLUTION-DEPENDENT (C# passes true). DEVIATIONS: no IFontManager — the source's font value
//          maps via ios_conversions (an unknown family → the system font; the empty-size default is
//          UIFont.systemFontSize, C# FontManager.DefaultFontSize); the port's color is a concrete value,
//          so C#'s null-color branch (Colors.White + UIImageRenderingModeAutomatic) is unreachable and
//          the glyph keeps AlwaysOriginal; the renderer keeps its default scale (the main screen's —
//          the value C#'s handler passes for `scale` in practice).
//
// HANDOFF (image_handler.mm, owned by its own M6 fan-out unit): apply_loaded_result sets
// UIImageView.image to the (possibly animated) UIImage carried by the result; map_is_animation_playing
// calls startAnimating/stopAnimating (ImageViewExtensions.UpdateIsAnimationPlaying); configure_loader
// installs the async NSURLSession uri fetch + the NSCachesDirectory disk-cache root (the apple
// image_handler.mm is the template for all three).

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>
#import <UIKit/UIKit.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ios_conversions.hpp"
#include "ios_image_ops.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_bytes.hpp"
#include "maui/core/uri_image_source_service.hpp"

namespace
{
    using maui::platform::ios::cf_ref;

    // Wrap a (non-nil) UIImage into a loaded result: the void* slot retains one reference; the disposer
    // CFReleases it when the result is dropped. A nil image yields a !loaded() result. resolution_dependent
    // mirrors C# IImageSourceServiceResult.IsResolutionDependent (true for the font service).
    maui::core::image_source_result make_result(UIImage* image, std::string kind, std::string detail,
                                                bool resolution_dependent = false)
    {
        if (image == nil)
        {
            return {};
        }
        void* const retained = (__bridge_retained void*)image; // result owns one reference
        return maui::core::image_source_result{retained, [retained] { CFRelease(retained); }, std::move(kind),
                                               std::move(detail), resolution_dependent};
    }

    NSData* to_ns_data(const maui::core::image_bytes& bytes)
    {
        if (bytes.empty())
        {
            return nil;
        }
        return [NSData dataWithBytes:bytes.data() length:static_cast<NSUInteger>(bytes.size())];
    }

    // EXIF orientation of frame 0 → UIImageOrientation. Ports ImageSourceExtensions.ToUIImageOrientation
    // (the CIImageOrientation switch — the EXIF codes 1..8). A missing property is Up; an out-of-range
    // value (impossible from ImageIO; C# throws) also falls to Up — the port's services report failures
    // as !loaded() results rather than exceptions.
    UIImageOrientation to_ui_image_orientation(CGImageSourceRef source)
    {
        const cf_ref<CFDictionaryRef> props{CGImageSourceCopyPropertiesAtIndex(source, 0, nullptr)};
        if (!props)
        {
            return UIImageOrientationUp;
        }
        auto* const dict = (__bridge NSDictionary*)props.get();
        NSNumber* const value = dict[(__bridge NSString*)kCGImagePropertyOrientation];
        if (![value isKindOfClass:[NSNumber class]])
        {
            return UIImageOrientationUp;
        }
        switch (static_cast<CGImagePropertyOrientation>(value.unsignedIntValue))
        {
            case kCGImagePropertyOrientationUp:
                return UIImageOrientationUp;
            case kCGImagePropertyOrientationUpMirrored:
                return UIImageOrientationUpMirrored;
            case kCGImagePropertyOrientationDown:
                return UIImageOrientationDown;
            case kCGImagePropertyOrientationDownMirrored:
                return UIImageOrientationDownMirrored;
            case kCGImagePropertyOrientationLeftMirrored:
                return UIImageOrientationLeftMirrored;
            case kCGImagePropertyOrientationRight:
                return UIImageOrientationRight;
            case kCGImagePropertyOrientationRightMirrored:
                return UIImageOrientationRightMirrored;
            case kCGImagePropertyOrientationLeft:
                return UIImageOrientationLeft;
        }
        return UIImageOrientationUp;
    }

    // Frame `index`'s GIF delay in centiseconds. Ports ImageAnimationHelper.AddFrameData: prefer
    // GIFUnclampedDelayTime, else GIFDelayTime, else 0.1s; then the "frame delay compatibility
    // adjustment" (<= 0.02s → 0.1s; GIF stores only centisecond resolution).
    int frame_delay_centiseconds(CGImageSourceRef source, std::size_t index)
    {
        double delay = 0.1;
        const cf_ref<CFDictionaryRef> props{CGImageSourceCopyPropertiesAtIndex(source, index, nullptr)};
        if (props)
        {
            auto* const dict = (__bridge NSDictionary*)props.get();
            NSDictionary* const gif = dict[(__bridge NSString*)kCGImagePropertyGIFDictionary];
            NSNumber* const unclamped = gif[(__bridge NSString*)kCGImagePropertyGIFUnclampedDelayTime];
            NSNumber* const clamped = gif[(__bridge NSString*)kCGImagePropertyGIFDelayTime];
            if ([unclamped isKindOfClass:[NSNumber class]])
            {
                delay = unclamped.doubleValue;
            }
            else if ([clamped isKindOfClass:[NSNumber class]])
            {
                delay = clamped.doubleValue;
            }
        }
        if (delay <= 0.02)
        {
            delay = 0.1;
        }
        return static_cast<int>(delay * 100.0);
    }

    // Ports ImageDataHelper.GetGreatestCommonDenominator + CheckPair — INCLUDING the 0/1 short-circuit
    // (a 0- or 1-centisecond delay adopts the other operand instead of collapsing the gcd to 1, an
    // FFImageLoading compatibility behavior kept as-is). The clamp above guarantees every delay >= 2,
    // so the result is always >= 1 (no division-by-zero downstream).
    int check_pair(int a, int b)
    {
        if (a == 0 || a == 1)
        {
            return b;
        }
        if (b == 0 || b == 1)
        {
            return a;
        }
        while (true)
        {
            const int r = a % b;
            if (r == 0)
            {
                return b;
            }
            a = b;
            b = r;
        }
    }

    int delays_gcd(const std::vector<int>& delays)
    {
        int gcd = delays.front();
        for (std::size_t i = 1; i < delays.size() && gcd != 1; ++i)
        {
            gcd = check_pair(delays[i], gcd);
        }
        return gcd;
    }

    // Multi-frame (animated GIF) decode. Ports ImageAnimationHelper.Create + ImageDataHelper.ToUIImage /
    // ToConsistentImageArray: one UIImage per SOURCE frame (UIImage.FromImage(keyFrame, scale, Up)),
    // repeated delay/gcd times so the single UIImage.duration plays the GIF's varying per-frame delays
    // at the right ratios; total duration = the delay sum in seconds. Returns nil when any frame fails
    // to decode (C# throws; the port reports !loaded()). REDUCTION (documented): C#'s Create also reads
    // the GIF loop count (GIFLoopCount) into a local it never uses — that dead read is not ported.
    UIImage* animated_image_from_source(CGImageSourceRef source, int scale)
    {
        const std::size_t count = CGImageSourceGetCount(source);
        NSMutableArray<UIImage*>* const key_frames = [NSMutableArray arrayWithCapacity:count];
        std::vector<int> delays;
        delays.reserve(count);
        int total = 0;
        for (std::size_t i = 0; i < count; ++i)
        {
            const cf_ref<CGImageRef> cg_image{CGImageSourceCreateImageAtIndex(source, i, nullptr)};
            if (!cg_image)
            {
                return nil; // C# AddFrameData throws "did not contain an image at index"
            }
            const int centiseconds = frame_delay_centiseconds(source, i);
            delays.push_back(centiseconds);
            total += centiseconds;
            [key_frames addObject:[UIImage imageWithCGImage:cg_image.get()
                                                      scale:static_cast<CGFloat>(scale)
                                                orientation:UIImageOrientationUp]];
        }

        const int gcd = delays_gcd(delays);
        NSMutableArray<UIImage*>* const frames =
            [NSMutableArray arrayWithCapacity:static_cast<NSUInteger>(total / gcd)];
        for (std::size_t i = 0; i < count; ++i)
        {
            for (int repeats = delays[i] / gcd; repeats > 0; --repeats)
            {
                [frames addObject:key_frames[i]]; // the SAME frame object, repeated (C# reuses `frame`)
            }
        }
        if (frames.count == 0)
        {
            return nil; // C# ToUIImage: frames.Length == 0 → null
        }
        return [UIImage animatedImageWithImages:frames duration:(total / 100.0)];
    }

    // The single decode pipeline behind every service. Ports ImageSourceExtensions.GetPlatformImage
    // (CGImageSource, scale): no frames → nil (C# throws "does not contain any images"); multi-frame →
    // the animated decode above (ImageAnimationHelper.IsAnimated == ImageCount > 1); single frame →
    // CGImageSourceCreateImageAtIndex(0, ShouldCache=false) wrapped at `scale` with the EXIF orientation.
    UIImage* image_from_cg_source(CGImageSourceRef source, int scale)
    {
        const std::size_t count = CGImageSourceGetCount(source);
        if (count == 0)
        {
            return nil;
        }
        if (count > 1)
        {
            return animated_image_from_source(source, scale);
        }
        NSDictionary* const options = @{(__bridge NSString*)kCGImageSourceShouldCache : @NO};
        const cf_ref<CGImageRef> cg_image{
            CGImageSourceCreateImageAtIndex(source, 0, (__bridge CFDictionaryRef)options)};
        if (!cg_image)
        {
            return nil;
        }
        return [[UIImage alloc] initWithCGImage:cg_image.get()
                                          scale:static_cast<CGFloat>(scale)
                                    orientation:to_ui_image_orientation(source)];
    }

    // Bytes → CGImageSource → UIImage at `scale` (ImageSourceExtensions.GetPlatformImageSource(NSData) +
    // GetPlatformImage). Undecodable bytes yield nil.
    UIImage* image_from_data(NSData* data, int scale)
    {
        const cf_ref<CGImageSourceRef> source{CGImageSourceCreateWithData((__bridge CFDataRef)data, nullptr)};
        if (!source)
        {
            return nil;
        }
        return image_from_cg_source(source.get(), scale);
    }

    // The current display scale. C# GetScaledFile reads UIScreen.MainScreen.Scale; the iOS 26 SDK
    // deprecates mainScreen in favor of "a traitCollection found through context" — and in a process
    // without scenes UIKit seeds the current trait collection with the main screen's traits, so
    // displayScale carries the same value (verified on-simulator). An unspecified displayScale (0)
    // falls back to 1, C#'s "no scale" path.
    int screen_scale()
    {
        const CGFloat scale = UITraitCollection.currentTraitCollection.displayScale;
        return scale >= 1 ? static_cast<int>(scale) : 1;
    }

    // File load (FileImageSourceService.iOS GetImageAsync): the @2x/@3x probe for the CURRENT screen
    // scale, the CGImageSource decode at the loaded scale — so an @2x asset reports half its pixel size
    // in points, the behavior the loader's set_scale/requires_reload seam expects — then the
    // imageWithContentsOfFile fallback.
    UIImage* image_from_file(std::string_view path)
    {
        const maui::platform::ios::scaled_file scaled = maui::platform::ios::get_scaled_file(path, screen_scale());
        NSString* const scaled_path = [NSString stringWithUTF8String:scaled.path.c_str()];
        if (scaled_path != nil)
        {
            NSURL* const url = [NSURL fileURLWithPath:scaled_path];
            const cf_ref<CGImageSourceRef> source{CGImageSourceCreateWithURL((__bridge CFURLRef)url, nullptr)};
            if (source)
            {
                UIImage* const image = image_from_cg_source(source.get(), scaled.scale);
                if (image != nil)
                {
                    return image;
                }
            }
        }
        // C#'s `?? imageSource.GetPlatformImage()` (FromBundle ?? FromFile) — the direct file load
        // stands in for both (no app bundle in the port's process; see the header DEVIATION note).
        const std::string file(path);
        NSString* const original = [NSString stringWithUTF8String:file.c_str()];
        return original != nil ? [UIImage imageWithContentsOfFile:original] : nil;
    }

    // Draw `glyph` in `font`/`color` into a UIImage sized to the glyph. Ports ImageSourceExtensions
    // .GetPlatformImage(IFontImageSource, ...): IsNullOrWhiteSpace → nil; sizeWithAttributes for the
    // canvas; a UIGraphicsImageRenderer (opaque NO) draws the attributed glyph centered on its bounding
    // rect; the rendered image keeps AlwaysOriginal (the port's color is always concrete — see header).
    UIImage* image_from_glyph(std::string_view glyph, UIFont* font, UIColor* color)
    {
        const std::string text(glyph);
        NSString* const ns_glyph = [NSString stringWithUTF8String:text.c_str()];
        if (ns_glyph == nil || font == nil ||
            [ns_glyph stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet].length == 0)
        {
            return nil; // C# string.IsNullOrWhiteSpace(imageSource.Glyph) → null
        }
        NSDictionary<NSAttributedStringKey, id>* const attrs = @{
            NSFontAttributeName : font,
            NSForegroundColorAttributeName : color,
        };
        const CGSize size = [ns_glyph sizeWithAttributes:attrs];
        if (size.width <= 0 || size.height <= 0)
        {
            return nil;
        }
        NSAttributedString* const attributed = [[NSAttributedString alloc] initWithString:ns_glyph attributes:attrs];

        UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
        format.opaque = NO; // C# UIGraphicsImageRendererFormat { Opaque = false, Scale = scale }
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:size format:format];
        UIImage* const image = [renderer imageWithActions:^(UIGraphicsImageRendererContext* /*context*/) {
          NSStringDrawingContext* const drawing = [[NSStringDrawingContext alloc] init];
          const CGRect bounding = [attributed boundingRectWithSize:size
                                                           options:static_cast<NSStringDrawingOptions>(0)
                                                           context:drawing];
          [attributed drawInRect:CGRectMake((size.width / 2) - (bounding.size.width / 2),
                                            (size.height / 2) - (bounding.size.height / 2), size.width, size.height)];
        }];
        return [image imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    }
} // namespace

namespace maui::core
{
    // iOS decode: bytes → NSData → CGImageSource → UIImage (animated-aware), retained into the result
    // (nil image → !loaded()). Shared by the stream/uri services and the loader's cached-uri path.
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        NSData* const data = to_ns_data(bytes);
        if (data == nil)
        {
            return {};
        }
        return make_result(image_from_data(data, /*scale*/ 1), std::move(kind), std::move(detail));
    }

    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        on_result(make_result(image_from_file(file_src->file()), "file", std::string(file_src->file())));
    }

    void uri_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                        completion on_result)
    {
        auto* uri_src = dynamic_cast<i_uri_image_source*>(&source);
        if (uri_src == nullptr || uri_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const std::string uri(uri_src->uri());

        // Local files go through the cross-platform reader (so `file://` matches the loader's fast-path);
        // an http(s) URI is fetched synchronously via NSData(contentsOfURL:) this cut (the loader's real
        // uri path uses its injectable ASYNC fetch instead — see the header note).
        image_bytes bytes = read_uri_bytes(uri);
        if (bytes.empty() && (uri.starts_with("http://") || uri.starts_with("https://")))
        {
            NSString* const ns_uri = [NSString stringWithUTF8String:uri.c_str()];
            NSURL* const url = ns_uri != nil ? [NSURL URLWithString:ns_uri] : nil;
            NSData* const data = url != nil ? [NSData dataWithContentsOfURL:url] : nil;
            if (data != nil && data.length > 0)
            {
                bytes.resize(static_cast<std::size_t>(data.length));
                std::memcpy(bytes.data(), data.bytes, static_cast<std::size_t>(data.length));
            }
        }
        on_result(decode_image_bytes(bytes, "uri", uri));
    }

    void stream_image_source_service::load(i_image_source& source, const cancellation_token& token,
                                           completion on_result)
    {
        auto* stream_src = dynamic_cast<i_stream_image_source*>(&source);
        if (stream_src == nullptr || stream_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const image_bytes bytes = stream_src->get_bytes(token);
        on_result(decode_image_bytes(bytes, "stream", "<bytes:" + std::to_string(bytes.size()) + ">"));
    }

    void font_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* font_src = dynamic_cast<const i_font_image_source*>(&source);
        if (font_src == nullptr || font_src->is_empty())
        {
            on_result(image_source_result{}); // not a font source / empty glyph → nothing rendered
            return;
        }
        // FontManager.GetFont + (Color ?? White).ToPlatform(): the port maps the source's font/color
        // directly (DEVIATIONS in the header). The empty-size fallback is UIFont.systemFontSize —
        // C# FontManager.DefaultFontSize on iOS.
        UIFont* const ui_font =
            maui::platform::ios::to_ui_font(font_src->font(), static_cast<double>(UIFont.systemFontSize));
        UIColor* const ui_color = maui::platform::ios::to_ui_color(font_src->color());
        UIImage* const image = image_from_glyph(font_src->glyph(), ui_font, ui_color);
        // Font results are RESOLUTION-DEPENDENT (the rasterized glyph depends on display density —
        // C# new ImageSourceServiceResult(image, true, ...)).
        on_result(make_result(image, "font", std::string(font_src->glyph()), /*resolution_dependent*/ true));
    }
} // namespace maui::core

// iOS (UIKit) backend tests for the image-source services — run ON the iOS simulator (MAUI_BACKEND=ios via
// tools/ios-sim-run.sh). The decode seam is the real UIImage pipeline (image_source_services.mm: ImageIO /
// CGImageSource, the @2x probe, the UIGraphicsImageRenderer glyph rasterize), while the image HANDLER is
// still the headless partial on ios (its UIKit twin is a separate M6 unit), so the control-level cases
// assert through the headless image_platform mirror exactly like the headless suite — but with REAL
// decodable bytes (a generated PNG/GIF), because the ios decode_image_bytes only "loads" what UIImage can
// actually decode. Replaces tests/core/image_source_tests.cpp + font_image_source_tests.cpp +
// tests/controls/image_tests.cpp on ios (those assert the headless any-bytes-decode mirror semantics).
// Compiled as Objective-C++ with ARC.
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>
#import <UIKit/UIKit.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <system_error>
#include <utility>

#include "ios_image_ops.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/font_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::cancellation_token;
    using maui::core::image_bytes;
    using maui::core::image_handler;
    using maui::core::image_source_result;
    using maui::core::manual_dispatcher;
    using maui::platform::ios::cf_ref;
    using maui::platform::ios::get_scaled_file;
    using maui::platform::ios::scaled_file;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    image_bytes to_bytes(NSData* data)
    {
        if (data == nil || data.length == 0)
        {
            return {};
        }
        const std::span<const std::byte> raw{static_cast<const std::byte*>(data.bytes), data.length};
        return {raw.begin(), raw.end()};
    }

    // A solid-fill UIImage of `pixels` x `pixels` (renderer scale 1, so points == pixels).
    UIImage* make_solid_image(CGFloat pixels, UIColor* color)
    {
        UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
        format.scale = 1;
        UIGraphicsImageRenderer* const renderer =
            [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(pixels, pixels) format:format];
        return [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
          [color setFill];
          [context fillRect:CGRectMake(0, 0, pixels, pixels)];
        }];
    }

    // Encoded PNG bytes for a tiny `pixels` x `pixels` image (no checked-in fixture). Empty on failure.
    image_bytes make_png_bytes(CGFloat pixels = 2)
    {
        return to_bytes(UIImagePNGRepresentation(make_solid_image(pixels, UIColor.redColor)));
    }

    // Encoded bytes of a 2-frame animated GIF (two 2x2 frames, 0.1s delay each) packed via ImageIO's
    // CGImageDestination — the encoder twin of the CGImageSource decode under test. Empty on failure.
    image_bytes make_animated_gif_bytes()
    {
        UIImage* const frame0 = make_solid_image(2, UIColor.redColor);
        UIImage* const frame1 = make_solid_image(2, UIColor.greenColor);
        if (frame0.CGImage == nil || frame1.CGImage == nil)
        {
            return {};
        }
        NSMutableData* const data = [NSMutableData data];
        const cf_ref<CGImageDestinationRef> destination{
            CGImageDestinationCreateWithData((__bridge CFMutableDataRef)data, CFSTR("com.compuserve.gif"), 2, nullptr)};
        if (!destination)
        {
            return {};
        }
        NSDictionary* const frame_props = @{
            (__bridge NSString*)
            kCGImagePropertyGIFDictionary : @{(__bridge NSString*)kCGImagePropertyGIFDelayTime : @0.1}
        };
        CGImageDestinationAddImage(destination.get(), frame0.CGImage, (__bridge CFDictionaryRef)frame_props);
        CGImageDestinationAddImage(destination.get(), frame1.CGImage, (__bridge CFDictionaryRef)frame_props);
        if (!CGImageDestinationFinalize(destination.get()))
        {
            return {};
        }
        return to_bytes(data);
    }

    // A unique path under the simulator's writable temp dir (the spawned test process may not write
    // elsewhere): "<NSTemporaryDirectory()>/<stem>-<uuid><ext>".
    std::string temp_path(NSString* stem, NSString* ext)
    {
        NSString* const name = [NSString stringWithFormat:@"%@-%@%@", stem, [[NSUUID UUID] UUIDString], ext];
        return to_std_string([NSTemporaryDirectory() stringByAppendingPathComponent:name]);
    }

    bool write_bytes(const std::string& path, const image_bytes& bytes)
    {
        NSData* const data = [NSData dataWithBytes:bytes.data() length:static_cast<NSUInteger>(bytes.size())];
        return [data writeToFile:@(path.c_str()) atomically:YES] != NO;
    }

    void remove_file(const std::string& path)
    {
        if (!path.empty())
        {
            [[NSFileManager defaultManager] removeItemAtPath:@(path.c_str()) error:nil];
        }
    }

    // Run one service load synchronously (every built-in service invokes its completion before returning).
    image_source_result load_via(maui::core::i_image_source_service& service, maui::core::i_image_source& source)
    {
        image_source_result out;
        service.load(source, cancellation_token{}, [&out](image_source_result result) { out = std::move(result); });
        return out;
    }

    UIImage* result_image(const image_source_result& result)
    {
        return (__bridge UIImage*)result.image();
    }

    // ---- decode_image_bytes (the per-backend decode primitive: PNG / GIF / garbage) ----

    TEST(ios_image_sources, decode_png_bytes_produces_uiimage)
    {
        const image_bytes png = make_png_bytes();
        ASSERT_FALSE(png.empty());

        const image_source_result result = maui::core::decode_image_bytes(png, "stream", "<bytes:n>");
        ASSERT_TRUE(result.loaded());
        EXPECT_EQ(result.kind(), "stream");
        EXPECT_FALSE(result.is_resolution_dependent());

        UIImage* const image = result_image(result);
        ASSERT_NE(image, nil);
        EXPECT_DOUBLE_EQ(image.size.width, 2.0);
        EXPECT_DOUBLE_EQ(image.size.height, 2.0);
        EXPECT_EQ(image.images, nil); // a single-frame decode is a still, not an animated image
    }

    TEST(ios_image_sources, undecodable_or_empty_bytes_yield_unloaded_result)
    {
        const image_bytes garbage{std::byte{0x12}, std::byte{0x34}, std::byte{0x56}, std::byte{0x78}};
        EXPECT_FALSE(maui::core::decode_image_bytes(garbage, "stream", "x").loaded());
        EXPECT_FALSE(maui::core::decode_image_bytes(image_bytes{}, "stream", "x").loaded());
    }

    // The multi-frame GIF decode: ImageIO expands the frames into UIImage.images with the summed
    // duration (2 frames x 0.1s → 10cs + 10cs, gcd 10 → 2 frames, 0.2s) — the iOS analog of AppKit's
    // natively-animating multi-frame NSImage (ImageAnimationHelper.Create).
    TEST(ios_image_sources, animated_gif_decodes_to_animated_uiimage)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        const image_source_result result = maui::core::decode_image_bytes(gif, "stream", "gif");
        ASSERT_TRUE(result.loaded());
        UIImage* const image = result_image(result);
        ASSERT_NE(image, nil);
        ASSERT_NE(image.images, nil);
        EXPECT_EQ(image.images.count, 2U);
        EXPECT_DOUBLE_EQ(image.duration, 0.2);
    }

    // ---- file service (the @2x/@3x probe + the CGImageSource decode + the missing-file fallback) ----

    TEST(ios_image_sources, file_service_loads_png_file_into_uiimage)
    {
        const std::string path = temp_path(@"maui_ios_file", @".png");
        ASSERT_TRUE(write_bytes(path, make_png_bytes()));

        maui::core::file_image_source_service service;
        maui::controls::file_image_source source{path};
        const image_source_result result = load_via(service, source);

        ASSERT_TRUE(result.loaded());
        EXPECT_EQ(result.kind(), "file");
        EXPECT_EQ(result.detail(), path);
        UIImage* const image = result_image(result);
        ASSERT_NE(image, nil);
        EXPECT_GT(image.size.width, 0.0);

        remove_file(path);
    }

    TEST(ios_image_sources, file_service_missing_file_yields_unloaded_result)
    {
        maui::core::file_image_source_service service;
        maui::controls::file_image_source source{"/tmp/does-not-exist-maui-ios.png"};
        EXPECT_FALSE(load_via(service, source).loaded());
    }

    // The pure @2x/@3x probe (ImageSourceExtensions.GetScaledFile): highest available scale <= the
    // screen scale wins; scale 1 screens (or no scaled sibling) keep the original at scale 1.
    TEST(ios_image_sources, scaled_file_probe_matches_csharp_get_scaled_file)
    {
        const std::string base = temp_path(@"maui_ios_scaled", @".png");
        const std::filesystem::path base_path{base};
        const std::string at2x =
            (base_path.parent_path() / (base_path.stem().string() + "@2x" + base_path.extension().string())).string();
        const std::string at3x =
            (base_path.parent_path() / (base_path.stem().string() + "@3x" + base_path.extension().string())).string();

        ASSERT_TRUE(write_bytes(base, make_png_bytes(2)));

        // No scaled sibling on disk → the original at scale 1, whatever the screen scale.
        scaled_file probed = get_scaled_file(base, 3);
        EXPECT_EQ(probed.path, base);
        EXPECT_EQ(probed.scale, 1);

        // @2x present: a 1x screen keeps the original; 2x and 3x screens pick the @2x (3x probes the
        // missing @3x first, then falls to @2x — C#'s descending loop).
        ASSERT_TRUE(write_bytes(at2x, make_png_bytes(4)));
        EXPECT_EQ(get_scaled_file(base, 1).scale, 1);
        probed = get_scaled_file(base, 2);
        EXPECT_EQ(probed.path, at2x);
        EXPECT_EQ(probed.scale, 2);
        probed = get_scaled_file(base, 3);
        EXPECT_EQ(probed.path, at2x);
        EXPECT_EQ(probed.scale, 2);

        // @3x also present: the 3x screen now prefers it; the 2x screen still takes @2x.
        ASSERT_TRUE(write_bytes(at3x, make_png_bytes(6)));
        probed = get_scaled_file(base, 3);
        EXPECT_EQ(probed.path, at3x);
        EXPECT_EQ(probed.scale, 3);
        EXPECT_EQ(get_scaled_file(base, 2).path, at2x);

        remove_file(base);
        remove_file(at2x);
        remove_file(at3x);
    }

    // Integration: on a retina simulator the file service decodes the @2x sibling at scale 2 (the
    // UIImage reports half its pixel size in points). Skipped on a 1x screen, where the probe is a no-op.
    TEST(ios_image_sources, file_service_prefers_scaled_sibling_on_retina_screen)
    {
        // The services read the scale off the current trait collection (image_source_services.mm
        // screen_scale — the iOS 26 replacement for the deprecated UIScreen.mainScreen).
        if (static_cast<int>(UITraitCollection.currentTraitCollection.displayScale) < 2)
        {
            GTEST_SKIP() << "the simulator's screen is 1x; the @2x probe never fires";
        }

        const std::string base = temp_path(@"maui_ios_retina", @".png");
        const std::filesystem::path base_path{base};
        const std::string at2x =
            (base_path.parent_path() / (base_path.stem().string() + "@2x" + base_path.extension().string())).string();
        ASSERT_TRUE(write_bytes(base, make_png_bytes(2)));
        ASSERT_TRUE(write_bytes(at2x, make_png_bytes(4))); // 4x4 pixels at @2x → 2x2 points

        maui::core::file_image_source_service service;
        maui::controls::file_image_source source{base};
        const image_source_result result = load_via(service, source);

        ASSERT_TRUE(result.loaded());
        UIImage* const image = result_image(result);
        ASSERT_NE(image, nil);
        EXPECT_DOUBLE_EQ(image.scale, 2.0);
        EXPECT_DOUBLE_EQ(image.size.width, 2.0); // 4 px / scale 2 = 2 points
        EXPECT_DOUBLE_EQ(image.size.height, 2.0);

        remove_file(base);
        remove_file(at2x);
    }

    // ---- font service (the UIGraphicsImageRenderer glyph rasterize) ----

    TEST(ios_image_sources, font_glyph_renders_nonempty_resolution_dependent_uiimage)
    {
        maui::core::font_image_source_service service;
        maui::controls::font_image_source source{
            "A", maui::core::font::of_size("", maui::controls::font_image_source::default_size),
            maui::graphics::colors::red};
        const image_source_result result = load_via(service, source);

        ASSERT_TRUE(result.loaded());
        EXPECT_EQ(result.kind(), "font");
        EXPECT_EQ(result.detail(), "A");
        EXPECT_TRUE(result.is_resolution_dependent()); // C# ImageSourceServiceResult(image, true, ...)

        UIImage* const image = result_image(result);
        ASSERT_NE(image, nil);
        EXPECT_GT(image.size.width, 0.0);
        EXPECT_GT(image.size.height, 0.0);
        // The port always has a concrete color → AlwaysOriginal (C#'s color-specified branch).
        EXPECT_EQ(image.renderingMode, UIImageRenderingModeAlwaysOriginal);
    }

    // A whitespace-only glyph is non-empty (IsEmpty is false) but renders nothing —
    // C# string.IsNullOrWhiteSpace(Glyph) inside GetPlatformImage.
    TEST(ios_image_sources, whitespace_glyph_renders_nothing)
    {
        maui::core::font_image_source_service service;
        maui::controls::font_image_source source{" ", maui::core::font::of_size("", 30), maui::graphics::colors::red};
        ASSERT_FALSE(source.is_empty());
        EXPECT_FALSE(load_via(service, source).loaded());
    }

    // ---- the loader seam on ios (control → headless handler mirror, REAL decodable bytes) ----

    TEST(ios_image_sources, stream_source_loads_through_loader_with_dispatcher)
    {
        const image_bytes png = make_png_bytes();
        ASSERT_FALSE(png.empty());
        const std::string expected_detail = "<bytes:" + std::to_string(png.size()) + ">";

        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher dispatcher;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(dispatcher);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Init-capture + mutable + move: a noexcept one-shot provider (the test triggers one load).
        control.set_source(image_source::from_stream(
            [bytes = png](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        EXPECT_FALSE(platform->source_loaded); // marshalled — nothing applied until pumped
        EXPECT_TRUE(control.is_loading());

        dispatcher.run_pending();
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "stream");
        EXPECT_EQ(platform->source_file, expected_detail);
        EXPECT_FALSE(control.is_loading()); // the gated completion cleared it
    }

    TEST(ios_image_sources, superseded_stream_load_is_dropped_by_the_identity_recheck)
    {
        const image_bytes png = make_png_bytes();
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(png.empty());
        ASSERT_FALSE(gif.empty());
        const std::string expected_detail = "<bytes:" + std::to_string(gif.size()) + ">";

        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher dispatcher;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(dispatcher);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Both sources queue before the pump; the first load is cancelled + superseded by the second.
        control.set_source(image_source::from_stream(
            [bytes = png](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        control.set_source(image_source::from_stream(
            [bytes = gif](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        EXPECT_FALSE(platform->source_loaded);

        dispatcher.run_pending();
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, expected_detail); // only the second source applied
    }

    TEST(ios_image_sources, uri_file_source_loads_and_serves_repeats_from_the_memory_cache)
    {
        const std::string path = temp_path(@"maui_ios_uri", @".png");
        ASSERT_TRUE(write_bytes(path, make_png_bytes()));
        const std::string uri = "file://" + path;

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler); // no dispatcher: the loader's apply runs inline
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_uri(uri));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "uri");
        EXPECT_EQ(platform->source_file, uri);

        // Delete the file, then load the SAME uri again: the in-memory cache still has the bytes
        // (CacheValidity defaults to one day), so the decode still succeeds.
        remove_file(path);
        control.set_source(image_source::from_uri(uri));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "uri");
    }

    // The on-disk uri cache round-trip under the simulator's writable temp dir: a fetched payload
    // persists at disk_cache_path(uri), and a FRESH loader (empty in-memory cache) re-serves it from
    // disk after the original file is gone (UriImageSourceService.iOS DownloadAndCacheImageAsync).
    TEST(ios_image_sources, uri_disk_cache_round_trip_survives_source_deletion)
    {
        const std::string cache_base = temp_path(@"maui_ios_disk_cache", @"");
        const std::string path = temp_path(@"maui_ios_disk_uri", @".png");
        ASSERT_TRUE(write_bytes(path, make_png_bytes()));
        const std::string uri = "file://" + path;

        {
            image control;
            auto handler = std::make_shared<image_handler>();
            control.set_handler(handler);
            handler->source_loader().set_disk_cache_directory(cache_base);
            auto* platform = handler->typed_platform_view();
            ASSERT_NE(platform, nullptr);

            control.set_source(image_source::from_uri(uri));
            ASSERT_TRUE(platform->source_loaded);

            const std::string cached = handler->source_loader().disk_cache_path(uri);
            ASSERT_FALSE(cached.empty());
            EXPECT_TRUE(std::filesystem::exists(cached)); // C# CacheImage persisted the payload
        }

        // The source file disappears; a brand-new handler/loader (nothing in memory) pointed at the
        // same cache directory serves the uri from DISK.
        remove_file(path);
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        handler->source_loader().set_disk_cache_directory(cache_base);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_uri(uri));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "uri");

        std::error_code ec;
        std::filesystem::remove_all(cache_base, ec); // best-effort cleanup; never fails the test
    }
} // namespace

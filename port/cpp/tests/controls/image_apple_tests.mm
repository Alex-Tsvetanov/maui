// Apple (AppKit) backend tests for the image seam — the cross-platform aspect maps to a real NSImageView's
// imageScaling; a FILE source loads synchronously into the view's image; and uri/stream sources load
// through the handler-owned image_source_loader (here inline — no dispatcher injected — since the file://
// read + in-memory PNG decode are fast and synchronous). Compiled as Objective-C++ with ARC for `apple`.
#import <AppKit/AppKit.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::cancellation_token;
    using maui::core::image_bytes;
    using maui::core::image_handler;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSImageView* native_image_view(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge NSImageView*)handler->typed_platform_view()->native;
    }

    // Writes a tiny 2x2 PNG to a unique path under NSTemporaryDirectory(); returns the path (empty on
    // failure). The caller deletes it. Built via NSBitmapImageRep so the test owns no checked-in fixture.
    std::string write_temp_png()
    {
        NSBitmapImageRep* const rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nullptr
                                                                              pixelsWide:2
                                                                              pixelsHigh:2
                                                                           bitsPerSample:8
                                                                         samplesPerPixel:4
                                                                                hasAlpha:YES
                                                                                isPlanar:NO
                                                                          colorSpaceName:NSDeviceRGBColorSpace
                                                                             bytesPerRow:0
                                                                            bitsPerPixel:0];
        if (rep == nil)
        {
            return {};
        }
        NSData* const png = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        if (png == nil)
        {
            return {};
        }
        NSString* const name = [NSString stringWithFormat:@"maui_image_test_%@.png", [[NSUUID UUID] UUIDString]];
        NSString* const path = [NSTemporaryDirectory() stringByAppendingPathComponent:name];
        if (![png writeToFile:path atomically:YES])
        {
            return {};
        }
        return to_std_string(path);
    }

    void remove_file(const std::string& path)
    {
        if (!path.empty())
        {
            [[NSFileManager defaultManager] removeItemAtPath:@(path.c_str()) error:nil];
        }
    }

    // Builds a tiny 2x2 PNG in memory (no file) and returns its encoded bytes — the payload a stream
    // source yields. Empty on failure.
    image_bytes make_png_bytes()
    {
        NSBitmapImageRep* const rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nullptr
                                                                              pixelsWide:2
                                                                              pixelsHigh:2
                                                                           bitsPerSample:8
                                                                         samplesPerPixel:4
                                                                                hasAlpha:YES
                                                                                isPlanar:NO
                                                                          colorSpaceName:NSDeviceRGBColorSpace
                                                                             bytesPerRow:0
                                                                            bitsPerPixel:0];
        NSData* const png = rep != nil ? [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}] : nil;
        if (png == nil)
        {
            return {};
        }
        const auto* const raw = static_cast<const std::byte*>(png.bytes);
        return image_bytes(raw, raw + png.length);
    }

    class apple_image_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_image_seam, attaching_handler_creates_nsimageview)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_NE(native_image_view(handler), nil);
    }

    TEST_F(apple_image_seam, aspect_maps_to_image_scaling)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        // aspect_fit (the default) -> proportional up/down.
        EXPECT_EQ(view.imageScaling, NSImageScaleProportionallyUpOrDown);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(view.imageScaling, NSImageScaleAxesIndependently);

        control.set_aspect(aspect::center);
        EXPECT_EQ(view.imageScaling, NSImageScaleNone);

        control.set_aspect(aspect::aspect_fill);
        EXPECT_EQ(view.imageScaling, NSImageScaleProportionallyUpOrDown);
    }

    // ---- source (file source, synchronous NSImage load) ----

    TEST_F(apple_image_seam, file_source_loads_into_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil); // nothing loaded before a source is set

        control.set_source(image_source::from_file(path));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);

        remove_file(path);
    }

    TEST_F(apple_image_seam, empty_source_clears_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file(path));
        ASSERT_NE(view.image, nil);

        // An empty source clears the loaded image.
        control.set_source(image_source::from_file(""));
        EXPECT_EQ(view.image, nil);

        remove_file(path);
    }

    TEST_F(apple_image_seam, clearing_source_to_null_clears_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file(path));
        ASSERT_NE(view.image, nil);

        control.set_source(nullptr);
        EXPECT_EQ(view.image, nil);

        remove_file(path);
    }

    TEST_F(apple_image_seam, missing_file_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file("/tmp/does-not-exist-maui-image-test.png"));
        EXPECT_EQ(view.image, nil); // a failed load clears rather than crashes
    }

    // ---- uri + stream sources via the async loader (inline: no dispatcher injected) ----

    TEST_F(apple_image_seam, uri_file_source_loads_into_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // A file:// uri routes through the loader; the apply runs inline (no dispatcher set), reading the
        // bytes off disk and decoding them to an NSImage. Caching is disabled so the test does not persist
        // into the handler's real NSCachesDirectory disk cache (configure_loader wires it in production).
        control.set_source(image_source::from_uri("file://" + path, /*caching*/ false));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);

        remove_file(path);
    }

    TEST_F(apple_image_seam, stream_source_from_memory_loads_into_native_image)
    {
        const image_bytes png = make_png_bytes();
        ASSERT_FALSE(png.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // The stream provider yields the in-memory PNG bytes; the loader's apply (inline) decodes them.
        control.set_source(image_source::from_stream([png](const cancellation_token&) { return png; }));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);
    }

    TEST_F(apple_image_seam, empty_stream_source_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        // A provider that yields no bytes decodes to nothing → the view stays cleared.
        control.set_source(image_source::from_stream([](const cancellation_token&) { return image_bytes{}; }));
        EXPECT_EQ(view.image, nil);
    }

    // ---- native GIF playback (IsAnimationPlaying → NSImageView.animates over a multi-frame NSImage) ----

    // Encodes a 2-frame animated GIF in memory (two 2x2 frames) and returns its bytes. NSBitmapImageRep's
    // GIF encoder packs the frames; the per-frame property gives them a delay so AppKit treats the decoded
    // NSImage as animatable. Empty on failure.
    image_bytes make_animated_gif_bytes()
    {
        const auto make_frame = [](CGFloat r, CGFloat g, CGFloat b) -> NSBitmapImageRep* {
            NSBitmapImageRep* const rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nullptr
                                                                                  pixelsWide:2
                                                                                  pixelsHigh:2
                                                                               bitsPerSample:8
                                                                             samplesPerPixel:4
                                                                                    hasAlpha:YES
                                                                                    isPlanar:NO
                                                                              colorSpaceName:NSDeviceRGBColorSpace
                                                                                 bytesPerRow:0
                                                                                bitsPerPixel:0];
            if (rep == nil)
            {
                return nil;
            }
            // Fill the 2x2 frame with a solid color so the two frames differ.
            for (NSInteger y = 0; y < 2; ++y)
            {
                for (NSInteger x = 0; x < 2; ++x)
                {
                    std::array<NSUInteger, 4> px = {static_cast<NSUInteger>(r * 255), static_cast<NSUInteger>(g * 255),
                                                    static_cast<NSUInteger>(b * 255), 255};
                    [rep setPixel:px.data() atX:x y:y];
                }
            }
            return rep;
        };

        NSBitmapImageRep* const f0 = make_frame(1, 0, 0);
        NSBitmapImageRep* const f1 = make_frame(0, 1, 0);
        if (f0 == nil || f1 == nil)
        {
            return {};
        }
        // A per-frame GIF delay marks the frames as timed (so the decoded NSImage is animatable).
        NSDictionary* const props = @{NSImageCurrentFrameDuration : @0.1, NSImageLoopCount : @0};
        NSData* const gif = [NSBitmapImageRep representationOfImageRepsInArray:@[ f0, f1 ]
                                                                     usingType:NSBitmapImageFileTypeGIF
                                                                    properties:props];
        if (gif == nil || gif.length == 0)
        {
            return {};
        }
        image_bytes bytes(static_cast<std::size_t>(gif.length));
        std::memcpy(bytes.data(), gif.bytes, static_cast<std::size_t>(gif.length));
        return bytes;
    }

    TEST_F(apple_image_seam, animated_gif_decodes_to_multiple_frames)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_stream([gif](const cancellation_token&) { return gif; }));
        ASSERT_NE(view.image, nil);

        // The decoded NSImage holds >1 frame (the GIF bitmap rep reports NSImageFrameCount > 1) — i.e. a
        // real multi-frame image the native animator can cycle, not a single still.
        NSBitmapImageRep* const rep = static_cast<NSBitmapImageRep*>(view.image.representations.firstObject);
        ASSERT_TRUE([rep isKindOfClass:[NSBitmapImageRep class]]);
        NSNumber* const frame_count = [rep valueForProperty:NSImageFrameCount];
        EXPECT_GT(frame_count.integerValue, 1) << "the animated GIF should decode to multiple frames";
    }

    TEST_F(apple_image_seam, is_animation_playing_starts_and_stops_native_animation)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        // Default: not playing → animates is NO.
        EXPECT_FALSE(view.animates);

        // Load the animated GIF, then start playing: NSImageView.animates flips on (StartAnimating analog),
        // and AppKit cycles the multi-frame NSImage natively.
        control.set_source(image_source::from_stream([gif](const cancellation_token&) { return gif; }));
        ASSERT_NE(view.image, nil);

        control.set_is_animation_playing(true);
        EXPECT_TRUE(view.animates);

        // Stop: animates flips off (StopAnimating analog), freezing on the current frame.
        control.set_is_animation_playing(false);
        EXPECT_FALSE(view.animates);
    }

    TEST_F(apple_image_seam, animation_flag_set_before_load_plays_once_the_image_arrives)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        // IsAnimationPlaying set BEFORE a source exists: the loader re-applies the flag after the image is
        // decoded (map_source → map_is_animation_playing), so a freshly-loaded animated image plays.
        control.set_is_animation_playing(true);
        control.set_source(image_source::from_stream([gif](const cancellation_token&) { return gif; }));
        ASSERT_NE(view.image, nil);
        EXPECT_TRUE(view.animates);
    }
} // namespace

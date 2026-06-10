// iOS (UIKit) backend tests for the image seam — the cross-platform aspect maps to a real UIImageView's
// contentMode (+ the ClipsToBounds rule), a FILE source decodes synchronously into the view's image,
// uri/stream sources load through the handler-owned image_source_loader (inline — no dispatcher
// injected — since the file:// read + in-memory decode are fast and synchronous), and an animated GIF
// decodes to the AnimationImages frame array that IsAnimationPlaying starts/stops natively. Run only for
// MAUI_BACKEND=ios (executed ON the iOS simulator via tools/ios-sim-run.sh); mirrors the AppKit twin's
// coverage (image_apple_tests.mm). Compiled as Objective-C++ with ARC.
#import <ImageIO/ImageIO.h>
#import <UIKit/UIKit.h>

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
#include "maui/core/visibility.hpp"
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

    UIImageView* native_image_view(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge UIImageView*)handler->typed_platform_view()->native;
    }

    // Copy an NSData into image_bytes (empty for nil).
    image_bytes to_image_bytes(NSData* data)
    {
        if (data == nil || data.length == 0)
        {
            return {};
        }
        image_bytes bytes(static_cast<std::size_t>(data.length));
        std::memcpy(bytes.data(), data.bytes, static_cast<std::size_t>(data.length));
        return bytes;
    }

    // Renders a tiny 2x2 solid-color UIImage (scale 1) — the building block of both fixtures.
    UIImage* make_solid_image(CGFloat r, CGFloat g, CGFloat b)
    {
        UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
        format.opaque = NO;
        format.scale = 1;
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(2, 2)
                                                                                         format:format];
        return [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
          [[UIColor colorWithRed:r green:g blue:b alpha:1] setFill];
          [context fillRect:CGRectMake(0, 0, 2, 2)];
        }];
    }

    // Builds a tiny 2x2 PNG in memory and returns its encoded bytes. Empty on failure.
    image_bytes make_png_bytes()
    {
        UIImage* const image = make_solid_image(1, 0, 0);
        return to_image_bytes(image != nil ? UIImagePNGRepresentation(image) : nil);
    }

    // Writes the tiny PNG to a unique path under NSTemporaryDirectory(); returns the path (empty on
    // failure). The caller deletes it.
    std::string write_temp_png()
    {
        UIImage* const image = make_solid_image(0, 1, 0);
        NSData* const png = image != nil ? UIImagePNGRepresentation(image) : nil;
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

    // Encodes a 2-frame animated GIF in memory (two 2x2 frames with a 0.1s delay each) via
    // CGImageDestination and returns its bytes. Empty on failure.
    image_bytes make_animated_gif_bytes()
    {
        UIImage* const frame0 = make_solid_image(1, 0, 0);
        UIImage* const frame1 = make_solid_image(0, 1, 0);
        if (frame0.CGImage == nullptr || frame1.CGImage == nullptr)
        {
            return {};
        }
        NSMutableData* const data = [NSMutableData data];
        CGImageDestinationRef destination = // owned; released below
            CGImageDestinationCreateWithData((__bridge CFMutableDataRef)data, CFSTR("com.compuserve.gif"), 2, nullptr);
        if (destination == nullptr)
        {
            return {};
        }
        NSDictionary* const frame_properties = @{
            (__bridge NSString*)
            kCGImagePropertyGIFDictionary : @{(__bridge NSString*)kCGImagePropertyGIFDelayTime : @0.1}
        };
        CGImageDestinationAddImage(destination, frame0.CGImage, (__bridge CFDictionaryRef)frame_properties);
        CGImageDestinationAddImage(destination, frame1.CGImage, (__bridge CFDictionaryRef)frame_properties);
        const bool finalized = CGImageDestinationFinalize(destination);
        CFRelease(destination);
        if (!finalized)
        {
            return {};
        }
        return to_image_bytes(data);
    }

    TEST(ios_image_seam, attaching_handler_creates_uiimageview)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_NE(native_image_view(handler), nil);
    }

    // ImageViewExtensions.UpdateAspect: the content mode AND the clipping rule (ScaleAspectFill/Center
    // clip; the others don't) — UIKit's REAL aspect-fill, which the AppKit twin only approximated.
    TEST(ios_image_seam, aspect_maps_to_content_mode_and_clipping)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // aspect_fit (the default) → ScaleAspectFit, no clipping.
        EXPECT_EQ(view.contentMode, UIViewContentModeScaleAspectFit);
        EXPECT_FALSE(view.clipsToBounds);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(view.contentMode, UIViewContentModeScaleToFill);
        EXPECT_FALSE(view.clipsToBounds);

        control.set_aspect(aspect::center);
        EXPECT_EQ(view.contentMode, UIViewContentModeCenter);
        EXPECT_TRUE(view.clipsToBounds);

        control.set_aspect(aspect::aspect_fill);
        EXPECT_EQ(view.contentMode, UIViewContentModeScaleAspectFill);
        EXPECT_TRUE(view.clipsToBounds);
    }

    // ---- source (file source, synchronous decode) ----

    TEST(ios_image_seam, file_source_loads_into_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil); // nothing loaded before a source is set

        control.set_source(image_source::from_file(path));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);

        remove_file(path);
    }

    TEST(ios_image_seam, empty_source_clears_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file(path));
        ASSERT_NE(view.image, nil);

        // An empty source clears the loaded image.
        control.set_source(image_source::from_file(""));
        EXPECT_EQ(view.image, nil);

        remove_file(path);
    }

    TEST(ios_image_seam, clearing_source_to_null_clears_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file(path));
        ASSERT_NE(view.image, nil);

        control.set_source(nullptr);
        EXPECT_EQ(view.image, nil);

        remove_file(path);
    }

    TEST(ios_image_seam, missing_file_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_file("/tmp/does-not-exist-maui-image-test.png"));
        EXPECT_EQ(view.image, nil); // a failed load clears rather than crashes
    }

    // ---- uri + stream sources via the async loader (inline: no dispatcher injected) ----

    TEST(ios_image_seam, uri_file_source_loads_into_native_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // A file:// uri routes through the loader; the apply runs inline (the injected fetch reads a
        // non-http uri synchronously). Caching is disabled so the test does not persist into the
        // handler's real NSCachesDirectory disk cache (configure_loader wires it in production).
        control.set_source(image_source::from_uri("file://" + path, /*caching*/ false));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);

        remove_file(path);
    }

    TEST(ios_image_seam, stream_source_from_memory_loads_into_native_image)
    {
        const image_bytes png = make_png_bytes();
        ASSERT_FALSE(png.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // The stream provider yields the in-memory PNG bytes; the loader's apply (inline) decodes them.
        // Init-capture (drops the source's const) + mutable + move: the vector move-ctor is noexcept, so
        // this one-shot provider cannot throw (the test triggers exactly one load).
        control.set_source(image_source::from_stream(
            [bytes = png](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);
    }

    TEST(ios_image_seam, empty_stream_source_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // A provider that yields no bytes decodes to nothing → the view stays cleared.
        control.set_source(image_source::from_stream([](const cancellation_token&) { return image_bytes{}; }));
        EXPECT_EQ(view.image, nil);
    }

    // ---- native GIF playback (IsAnimationPlaying → Start/StopAnimating over AnimationImages) ----

    // The decode produces an animated UIImage whose frames land in AnimationImages (the
    // ImageImageSourcePartSetter shape: Image = first frame, AnimationImages = the frame array).
    TEST(ios_image_seam, animated_gif_decodes_to_animation_images)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // Init-capture + mutable + move: a noexcept one-shot provider (each test triggers one load).
        control.set_source(image_source::from_stream(
            [bytes = gif](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        ASSERT_NE(view.image, nil);
        ASSERT_NE(view.animationImages, nil);
        EXPECT_GT(view.animationImages.count, 1U) << "the animated GIF should decode to multiple frames";
        EXPECT_GT(view.animationDuration, 0.0);
    }

    TEST(ios_image_seam, is_animation_playing_starts_and_stops_native_animation)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // Default: not playing.
        EXPECT_FALSE(view.isAnimating);

        // Load the animated GIF, then start playing: StartAnimating cycles the AnimationImages frames.
        // Init-capture + mutable + move: a noexcept one-shot provider (each test triggers one load).
        control.set_source(image_source::from_stream(
            [bytes = gif](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        ASSERT_NE(view.animationImages, nil);

        control.set_is_animation_playing(true);
        EXPECT_TRUE(view.isAnimating);

        // Stop: StopAnimating freezes on the still image.
        control.set_is_animation_playing(false);
        EXPECT_FALSE(view.isAnimating);
    }

    TEST(ios_image_seam, animation_flag_set_before_load_plays_once_the_image_arrives)
    {
        const image_bytes gif = make_animated_gif_bytes();
        ASSERT_FALSE(gif.empty());

        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // IsAnimationPlaying set BEFORE a source exists: the apply re-asserts the flag after the image
        // is decoded (SetImageSource → UpdateValue(IsAnimationPlaying)), so a freshly-loaded GIF plays.
        control.set_is_animation_playing(true);
        // Init-capture + mutable + move: a noexcept one-shot provider (each test triggers one load).
        control.set_source(image_source::from_stream(
            [bytes = gif](const cancellation_token&) mutable noexcept { return std::move(bytes); }));
        ASSERT_NE(view.animationImages, nil);
        EXPECT_TRUE(view.isAnimating);
    }

    // The generic-IView pushes (the shared view_mapper through image_platform's ios update_* overrides).
    TEST(ios_image_seam, generic_iview_properties_reach_the_uiimageview)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);
        control.set_visibility(maui::core::visibility::visible);
        EXPECT_FALSE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);

        // UpdateIsEnabled's non-UIControl branch: the interaction toggle.
        control.set_is_enabled(false);
        EXPECT_FALSE(view.userInteractionEnabled);

        control.set_automation_id("hero_image");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "hero_image");
    }
} // namespace

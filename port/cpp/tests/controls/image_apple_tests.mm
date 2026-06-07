// Apple (AppKit) backend tests for the image seam — the cross-platform aspect maps to a real NSImageView's
// imageScaling, and a file source loads SYNCHRONOUSLY into the view's image. Compiled as Objective-C++ with
// ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/image_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::aspect;
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
} // namespace

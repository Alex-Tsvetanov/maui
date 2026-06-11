// Apple (AppKit) backend tests for the image_button seam — a FILE source loads synchronously into a
// real NSButton's image, the stroke/corner ride the layer, and a native click (performClick) flows back
// as released + clicked. Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image_button;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::image_button_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSButton* native_button(const std::shared_ptr<image_button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    // Writes a tiny 2x2 PNG to a unique path under NSTemporaryDirectory() (the image test convention).
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
        NSString* const name = [NSString stringWithFormat:@"maui_image_button_test_%@.png", [[NSUUID UUID] UUIDString]];
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

    class apple_image_button_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_image_button_seam, creates_a_borderless_image_button)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_button(handler) isKindOfClass:[NSButton class]]);
        EXPECT_FALSE(native_button(handler).bordered);
        EXPECT_EQ(native_button(handler).imagePosition, NSImageOnly);
    }

    TEST_F(apple_image_button_seam, file_source_loads_into_the_button_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image_button control;
        control.set_source(image_source::from_file(path));
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        EXPECT_NE(native_button(handler).image, nil);

        control.set_source(nullptr);
        EXPECT_EQ(native_button(handler).image, nil);
        remove_file(path);
    }

    TEST_F(apple_image_button_seam, aspect_maps_to_image_scaling)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_button(handler).imageScaling, NSImageScaleProportionallyUpOrDown);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(native_button(handler).imageScaling, NSImageScaleAxesIndependently);

        control.set_aspect(aspect::center);
        EXPECT_EQ(native_button(handler).imageScaling, NSImageScaleNone);
    }

    TEST_F(apple_image_button_seam, stroke_and_corner_ride_the_layer)
    {
        image_button control;
        control.set_stroke_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        control.set_stroke_thickness(3.0);
        control.set_corner_radius(7);
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        NSButton* const button = native_button(handler);
        ASSERT_NE(button.layer, nil);
        EXPECT_EQ(button.layer.borderWidth, 3.0);
        EXPECT_EQ(button.layer.cornerRadius, 7.0);
    }

    TEST_F(apple_image_button_seam, padding_enlarges_the_desired_size)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image_button control;
        control.set_source(image_source::from_file(path));
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        const auto base = handler->get_desired_size(1000, 1000);
        control.set_padding(maui::core::thickness{10, 20, 10, 20});
        const auto padded = handler->get_desired_size(1000, 1000);
        EXPECT_EQ(padded.width, base.width + 20.0);
        EXPECT_EQ(padded.height, base.height + 40.0);
        remove_file(path);
    }

    TEST_F(apple_image_button_seam, native_click_fires_released_then_clicked)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        std::vector<std::string> order;
        control.released.connect([&order] { order.emplace_back("released"); });
        control.clicked.connect([&order] { order.emplace_back("clicked"); });

        [native_button(handler) performClick:nil];
        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "released");
        EXPECT_EQ(order[1], "clicked");
    }

    TEST_F(apple_image_button_seam, disabled_button_suppresses_clicked)
    {
        image_button control;
        control.set_is_enabled(false);
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(native_button(handler).enabled);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        [native_button(handler) performClick:nil]; // a disabled NSButton swallows the click
        EXPECT_EQ(clicks, 0);
    }

    TEST_F(apple_image_button_seam, clearing_handler_disconnects)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

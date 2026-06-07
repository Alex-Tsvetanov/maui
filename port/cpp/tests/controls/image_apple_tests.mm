// Apple (AppKit) backend tests for the image seam — the cross-platform aspect maps to a real NSImageView's
// imageScaling (no image bytes loaded this cut). Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/image.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/image_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::core::aspect;
    using maui::core::image_handler;

    NSImageView* native_image_view(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge NSImageView*)handler->typed_platform_view()->native;
    }

    struct apple_image_seam : ::testing::Test
    {
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
} // namespace

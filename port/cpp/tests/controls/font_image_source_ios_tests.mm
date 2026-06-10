// iOS (UIKit) smoke tests for font_image_source + the IsOpaque mapping. A font source rasterizes its
// glyph (measured + drawn with its font/color attributes through a UIGraphicsImageRenderer) into a real
// UIImage set on the UIImageView; the load runs inline (no dispatcher injected — the glyph draw is
// synchronous). Run only for MAUI_BACKEND=ios (on the simulator); mirrors the AppKit twin
// (font_image_source_apple_tests.mm) and FontImageSourceServiceTests.iOS GetImageAsync's "the produced
// UIImage exists" assertion (without the pixel-color check, which needs a snapshot harness). Compiled
// as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/font_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/font.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::font_image_source;
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::font;
    using maui::core::image_handler;

    UIImageView* native_image_view(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge UIImageView*)handler->typed_platform_view()->native;
    }

    TEST(ios_font_image_source, font_source_rasterizes_into_native_image)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // A non-empty glyph in the system font, red — the loader's apply runs inline and sets a real
        // UIImage (rendered AlwaysOriginal so the explicit tint survives).
        control.set_source(image_source::from_font("A", font::of_size("", font_image_source::default_size),
                                                   maui::graphics::colors::red));
        ASSERT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);
        EXPECT_EQ(view.image.renderingMode, UIImageRenderingModeAlwaysOriginal);
    }

    TEST(ios_font_image_source, empty_glyph_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_font("", font::of_size("", 30), maui::graphics::colors::black));
        EXPECT_EQ(view.image, nil); // an empty glyph renders nothing
    }

    // IsOpaque → UIView.opaque (the may-skip-blending rendering hint).
    TEST(ios_font_image_source, is_opaque_maps_to_view_opaque)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        UIImageView* const view = native_image_view(handler);

        // The control default (false) was pushed at connect (UIView's own default is YES).
        EXPECT_FALSE(view.opaque);

        control.set_is_opaque(true);
        EXPECT_TRUE(view.opaque);

        control.set_is_opaque(false);
        EXPECT_FALSE(view.opaque);
    }
} // namespace

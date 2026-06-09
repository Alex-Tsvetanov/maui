// Apple (AppKit) smoke tests for font_image_source + the new IsOpaque / IsAnimationPlaying mapping. A font
// source rasterizes its glyph (NSAttributedString in the font/color) into a real NSImage set on the
// NSImageView; the load runs inline (no dispatcher injected — the glyph draw is synchronous). Compiled as
// Objective-C++ with ARC for the `apple` backend. Mirrors FontImageSourceServiceTests.iOS.GetImageAsync's
// "the produced UIImage exists" assertion (without the pixel-color check, which needs a snapshot harness).
#import <AppKit/AppKit.h>

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

    NSImageView* native_image_view(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge NSImageView*)handler->typed_platform_view()->native;
    }

    class apple_font_image_source : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_font_image_source, font_source_rasterizes_into_native_image)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);
        ASSERT_EQ(view.image, nil);

        // A non-empty glyph in the system font, red — the loader's apply runs inline and sets a real NSImage.
        control.set_source(image_source::from_font("A", font::of_size("", font_image_source::default_size),
                                                   maui::graphics::colors::red));
        EXPECT_NE(view.image, nil);
        EXPECT_GT(view.image.size.width, 0.0);
        EXPECT_GT(view.image.size.height, 0.0);
    }

    TEST_F(apple_font_image_source, empty_glyph_leaves_native_image_nil)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_source(image_source::from_font("", font::of_size("", 30), maui::graphics::colors::black));
        EXPECT_EQ(view.image, nil); // an empty glyph renders nothing
    }

    TEST_F(apple_font_image_source, is_opaque_maps_to_layer)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_is_opaque(true);
        EXPECT_TRUE(view.wantsLayer);
        EXPECT_TRUE(view.layer.opaque);
    }

    TEST_F(apple_font_image_source, is_animation_playing_maps_to_animates)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image_view(handler);

        control.set_is_animation_playing(true);
        EXPECT_TRUE(view.animates);

        control.set_is_animation_playing(false);
        EXPECT_FALSE(view.animates);
    }
} // namespace

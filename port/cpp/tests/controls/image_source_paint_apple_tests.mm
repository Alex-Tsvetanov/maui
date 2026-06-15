// Apple (AppKit) tests for the ImageSourcePaint background branch (apple_visual_ops apply_background): when
// a paint is an image_source_paint, the source is resolved through the image service registry, loaded
// synchronously, and installed as a named backing CALayer (contents = the source's CGImage) with the solid
// background cleared. A non-image paint removes a previously-installed image layer. Mirrors C#'s
// ViewExtensions.UpdateBackgroundImageSourceAsync (the StaticCALayer install). Compiled as Obj-C++ ARC.
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <memory>

#include "apple_visual_ops.hpp"
#include "maui/controls/file_image_source.hpp" // image_source factory (from_font)
#include "maui/controls/font_image_source.hpp"
#include "maui/core/font.hpp"
#include "maui/core/image_source_paint.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::font_image_source;
    using maui::controls::image_source;
    using maui::core::font;
    using maui::core::image_source_paint;

    NSString* const k_image_layer_name = @"maui.background.image";

    CALayer* image_sublayer(NSView* view)
    {
        for (CALayer* sub in view.layer.sublayers)
        {
            if ([sub.name isEqualToString:k_image_layer_name])
            {
                return sub;
            }
        }
        return nil;
    }

    class apple_image_source_paint : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_image_source_paint, font_image_paint_installs_a_background_image_layer)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 40, 40)];

        // A font source rasterizes synchronously into a real NSImage, so the resolve is deterministic.
        auto source = image_source::from_font("A", font::of_size("", font_image_source::default_size),
                                              maui::graphics::colors::red);
        const image_source_paint paint(source.get());

        maui::platform::apple::apply_background((__bridge void*)view, &paint);

        CALayer* const layer = image_sublayer(view);
        ASSERT_NE(layer, nil);
        EXPECT_NE(layer.contents, nil);             // the source's CGImage is set as the layer contents
        EXPECT_EQ(view.layer.backgroundColor, nil); // the solid background is cleared (C# UIColor.Clear)
    }

    TEST_F(apple_image_source_paint, switching_to_a_solid_paint_removes_the_image_layer)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 40, 40)];
        auto source = image_source::from_font("B", font::of_size("", font_image_source::default_size),
                                              maui::graphics::colors::blue);
        const image_source_paint image_paint(source.get());
        maui::platform::apple::apply_background((__bridge void*)view, &image_paint);
        ASSERT_NE(image_sublayer(view), nil);

        // A solid paint replaces the image background — the image layer is removed.
        const maui::graphics::solid_paint solid(maui::graphics::colors::green);
        maui::platform::apple::apply_background((__bridge void*)view, &solid);
        EXPECT_EQ(image_sublayer(view), nil);
        EXPECT_NE(view.layer.backgroundColor, nil); // the solid color is now the base fill
    }

    TEST_F(apple_image_source_paint, empty_source_installs_no_layer)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 40, 40)];
        auto source = image_source::from_font("", font::of_size("", 30), maui::graphics::colors::red);
        const image_source_paint paint(source.get()); // empty glyph → no image resolves
        maui::platform::apple::apply_background((__bridge void*)view, &paint);
        EXPECT_EQ(image_sublayer(view), nil);
    }
} // namespace

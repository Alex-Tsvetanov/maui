// Apple (AppKit) backend tests for the shape_view seam — run only for MAUI_BACKEND=apple. The
// shape controls render through the SAME NSView drawing host as graphics_view (graphics_host.mm),
// pointed at the platform's shape_drawable (the MauiShapeView recipe): readback proves a filled
// rectangle puts ink through the CG stack, with the stroke color on the edge. Compiled as
// Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::shape_view_handler;
    namespace shapes = maui::controls::shapes;

    constexpr std::size_t k_size = 64;

    class apple_shape_view_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }

        static std::vector<std::uint8_t> render_host(const std::shared_ptr<shape_view_handler>& handler)
        {
            std::vector<std::uint8_t> buffer(k_size * k_size * 4, 0);
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(buffer.data(), k_size, k_size, 8, k_size * 4, space,
                                                         kCGImageAlphaPremultipliedLast);
            CGColorSpaceRelease(space);
            CGContextTranslateCTM(context, 0, k_size);
            CGContextScaleCTM(context, 1, -1);

            auto* const host = (__bridge NSView*)handler->native_view();
            NSGraphicsContext* const graphics = [NSGraphicsContext graphicsContextWithCGContext:context flipped:YES];
            [host displayRectIgnoringOpacity:host.bounds inContext:graphics];

            CGContextRelease(context);
            return buffer;
        }

        static std::uint8_t channel(const std::vector<std::uint8_t>& buffer, std::size_t x, std::size_t y,
                                    std::size_t component)
        {
            return buffer[(((y * k_size) + x) * 4) + component];
        }
    };

    TEST_F(apple_shape_view_seam, filled_rectangle_renders_through_coregraphics) // the CG readback smoke
    {
        shapes::rectangle view;
        view.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.0F, 0.0F, 1.0F)));
        view.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(1.0F, 0.0F, 0.0F)));
        view.set_stroke_thickness(4);

        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        ASSERT_NE(handler->typed_platform_view(), nullptr);
        EXPECT_EQ(handler->typed_platform_view()->drawable.shape_view(), &view);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler);
        // center: the blue fill.
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 2), 200); // B
        EXPECT_LT(channel(buffer, k_size / 2, k_size / 2, 0), 50);  // R
        // the edge (inside the 4px stroke band): the red stroke.
        EXPECT_GT(channel(buffer, k_size / 2, 2, 0), 200); // R at the top edge
    }

    // R7d: when a shape sets BOTH Fill and Background (the PathAspectGallery pattern: red Fill +
    // LightGray Background), Fill paints the shape and Background paints the shape's host — the gray
    // 100x100 rect behind the figure. An ellipse leaves the box corners uncovered, so a corner pixel
    // must show the gray background (green here for channel clarity), while the center shows the fill.
    TEST_F(apple_shape_view_seam, background_with_fill_paints_the_host_behind_the_shape)
    {
        shapes::ellipse view;
        view.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(1.0F, 0.0F, 0.0F))); // red
        view.set_background(
            std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.0F, 1.0F, 0.0F))); // green host

        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        ASSERT_NE(handler->typed_platform_view(), nullptr);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler);
        // a box corner the ellipse does not cover: the green host background.
        EXPECT_GT(channel(buffer, 2, 2, 1), 200); // G at the corner
        EXPECT_LT(channel(buffer, 2, 2, 0), 50);  // not red
        // the center: the red ellipse fill.
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 0), 200); // R at the center
    }

    TEST_F(apple_shape_view_seam, no_shape_renders_nothing)
    {
        // box-less seat: a handler with a connected control but no fill/stroke still draws no ink
        // outside the transparent fill (ShapeDrawable stages a transparent fill color).
        shapes::rectangle view;
        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler);
        EXPECT_EQ(channel(buffer, k_size / 2, k_size / 2, 3), 0);
    }
} // namespace

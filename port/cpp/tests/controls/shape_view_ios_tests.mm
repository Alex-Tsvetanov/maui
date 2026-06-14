// iOS (UIKit) backend tests for the shape_view seam — run on-simulator for MAUI_BACKEND=ios. The
// shape controls render through the SAME UIView drawing host as graphics_view (graphics_host.mm),
// pointed at the platform's shape_drawable (the MauiShapeView recipe): readback proves a filled +
// stroked rectangle puts the right ink through the CG stack. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

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

    std::vector<std::uint8_t> render_host(void* native)
    {
        std::vector<std::uint8_t> buffer(k_size * k_size * 4, 0);
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGContextRef context =
            CGBitmapContextCreate(buffer.data(), k_size, k_size, 8, k_size * 4, space, kCGImageAlphaPremultipliedLast);
        CGColorSpaceRelease(space);

        auto* const host = (__bridge UIView*)native;
        [host.layer renderInContext:context];

        CGContextRelease(context);
        return buffer;
    }

    std::uint8_t channel(const std::vector<std::uint8_t>& buffer, std::size_t x, std::size_t y, std::size_t component)
    {
        return buffer[(((y * k_size) + x) * 4) + component];
    }

    TEST(ios_shape_view_seam, filled_rectangle_renders_through_coregraphics) // the CG readback smoke
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

        const std::vector<std::uint8_t> buffer = render_host(handler->native_view());
        // center: the blue fill; edge band: the red stroke. renderInContext may flip vertically vs
        // the raw buffer, so both checks use vertically symmetric sample points.
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 2), 200); // B
        EXPECT_LT(channel(buffer, k_size / 2, k_size / 2, 0), 50);  // R
        EXPECT_GT(channel(buffer, 2, k_size / 2, 0), 200);          // R at the left edge (symmetric)
    }

    TEST(ios_shape_view_seam, no_shape_paint_renders_nothing)
    {
        shapes::rectangle view;
        auto handler = std::make_shared<shape_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler->native_view());
        EXPECT_EQ(channel(buffer, k_size / 2, k_size / 2, 3), 0);
    }
} // namespace

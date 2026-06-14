// iOS (UIKit) backend tests for the graphics_view seam — run on-simulator for MAUI_BACKEND=ios.
// Drives the real UIView drawing host (graphics_host.mm): the handler points it at the control's
// drawable, and layer renderInContext renders the host into a CGBitmapContext through the shared
// coregraphics_canvas — the pixels are read back. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "maui/controls/graphics_view.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::graphics_view;
    using maui::core::graphics_view_handler;

    constexpr std::size_t k_size = 64;

    // A drawable filling the whole dirty rect red.
    class red_fill_drawable final : public maui::graphics::i_drawable
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
        {
            canvas.set_fill_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
            canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
        }
    };

    // Render the handler's native host into a zeroed RGBA bitmap via the layer (renderInContext
    // funnels through the view's drawRect — the standard offscreen technique).
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

    TEST(ios_graphics_view_seam, attaching_handler_creates_the_drawing_host)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);

        ASSERT_NE(handler->native_view(), nullptr);
        auto* const host = (__bridge UIView*)handler->native_view();
        EXPECT_TRUE([host isKindOfClass:[UIView class]]);
        EXPECT_FALSE(host.opaque); // C# PlatformTouchGraphicsView: Opaque = false
    }

    TEST(ios_graphics_view_seam, drawable_renders_through_coregraphics) // the CG readback smoke
    {
        graphics_view view;
        view.set_drawable(std::make_shared<red_fill_drawable>());

        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler->native_view());
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 0), 200); // R
        EXPECT_LT(channel(buffer, k_size / 2, k_size / 2, 1), 50);  // G
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 3), 200); // A
    }

    TEST(ios_graphics_view_seam, no_drawable_renders_nothing)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler->native_view());
        EXPECT_EQ(channel(buffer, k_size / 2, k_size / 2, 3), 0);
    }
} // namespace

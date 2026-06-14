// Apple (AppKit) backend tests for the graphics_view seam — run only for MAUI_BACKEND=apple.
// Drives the real NSView drawing host (graphics_host.mm): the handler points it at the control's
// drawable, and displayRectIgnoringOpacity renders the host into a CGBitmapContext through the
// shared coregraphics_canvas — the pixels are read back (the W1-13 readback strategy). Compiled as
// Objective-C++ with ARC.
#import <AppKit/AppKit.h>

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

    class apple_graphics_view_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }

        // Render the handler's native host into a zeroed RGBA bitmap and return the buffer.
        static std::vector<std::uint8_t> render_host(const std::shared_ptr<graphics_view_handler>& handler)
        {
            std::vector<std::uint8_t> buffer(k_size * k_size * 4, 0);
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(buffer.data(), k_size, k_size, 8, k_size * 4, space,
                                                         kCGImageAlphaPremultipliedLast);
            CGColorSpaceRelease(space);

            // top-left origin for the flipped host (the readback fixture convention)
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

    TEST_F(apple_graphics_view_seam, attaching_handler_creates_the_drawing_host)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);

        ASSERT_NE(handler->native_view(), nullptr);
        auto* const host = (__bridge NSView*)handler->native_view();
        EXPECT_TRUE([host isKindOfClass:[NSView class]]);
        EXPECT_TRUE(host.flipped); // the top-left drawing origin (PlatformGraphicsView)
    }

    TEST_F(apple_graphics_view_seam, drawable_renders_through_coregraphics) // the CG readback smoke
    {
        graphics_view view;
        view.set_drawable(std::make_shared<red_fill_drawable>());

        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler);
        // the center pixel is the drawable's red.
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 0), 200); // R
        EXPECT_LT(channel(buffer, k_size / 2, k_size / 2, 1), 50);  // G
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 3), 200); // A
    }

    TEST_F(apple_graphics_view_seam, no_drawable_renders_nothing)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        const std::vector<std::uint8_t> buffer = render_host(handler);
        EXPECT_EQ(channel(buffer, k_size / 2, k_size / 2, 3), 0); // fully transparent
    }
} // namespace

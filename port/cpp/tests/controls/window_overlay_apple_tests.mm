// Apple (AppKit) backend smoke for the window overlay draw seam (G4) — run only for
// MAUI_BACKEND=apple. The overlay renders through its owned graphics_view's REAL NSView drawing host
// (graphics_host.mm): add_overlay attaches the handler + points the host at the overlay self-drawable,
// and displayRectIgnoringOpacity renders the host into a CGBitmapContext through the shared
// coregraphics_canvas — the overlay's elements paint pixels, read back (the W1-13 readback strategy +
// the graphics_view_apple_tests recipe). Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "maui/controls/window.hpp"
#include "maui/controls/window_overlay.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/i_window_overlay_element.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::window;
    using maui::controls::window_overlay;

    constexpr std::size_t k_size = 64;

    // An overlay element that fills the whole dirty rect red.
    class red_fill_element final : public maui::core::i_window_overlay_element
    {
    public:
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override
        {
            canvas.set_fill_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
            canvas.fill_rectangle(dirty_rect.x, dirty_rect.y, dirty_rect.width, dirty_rect.height);
        }
        [[nodiscard]] bool contains(const maui::graphics::point& /*point*/) const override
        {
            return true;
        }
    };

    class apple_window_overlay_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }

        // The overlay's native drawing host (the graphics_view handler's NSView).
        static NSView* host_view(window_overlay& overlay)
        {
            const auto handler = overlay.graphics_surface().handler();
            auto* const gvh = dynamic_cast<maui::core::graphics_view_handler*>(handler.get());
            return gvh != nullptr ? (__bridge NSView*)gvh->native_view() : nil;
        }

        // Render the overlay's native drawing host into a zeroed RGBA bitmap and return the buffer.
        static std::vector<std::uint8_t> render(window_overlay& overlay)
        {
            std::vector<std::uint8_t> buffer(k_size * k_size * 4, 0);
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(buffer.data(), k_size, k_size, 8, k_size * 4, space,
                                                         kCGImageAlphaPremultipliedLast);
            CGColorSpaceRelease(space);

            CGContextTranslateCTM(context, 0, k_size);
            CGContextScaleCTM(context, 1, -1);

            NSView* const host = host_view(overlay);
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

    TEST_F(apple_window_overlay_seam, add_overlay_creates_the_native_drawing_host)
    {
        window win;
        window_overlay overlay(&win);
        win.add_overlay(overlay);

        NSView* const host = host_view(overlay);
        ASSERT_NE(host, nil);
        EXPECT_TRUE([host isKindOfClass:[NSView class]]);

        win.remove_overlay(overlay);
    }

    TEST_F(apple_window_overlay_seam, overlay_elements_render_through_coregraphics)
    {
        window win;
        window_overlay overlay(&win);
        red_fill_element element;
        win.add_overlay(overlay);
        overlay.add_window_element(element);

        // Frame the host so it has bounds to draw into.
        [host_view(overlay) setFrame:NSMakeRect(0, 0, k_size, k_size)];

        const std::vector<std::uint8_t> buffer = render(overlay);
        // the center pixel is the overlay element's red.
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 0), 200); // R
        EXPECT_LT(channel(buffer, k_size / 2, k_size / 2, 1), 50);  // G
        EXPECT_GT(channel(buffer, k_size / 2, k_size / 2, 3), 200); // A

        win.remove_overlay(overlay);
    }

    TEST_F(apple_window_overlay_seam, hidden_overlay_renders_nothing)
    {
        window win;
        window_overlay overlay(&win);
        red_fill_element element;
        win.add_overlay(overlay);
        overlay.add_window_element(element);
        overlay.set_is_visible(false);

        [host_view(overlay) setFrame:NSMakeRect(0, 0, k_size, k_size)];

        const std::vector<std::uint8_t> buffer = render(overlay);
        EXPECT_EQ(channel(buffer, k_size / 2, k_size / 2, 3), 0); // fully transparent (Draw was a no-op)

        win.remove_overlay(overlay);
    }
} // namespace

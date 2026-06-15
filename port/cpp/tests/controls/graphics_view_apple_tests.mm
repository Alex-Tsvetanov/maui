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
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

// The drag-notify methods MauiCppDrawableHostView (graphics_host.mm) defines; declared as a category
// so the test can call them directly without synthesizing blocking AppKit mouse events (the slider
// suite's documented compromise — the real mouseDown:/Dragged:/Up: forward to these).
@interface NSView (MauiGraphicsTouchTesting)
- (void)notifyStartInteractionAtPoint:(NSPoint)point;
- (void)notifyDragInteractionAtPoint:(NSPoint)point;
- (void)notifyEndInteractionAtPoint:(NSPoint)point;
- (void)notifyCancelInteraction;
@end

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

    // ---- the touch plumbing (PlatformTouchGraphicsView, apple) ----

    TEST_F(apple_graphics_view_seam, mouse_events_drive_the_interaction_callbacks)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        std::vector<maui::graphics::point_f> started;
        std::vector<maui::graphics::point_f> dragged;
        std::vector<maui::graphics::point_f> ended;
        bool end_inside = false;
        view.start_interaction.connect([&started](const std::vector<maui::graphics::point_f>& p) { started = p; });
        view.drag_interaction.connect([&dragged](const std::vector<maui::graphics::point_f>& p) { dragged = p; });
        view.end_interaction.connect([&ended, &end_inside](const std::vector<maui::graphics::point_f>& p, bool inside) {
            ended = p;
            end_inside = inside;
        });

        auto* const host = (__bridge NSView*)handler->native_view();
        [host notifyStartInteractionAtPoint:NSMakePoint(10, 12)];
        ASSERT_EQ(started.size(), 1U);
        EXPECT_FLOAT_EQ(started[0].x, 10.0F);
        EXPECT_FLOAT_EQ(started[0].y, 12.0F);

        [host notifyDragInteractionAtPoint:NSMakePoint(20, 22)];
        ASSERT_EQ(dragged.size(), 1U);
        EXPECT_FLOAT_EQ(dragged[0].x, 20.0F);

        // ending inside the bounds reports is_inside_bounds = true.
        [host notifyEndInteractionAtPoint:NSMakePoint(30, 32)];
        ASSERT_EQ(ended.size(), 1U);
        EXPECT_FLOAT_EQ(ended[0].x, 30.0F);
        EXPECT_TRUE(end_inside);
    }

    TEST_F(apple_graphics_view_seam, drag_outside_bounds_reports_not_contained_on_end)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool end_inside = true;
        view.end_interaction.connect(
            [&end_inside](const std::vector<maui::graphics::point_f>&, bool inside) { end_inside = inside; });

        auto* const host = (__bridge NSView*)handler->native_view();
        [host notifyStartInteractionAtPoint:NSMakePoint(10, 10)];
        [host notifyDragInteractionAtPoint:NSMakePoint(500, 500)]; // outside the 64x64 host
        [host notifyEndInteractionAtPoint:NSMakePoint(500, 500)];
        EXPECT_FALSE(end_inside); // _pressedContained went false on the out-of-bounds drag
    }

    TEST_F(apple_graphics_view_seam, cancel_interaction_fires)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool cancelled = false;
        view.cancel_interaction.connect([&cancelled] { cancelled = true; });

        auto* const host = (__bridge NSView*)handler->native_view();
        [host notifyStartInteractionAtPoint:NSMakePoint(5, 5)];
        [host notifyCancelInteraction];
        EXPECT_TRUE(cancelled);
    }

    TEST_F(apple_graphics_view_seam, disabled_view_swallows_interactions)
    {
        graphics_view view;
        view.set_is_enabled(false);
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool started = false;
        view.start_interaction.connect([&started](const std::vector<maui::graphics::point_f>&) { started = true; });

        auto* const host = (__bridge NSView*)handler->native_view();
        [host notifyStartInteractionAtPoint:NSMakePoint(10, 10)];
        EXPECT_FALSE(started); // C# IsEnabled gate: a disabled view receives no interactions
    }

    TEST_F(apple_graphics_view_seam, disconnecting_clears_the_interaction_target)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));
        // Hold a STRONG ARC reference so the host survives the platform view's CFRelease on disconnect —
        // then a stray mouse event must be a safe no-op (the on_disconnect cleared the interaction target).
        NSView* const host = (__bridge NSView*)handler->native_view();

        bool started = false;
        view.start_interaction.connect([&started](const std::vector<maui::graphics::point_f>&) { started = true; });

        view.set_handler(nullptr); // PlatformTouchGraphicsView.Disconnect: clears the host's target
        [host notifyStartInteractionAtPoint:NSMakePoint(1, 1)];
        EXPECT_FALSE(started); // the borrow was cleared, so the event is dropped (no UAF, no callback)
    }
} // namespace

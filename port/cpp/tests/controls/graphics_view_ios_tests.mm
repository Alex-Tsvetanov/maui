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
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include <gtest/gtest.h>

// The drag-notify methods MauiCppDrawableHostView (graphics_host.mm) defines; declared as a category so
// the test drives the same paths a real UITouch gesture takes (UITouch sets cannot be synthesized in a
// unit test — the documented compromise shared with the other ios suites).
@interface UIView (MauiGraphicsTouchTesting)
- (void)notifyStartInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyDragInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyEndInteraction:(const std::vector<maui::graphics::point_f>&)points;
- (void)notifyCancelInteraction;
@end

namespace
{
    using maui::controls::graphics_view;
    using maui::core::graphics_view_handler;
    using maui::graphics::point_f;

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

    // ---- the touch plumbing (PlatformTouchGraphicsView, iOS) ----
    //
    // W8-56 fix (#9): pointsFromEvent: now builds its points from [event touchesForView:self] (C#'s
    // UIViewExtensions.GetPointsInView(evt) → evt.TouchesForView(target)), not the per-callback `touches`
    // NSSet and not event.allTouches. That path is NOT unit-testable here: a UIEvent with a populated
    // touchesForView: set cannot be synthesized (UITouch/UIEvent have no public constructors — the
    // documented compromise shared with the other ios suites), so the tests below exercise the notify*
    // layer directly. Fix #9 is verified by oracle parity (PlatformTouchGraphicsView.cs:57/68/79).

    TEST(ios_graphics_view_seam, touch_events_drive_the_interaction_callbacks)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        std::vector<point_f> started;
        std::vector<point_f> ended;
        bool end_inside = false;
        view.start_interaction.connect([&started](const std::vector<point_f>& p) { started = p; });
        view.end_interaction.connect([&ended, &end_inside](const std::vector<point_f>& p, bool inside) {
            ended = p;
            end_inside = inside;
        });

        auto* const host = (__bridge UIView*)handler->native_view();
        [host notifyStartInteraction:std::vector<point_f>{point_f(10, 12)}];
        ASSERT_EQ(started.size(), 1U);
        EXPECT_FLOAT_EQ(started[0].x, 10.0F);

        [host notifyDragInteraction:std::vector<point_f>{point_f(20, 22)}];
        [host notifyEndInteraction:std::vector<point_f>{point_f(30, 32)}];
        ASSERT_EQ(ended.size(), 1U);
        EXPECT_FLOAT_EQ(ended[0].x, 30.0F);
        EXPECT_TRUE(end_inside); // the drag stayed inside the 64x64 host
    }

    TEST(ios_graphics_view_seam, drag_outside_bounds_reports_not_contained_on_end)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool end_inside = true;
        view.end_interaction.connect([&end_inside](const std::vector<point_f>&, bool inside) { end_inside = inside; });

        auto* const host = (__bridge UIView*)handler->native_view();
        [host notifyStartInteraction:std::vector<point_f>{point_f(10, 10)}];
        [host notifyDragInteraction:std::vector<point_f>{point_f(500, 500)}]; // outside the host
        [host notifyEndInteraction:std::vector<point_f>{point_f(500, 500)}];
        EXPECT_FALSE(end_inside);
    }

    TEST(ios_graphics_view_seam, cancel_interaction_fires)
    {
        graphics_view view;
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool cancelled = false;
        view.cancel_interaction.connect([&cancelled] { cancelled = true; });

        auto* const host = (__bridge UIView*)handler->native_view();
        [host notifyStartInteraction:std::vector<point_f>{point_f(5, 5)}];
        [host notifyCancelInteraction];
        EXPECT_TRUE(cancelled);
    }

    TEST(ios_graphics_view_seam, disabled_view_swallows_interactions)
    {
        graphics_view view;
        view.set_is_enabled(false);
        auto handler = std::make_shared<graphics_view_handler>();
        view.set_handler(handler);
        handler->platform_arrange(maui::graphics::rect(0, 0, k_size, k_size));

        bool started = false;
        view.start_interaction.connect([&started](const std::vector<point_f>&) { started = true; });

        auto* const host = (__bridge UIView*)handler->native_view();
        [host notifyStartInteraction:std::vector<point_f>{point_f(10, 10)}];
        EXPECT_FALSE(started); // C# IsEnabled gate
    }
} // namespace

// iOS (UIKit) backend tests for the border seam — the on-simulator twin of border_apple_tests.mm:
// the host is a real UIView whose layer carries the border (the tagged CAShapeLayer stroke + the
// shape mask from ios_border_ops.hpp), the content's native view becomes a subview, and arranging a
// new size re-pushes the bounds-dependent stroke. The frame facade rides the same machinery.
// Compiled as Objective-C++ with ARC for the `ios` backend.
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <memory>

#include "ios_border_ops.hpp"
#include "maui/controls/border.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::border;
    using maui::controls::frame;
    using maui::controls::label;
    using maui::core::border_handler;
    using maui::core::label_handler;
    using maui::graphics::color;
    using maui::graphics::rect;
    using maui::graphics::solid_paint;

    UIView* native_host(const std::shared_ptr<border_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    CAShapeLayer* stroke_layer(const std::shared_ptr<border_handler>& handler)
    {
        return maui::platform::ios::find_border_layer(native_host(handler).layer);
    }

    TEST(ios_border_seam, host_is_a_uiview_with_no_stroke_by_default)
    {
        border view;
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_host(handler) isKindOfClass:[UIView class]]);
        EXPECT_EQ(stroke_layer(handler), nil); // no stroke brush -> DrawBorder's early-out
    }

    TEST(ios_border_seam, content_becomes_a_subview)
    {
        border view;
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);

        label child;
        auto child_handler = std::make_shared<label_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge UIView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        view.set_content(child);

        EXPECT_EQ(child_native.superview, native_host(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);
    }

    TEST(ios_border_seam, stroke_push_builds_the_tagged_shape_layer)
    {
        border view;
        view.set_stroke(std::make_shared<solid_paint>(color(1.0F, 0.0F, 0.0F)));
        view.set_stroke_thickness(3);
        view.set_stroke_dash_array({4.0, 2.0});
        view.set_stroke_dash_offset(1.0);
        view.set_stroke_line_cap(maui::graphics::line_cap::round);
        view.set_stroke_line_join(maui::graphics::line_join::bevel);

        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        view.arrange(rect(0, 0, 100, 50)); // size the host so the stroke path has bounds

        CAShapeLayer* const layer = stroke_layer(handler);
        ASSERT_NE(layer, nil);
        EXPECT_NE(layer.path, nullptr);
        // MauiCALayer.DrawBorder: line width is DOUBLE the thickness (the mask cuts the outer half).
        EXPECT_EQ(layer.lineWidth, 6.0);
        EXPECT_TRUE([layer.lineCap isEqualToString:kCALineCapRound]);
        EXPECT_TRUE([layer.lineJoin isEqualToString:kCALineJoinBevel]);
        // Dash lengths + phase scale by the thickness (SetBorderDash).
        ASSERT_EQ(layer.lineDashPattern.count, 2U);
        EXPECT_EQ(layer.lineDashPattern[0].doubleValue, 12.0);
        EXPECT_EQ(layer.lineDashPattern[1].doubleValue, 6.0);
        EXPECT_EQ(layer.lineDashPhase, 3.0);
        // The shape mask clips the host (the C# context clip + content mask, collapsed).
        EXPECT_NE(native_host(handler).layer.mask, nil);
    }

    TEST(ios_border_seam, clearing_the_stroke_removes_the_layer)
    {
        border view;
        view.set_stroke(std::make_shared<solid_paint>(color(0.0F, 0.0F, 1.0F)));
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        view.arrange(rect(0, 0, 100, 50));
        ASSERT_NE(stroke_layer(handler), nil);

        view.set_stroke(nullptr);
        EXPECT_EQ(stroke_layer(handler), nil);
    }

    TEST(ios_border_seam, resizing_repushes_the_stroke_geometry)
    {
        border view;
        view.set_stroke(std::make_shared<solid_paint>(color(0.0F, 1.0F, 0.0F)));
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);

        view.arrange(rect(0, 0, 100, 50));
        CAShapeLayer* const layer = stroke_layer(handler);
        ASSERT_NE(layer, nil);
        EXPECT_EQ(layer.frame.size.width, 100.0);

        view.arrange(rect(0, 0, 200, 80)); // a size change re-runs update_border (PlatformArrange)
        EXPECT_EQ(stroke_layer(handler).frame.size.width, 200.0);
        EXPECT_EQ(stroke_layer(handler).frame.size.height, 80.0);
    }

    TEST(ios_border_seam, frame_facade_rides_the_same_machinery)
    {
        frame view;
        view.set_border_color(color(0.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<border_handler>();
        view.set_handler(handler);
        view.arrange(rect(0, 0, 120, 60));

        CAShapeLayer* const layer = stroke_layer(handler);
        ASSERT_NE(layer, nil);
        EXPECT_EQ(layer.lineWidth, 2.0); // the facade's fixed 1px border, doubled by the recipe
    }
} // namespace

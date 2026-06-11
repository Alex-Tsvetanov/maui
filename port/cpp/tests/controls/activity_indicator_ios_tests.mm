// iOS (UIKit) backend tests for the activity_indicator seam — run only for MAUI_BACKEND=ios (executed
// ON the iOS simulator via tools/ios-sim-run.sh). Drives a genuine UIActivityIndicatorView: IsRunning
// maps to startAnimating/stopAnimating (the REAL isAnimating state is asserted) with the
// UpdateIsRunning visibility coupling, Color to the view's color. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/activity_indicator.hpp"
#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::activity_indicator;
    using maui::core::activity_indicator_handler;
    using maui::core::i_element_handler;

    UIActivityIndicatorView* native_spinner(const std::shared_ptr<activity_indicator_handler>& handler)
    {
        return (__bridge UIActivityIndicatorView*)handler->typed_platform_view()->native;
    }

    TEST(ios_activity_indicator_seam, attaching_handler_creates_a_medium_spinner)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_FALSE(native_spinner(handler).isAnimating); // IsRunning defaults to false
    }

    TEST(ios_activity_indicator_seam, is_running_drives_the_real_animation)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        control.set_is_running(true);
        EXPECT_TRUE(native_spinner(handler).isAnimating);
        EXPECT_FALSE(native_spinner(handler).hidden);

        control.set_is_running(false);
        EXPECT_FALSE(native_spinner(handler).isAnimating);
    }

    TEST(ios_activity_indicator_seam, hiding_a_running_indicator_stops_the_animation)
    {
        // The Visibility key is remapped onto MapIsRunning (the C# iOS override): hiding the view both
        // hides the native spinner AND stops the animation; showing it again restarts.
        activity_indicator control;
        control.set_is_running(true);
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_spinner(handler).isAnimating);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_FALSE(native_spinner(handler).isAnimating);
        EXPECT_TRUE(native_spinner(handler).hidden);

        control.set_visibility(maui::core::visibility::visible);
        EXPECT_TRUE(native_spinner(handler).isAnimating);
        EXPECT_FALSE(native_spinner(handler).hidden);
    }

    TEST(ios_activity_indicator_seam, color_maps_to_the_spinner_color)
    {
        activity_indicator control;
        control.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        UIColor* const spinner_color = native_spinner(handler).color;
        ASSERT_NE(spinner_color, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([spinner_color getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(green, 0.0, 0.01);
    }

    TEST(ios_activity_indicator_seam, clearing_handler_disconnects)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_activity_indicator_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<activity_indicator>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<activity_indicator_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        activity_indicator control;
        control.set_is_running(true);
        control.set_handler(handler);
        EXPECT_TRUE(((__bridge UIActivityIndicatorView*)resolved->typed_platform_view()->native).isAnimating);
    }
} // namespace

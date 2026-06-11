// Apple (AppKit) backend tests for the activity_indicator seam — run only for MAUI_BACKEND=apple.
// Drives a genuine spinning-style NSProgressIndicator: IsRunning maps to startAnimation/stopAnimation
// with the UpdateIsRunning visibility coupling (observed through the cross-platform `is_running`
// mirror — NSProgressIndicator has no isAnimating getter — plus the native hidden flag). Compiled as
// Objective-C++ with ARC.
#import <AppKit/AppKit.h>

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

    NSProgressIndicator* native_spinner(const std::shared_ptr<activity_indicator_handler>& handler)
    {
        return (__bridge NSProgressIndicator*)handler->typed_platform_view()->native;
    }

    // NSProgressIndicator creation needs the shared application object (no run loop required).
    class apple_activity_indicator_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_activity_indicator_seam, attaching_handler_creates_a_spinning_indicator)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        NSProgressIndicator* const view = native_spinner(handler);
        EXPECT_EQ(view.style, NSProgressIndicatorStyleSpinning);
        EXPECT_TRUE(view.indeterminate);
        EXPECT_FALSE(view.displayedWhenStopped); // the HidesWhenStopped analog
    }

    TEST_F(apple_activity_indicator_seam, is_running_drives_the_animation_state)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);

        control.set_is_running(true);
        EXPECT_TRUE(handler->typed_platform_view()->is_running);
        EXPECT_FALSE(native_spinner(handler).hidden);

        control.set_is_running(false);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);
    }

    TEST_F(apple_activity_indicator_seam, hiding_a_running_indicator_stops_the_animation)
    {
        activity_indicator control;
        control.set_is_running(true);
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(handler->typed_platform_view()->is_running);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);
        EXPECT_TRUE(native_spinner(handler).hidden);

        control.set_visibility(maui::core::visibility::visible);
        EXPECT_TRUE(handler->typed_platform_view()->is_running);
        EXPECT_FALSE(native_spinner(handler).hidden);
    }

    TEST_F(apple_activity_indicator_seam, color_records_the_mirror)
    {
        // AppKit deviation (documented in activity_indicator_handler.mm): NSProgressIndicator has no
        // public spinner color API, so the color lands on the cross-platform mirror.
        activity_indicator control;
        control.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->color, maui::graphics::color(1.0F, 0.0F, 0.0F));
    }

    TEST_F(apple_activity_indicator_seam, clearing_handler_disconnects)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_activity_indicator_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<activity_indicator>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<activity_indicator_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        activity_indicator control;
        control.set_is_running(true);
        control.set_handler(handler);
        EXPECT_TRUE(resolved->typed_platform_view()->is_running);
    }
} // namespace

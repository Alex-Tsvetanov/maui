// Tests for the activity_indicator control (maui::controls::activity_indicator <= ActivityIndicator)
// and the headless handler seam. C# has no ActivityIndicator Core.UnitTests suite (the control is two
// plain bindables), so the control half captures the source defaults and the seam half exercises the
// UpdateIsRunning visibility coupling — the C# iOS/Android Visibility → MapIsRunning override — per
// the headless conventions (button_tests.cpp).
#include "maui/controls/activity_indicator.hpp"

#include <memory>

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
    using maui::graphics::color;

    // ---- the control in isolation ----

    TEST(activity_indicator, defaults_to_not_running)
    {
        const activity_indicator control;
        EXPECT_FALSE(control.is_running());
    }

    TEST(activity_indicator, is_running_and_color_are_settable)
    {
        activity_indicator control;
        control.set_is_running(true);
        control.set_color(color(1.0F, 0.0F, 0.0F));
        EXPECT_TRUE(control.is_running());
        EXPECT_EQ(control.color(), color(1.0F, 0.0F, 0.0F));
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(activity_indicator_seam, attaching_handler_maps_initial_state)
    {
        activity_indicator control;
        control.set_is_running(true);
        control.set_color(color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->is_running);
        EXPECT_FALSE(platform->hidden);
        EXPECT_EQ(platform->color, color(1.0F, 0.0F, 0.0F));
    }

    TEST(activity_indicator_seam, is_running_drives_the_animation_state)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);

        control.set_is_running(true);
        EXPECT_TRUE(handler->typed_platform_view()->is_running);

        control.set_is_running(false);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);
    }

    TEST(activity_indicator_seam, hiding_a_running_indicator_stops_the_animation)
    {
        // The Visibility key is remapped onto MapIsRunning (the C# iOS/Android override): hiding the
        // view both hides the native spinner AND stops the animation; showing it again restarts.
        activity_indicator control;
        control.set_is_running(true);
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->is_running);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_FALSE(platform->is_running); // not animating while invisible
        EXPECT_TRUE(platform->hidden);

        control.set_visibility(maui::core::visibility::visible);
        EXPECT_TRUE(platform->is_running); // IsRunning is still true: the animation resumes
        EXPECT_FALSE(platform->hidden);
    }

    TEST(activity_indicator_seam, running_while_collapsed_stays_hidden)
    {
        activity_indicator control;
        control.set_visibility(maui::core::visibility::collapsed);
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);

        control.set_is_running(true);
        EXPECT_FALSE(handler->typed_platform_view()->is_running);
        EXPECT_TRUE(handler->typed_platform_view()->hidden);
    }

    TEST(activity_indicator_seam, clearing_handler_disconnects)
    {
        activity_indicator control;
        auto handler = std::make_shared<activity_indicator_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(activity_indicator_seam, handler_resolved_from_default_registry)
    {
        // activity_indicator -> activity_indicator_handler is self-registered in activity_indicator.cpp.
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

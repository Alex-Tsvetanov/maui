// Apple (AppKit) backend tests for the progress_bar seam — run only for MAUI_BACKEND=apple. Drives a
// genuine determinate bar-style NSProgressIndicator: Progress maps to doubleValue over the fixed
// [0, 1] range. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/progress_bar.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::progress_bar;
    using maui::core::i_element_handler;
    using maui::core::progress_bar_handler;

    NSProgressIndicator* native_bar(const std::shared_ptr<progress_bar_handler>& handler)
    {
        return (__bridge NSProgressIndicator*)handler->typed_platform_view()->native;
    }

    // NSProgressIndicator creation needs the shared application object (no run loop required).
    class apple_progress_bar_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_progress_bar_seam, attaching_handler_creates_a_determinate_bar)
    {
        progress_bar control;
        control.set_progress(0.6);
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        NSProgressIndicator* const view = native_bar(handler);
        EXPECT_FALSE(view.indeterminate);
        EXPECT_EQ(view.style, NSProgressIndicatorStyleBar);
        EXPECT_EQ(view.minValue, 0);
        EXPECT_EQ(view.maxValue, 1);
        EXPECT_EQ(view.doubleValue, 0.6);
    }

    TEST_F(apple_progress_bar_seam, setting_progress_updates_the_bar_clamped)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        control.set_progress(0.25);
        EXPECT_EQ(native_bar(handler).doubleValue, 0.25);

        control.set_progress(7); // clamps to 1 before reaching the native bar
        EXPECT_EQ(native_bar(handler).doubleValue, 1.0);
    }

    TEST_F(apple_progress_bar_seam, progress_color_records_the_mirror)
    {
        // AppKit deviation (documented in progress_bar_handler.mm): NSProgressIndicator has no public
        // fill-color API, so the color lands on the cross-platform mirror.
        progress_bar control;
        control.set_progress_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->progress_color, maui::graphics::color(0.0F, 1.0F, 0.0F));
    }

    TEST_F(apple_progress_bar_seam, generic_iview_properties_reach_the_bar)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        NSProgressIndicator* const view = native_bar(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);
    }

    TEST_F(apple_progress_bar_seam, clearing_handler_disconnects)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_progress_bar_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<progress_bar>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<progress_bar_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        progress_bar control;
        control.set_progress(0.5);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSProgressIndicator*)resolved->typed_platform_view()->native).doubleValue, 0.5);
    }
} // namespace

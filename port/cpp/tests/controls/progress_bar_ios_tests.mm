// iOS (UIKit) backend tests for the progress_bar seam — run only for MAUI_BACKEND=ios (executed ON
// the iOS simulator via tools/ios-sim-run.sh). Drives a genuine UIProgressView: Progress maps to the
// float fraction and ProgressColor to progressTintColor. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/progress_bar.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::progress_bar;
    using maui::core::flow_direction;
    using maui::core::i_element_handler;
    using maui::core::progress_bar_handler;

    UIProgressView* native_bar(const std::shared_ptr<progress_bar_handler>& handler)
    {
        return (__bridge UIProgressView*)handler->typed_platform_view()->native;
    }

    TEST(ios_progress_bar_seam, attaching_handler_creates_uiprogressview_and_maps_progress)
    {
        progress_bar control;
        control.set_progress(0.6);
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_FLOAT_EQ(native_bar(handler).progress, 0.6F);
    }

    TEST(ios_progress_bar_seam, setting_progress_updates_the_bar_clamped)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        control.set_progress(0.25);
        EXPECT_FLOAT_EQ(native_bar(handler).progress, 0.25F);

        control.set_progress(7); // clamps to 1 before reaching the native bar
        EXPECT_FLOAT_EQ(native_bar(handler).progress, 1.0F);
    }

    TEST(ios_progress_bar_seam, progress_color_maps_to_progress_tint)
    {
        progress_bar control;
        control.set_progress_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        UIColor* const tint = native_bar(handler).progressTintColor;
        ASSERT_NE(tint, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([tint getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(green, 1.0, 0.01);
        EXPECT_NEAR(red, 0.0, 0.01);
    }

    TEST(ios_progress_bar_seam, generic_iview_properties_reach_the_bar)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        UIProgressView* const view = native_bar(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);
    }

    TEST(ios_progress_bar_seam, flow_direction_maps_to_semantic_content_attribute)
    {
        // ProgressBarHandler.MapFlowDirection: the resolved direction sets the bar's
        // UISemanticContentAttribute and is re-applied to each subview (the iOS-26 walk).
        progress_bar control;
        control.set_flow_direction(flow_direction::right_to_left);
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        UIProgressView* const view = native_bar(handler);
        EXPECT_EQ(view.semanticContentAttribute, UISemanticContentAttributeForceRightToLeft);
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::right_to_left);
        for (UIView* subview in view.subviews)
        {
            EXPECT_EQ(subview.semanticContentAttribute, UISemanticContentAttributeForceRightToLeft);
        }

        control.set_flow_direction(flow_direction::left_to_right);
        EXPECT_EQ(view.semanticContentAttribute, UISemanticContentAttributeForceLeftToRight);
    }

    TEST(ios_progress_bar_seam, clearing_handler_disconnects)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_progress_bar_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<progress_bar>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<progress_bar_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        progress_bar control;
        control.set_progress(0.5);
        control.set_handler(handler);
        EXPECT_FLOAT_EQ(((__bridge UIProgressView*)resolved->typed_platform_view()->native).progress, 0.5F);
    }
} // namespace

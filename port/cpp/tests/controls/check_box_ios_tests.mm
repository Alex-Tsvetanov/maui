// iOS (UIKit) backend tests for the check_box seam — run only for MAUI_BACKEND=ios (executed ON the
// iOS simulator via tools/ios-sim-run.sh). UIKit has no native check box, so these drive the DRAWN
// MauiCheckBox port (src/platform/ios/check_box_handler.mm — the UIButton subclass with the
// CoreGraphics-rendered check): IsChecked maps to its state + rendered image, the foreground tint
// flows to the image view, and the native TouchUpInside toggle flows back through the handler to the
// control's `checked_changed` event. Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION: as in button_ios_tests.mm, -[UIControl sendActionsForControlEvents:] needs a
// UIApplication this spawned test process cannot create, so send_control_event replicates UIControl's
// documented dispatch walk over the REAL TouchUpInside registration the drawn control made on itself.
#import <UIKit/UIKit.h>

#include <limits>
#include <memory>
#include <string>

#include "maui/controls/check_box.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::check_box;
    using maui::core::check_box_handler;
    using maui::core::i_element_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIButton* native_check_box(const std::shared_ptr<check_box_handler>& handler)
    {
        return (__bridge UIButton*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see
    // the header comment).
    void send_control_event(UIControl* control, UIControlEvents event)
    {
        NSArray* const targets = control.allTargets.allObjects;
        for (NSUInteger t = 0; t < targets.count; ++t)
        {
            id const target = targets[t];
            NSArray<NSString*>* const actions = [control actionsForTarget:target forControlEvent:event];
            for (NSUInteger a = 0; a < actions.count; ++a)
            {
                SEL const action = NSSelectorFromString(actions[a]);
                NSMethodSignature* const signature = [target methodSignatureForSelector:action];
                ASSERT_NE(signature, nil);
                NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
                invocation.selector = action;
                id sender = control;
                [invocation setArgument:&sender atIndex:2]; // 0 = self, 1 = _cmd, 2 = the sender
                [invocation invokeWithTarget:target];
            }
        }
    }

    TEST(ios_check_box_seam, attaching_handler_creates_drawn_checkbox_with_rendered_image)
    {
        check_box control;
        control.set_is_checked(true);
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UIButton* const view = native_check_box(handler);
        // The drawn control rendered its CoreGraphics check image into the Normal state.
        EXPECT_NE([view imageForState:UIControlStateNormal], nil);
    }

    TEST(ios_check_box_seam, checked_and_unchecked_render_distinct_images)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        UIButton* const view = native_check_box(handler);

        UIImage* const unchecked = [view imageForState:UIControlStateNormal];
        control.set_is_checked(true);
        UIImage* const checked = [view imageForState:UIControlStateNormal];
        ASSERT_NE(unchecked, nil);
        ASSERT_NE(checked, nil);
        EXPECT_NE(unchecked, checked); // the check-mark image swapped in
    }

    TEST(ios_check_box_seam, native_tap_toggles_the_control)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.checked_changed.connect([&reported](bool value) { reported = value; });

        // Simulate the user's tap: the drawn control's own TouchUpInside toggles IsChecked and raises
        // CheckedChanged, which the handler routes back to the virtual view.
        send_control_event(native_check_box(handler), UIControlEventTouchUpInside);

        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(reported);

        send_control_event(native_check_box(handler), UIControlEventTouchUpInside);
        EXPECT_FALSE(control.is_checked());
        EXPECT_FALSE(reported);
    }

    TEST(ios_check_box_seam, color_maps_to_the_checkbox_tint)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        UIButton* const view = native_check_box(handler);

        control.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        // CheckBoxExtensions.UpdateForeground → CheckBoxTintColor → ImageView.TintColor + TintColor.
        UIColor* const tint = view.tintColor;
        ASSERT_NE(tint, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([tint getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(green, 0.0, 0.01);
    }

    TEST(ios_check_box_seam, desired_size_is_the_default_18pt_glyph_not_the_44_floor)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        // The drawn control squares itself at DefaultSize (18pt) under free constraints — NOT the 44pt
        // MinimumSize touch-target floor src/'s CheckBoxHandler.iOS.cs applies. User rule 4 (RENDER-BREAKS-TIES)
        // (2026-07-16): the shipped MAUI (MauiVersion 10.0.71) renders 18pt on both Catalyst and iOS and
        // the render wins over src/'s stale 2025 snapshot. See docs/comparison/PARITY_REVIEW.md item 1.
        const maui::graphics::size measured =
            handler->get_desired_size(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        EXPECT_EQ(measured.width, 18.0);
        EXPECT_EQ(measured.height, 18.0);
    }

    TEST(ios_check_box_seam, accessibility_value_reports_the_checked_state)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        UIButton* const view = native_check_box(handler);

        EXPECT_EQ(to_std_string(view.accessibilityValue), "0");
        control.set_is_checked(true);
        EXPECT_EQ(to_std_string(view.accessibilityValue), "1");
    }

    TEST(ios_check_box_seam, generic_iview_properties_reach_the_drawn_control)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        UIButton* const view = native_check_box(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);
        EXPECT_FALSE(view.userInteractionEnabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);

        control.set_automation_id("accept_terms");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "accept_terms");
    }

    TEST(ios_check_box_seam, clearing_handler_disconnects)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_check_box_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<check_box>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<check_box_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        check_box control;
        control.set_is_checked(true);
        control.set_handler(handler);
        EXPECT_NE([(__bridge UIButton*)resolved->typed_platform_view()->native imageForState:UIControlStateNormal],
                  nil);
    }
} // namespace

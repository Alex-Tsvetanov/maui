// iOS (UIKit) backend tests for the radio_button seam — the NATIVE DEFAULT FALLBACK (see
// src/platform/ios/radio_button_handler.mm): the string content maps to a real UIButton's title,
// IsChecked rides UIButton.selected + the AccessibilityValue "1"/"0" push (the C# MapIsChecked port),
// the circle / filled-circle SF-symbol pair stands in for the DefaultTemplate's indicator, a native
// tap flows back as a from-handler SELECT, and the cross-platform group exclusion pushes the uncheck
// to the other native button. Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via
// tools/ios-sim-run.sh). Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION: same constraint as button_ios_tests.mm — a spawned simulator process has no
// UIApplication, so send_control_event replicates -[UIControl sendActionsForControlEvents:]'s
// dispatch-table walk over the REAL registrations the handler made on the REAL UIButton.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/grid.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/core/font.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::radio_button;
    using maui::core::radio_button_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIButton* native_button(const std::shared_ptr<radio_button_handler>& handler)
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

    TEST(ios_radio_button_seam, attaching_handler_creates_the_fallback_button_and_maps_content)
    {
        radio_button control;
        control.set_content("Option A");
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UIButton* const button = native_button(handler);
        EXPECT_TRUE([button isKindOfClass:[UIButton class]]);
        EXPECT_EQ(to_std_string([button titleForState:UIControlStateNormal]), "Option A");
        EXPECT_FALSE(button.selected);
        // The DefaultTemplate's indicator pair rides the state images.
        EXPECT_NE([button imageForState:UIControlStateNormal], nil);
        EXPECT_NE([button imageForState:UIControlStateSelected], nil);
    }

    TEST(ios_radio_button_seam, is_checked_maps_to_selected_and_accessibility_value)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        UIButton* const button = native_button(handler);

        control.set_is_checked(true);
        EXPECT_TRUE(button.selected);
        EXPECT_EQ(to_std_string(button.accessibilityValue), "1"); // RadioButtonHandler.iOS MapIsChecked

        control.set_is_checked(false);
        EXPECT_FALSE(button.selected);
        EXPECT_EQ(to_std_string(button.accessibilityValue), "0");
    }

    TEST(ios_radio_button_seam, native_tap_selects_the_control)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        bool changed = false;
        control.checked_changed.connect([&changed](bool value) { changed = value; });

        send_control_event(native_button(handler), UIControlEventTouchUpInside);
        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(changed);
        EXPECT_TRUE(native_button(handler).selected);
    }

    TEST(ios_radio_button_seam, native_tap_drives_the_group_exclusion)
    {
        maui::controls::grid layout;
        radio_button button1;
        radio_button button2;
        button1.set_group_name("foo");
        button2.set_group_name("foo");
        layout.add(button1);
        layout.add(button2);

        auto handler1 = std::make_shared<radio_button_handler>();
        auto handler2 = std::make_shared<radio_button_handler>();
        button1.set_handler(handler1);
        button2.set_handler(handler2);

        send_control_event(native_button(handler1), UIControlEventTouchUpInside);
        EXPECT_TRUE(button1.is_checked());

        send_control_event(native_button(handler2), UIControlEventTouchUpInside);
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button1.is_checked());
        // The cross-platform uncheck reached the other native button.
        EXPECT_FALSE(native_button(handler1).selected);
        EXPECT_TRUE(native_button(handler2).selected);
    }

    TEST(ios_radio_button_seam, text_style_maps_to_the_native_button)
    {
        radio_button control;
        control.set_content("Styled");
        control.set_font(maui::core::font::of_size("Helvetica", 18));
        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        UIButton* const button = native_button(handler);
        EXPECT_EQ(button.titleLabel.font.pointSize, 18.0);
        EXPECT_NE([button titleColorForState:UIControlStateNormal], nil);
    }

    TEST(ios_radio_button_seam, stroke_and_corner_ride_the_layer)
    {
        radio_button control;
        control.set_stroke_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        control.set_stroke_thickness(2.0);
        control.set_corner_radius(5);
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        UIButton* const button = native_button(handler);
        EXPECT_EQ(button.layer.borderWidth, 2.0);
        EXPECT_EQ(button.layer.cornerRadius, 5.0);
    }

    TEST(ios_radio_button_seam, generic_iview_properties_reach_the_uibutton)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        UIButton* const button = native_button(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(button.enabled);
        control.set_opacity(0.5);
        EXPECT_EQ(button.alpha, 0.5);
        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(button.hidden);
        control.set_automation_id("radio-button-id");
        EXPECT_EQ(to_std_string(button.accessibilityIdentifier), "radio-button-id");
    }

    TEST(ios_radio_button_seam, clearing_handler_disconnects)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_GT(native_button(handler).allTargets.count, 0U); // the proxy registration

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

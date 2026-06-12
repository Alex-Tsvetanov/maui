// Apple (AppKit) backend tests for the radio_button seam — the string content maps to a real radio
// NSButton's title, IsChecked rides the control state, a native click (performClick) flows back as a
// from-handler SELECT (checking the control), and the cross-platform group exclusion pushes the
// uncheck back to the other button's native state. Compiled as Objective-C++ with ARC for the `apple`
// backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/grid.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/core/font.hpp"
#include "maui/core/radio_button_handler.hpp"
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

    NSButton* native_button(const std::shared_ptr<radio_button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    class apple_radio_button_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_radio_button_seam, attaching_handler_creates_a_radio_button_and_maps_content)
    {
        radio_button control;
        control.set_content("Option A");
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        NSButton* const button = native_button(handler);
        EXPECT_TRUE([button isKindOfClass:[NSButton class]]);
        EXPECT_EQ(to_std_string(button.title), "Option A");
        EXPECT_EQ(button.state, NSControlStateValueOff);
    }

    TEST_F(apple_radio_button_seam, is_checked_maps_to_the_control_state)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        control.set_is_checked(true);
        EXPECT_EQ(native_button(handler).state, NSControlStateValueOn);

        control.set_is_checked(false);
        EXPECT_EQ(native_button(handler).state, NSControlStateValueOff);
    }

    TEST_F(apple_radio_button_seam, native_click_selects_the_control)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        bool changed = false;
        control.checked_changed.connect([&changed](bool value) { changed = value; });

        [native_button(handler) performClick:nil]; // a radio click latches the state On + fires the action
        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(changed);
        EXPECT_EQ(native_button(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_radio_button_seam, native_click_drives_the_group_exclusion)
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

        [native_button(handler1) performClick:nil];
        EXPECT_TRUE(button1.is_checked());

        [native_button(handler2) performClick:nil];
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button1.is_checked());
        // The cross-platform uncheck reached the other native button.
        EXPECT_EQ(native_button(handler1).state, NSControlStateValueOff);
        EXPECT_EQ(native_button(handler2).state, NSControlStateValueOn);
    }

    TEST_F(apple_radio_button_seam, disabled_button_suppresses_the_click)
    {
        radio_button control;
        control.set_is_enabled(false);
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(native_button(handler).enabled);

        [native_button(handler) performClick:nil]; // a disabled NSButton swallows the click
        EXPECT_FALSE(control.is_checked());
    }

    TEST_F(apple_radio_button_seam, text_style_maps_to_the_native_button)
    {
        radio_button control;
        control.set_content("Styled");
        control.set_font(maui::core::font::of_size("Helvetica", 18));
        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        NSButton* const button = native_button(handler);
        EXPECT_EQ(button.font.pointSize, 18.0);
        EXPECT_NE(button.contentTintColor, nil);
    }

    TEST_F(apple_radio_button_seam, stroke_and_corner_ride_the_layer)
    {
        radio_button control;
        control.set_stroke_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        control.set_stroke_thickness(2.0);
        control.set_corner_radius(5);
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        NSButton* const button = native_button(handler);
        ASSERT_NE(button.layer, nil);
        EXPECT_EQ(button.layer.borderWidth, 2.0);
        EXPECT_EQ(button.layer.cornerRadius, 5.0);
    }

    TEST_F(apple_radio_button_seam, clearing_handler_disconnects)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

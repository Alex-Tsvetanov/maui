// Apple (AppKit) backend tests for the check_box seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSButton in the native CHECKBOX style (NSButtonTypeSwitch): IsChecked maps to the button
// state, and a native toggle ([NSButton performClick:] flips the state and fires the target-action
// without a run loop) flows back through the handler to the control's `checked_changed` event.
// Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

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

    NSButton* native_check_box(const std::shared_ptr<check_box_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    // NSButton creation needs the shared application object (no run loop required).
    class apple_check_box_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_check_box_seam, attaching_handler_creates_checkbox_nsbutton_and_maps_state)
    {
        check_box control;
        control.set_is_checked(true);
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(native_check_box(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_check_box_seam, setting_is_checked_updates_the_nsbutton)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_check_box(handler).state, NSControlStateValueOff);

        control.set_is_checked(true);
        EXPECT_EQ(native_check_box(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_check_box_seam, native_click_toggles_the_control)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.checked_changed.connect([&reported](bool value) { reported = value; });

        [native_check_box(handler) performClick:nil]; // a real native toggle (state flip + action)

        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(reported);

        [native_check_box(handler) performClick:nil]; // and back off
        EXPECT_FALSE(control.is_checked());
        EXPECT_FALSE(reported);
    }

    TEST_F(apple_check_box_seam, color_maps_to_content_tint_and_clears_to_default)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        NSButton* const view = native_check_box(handler);

        EXPECT_EQ(view.contentTintColor, nil); // Color unset → null foreground → system accent

        control.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        NSColor* const tint = [view.contentTintColor colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
        ASSERT_NE(tint, nil);
        EXPECT_NEAR(tint.redComponent, 1.0, 0.01);
        EXPECT_NEAR(tint.greenComponent, 0.0, 0.01);
    }

    TEST_F(apple_check_box_seam, generic_iview_properties_reach_the_nsbutton)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        NSButton* const view = native_check_box(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);

        control.set_automation_id("accept_terms");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "accept_terms");
    }

    TEST_F(apple_check_box_seam, clearing_handler_disconnects)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_check_box_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<check_box>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<check_box_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        check_box control;
        control.set_is_checked(true);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSButton*)resolved->typed_platform_view()->native).state, NSControlStateValueOn);
    }
} // namespace

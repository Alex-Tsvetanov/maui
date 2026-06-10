// Apple (AppKit) backend tests for the toggle_switch seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSSwitch: IsToggled maps to NSSwitch.state, and a native toggle ([NSSwitch performClick:]
// flips the state and fires the target-action without a run loop) flows back through the handler to
// the control's `toggled` event. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/toggle_switch.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::toggle_switch;
    using maui::core::i_element_handler;
    using maui::core::switch_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSSwitch* native_switch(const std::shared_ptr<switch_handler>& handler)
    {
        return (__bridge NSSwitch*)handler->typed_platform_view()->native;
    }

    // NSSwitch creation needs the shared application object (no run loop required).
    class apple_switch_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_switch_seam, attaching_handler_creates_nsswitch_and_maps_state)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_switch_seam, setting_is_toggled_updates_the_nsswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOff);

        control.set_is_toggled(true);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_switch_seam, native_click_toggles_the_control)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.toggled.connect([&reported](bool value) { reported = value; });

        [native_switch(handler) performClick:nil]; // a real native toggle (state flip + action)

        EXPECT_TRUE(control.is_toggled());
        EXPECT_TRUE(reported);

        [native_switch(handler) performClick:nil]; // and back off
        EXPECT_FALSE(control.is_toggled());
        EXPECT_FALSE(reported);
    }

    TEST_F(apple_switch_seam, track_and_thumb_colors_record_the_mirrors)
    {
        // AppKit deviation (documented in switch_handler.mm): NSSwitch exposes no public tint API, so
        // the colors land on the cross-platform mirrors — the observable native-adjacent state.
        toggle_switch control;
        control.set_is_toggled(true);
        control.set_on_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->track_color, maui::graphics::color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(handler->typed_platform_view()->thumb_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_is_toggled(false);
        control.set_off_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        EXPECT_EQ(handler->typed_platform_view()->track_color, maui::graphics::color(0.0F, 0.0F, 1.0F));
    }

    TEST_F(apple_switch_seam, generic_iview_properties_reach_the_nsswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        NSSwitch* const view = native_switch(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);

        control.set_automation_id("dark_mode_switch");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "dark_mode_switch");
    }

    TEST_F(apple_switch_seam, clearing_handler_disconnects)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_switch_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<toggle_switch>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<switch_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        toggle_switch control;
        control.set_is_toggled(true);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSSwitch*)resolved->typed_platform_view()->native).state, NSControlStateValueOn);
    }
} // namespace

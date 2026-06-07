// Apple (AppKit) backend tests for the button seam — the real-native half of the M2 Rosetta Stone,
// run only for MAUI_BACKEND=apple. Drives a genuine NSButton: Text maps to NSButton.title, and a
// native click ([NSButton performClick:] fires the target-action without a run loop) flows back through
// the handler to the control's `clicked` event. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::core::button_handler;
    using maui::core::i_element_handler;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    // NSButton creation needs the shared application object (no run loop required).
    class apple_button_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_button_seam, attaching_handler_creates_nsbutton_and_maps_text)
    {
        button control;
        control.set_text("Start");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_button(handler).title), "Start");
    }

    TEST_F(apple_button_seam, setting_text_updates_nsbutton_title)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        control.set_text("Changed");
        EXPECT_EQ(to_std_string(native_button(handler).title), "Changed");
    }

    TEST_F(apple_button_seam, native_click_flows_back_to_clicked_event)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });

        [native_button(handler) performClick:nil]; // simulate a real tap

        EXPECT_EQ(clicks, 1);
    }

    TEST_F(apple_button_seam, disabled_button_click_is_suppressed)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        control.set_is_enabled(false);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        [native_button(handler) performClick:nil];

        EXPECT_EQ(clicks, 0);
    }

    TEST_F(apple_button_seam, clearing_handler_disconnects)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_button_seam, appearance_maps_to_nsbutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(view.font.pointSize, 18.0);

        control.set_stroke_thickness(3.0);
        EXPECT_TRUE(view.wantsLayer);
        EXPECT_EQ(view.layer.borderWidth, 3.0);

        control.set_corner_radius(7);
        EXPECT_EQ(view.layer.cornerRadius, 7.0);
    }

    TEST_F(apple_button_seam, handler_resolved_from_default_registry)
    {
        // button -> button_handler is self-registered in button.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<button>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<button_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        button control;
        control.set_text("Registered");
        control.set_handler(handler);
        auto const button_view = (__bridge NSButton*)resolved->typed_platform_view()->native;
        EXPECT_EQ(to_std_string(button_view.title), "Registered");
    }
} // namespace

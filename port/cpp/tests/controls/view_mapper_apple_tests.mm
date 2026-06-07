// Apple (AppKit) backend tests for the shared ViewMapper — the generic IView properties (Visibility /
// Opacity / IsEnabled / AutomationId) pushed to a real NSView via view_platform_base overrides. Run only
// for MAUI_BACKEND=apple. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::label;
    using maui::core::button_handler;
    using maui::core::label_handler;
    using maui::core::visibility;

    NSButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    struct apple_view_mapper : ::testing::Test
    {
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_view_mapper, button_generic_properties_push_to_nsbutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);
        control.set_visibility(visibility::visible);
        EXPECT_FALSE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);
        control.set_is_enabled(true);
        EXPECT_TRUE(view.enabled);

        control.set_automation_id("submit_button");
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "submit_button");
    }

    TEST_F(apple_view_mapper, label_generic_properties_push_to_nstextfield)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.25);
        EXPECT_EQ(view.alphaValue, 0.25);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("caption");
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "caption");
    }
} // namespace

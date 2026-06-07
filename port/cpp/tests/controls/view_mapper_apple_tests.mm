// Apple (AppKit) backend tests for the shared ViewMapper — the generic IView properties (Visibility /
// Opacity / IsEnabled / AutomationId) pushed to a real NSView via view_platform_base overrides. Run only
// for MAUI_BACKEND=apple. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::entry;
    using maui::controls::image;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::button_handler;
    using maui::core::entry_handler;
    using maui::core::image_handler;
    using maui::core::label_handler;
    using maui::core::layout_handler;
    using maui::core::visibility;

    NSButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    NSTextField* native_entry(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    NSImageView* native_image(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge NSImageView*)handler->typed_platform_view()->native;
    }

    NSView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
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

    TEST_F(apple_view_mapper, entry_generic_properties_push_to_nstextfield)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_entry(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.6);
        EXPECT_EQ(view.alphaValue, 0.6);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("email_entry");
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "email_entry");
    }

    TEST_F(apple_view_mapper, image_generic_properties_push_to_nsimageview)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image(handler);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.3);
        EXPECT_EQ(view.alphaValue, 0.3);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("avatar");
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "avatar");
    }

    // The layout panel is a plain NSView — it has no native enabled state, so is_enabled is not asserted
    // here (it keeps the headless base mirror, covered by the headless view_mapper_layout test).
    TEST_F(apple_view_mapper, layout_generic_properties_push_to_nsview)
    {
        vertical_stack_layout control;
        auto handler = std::make_shared<layout_handler>();
        control.set_handler(handler);
        NSView* const view = native_panel(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.4);
        EXPECT_EQ(view.alphaValue, 0.4);

        control.set_automation_id("form_stack");
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "form_stack");
    }
} // namespace

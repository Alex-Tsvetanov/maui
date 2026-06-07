// Apple (AppKit) backend tests for the entry seam — properties pushed to a real editable NSTextField
// (stringValue / placeholderString / editable / secure-cell), and a native end-of-edit notification
// flowing back to the control's `completed` event. Compiled as Objective-C++ with ARC for the `apple`
// backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/entry.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/text_alignment.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::entry;
    using maui::core::entry_handler;
    using maui::core::text_alignment;

    NSTextField* native_field(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    struct apple_entry_seam : ::testing::Test
    {
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_entry_seam, maps_text_and_placeholder_to_nstextfield)
    {
        entry control;
        control.set_text("Start");
        control.set_placeholder("Hint");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(std::string(native_field(handler).stringValue.UTF8String), "Start");
        NSString* const placeholder = native_field(handler).placeholderString;
        ASSERT_NE(placeholder, nil);
        EXPECT_EQ(std::string(placeholder.UTF8String), "Hint");
    }

    TEST_F(apple_entry_seam, field_is_editable_by_default_and_read_only_toggles_it)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_field(handler).editable);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_field(handler).editable);

        control.set_is_read_only(false);
        EXPECT_TRUE(native_field(handler).editable);
    }

    TEST_F(apple_entry_seam, password_uses_a_secure_cell)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_FALSE([native_field(handler).cell isKindOfClass:[NSSecureTextFieldCell class]]);

        control.set_is_password(true);
        EXPECT_TRUE([native_field(handler).cell isKindOfClass:[NSSecureTextFieldCell class]]);
    }

    TEST_F(apple_entry_seam, toggling_password_at_runtime_preserves_font_and_text)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        control.set_text("secret");
        control.set_font(maui::core::font::of_size("Helvetica", 18));
        // Toggling to a secure cell must not drop the already-applied font or the text.
        control.set_is_password(true);
        EXPECT_EQ(native_field(handler).font.pointSize, 18.0);
        EXPECT_EQ(std::string(native_field(handler).stringValue.UTF8String), "secret");
    }

    TEST_F(apple_entry_seam, maps_font_and_alignment)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(native_field(handler).font.pointSize, 18.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_field(handler).alignment, NSTextAlignmentCenter);
    }

    TEST_F(apple_entry_seam, end_of_edit_notification_fires_completed)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });

        // Simulate AppKit posting the end-of-editing notification to the field's delegate.
        NSTextField* const field = native_field(handler);
        NSNotification* const note = [NSNotification notificationWithName:NSControlTextDidEndEditingNotification
                                                                   object:field];
        [(id<NSTextFieldDelegate>)field.delegate controlTextDidEndEditing:note];
        EXPECT_EQ(completes, 1);
    }

    TEST_F(apple_entry_seam, clearing_handler_disconnects)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

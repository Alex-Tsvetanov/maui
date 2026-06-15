// Apple (AppKit) backend tests for the search_bar seam — properties pushed to a real NSSearchField,
// and the native edit / search-action events flowing back to the control's text_changed /
// search_button_pressed events. Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "apple_text_ops.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::search_bar;
    using maui::core::search_bar_handler;
    using maui::core::text_alignment;
    using maui::platform::apple::kerning_of;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSSearchField* native_field(const std::shared_ptr<search_bar_handler>& handler)
    {
        return (__bridge NSSearchField*)handler->typed_platform_view()->native;
    }

    class apple_search_bar_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_search_bar_seam, creates_a_search_field)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_field(handler) isKindOfClass:[NSSearchField class]]);
        EXPECT_TRUE(native_field(handler).sendsWholeSearchString);
    }

    TEST_F(apple_search_bar_seam, maps_text_and_placeholder)
    {
        search_bar control;
        control.set_text("query");
        control.set_placeholder("Search…");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        EXPECT_EQ(to_std_string(native_field(handler).stringValue), "query");
        ASSERT_NE(native_field(handler).placeholderString, nil);
        EXPECT_EQ(to_std_string(native_field(handler).placeholderString), "Search…");
    }

    TEST_F(apple_search_bar_seam, read_only_toggles_editable)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_field(handler).editable);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_field(handler).editable);
    }

    TEST_F(apple_search_bar_seam, maps_font_and_alignment)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 15));
        EXPECT_EQ(native_field(handler).font.pointSize, 15.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_field(handler).alignment, NSTextAlignmentCenter);
    }

    TEST_F(apple_search_bar_seam, character_spacing_kerns_the_text)
    {
        search_bar control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_field(handler).attributedStringValue), 4.0);
    }

    TEST_F(apple_search_bar_seam, max_length_trims_existing_text)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        native_field(handler).stringValue = @"abcdef";
        control.set_max_length(3);
        EXPECT_EQ(to_std_string(native_field(handler).stringValue), "abc");
    }

    TEST_F(apple_search_bar_seam, cancel_and_icon_colors_recorded_on_the_mirror)
    {
        // The NSSearchFieldCell's cancel/loupe button cells have no public tint (documented deviation);
        // the mirrors record what the mapper pushed.
        search_bar control;
        control.set_cancel_button_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        control.set_search_icon_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->cancel_button_color, maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(handler->typed_platform_view()->search_icon_color, maui::graphics::color(0.0F, 0.0F, 1.0F));
    }

    TEST_F(apple_search_bar_seam, native_edit_notification_fires_text_changed)
    {
        search_bar control;
        control.set_text("ab");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        NSSearchField* const field = native_field(handler);
        field.stringValue = @"abc";
        NSNotification* const note = [NSNotification notificationWithName:NSControlTextDidChangeNotification
                                                                   object:field];
        [(id<NSSearchFieldDelegate>)field.delegate controlTextDidChange:note];

        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "ab");
        EXPECT_EQ(changes[0].second, "abc");
    }

    TEST_F(apple_search_bar_seam, search_action_fires_search_button_pressed)
    {
        search_bar control;
        control.set_text("find me");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        int presses = 0;
        control.search_button_pressed.connect([&presses] { ++presses; });

        // Drive the field's target-action the way AppKit does when the user commits the search.
        NSSearchField* const field = native_field(handler);
        ASSERT_NE(field.target, nil);
        [NSApp sendAction:field.action to:field.target from:field];
        EXPECT_EQ(presses, 1);
    }

    TEST_F(apple_search_bar_seam, generic_iview_properties_reach_the_field)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(native_field(handler).enabled);

        control.set_opacity(0.25);
        EXPECT_NEAR(native_field(handler).alphaValue, 0.25, 0.001);

        control.set_automation_id("search-id");
        EXPECT_EQ(to_std_string(native_field(handler).accessibilityIdentifier), "search-id");
    }

    // Keyboard (W8-53): AppKit has no soft keyboard — MapKeyboard records the cross-platform mirror only.
    TEST_F(apple_search_bar_seam, keyboard_records_mirror_only)
    {
        search_bar control;
        control.set_keyboard(maui::core::keyboard::email());
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->keyboard, maui::core::keyboard::email());
    }

    // Focus (W8-53): the native focus-callback path funnels Focused/Unfocused via set_is_focused.
    TEST_F(apple_search_bar_seam, set_is_focused_funnels_events)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        int focused_count = 0;
        int unfocused_count = 0;
        control.focused.connect([&focused_count](bool) { ++focused_count; });
        control.unfocused.connect([&unfocused_count](bool) { ++unfocused_count; });

        control.set_is_focused(true);
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(focused_count, 1);
        control.set_is_focused(false);
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(unfocused_count, 1);
    }

    TEST_F(apple_search_bar_seam, clearing_handler_disconnects)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

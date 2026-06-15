// Apple (AppKit) backend tests for the editor seam — properties pushed to a real NSTextView (hosted in
// an NSScrollView), and the native edit / end-of-edit notifications flowing back to the control's
// text_changed / completed events. Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/editor.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::editor;
    using maui::core::editor_handler;
    using maui::core::text_alignment;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSScrollView* native_scroll(const std::shared_ptr<editor_handler>& handler)
    {
        return (__bridge NSScrollView*)handler->typed_platform_view()->native;
    }

    NSTextView* native_text_view(const std::shared_ptr<editor_handler>& handler)
    {
        return (NSTextView*)native_scroll(handler).documentView;
    }

    class apple_editor_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_editor_seam, creates_scrollable_text_view)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_text_view(handler) isKindOfClass:[NSTextView class]]);
        EXPECT_TRUE(native_text_view(handler).editable);
    }

    TEST_F(apple_editor_seam, maps_text_to_nstextview)
    {
        editor control;
        control.set_text("Multi\nline");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_EQ(to_std_string(native_text_view(handler).string), "Multi\nline");
    }

    TEST_F(apple_editor_seam, placeholder_label_shows_until_text_is_present)
    {
        editor control;
        control.set_placeholder("Type here");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        // The placeholder label (the MauiTextView.PlaceholderLabel translation) is an NSTextField
        // subview of the text view, visible while the editor is empty.
        NSTextField* label = nil;
        for (NSView* subview in native_text_view(handler).subviews)
        {
            if ([subview isKindOfClass:[NSTextField class]])
            {
                label = (NSTextField*)subview;
            }
        }
        ASSERT_NE(label, nil);
        EXPECT_EQ(to_std_string(label.stringValue), "Type here");
        EXPECT_FALSE(label.hidden);

        control.set_text("content");
        EXPECT_TRUE(label.hidden);

        control.set_text("");
        EXPECT_FALSE(label.hidden);
    }

    TEST_F(apple_editor_seam, read_only_toggles_editable)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_text_view(handler).editable);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_text_view(handler).editable);

        control.set_is_read_only(false);
        EXPECT_TRUE(native_text_view(handler).editable);
    }

    TEST_F(apple_editor_seam, maps_font_and_alignment)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(native_text_view(handler).font.pointSize, 18.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_text_view(handler).alignment, NSTextAlignmentCenter);
    }

    TEST_F(apple_editor_seam, max_length_trims_existing_text)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        // Drive the native view longer than the cap (bypassing the control's own truncation), then map.
        [native_text_view(handler) setString:@"abcdef"];
        control.set_max_length(3);
        EXPECT_EQ(to_std_string(native_text_view(handler).string), "abc");
    }

    TEST_F(apple_editor_seam, prediction_and_spellcheck_map_to_text_view)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_is_text_prediction_enabled(false);
        EXPECT_FALSE(native_text_view(handler).automaticTextReplacementEnabled);

        control.set_is_spell_check_enabled(false);
        EXPECT_FALSE(native_text_view(handler).continuousSpellCheckingEnabled);
    }

    TEST_F(apple_editor_seam, cursor_and_selection_move_the_native_range)
    {
        editor control;
        control.set_text("Hello world");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_cursor_position(6);
        control.set_selection_length(5);
        const NSRange selection = native_text_view(handler).selectedRange;
        EXPECT_EQ(selection.location, 6U);
        EXPECT_EQ(selection.length, 5U);
    }

    TEST_F(apple_editor_seam, native_edit_notification_fires_text_changed)
    {
        editor control;
        control.set_text("ab");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        // Simulate the user typing: mutate the text view, then post the did-change notification to the
        // delegate (what AppKit does after a keystroke).
        NSTextView* const text_view = native_text_view(handler);
        [text_view setString:@"abc"];
        NSNotification* const note = [NSNotification notificationWithName:NSTextDidChangeNotification object:text_view];
        [(id<NSTextViewDelegate>)text_view.delegate textDidChange:note];

        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "ab");
        EXPECT_EQ(changes[0].second, "abc");
    }

    TEST_F(apple_editor_seam, end_of_edit_notification_fires_completed)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });

        NSTextView* const text_view = native_text_view(handler);
        NSNotification* const note = [NSNotification notificationWithName:NSTextDidEndEditingNotification
                                                                   object:text_view];
        [(id<NSTextViewDelegate>)text_view.delegate textDidEndEditing:note];
        EXPECT_EQ(completes, 1);
    }

    TEST_F(apple_editor_seam, should_change_text_enforces_max_length)
    {
        editor control;
        control.set_text("abc");
        control.set_max_length(3);
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        NSTextView* const text_view = native_text_view(handler);
        auto* const delegate = (id<NSTextViewDelegate>)text_view.delegate;
        // Appending beyond the cap is rejected; replacing within it is allowed.
        EXPECT_FALSE([delegate textView:text_view shouldChangeTextInRange:NSMakeRange(3, 0) replacementString:@"d"]);
        EXPECT_TRUE([delegate textView:text_view shouldChangeTextInRange:NSMakeRange(2, 1) replacementString:@"x"]);
    }

    TEST_F(apple_editor_seam, generic_iview_properties_reach_the_scroll_view)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_opacity(0.5);
        EXPECT_NEAR(native_scroll(handler).alphaValue, 0.5, 0.001);

        control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_TRUE(native_scroll(handler).hidden);

        control.set_automation_id("editor-id");
        EXPECT_EQ(to_std_string(native_scroll(handler).accessibilityIdentifier), "editor-id");
    }

    // Keyboard (W8-53): AppKit has no soft keyboard — MapKeyboard records the cross-platform mirror only.
    TEST_F(apple_editor_seam, keyboard_records_mirror_only)
    {
        editor control;
        control.set_keyboard(maui::core::keyboard::numeric());
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->keyboard, maui::core::keyboard::numeric());
    }

    // Focus (W8-53): the native focus-callback path funnels Focused/Unfocused via set_is_focused.
    TEST_F(apple_editor_seam, set_is_focused_funnels_events)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
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

    TEST_F(apple_editor_seam, clearing_handler_disconnects)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }
} // namespace

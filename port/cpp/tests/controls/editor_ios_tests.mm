// iOS (UIKit) backend tests for the editor seam, run ON the simulator — properties pushed to a real
// UITextView (the MauiTextView port with its placeholder label), and the native delegate callbacks
// flowing back to the control's text_changed / completed events. Compiled as Objective-C++ with ARC for
// the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ios_text_ops.hpp"
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
    using maui::core::keyboard;
    using maui::core::text_alignment;
    using maui::platform::ios::kerning_of;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITextView* native_text_view(const std::shared_ptr<editor_handler>& handler)
    {
        return (__bridge UITextView*)handler->typed_platform_view()->native;
    }

    UILabel* placeholder_label(UITextView* text_view)
    {
        for (UIView* subview in text_view.subviews)
        {
            if ([subview isKindOfClass:[UILabel class]])
            {
                return (UILabel*)subview;
            }
        }
        return nil;
    }

    TEST(ios_editor_seam, maps_text_to_uitextview)
    {
        editor control;
        control.set_text("Multi\nline");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_text_view(handler).text), "Multi\nline");
    }

    TEST(ios_editor_seam, placeholder_label_shows_until_text_is_present)
    {
        editor control;
        control.set_placeholder("Type here");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        UILabel* const label = placeholder_label(native_text_view(handler));
        ASSERT_NE(label, nil);
        EXPECT_EQ(to_std_string(label.text), "Type here");
        EXPECT_FALSE(label.hidden);

        control.set_text("content");
        EXPECT_TRUE(label.hidden);

        control.set_text("");
        EXPECT_FALSE(label.hidden);
    }

    TEST(ios_editor_seam, placeholder_color_tints_the_label)
    {
        editor control;
        control.set_placeholder("Hint");
        control.set_placeholder_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        UILabel* const label = placeholder_label(native_text_view(handler));
        ASSERT_NE(label, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        [label.textColor getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(green, 0.0, 0.01);
    }

    TEST(ios_editor_seam, read_only_toggles_editable)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_text_view(handler).editable);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_text_view(handler).editable);
    }

    TEST(ios_editor_seam, maps_font_and_alignment)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(native_text_view(handler).font.pointSize, 18.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_text_view(handler).textAlignment, NSTextAlignmentCenter);
    }

    TEST(ios_editor_seam, character_spacing_kerns_the_text)
    {
        editor control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_text_view(handler).attributedText), 4.0);
    }

    TEST(ios_editor_seam, prediction_and_spellcheck_map_to_native_types)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_is_text_prediction_enabled(false);
        EXPECT_EQ(native_text_view(handler).autocorrectionType, UITextAutocorrectionTypeNo);

        control.set_is_spell_check_enabled(false);
        EXPECT_EQ(native_text_view(handler).spellCheckingType, UITextSpellCheckingTypeNo);
    }

    TEST(ios_editor_seam, max_length_trims_existing_text)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        native_text_view(handler).text = @"abcdef";
        control.set_max_length(3);
        EXPECT_EQ(to_std_string(native_text_view(handler).text), "abc");
    }

    TEST(ios_editor_seam, should_change_text_enforces_max_length)
    {
        editor control;
        control.set_text("abc");
        control.set_max_length(3);
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        UITextView* const text_view = native_text_view(handler);
        auto* const delegate = (id<UITextViewDelegate>)text_view.delegate;
        EXPECT_FALSE([delegate textView:text_view shouldChangeTextInRange:NSMakeRange(3, 0) replacementText:@"d"]);
        EXPECT_TRUE([delegate textView:text_view shouldChangeTextInRange:NSMakeRange(2, 1) replacementText:@"x"]);
    }

    TEST(ios_editor_seam, native_edit_callback_fires_text_changed)
    {
        editor control;
        control.set_text("ab");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        UITextView* const text_view = native_text_view(handler);
        text_view.text = @"abc";
        [(id<UITextViewDelegate>)text_view.delegate textViewDidChange:text_view];

        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "ab");
        EXPECT_EQ(changes[0].second, "abc");
    }

    TEST(ios_editor_seam, end_of_edit_fires_completed)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });

        UITextView* const text_view = native_text_view(handler);
        [(id<UITextViewDelegate>)text_view.delegate textViewDidEndEditing:text_view];
        EXPECT_EQ(completes, 1);
    }

    TEST(ios_editor_seam, cursor_and_selection_move_the_native_range)
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

    TEST(ios_editor_seam, generic_iview_properties_reach_the_uitextview)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        control.set_opacity(0.5);
        EXPECT_NEAR(native_text_view(handler).alpha, 0.5, 0.001);

        control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_TRUE(native_text_view(handler).hidden);

        control.set_is_enabled(false);
        EXPECT_FALSE(native_text_view(handler).userInteractionEnabled);

        control.set_automation_id("editor-id");
        EXPECT_EQ(to_std_string(native_text_view(handler).accessibilityIdentifier), "editor-id");
    }

    // Keyboard (W8-53): UpdateKeyboard sets the UITextView's keyboardType.
    TEST(ios_editor_seam, keyboard_maps_to_native_type)
    {
        editor control;
        control.set_keyboard(keyboard::email());
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_text_view(handler).keyboardType, UIKeyboardTypeEmailAddress);

        control.set_keyboard(keyboard::numeric());
        EXPECT_EQ(native_text_view(handler).keyboardType, UIKeyboardTypeDecimalPad);
    }

    // AddMauiDoneAccessoryView: the text view carries a Done input-accessory toolbar.
    TEST(ios_editor_seam, has_done_input_accessory_toolbar)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        UITextView* const text_view = native_text_view(handler);
        ASSERT_NE(text_view.inputAccessoryView, nil);
        EXPECT_TRUE([text_view.inputAccessoryView isKindOfClass:[UIToolbar class]]);
    }

    // Focus (W8-53): the begin/end editing delegate callbacks reflect IsFocused (firing Focused/Unfocused).
    TEST(ios_editor_seam, begin_end_editing_reflect_is_focused)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        UITextView* const text_view = native_text_view(handler);
        auto* const delegate = (id<UITextViewDelegate>)text_view.delegate;

        int focused_count = 0;
        int unfocused_count = 0;
        control.focused.connect([&focused_count](bool) { ++focused_count; });
        control.unfocused.connect([&unfocused_count](bool) { ++unfocused_count; });

        [delegate textViewDidBeginEditing:text_view];
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(focused_count, 1);

        [delegate textViewDidEndEditing:text_view];
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(unfocused_count, 1);
    }

    TEST(ios_editor_seam, clearing_handler_disconnects)
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

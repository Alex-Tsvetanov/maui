// Apple (AppKit) backend tests for the entry seam — properties pushed to a real editable NSTextField
// (stringValue / placeholderString / editable / secure-cell), and a native end-of-edit notification
// flowing back to the control's `completed` event. Compiled as Objective-C++ with ARC for the `apple`
// backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "apple_text_ops.hpp"
#include "maui/controls/entry.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::entry;
    using maui::core::clear_button_visibility;
    using maui::core::entry_handler;
    using maui::core::return_type;
    using maui::core::text_alignment;
    using maui::platform::apple::kerning_of;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSTextField* native_field(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    class apple_entry_seam : public ::testing::Test
    {
    protected:
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
        EXPECT_EQ(to_std_string(native_field(handler).stringValue), "Start");
        NSString* const placeholder = native_field(handler).placeholderString;
        ASSERT_NE(placeholder, nil);
        EXPECT_EQ(to_std_string(placeholder), "Hint");
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
        EXPECT_EQ(to_std_string(native_field(handler).stringValue), "secret");
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

    // Ports EntryHandlerTests.iOS CharacterSpacing: the kerning on the attributed text equals the
    // cross-platform CharacterSpacing.
    TEST_F(apple_entry_seam, character_spacing_kerns_the_text)
    {
        entry control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        NSTextField* const field = native_field(handler);
        EXPECT_EQ(kerning_of(field.attributedStringValue), 4.0);
        EXPECT_EQ(to_std_string(field.stringValue), "Some Test Text");
    }

    // Setting character_spacing back to 0 removes the prior kerning (C# WithCharacterSpacing un-sets it).
    TEST_F(apple_entry_seam, clearing_character_spacing_reverts_to_plain_text)
    {
        entry control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_field(handler).attributedStringValue), 4.0);

        control.set_character_spacing(0);
        EXPECT_EQ(kerning_of(native_field(handler).attributedStringValue), 0.0);
        EXPECT_EQ(to_std_string(native_field(handler).stringValue), "Some Test Text");
    }

    // A plain placeholder (no color, no kerning) stays a readable placeholderString.
    TEST_F(apple_entry_seam, plain_placeholder_stays_readable)
    {
        entry control;
        control.set_placeholder("Hint");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        ASSERT_NE(native_field(handler).placeholderString, nil);
        EXPECT_EQ(to_std_string(native_field(handler).placeholderString), "Hint");
    }

    // placeholder_color builds an attributed placeholder carrying the foreground color (UpdatePlaceholder).
    TEST_F(apple_entry_seam, placeholder_color_sets_attributed_placeholder_foreground)
    {
        entry control;
        control.set_placeholder("Hint");
        control.set_placeholder_color(maui::graphics::color(1.0F, 0.0F, 0.5F));
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        NSAttributedString* const ph = native_field(handler).placeholderAttributedString;
        ASSERT_NE(ph, nil);
        EXPECT_EQ(to_std_string(ph.string), "Hint");
        NSColor* const fg = [ph attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:nullptr];
        ASSERT_NE(fg, nil);
        NSColor* const srgb = [fg colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
        EXPECT_NEAR(srgb.redComponent, 1.0, 0.01);
        EXPECT_NEAR(srgb.blueComponent, 0.5, 0.01);
    }

    // character_spacing kerns the placeholder too (TextFieldExtensions.UpdateCharacterSpacing's
    // placeholder branch).
    TEST_F(apple_entry_seam, character_spacing_kerns_the_placeholder)
    {
        entry control;
        control.set_placeholder("Hint");
        control.set_character_spacing(3);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_field(handler).placeholderAttributedString), 3.0);
    }

    // vertical_text_alignment reaches the custom editable cell (and survives the password cell swap).
    TEST_F(apple_entry_seam, vertical_text_alignment_is_stored_and_survives_password_swap)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        NSTextField* const field = native_field(handler);
        ASSERT_TRUE([field.cell respondsToSelector:@selector(verticalAlignment)]);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ((int)[(id)field.cell verticalAlignment], (int)text_alignment::end);

        // The secure-cell swap must preserve the vertical alignment.
        control.set_is_password(true);
        ASSERT_TRUE([field.cell isKindOfClass:[NSSecureTextFieldCell class]]);
        ASSERT_TRUE([field.cell respondsToSelector:@selector(verticalAlignment)]);
        EXPECT_EQ((int)[(id)field.cell verticalAlignment], (int)text_alignment::end);
    }

    // Ports EntryHandlerTests.iOS ReturnTypeInitializesCorrectly (mirror-only on AppKit — no software
    // return-key styling on desktop).
    TEST_F(apple_entry_seam, return_type_and_clear_button_recorded)
    {
        entry control;
        control.set_return_type(return_type::next);
        control.set_clear_button_visibility(clear_button_visibility::while_editing);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        auto* const platform = handler->typed_platform_view();
        EXPECT_EQ(platform->entry_return_type, return_type::next);
        EXPECT_EQ(platform->clear_button, clear_button_visibility::while_editing);
    }

    // Prediction / spellcheck have no static NSTextField property without a field editor; the mirror
    // records the requested state (GetNativeIsTextPredictionEnabled analog).
    TEST_F(apple_entry_seam, prediction_and_spellcheck_recorded)
    {
        entry control;
        control.set_is_text_prediction_enabled(false);
        control.set_is_spell_check_enabled(false);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        auto* const platform = handler->typed_platform_view();
        EXPECT_FALSE(platform->is_text_prediction_enabled);
        EXPECT_FALSE(platform->is_spell_check_enabled);
    }

    // Cursor/selection: without a window/first-responder there is no field editor, so the mirror records
    // the values (the live field-editor selection is exercised by the cross-platform send-back path).
    TEST_F(apple_entry_seam, cursor_and_selection_recorded)
    {
        entry control;
        control.set_text("abcdef");
        control.set_cursor_position(2);
        control.set_selection_length(3);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        auto* const platform = handler->typed_platform_view();
        EXPECT_EQ(platform->cursor_position, 2);
        EXPECT_EQ(platform->selection_length, 3);
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

    // Keyboard (W8-53): AppKit has no soft keyboard, so MapKeyboard records the cross-platform mirror only
    // (the documented deviation) — there is no UIKeyboardType analog to assert on the NSTextField.
    TEST_F(apple_entry_seam, keyboard_records_mirror_only)
    {
        entry control;
        control.set_keyboard(maui::core::keyboard::email());
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->keyboard, maui::core::keyboard::email());

        control.set_keyboard(maui::core::keyboard::numeric());
        EXPECT_EQ(handler->typed_platform_view()->keyboard, maui::core::keyboard::numeric());
    }

    // Focus (W8-53): the shared view_command_mapper drives the apple view_focus_ops path (window
    // makeFirstResponder:). A field NOT yet hosted in a window cannot join a responder chain, so
    // focus_native_view returns false (AppKit's "no window" guard) and the state machine stays
    // consistent — focus() reports false and IsFocused is not set. (The real key-window first-responder
    // round trip needs an on-screen window + run loop, which in-process AppKit global state makes flaky to
    // host here; the cross-platform focus state machine is verified headless in entry_tests.cpp.)
    TEST_F(apple_entry_seam, focus_without_a_window_does_not_take_focus)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        int focused_count = 0;
        control.focused.connect([&focused_count](bool) { ++focused_count; });

        EXPECT_FALSE(control.focus()); // no window → makeFirstResponder cannot run
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(focused_count, 0);
    }

    // The native focus-callback path still funnels Focused/Unfocused on apple (a backend setting
    // IsFocused directly — the AppKit window-did-become/resign-key analog the mapper relies on).
    TEST_F(apple_entry_seam, set_is_focused_funnels_events)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
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
} // namespace

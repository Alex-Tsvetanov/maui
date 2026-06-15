// iOS (UIKit) backend tests for the entry seam — properties pushed to a real UITextField (the
// MauiIosTextField subclass), and native editing flowing back through the handler's target-action +
// delegate proxy to the control's text_changed / completed events. Run only for MAUI_BACKEND=ios
// (executed ON the iOS simulator via tools/ios-sim-run.sh). Mirrors the AppKit twin's coverage
// (entry_apple_tests.mm) plus the UIKit-real pieces the desktop twin could only mirror: returnKeyType,
// clearButtonMode, autocorrectionType/spellCheckingType, secureTextEntry, contentVerticalAlignment, and
// the ShouldChangeCharacters max-length gate. Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION (send_control_event below) replicates -[UIControl sendActionsForControlEvents:]'s
// dispatch-table walk, because a spawned simulator process has no UIApplication to relay the actions —
// see the full rationale in button_ios_tests.mm (the M6 Rosetta Stone). Editing sessions (first
// responder) need a UIWindow + UIApplication, so the live selectedTextRange paths are exercised by the
// app-bundle test-host lane (deferred with the scaffold); here the cursor/selection maps record their
// mirrors, exactly like the AppKit twin without a field editor.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ios_text_ops.hpp"
#include "maui/controls/entry.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/keyboard_flags.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::entry;
    using maui::core::clear_button_visibility;
    using maui::core::entry_handler;
    using maui::core::keyboard;
    using maui::core::keyboard_flags;
    using maui::core::return_type;
    using maui::core::text_alignment;
    using maui::platform::ios::kerning_of;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITextField* native_field(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge UITextField*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see the
    // header comment + button_ios_tests.mm): every (target, action) pair registered for `event` is
    // invoked with the control as sender, exactly as UIApplication's sendAction:to:from:forEvent: would.
    void send_control_event(UIControl* control, UIControlEvents event)
    {
        NSArray* const targets = control.allTargets.allObjects;
        for (NSUInteger t = 0; t < targets.count; ++t)
        {
            id const target = targets[t];
            NSArray<NSString*>* const actions = [control actionsForTarget:target forControlEvent:event];
            for (NSUInteger a = 0; a < actions.count; ++a)
            {
                SEL const action = NSSelectorFromString(actions[a]);
                NSMethodSignature* const signature = [target methodSignatureForSelector:action];
                ASSERT_NE(signature, nil);
                NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
                invocation.selector = action;
                id sender = control;
                [invocation setArgument:&sender atIndex:2]; // 0 = self, 1 = _cmd, 2 = the sender
                [invocation invokeWithTarget:target];
            }
        }
    }

    TEST(ios_entry_seam, maps_text_and_placeholder_to_uitextfield)
    {
        entry control;
        control.set_text("Start");
        control.set_placeholder("Hint");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UITextField* const field = native_field(handler);
        EXPECT_EQ(to_std_string(field.text), "Start");
        ASSERT_NE(field.attributedPlaceholder, nil);
        EXPECT_EQ(to_std_string(field.attributedPlaceholder.string), "Hint");
    }

    TEST(ios_entry_seam, created_field_matches_the_create_platform_view_recipe)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        // CreatePlatformView: BorderStyle = RoundedRect, ClipsToBounds = true.
        EXPECT_EQ(field.borderStyle, UITextBorderStyleRoundedRect);
        EXPECT_TRUE(field.clipsToBounds);
    }

    // TextFieldExtensions.UpdateIsReadOnly: read-only disables interaction (UserInteractionEnabled).
    TEST(ios_entry_seam, read_only_toggles_user_interaction)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_field(handler).userInteractionEnabled);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_field(handler).userInteractionEnabled);

        control.set_is_read_only(false);
        EXPECT_TRUE(native_field(handler).userInteractionEnabled);
    }

    // SecureTextEntry is REAL on iOS (the AppKit twin had to swap text-field cells).
    TEST(ios_entry_seam, password_sets_secure_text_entry_and_preserves_text)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(native_field(handler).secureTextEntry);

        control.set_text("secret");
        control.set_is_password(true);
        EXPECT_TRUE(native_field(handler).secureTextEntry);
        EXPECT_EQ(to_std_string(native_field(handler).text), "secret");

        control.set_is_password(false);
        EXPECT_FALSE(native_field(handler).secureTextEntry);
        EXPECT_EQ(to_std_string(native_field(handler).text), "secret");
    }

    TEST(ios_entry_seam, maps_font_and_alignment)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(native_field(handler).font.pointSize, 18.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_field(handler).textAlignment, NSTextAlignmentCenter);
    }

    // UpdateVerticalTextAlignment → UIControl.contentVerticalAlignment (REAL on iOS, no custom cell).
    TEST(ios_entry_seam, vertical_alignment_maps_to_content_vertical_alignment)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        // The control's default (Center) mapped at connect.
        EXPECT_EQ(field.contentVerticalAlignment, UIControlContentVerticalAlignmentCenter);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ(field.contentVerticalAlignment, UIControlContentVerticalAlignmentBottom);

        control.set_vertical_text_alignment(text_alignment::start);
        EXPECT_EQ(field.contentVerticalAlignment, UIControlContentVerticalAlignmentTop);
    }

    // The inbound channel: a native edit (EditingChanged) reports the (old, new) pair through
    // send_text_changed to the control's text_changed event.
    TEST(ios_entry_seam, native_editing_changed_flows_to_text_changed)
    {
        entry control;
        control.set_text("a");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        // Simulate a keystroke: UIKit mutates the field's text, then fires EditingChanged.
        UITextField* const field = native_field(handler);
        field.text = [NSString stringWithUTF8String:"ab"]; // not a UI literal: simulated keystroke payload
        send_control_event(field, UIControlEventEditingChanged);

        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "a");
        EXPECT_EQ(changes[0].second, "ab");

        // A second event without an actual change is silent (the C# UpdateText property no-op analog).
        send_control_event(field, UIControlEventEditingChanged);
        EXPECT_EQ(changes.size(), 1U);
    }

    // EditingDidEnd re-syncs the text (OnEditingEnded's UpdateText) without firing completed.
    TEST(ios_entry_seam, editing_did_end_syncs_text_without_completed)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        int completes = 0;
        std::vector<std::string> new_values;
        control.completed.connect([&completes] { ++completes; });
        control.text_changed.connect([&new_values](const std::string& /*old_value*/, const std::string& new_value) {
            new_values.push_back(new_value);
        });

        UITextField* const field = native_field(handler);
        field.text = [NSString stringWithUTF8String:"finished"]; // simulated edit payload
        send_control_event(field, UIControlEventEditingDidEnd);

        ASSERT_EQ(new_values.size(), 1U);
        EXPECT_EQ(new_values[0], "finished");
        EXPECT_EQ(completes, 0); // Completed is ShouldReturn's job
    }

    // textFieldShouldReturn → send_completed (the return key), returning NO so no newline is inserted.
    TEST(ios_entry_seam, should_return_fires_completed)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });

        UITextField* const field = native_field(handler);
        ASSERT_NE(field.delegate, nil);
        EXPECT_FALSE([field.delegate textFieldShouldReturn:field]);
        EXPECT_EQ(completes, 1);
    }

    // Ports EntryHandlerTests.iOS CharacterSpacingInitializesCorrectly.
    TEST(ios_entry_seam, character_spacing_kerns_the_text)
    {
        entry control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        EXPECT_EQ(kerning_of(field.attributedText), 4.0);
        EXPECT_EQ(to_std_string(field.text), "Some Test Text");
    }

    // Setting character_spacing back to 0 removes the prior kerning (WithCharacterSpacing un-sets it).
    TEST(ios_entry_seam, clearing_character_spacing_reverts_to_plain_text)
    {
        entry control;
        control.set_text("Some Test Text");
        control.set_character_spacing(4);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_field(handler).attributedText), 4.0);

        control.set_character_spacing(0);
        EXPECT_EQ(kerning_of(native_field(handler).attributedText), 0.0);
        EXPECT_EQ(to_std_string(native_field(handler).text), "Some Test Text");
    }

    // A plain placeholder (no color) stays an unstyled attributed string (the system's muted rendering).
    TEST(ios_entry_seam, plain_placeholder_carries_no_foreground_attribute)
    {
        entry control;
        control.set_placeholder("Hint");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        NSAttributedString* const placeholder = native_field(handler).attributedPlaceholder;
        ASSERT_NE(placeholder, nil);
        EXPECT_EQ(to_std_string(placeholder.string), "Hint");
        EXPECT_EQ([placeholder attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:nullptr], nil);
    }

    // placeholder_color builds the attributed placeholder carrying the foreground (UpdatePlaceholder).
    TEST(ios_entry_seam, placeholder_color_sets_attributed_placeholder_foreground)
    {
        entry control;
        control.set_placeholder("Hint");
        control.set_placeholder_color(maui::graphics::color(1.0F, 0.0F, 0.5F));
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        NSAttributedString* const placeholder = native_field(handler).attributedPlaceholder;
        ASSERT_NE(placeholder, nil);
        EXPECT_EQ(to_std_string(placeholder.string), "Hint");
        UIColor* const foreground = [placeholder attribute:NSForegroundColorAttributeName
                                                   atIndex:0
                                            effectiveRange:nullptr];
        ASSERT_NE(foreground, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([foreground getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(blue, 0.5, 0.01);
    }

    // character_spacing kerns the placeholder too (UpdateCharacterSpacing's placeholder branch — at
    // connect the mapper applies placeholder first, then character_spacing).
    TEST(ios_entry_seam, character_spacing_kerns_the_placeholder)
    {
        entry control;
        control.set_placeholder("Hint");
        control.set_character_spacing(3);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_field(handler).attributedPlaceholder), 3.0);
    }

    // UpdateReturnType → returnKeyType: REAL on iOS (the AppKit twin recorded a mirror).
    TEST(ios_entry_seam, return_type_maps_to_return_key_type)
    {
        entry control;
        control.set_return_type(return_type::next);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_field(handler).returnKeyType, UIReturnKeyNext);

        control.set_return_type(return_type::send);
        EXPECT_EQ(native_field(handler).returnKeyType, UIReturnKeySend);

        control.set_return_type(return_type::default_);
        EXPECT_EQ(native_field(handler).returnKeyType, UIReturnKeyDefault);
    }

    // UpdateClearButtonVisibility → clearButtonMode: REAL on iOS (the in-field clear affordance).
    TEST(ios_entry_seam, clear_button_visibility_maps_to_clear_button_mode)
    {
        entry control;
        control.set_clear_button_visibility(clear_button_visibility::while_editing);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_field(handler).clearButtonMode, UITextFieldViewModeWhileEditing);

        control.set_clear_button_visibility(clear_button_visibility::never);
        EXPECT_EQ(native_field(handler).clearButtonMode, UITextFieldViewModeNever);
    }

    // Prediction / spellcheck → autocorrectionType / spellCheckingType: REAL on iOS.
    TEST(ios_entry_seam, prediction_and_spellcheck_map_to_native_types)
    {
        entry control;
        control.set_is_text_prediction_enabled(false);
        control.set_is_spell_check_enabled(false);
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        EXPECT_EQ(field.autocorrectionType, UITextAutocorrectionTypeNo);
        EXPECT_EQ(field.spellCheckingType, UITextSpellCheckingTypeNo);

        control.set_is_text_prediction_enabled(true);
        control.set_is_spell_check_enabled(true);
        EXPECT_EQ(field.autocorrectionType, UITextAutocorrectionTypeYes);
        EXPECT_EQ(field.spellCheckingType, UITextSpellCheckingTypeYes);
    }

    // Cursor/selection: without an editing session there is no SelectedTextRange (UpdateCursorPosition's
    // own early-out), so the mirror records the values — the live-editor path needs the app-bundle
    // test-host lane (first responder requires a UIWindow + UIApplication).
    TEST(ios_entry_seam, cursor_and_selection_recorded)
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

    // The ShouldChangeCharacters delegate gate (ITextInputExtensions.TextWithinMaxLength): typing past
    // MaxLength is rejected, in-budget edits pass, and an over-long paste lands truncated.
    TEST(ios_entry_seam, should_change_characters_enforces_max_length)
    {
        entry control;
        control.set_max_length(5);
        control.set_text("12345");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        id<UITextFieldDelegate> const delegate = field.delegate;
        ASSERT_NE(delegate, nil);

        // Appending a 6th character: rejected.
        EXPECT_FALSE([delegate textField:field shouldChangeCharactersInRange:NSMakeRange(5, 0) replacementString:@"6"]);
        // Replacing one character keeps the length: allowed.
        EXPECT_TRUE([delegate textField:field shouldChangeCharactersInRange:NSMakeRange(4, 1) replacementString:@"X"]);
        // An out-of-document range (the undo crash fix): rejected.
        EXPECT_FALSE([delegate textField:field shouldChangeCharactersInRange:NSMakeRange(4, 9) replacementString:@""]);

        // Pasting 8 characters over the whole text: rejected, but the field lands the truncated paste
        // and the change is reported (the C# paste-truncation branch).
        std::vector<std::string> new_values;
        control.text_changed.connect([&new_values](const std::string& /*old_value*/, const std::string& new_value) {
            new_values.push_back(new_value);
        });
        EXPECT_FALSE([delegate textField:field
            shouldChangeCharactersInRange:NSMakeRange(0, 5)
                        replacementString:@"abcdefgh"]);
        EXPECT_EQ(to_std_string(field.text), "abcde");
        ASSERT_EQ(new_values.size(), 1U);
        EXPECT_EQ(new_values[0], "abcde");
    }

    // Without a MaxLength cap (the int.MaxValue default) every change passes.
    TEST(ios_entry_seam, should_change_characters_unrestricted_by_default)
    {
        entry control;
        control.set_text("abc");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        EXPECT_TRUE([field.delegate textField:field
                shouldChangeCharactersInRange:NSMakeRange(3, 0)
                            replacementString:@"defghijklmnop"]);
    }

    // The generic-IView pushes (the shared view_mapper through entry_platform's ios update_* overrides).
    TEST(ios_entry_seam, generic_iview_properties_reach_the_uitextfield)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(field.hidden);
        control.set_visibility(maui::core::visibility::visible);
        EXPECT_FALSE(field.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(field.alpha, 0.5);

        // UpdateIsEnabled's UIControl branch: the native enabled flag.
        control.set_is_enabled(false);
        EXPECT_FALSE(field.enabled);
        control.set_is_enabled(true);
        EXPECT_TRUE(field.enabled);

        control.set_automation_id("name_entry");
        EXPECT_EQ(to_std_string(field.accessibilityIdentifier), "name_entry");
    }

    // Keyboard (W8-53): UpdateKeyboard → ApplyKeyboard sets the UIKeyboardType + the autocapitalization /
    // spellcheck / autocorrection traits (REAL on iOS).
    TEST(ios_entry_seam, keyboard_maps_to_native_traits)
    {
        entry control;
        control.set_keyboard(keyboard::email());
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.keyboardType, UIKeyboardTypeEmailAddress);

        control.set_keyboard(keyboard::numeric());
        EXPECT_EQ(field.keyboardType, UIKeyboardTypeDecimalPad);

        control.set_keyboard(keyboard::telephone());
        EXPECT_EQ(field.keyboardType, UIKeyboardTypePhonePad);

        control.set_keyboard(keyboard::url());
        EXPECT_EQ(field.keyboardType, UIKeyboardTypeURL);
    }

    // The Text/Default/Chat keyboards force Sentences capitalization + autocorrect + spellcheck on iOS.
    TEST(ios_entry_seam, text_keyboard_forces_sentences_and_corrections)
    {
        entry control;
        control.set_keyboard(keyboard::text());
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.autocapitalizationType, UITextAutocapitalizationTypeSentences);
        EXPECT_EQ(field.autocorrectionType, UITextAutocorrectionTypeYes);
        EXPECT_EQ(field.spellCheckingType, UITextSpellCheckingTypeYes);
    }

    // A custom keyboard's KeyboardFlags drive capitalization + suggestions + spellcheck.
    TEST(ios_entry_seam, custom_keyboard_flags_drive_traits)
    {
        entry control;
        control.set_keyboard(keyboard::create(keyboard_flags::capitalize_word | keyboard_flags::spellcheck));
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.autocapitalizationType, UITextAutocapitalizationTypeWords);
        EXPECT_EQ(field.spellCheckingType, UITextSpellCheckingTypeYes);
        EXPECT_EQ(field.autocorrectionType, UITextAutocorrectionTypeNo); // no Suggestions flag
    }

    // AddMauiDoneAccessoryView: the field carries a Done input-accessory toolbar.
    TEST(ios_entry_seam, has_done_input_accessory_toolbar)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);
        ASSERT_NE(field.inputAccessoryView, nil);
        EXPECT_TRUE([field.inputAccessoryView isKindOfClass:[UIToolbar class]]);
    }

    // Focus (W8-53): the EditingDidBegin / EditingDidEnd control events reflect IsFocused onto the control
    // (firing Focused / Unfocused) — the native focus callback path. Injected via the dispatch-table walk
    // (no UIApplication needed), exactly like the editing-changed tests above.
    TEST(ios_entry_seam, editing_begin_end_reflect_is_focused)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        int focused_count = 0;
        int unfocused_count = 0;
        control.focused.connect([&focused_count](bool) { ++focused_count; });
        control.unfocused.connect([&unfocused_count](bool) { ++unfocused_count; });

        send_control_event(field, UIControlEventEditingDidBegin);
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(focused_count, 1);

        send_control_event(field, UIControlEventEditingDidEnd);
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(unfocused_count, 1);
    }

    TEST(ios_entry_seam, clearing_handler_disconnects)
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

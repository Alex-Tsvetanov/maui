// iOS (UIKit) backend tests for the search_bar seam, run ON the simulator — properties pushed to a
// real UISearchBar (and its inner UISearchTextField, C#'s QueryEditor), and the native delegate
// callbacks flowing back to the control's text_changed / search_button_pressed events. Compiled as
// Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/search_bar.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include "tests/support/run_loop_pump.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::search_bar;
    using maui::core::keyboard;
    using maui::core::return_type;
    using maui::core::search_bar_handler;
    using maui::core::text_alignment;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UISearchBar* native_bar(const std::shared_ptr<search_bar_handler>& handler)
    {
        return (__bridge UISearchBar*)handler->typed_platform_view()->native;
    }

    TEST(ios_search_bar_seam, creates_a_uisearchbar)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_bar(handler) isKindOfClass:[UISearchBar class]]);
        EXPECT_EQ(native_bar(handler).barStyle, UIBarStyleDefault);
    }

    TEST(ios_search_bar_seam, maps_text_and_placeholder)
    {
        search_bar control;
        control.set_text("query");
        control.set_placeholder("Search…");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        EXPECT_EQ(to_std_string(native_bar(handler).text), "query");
        EXPECT_EQ(to_std_string(native_bar(handler).placeholder), "Search…");
    }

    TEST(ios_search_bar_seam, placeholder_color_sets_attributed_placeholder)
    {
        search_bar control;
        control.set_placeholder("Hint");
        control.set_placeholder_color(maui::graphics::color(1.0F, 0.0F, 0.5F));
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        NSAttributedString* const ph = native_bar(handler).searchTextField.attributedPlaceholder;
        ASSERT_NE(ph, nil);
        EXPECT_EQ(to_std_string(ph.string), "Hint");
        UIColor* const fg = [ph attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:nullptr];
        ASSERT_NE(fg, nil);
    }

    TEST(ios_search_bar_seam, read_only_disables_interaction)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_TRUE(native_bar(handler).userInteractionEnabled);

        control.set_is_read_only(true);
        EXPECT_FALSE(native_bar(handler).userInteractionEnabled);
    }

    TEST(ios_search_bar_seam, maps_font_alignment_and_return_key_onto_the_query_editor)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        UISearchTextField* const editor = native_bar(handler).searchTextField;

        control.set_font(maui::core::font::of_size("Helvetica", 15));
        EXPECT_EQ(editor.font.pointSize, 15.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(editor.textAlignment, NSTextAlignmentCenter);

        // SearchBar's ReturnType defaults to Search.
        EXPECT_EQ(editor.returnKeyType, UIReturnKeySearch);
        control.set_return_type(return_type::go);
        EXPECT_EQ(editor.returnKeyType, UIReturnKeyGo);
    }

    TEST(ios_search_bar_seam, prediction_and_spellcheck_map_to_native_types)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        UISearchTextField* const editor = native_bar(handler).searchTextField;

        control.set_is_text_prediction_enabled(false);
        EXPECT_EQ(editor.autocorrectionType, UITextAutocorrectionTypeNo);

        control.set_is_spell_check_enabled(false);
        EXPECT_EQ(editor.spellCheckingType, UITextSpellCheckingTypeNo);
    }

    TEST(ios_search_bar_seam, max_length_trims_existing_text)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        native_bar(handler).text = @"abcdef";
        control.set_max_length(3);
        EXPECT_EQ(to_std_string(native_bar(handler).text), "abc");
    }

    TEST(ios_search_bar_seam, should_change_text_enforces_max_length)
    {
        search_bar control;
        control.set_text("abc");
        control.set_max_length(3);
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        UISearchBar* const bar = native_bar(handler);
        auto* const delegate = (id<UISearchBarDelegate>)bar.delegate;
        EXPECT_FALSE([delegate searchBar:bar shouldChangeTextInRange:NSMakeRange(3, 0) replacementText:@"d"]);
        EXPECT_TRUE([delegate searchBar:bar shouldChangeTextInRange:NSMakeRange(2, 1) replacementText:@"x"]);
    }

    TEST(ios_search_bar_seam, native_edit_callback_fires_text_changed)
    {
        search_bar control;
        control.set_text("ab");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        UISearchBar* const bar = native_bar(handler);
        bar.text = @"abc";
        [(id<UISearchBarDelegate>)bar.delegate searchBar:bar textDidChange:@"abc"];

        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "ab");
        EXPECT_EQ(changes[0].second, "abc");
    }

    TEST(ios_search_bar_seam, search_button_clicked_fires_event)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        int presses = 0;
        control.search_button_pressed.connect([&presses] { ++presses; });

        UISearchBar* const bar = native_bar(handler);
        [(id<UISearchBarDelegate>)bar.delegate searchBarSearchButtonClicked:bar];
        EXPECT_EQ(presses, 1);
    }

    TEST(ios_search_bar_seam, cancel_button_clears_the_text)
    {
        search_bar control;
        control.set_text("query");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        UISearchBar* const bar = native_bar(handler);
        [(id<UISearchBarDelegate>)bar.delegate searchBarCancelButtonClicked:bar];

        EXPECT_EQ(to_std_string(bar.text), "");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "query");
        EXPECT_EQ(changes[0].second, "");
    }

    TEST(ios_search_bar_seam, cancel_button_shows_while_text_is_present)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(native_bar(handler).showsCancelButton);

        control.set_text("q");
        EXPECT_TRUE(native_bar(handler).showsCancelButton);

        control.set_text("");
        EXPECT_FALSE(native_bar(handler).showsCancelButton);
    }

    // The cancel UIButton is a descendant of the bar built by UIKit only once the control is in the
    // window hierarchy — the same tree-walk the handler's UpdateCancelButton uses, with the C# predicate
    // that excludes buttons inside the search field (the clear "x" button, not the cancel button).
    bool button_has_text_field_ancestor(UIView* view)
    {
        for (UIView* parent = view.superview; parent != nil; parent = parent.superview)
        {
            if ([parent isKindOfClass:[UITextField class]])
            {
                return true;
            }
        }
        return false;
    }

    UIButton* find_cancel_button(UIView* root)
    {
        for (UIView* subview in root.subviews)
        {
            if ([subview isKindOfClass:[UIButton class]] && !button_has_text_field_ancestor(subview))
            {
                return (UIButton*)subview;
            }
            if (UIButton* const nested = find_cancel_button(subview))
            {
                return nested;
            }
        }
        return nil;
    }

    // MovedToWindow re-fire (MauiSearchBar.cs + SearchBarHandler.iOS.cs:227-235): the cancel button
    // doesn't exist until the bar joins the window hierarchy, so a CancelButtonColor set earlier is
    // lost. MauiSearchBar overrides the moved-to-window lifecycle and the proxy re-fires
    // UpdateValue(CancelButtonColor) once the button is realized. Driving a real UIWindow exercises the
    // genuine -didMoveToWindow path (rather than a direct call), so the re-fire must land the tint.
    TEST(ios_search_bar_seam, moved_to_window_refires_cancel_button_color)
    {
        search_bar control;
        control.set_text("query"); // ShouldShowCancelButton() — the cancel button is shown
        control.set_cancel_button_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        UISearchBar* const bar = native_bar(handler);

        // Place the bar in a real, key+visible window so UIKit runs a layout cycle and -didMoveToWindow
        // fires — building the internal cancel-button hierarchy and triggering the re-fire. iOS 26
        // deprecates the scene-less UIWindow initializer; mirror window_handler.mm's suppressed `init`.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init];
#pragma clang diagnostic pop
        window.frame = CGRectMake(0, 0, 320, 64);
        bar.frame = window.bounds;
        [window addSubview:bar];
        [window makeKeyAndVisible];
        [bar layoutIfNeeded];
        maui::tests::pump_until([&] { return find_cancel_button(bar) != nil; });

        UIButton* const cancel = find_cancel_button(bar);
        ASSERT_NE(cancel, nil);
        // SearchBarExtensions.UpdateCancelButton tints the cancel button via its Normal-state title color
        // on iOS (TintColor is the Mac-idiom path). The re-fire must have applied the explicit red.
        UIColor* const title = [cancel titleColorForState:UIControlStateNormal];
        ASSERT_NE(title, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        [title getRed:&red green:&green blue:&blue alpha:&alpha];
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(green, 0.0, 0.01);
        EXPECT_NEAR(blue, 0.0, 0.01);

        [bar removeFromSuperview];
        window.hidden = YES;
    }

    TEST(ios_search_bar_seam, generic_iview_properties_reach_the_bar)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        control.set_opacity(0.25);
        EXPECT_NEAR(native_bar(handler).alpha, 0.25, 0.001);

        control.set_visibility(maui::core::visibility::collapsed);
        EXPECT_TRUE(native_bar(handler).hidden);

        control.set_automation_id("search-id");
        EXPECT_EQ(to_std_string(native_bar(handler).accessibilityIdentifier), "search-id");
    }

    // Keyboard (W8-53): UpdateKeyboard sets the search field's keyboardType (ApplyKeyboard on the
    // UISearchTextField, which conforms to UITextInputTraits).
    TEST(ios_search_bar_seam, keyboard_maps_to_search_field_type)
    {
        search_bar control;
        control.set_keyboard(keyboard::email());
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_bar(handler).searchTextField.keyboardType, UIKeyboardTypeEmailAddress);

        control.set_keyboard(keyboard::url());
        EXPECT_EQ(native_bar(handler).searchTextField.keyboardType, UIKeyboardTypeURL);
    }

    // Keyboard.Plain (W8-53 regression): `Keyboard.Plain` is a CustomKeyboard(None), so UpdateKeyboard's
    // `if (keyboard is not CustomKeyboard)` gate is FALSE — IsTextPrediction / IsSpellCheck are NOT
    // re-applied. ApplyKeyboard's `case plain` leaves autocorrect=No / spellcheck=No on the search field,
    // which must hold even though both control properties default to true.
    TEST(ios_search_bar_seam, plain_keyboard_does_not_reapply_prediction_or_spellcheck)
    {
        search_bar control;
        ASSERT_TRUE(control.is_text_prediction_enabled());
        ASSERT_TRUE(control.is_spell_check_enabled());
        control.set_keyboard(keyboard::plain());
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        UISearchTextField* const editor = native_bar(handler).searchTextField;
        EXPECT_EQ(editor.autocorrectionType, UITextAutocorrectionTypeNo);
        EXPECT_EQ(editor.spellCheckingType, UITextSpellCheckingTypeNo);
    }

    // Focus (W8-53): the begin/end editing delegate callbacks reflect IsFocused (firing Focused/Unfocused).
    TEST(ios_search_bar_seam, begin_end_editing_reflect_is_focused)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        UISearchBar* const bar = native_bar(handler);
        auto* const delegate = (id<UISearchBarDelegate>)bar.delegate;

        int focused_count = 0;
        int unfocused_count = 0;
        control.focused.connect([&focused_count](bool) { ++focused_count; });
        control.unfocused.connect([&unfocused_count](bool) { ++unfocused_count; });

        [delegate searchBarTextDidBeginEditing:bar];
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(focused_count, 1);

        [delegate searchBarTextDidEndEditing:bar];
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(unfocused_count, 1);
    }

    TEST(ios_search_bar_seam, clearing_handler_disconnects)
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

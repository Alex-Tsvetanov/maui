// iOS (UIKit) backend tests for the picker seam — run only for MAUI_BACKEND=ios (executed ON the iOS
// simulator via tools/ios-sim-run.sh). Drives the real MauiPicker shape: a UITextField whose
// inputView is a UIPickerView fed by the UIPickerViewModel port (rows/titles through the
// i_item_delegate face), the Title as the attributed placeholder, and the Done commit
// (FinishSelectItem) through the portable on_done channel — first-responder sessions need a
// UIWindow/UIApplication, which a spawned simulator process lacks (see button_ios_tests.mm), so the
// wheel's data source is exercised directly. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/observable_collection.hpp"
#include "maui/controls/picker.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::picker;
    using maui::core::i_element_handler;
    using maui::core::picker_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITextField* native_field(const std::shared_ptr<picker_handler>& handler)
    {
        return (__bridge UITextField*)handler->typed_platform_view()->native;
    }

    UIPickerView* wheel_of(UITextField* field)
    {
        return (UIPickerView*)field.inputView;
    }

    TEST(ios_picker_seam, attaching_handler_builds_the_maui_picker_shape)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.set_selected_index(1);
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_EQ(field.borderStyle, UITextBorderStyleRoundedRect);
        ASSERT_TRUE([field.inputView isKindOfClass:[UIPickerView class]]);
        EXPECT_NE(field.inputAccessoryView, nil); // the Done toolbar
        EXPECT_EQ(to_std_string(field.text), "Paul");

        // The wheel reads rows/titles through the i_item_delegate face.
        UIPickerView* const wheel = wheel_of(field);
        ASSERT_NE(wheel.dataSource, nil);
        EXPECT_EQ([wheel.dataSource pickerView:wheel numberOfRowsInComponent:0], 2);
        EXPECT_EQ(to_std_string([wheel.delegate pickerView:wheel titleForRow:0 forComponent:0]), "John");
        EXPECT_EQ(to_std_string([wheel.delegate pickerView:wheel titleForRow:1 forComponent:0]), "Paul");
    }

    TEST(ios_picker_seam, no_selection_shows_the_title_placeholder)
    {
        picker control;
        control.items().add("one");
        control.set_title("Pick one");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.text.length, 0U);
        ASSERT_NE(field.attributedPlaceholder, nil);
        EXPECT_EQ(to_std_string(field.attributedPlaceholder.string), "Pick one");
    }

    TEST(ios_picker_seam, items_changes_reload_the_wheel)
    {
        picker control;
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        UIPickerView* const wheel = wheel_of(native_field(handler));
        EXPECT_EQ([wheel.dataSource pickerView:wheel numberOfRowsInComponent:0], 0);

        control.items().add("one");
        control.items().add("two");
        EXPECT_EQ([wheel.dataSource pickerView:wheel numberOfRowsInComponent:0], 2);

        control.set_items_source(std::make_shared<maui::controls::observable_collection<std::string>>(
            std::vector<std::string>{"x", "y", "z"}));
        EXPECT_EQ([wheel.dataSource pickerView:wheel numberOfRowsInComponent:0], 3);
        EXPECT_EQ(to_std_string([wheel.delegate pickerView:wheel titleForRow:0 forComponent:0]), "x");
    }

    TEST(ios_picker_seam, wheel_pick_plus_done_commits_the_row)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        int raised = 0;
        control.selected_index_changed.connect([&raised] { ++raised; });

        UITextField* const field = native_field(handler);
        UIPickerView* const wheel = wheel_of(field);
        [wheel.delegate pickerView:wheel didSelectRow:1 inComponent:0]; // the user spins to row 1...
        handler->typed_platform_view()->on_done(1);                     // ...and taps Done

        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Paul");
        EXPECT_EQ(to_std_string(field.text), "Paul");
        EXPECT_EQ(raised, 1);
    }

    TEST(ios_picker_seam, done_with_no_pending_row_selects_the_first_item)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        handler->typed_platform_view()->on_done(-1); // FinishSelectItem: unset row -> row 0
        EXPECT_EQ(control.selected_index(), 0);
        EXPECT_EQ(to_std_string(native_field(handler).text), "John");
    }

    TEST(ios_picker_seam, generic_iview_properties_reach_the_field)
    {
        picker control;
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(field.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(field.hidden);

        control.set_opacity(0.5);
        EXPECT_NEAR(field.alpha, 0.5, 0.001);

        control.set_automation_id("beatle_picker");
        EXPECT_EQ(to_std_string(field.accessibilityIdentifier), "beatle_picker");
    }

    TEST(ios_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<picker_handler*>(handler.get()), nullptr);
    }
} // namespace

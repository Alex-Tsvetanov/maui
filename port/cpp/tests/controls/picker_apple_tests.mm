// Apple (AppKit) backend tests for the picker seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSPopUpButton: items rebuild the popup menu (duplicates preserved — the addItemWithTitle:
// de-dup trap), the selection maps both ways (a native pick is simulated by selecting a row and
// sending the wired target-action, no run loop needed), and the Title plays its placeholder role on
// the detached cell item while nothing is selected. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

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

    NSPopUpButton* native_popup(const std::shared_ptr<picker_handler>& handler)
    {
        return (__bridge NSPopUpButton*)handler->typed_platform_view()->native;
    }

    // NSPopUpButton creation needs the shared application object (no run loop required).
    class apple_picker_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_picker_seam, attaching_handler_populates_the_popup_menu_and_selection)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        control.set_selected_index(1);
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        NSPopUpButton* const popup = native_popup(handler);
        ASSERT_NE(popup, nil);
        EXPECT_EQ(popup.numberOfItems, 2);
        EXPECT_EQ(to_std_string([popup itemTitleAtIndex:0]), "John");
        EXPECT_EQ(to_std_string([popup itemTitleAtIndex:1]), "Paul");
        EXPECT_EQ(popup.indexOfSelectedItem, 1);
    }

    TEST_F(apple_picker_seam, duplicate_item_titles_are_preserved)
    {
        picker control;
        control.items().add("same");
        control.items().add("same");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        EXPECT_EQ(native_popup(handler).numberOfItems, 2); // addItemWithTitle: would de-duplicate
    }

    TEST_F(apple_picker_seam, items_changes_reload_the_menu)
    {
        picker control;
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_popup(handler).numberOfItems, 0);

        control.items().add("one");
        control.items().add("two");
        EXPECT_EQ(native_popup(handler).numberOfItems, 2);

        control.set_items_source(std::make_shared<maui::controls::observable_collection<std::string>>(
            std::vector<std::string>{"x", "y", "z"}));
        EXPECT_EQ(native_popup(handler).numberOfItems, 3);
        EXPECT_EQ(to_std_string([native_popup(handler) itemTitleAtIndex:0]), "x");
    }

    TEST_F(apple_picker_seam, no_selection_shows_the_title_placeholder)
    {
        picker control;
        control.items().add("one");
        control.set_title("Pick one");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        NSPopUpButton* const popup = native_popup(handler);
        EXPECT_EQ(popup.indexOfSelectedItem, -1);
        // The placeholder lives on a DETACHED cell item (not in the menu).
        NSMenuItem* const displayed = ((NSPopUpButtonCell*)popup.cell).menuItem;
        ASSERT_NE(displayed, nil);
        EXPECT_EQ(to_std_string(displayed.title), "Pick one");
        EXPECT_EQ(popup.numberOfItems, 1);
    }

    TEST_F(apple_picker_seam, native_pick_flows_back_into_the_control)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        int raised = 0;
        control.selected_index_changed.connect([&raised] { ++raised; });

        NSPopUpButton* const popup = native_popup(handler);
        [popup selectItemAtIndex:1];                     // the user picks row 1...
        [popup sendAction:popup.action to:popup.target]; // ...and the menu commits (target-action)

        EXPECT_EQ(control.selected_index(), 1);
        ASSERT_TRUE(control.selected_item().has_value());
        EXPECT_EQ(*control.selected_item(), "Paul");
        EXPECT_EQ(raised, 1);
        EXPECT_EQ(native_popup(handler).indexOfSelectedItem, 1);
    }

    TEST_F(apple_picker_seam, on_done_with_no_pending_row_selects_the_first_item)
    {
        picker control;
        control.items().add("John");
        control.items().add("Paul");
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);

        handler->typed_platform_view()->on_done(-1); // FinishSelectItem: unset row -> row 0
        EXPECT_EQ(control.selected_index(), 0);
        EXPECT_EQ(native_popup(handler).indexOfSelectedItem, 0);
    }

    TEST_F(apple_picker_seam, generic_iview_properties_reach_the_popup)
    {
        picker control;
        auto handler = std::make_shared<picker_handler>();
        control.set_handler(handler);
        NSPopUpButton* const popup = native_popup(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(popup.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(popup.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(popup.alphaValue, 0.5);

        control.set_automation_id("beatle_picker");
        EXPECT_EQ(to_std_string(popup.accessibilityIdentifier), "beatle_picker");
    }

    TEST_F(apple_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<picker_handler*>(handler.get()), nullptr);
    }
} // namespace

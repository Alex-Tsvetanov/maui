// Tests for the input_controls_page demo (src/samples/pages/input_controls_page.hpp) — backend-
// agnostic: the page is pure cross-platform control wiring, so this suite compiles in every preset
// and proves the demo's interactions (editor→readout, search→editor, radio group→casing,
// image button→clear) without a hosting main.
#include "src/samples/pages/input_controls_page.hpp"

#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::samples::input_controls_page;

    TEST(input_controls_page, builds_the_input_stack)
    {
        input_controls_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 6); // readout + editor + search + 2 radios + clear button
        // The radio group is attached to the stack with the UPPER choice pre-selected.
        EXPECT_EQ(demo.upper_choice().group_name(), "casing");
        EXPECT_EQ(demo.lower_choice().group_name(), "casing");
        EXPECT_TRUE(demo.upper_choice().is_checked());
        EXPECT_FALSE(demo.lower_choice().is_checked());
    }

    TEST(input_controls_page, editor_text_drives_the_readout)
    {
        input_controls_page demo;
        demo.text_editor().set_text("hello");
        EXPECT_EQ(demo.readout().text(), "LENGTH: 5");
    }

    TEST(input_controls_page, search_button_copies_the_query_into_the_editor)
    {
        input_controls_page demo;
        demo.search().set_text("query");
        demo.search().send_search_button_pressed(); // the native Search action channel
        EXPECT_EQ(demo.text_editor().text(), "query");
        EXPECT_EQ(demo.readout().text(), "LENGTH: 5");
    }

    TEST(input_controls_page, radio_group_picks_the_readout_casing)
    {
        input_controls_page demo;
        demo.text_editor().set_text("abc");
        EXPECT_EQ(demo.readout().text(), "LENGTH: 3");

        demo.lower_choice().set_is_checked(true); // mutual exclusion + SelectedValue → casing
        EXPECT_FALSE(demo.upper_choice().is_checked());
        EXPECT_EQ(demo.readout().text(), "length: 3");

        demo.upper_choice().set_is_checked(true);
        EXPECT_FALSE(demo.lower_choice().is_checked());
        EXPECT_EQ(demo.readout().text(), "LENGTH: 3");
    }

    TEST(input_controls_page, image_button_clears_the_editor)
    {
        input_controls_page demo;
        demo.text_editor().set_text("to be cleared");
        demo.clear_button().send_clicked(); // the native click channel
        EXPECT_EQ(demo.text_editor().text(), "");
        EXPECT_EQ(demo.readout().text(), "LENGTH: 0");
    }
} // namespace

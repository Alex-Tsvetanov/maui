// Tests for the value_controls_page demo (src/samples/pages/value_controls_page.hpp) — backend-
// agnostic: the page is pure cross-platform control wiring, so this suite compiles in every preset
// and proves the demo's interactions (slider→progress/readout, stepper→slider, switch→spinner,
// check_box→thumb color) without a hosting main.
#include "src/samples/pages/value_controls_page.hpp"

#include <string>

#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::value_controls_page;

    TEST(value_controls_page, builds_the_six_control_stack)
    {
        value_controls_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 7); // readout + the six value controls
    }

    TEST(value_controls_page, slider_drives_progress_and_readout)
    {
        value_controls_page demo;
        demo.value_slider().set_value(80);
        EXPECT_EQ(demo.progress().progress(), 0.8);
        EXPECT_EQ(demo.readout().text(), "Value: 80 / 100");
    }

    TEST(value_controls_page, stepper_steps_the_slider)
    {
        value_controls_page demo;
        demo.value_stepper().set_value(demo.value_stepper().value() + demo.value_stepper().increment());
        EXPECT_EQ(demo.value_slider().value(), 30); // 25 + the 5 increment
        EXPECT_EQ(demo.progress().progress(), 0.3);
    }

    TEST(value_controls_page, switch_runs_the_spinner)
    {
        value_controls_page demo;
        EXPECT_FALSE(demo.spinner().is_running());
        demo.busy_switch().set_is_toggled(true);
        EXPECT_TRUE(demo.spinner().is_running());
        demo.busy_switch().set_is_toggled(false);
        EXPECT_FALSE(demo.spinner().is_running());
    }

    TEST(value_controls_page, check_box_recolors_the_slider_thumb)
    {
        value_controls_page demo;
        demo.accent_check().set_is_checked(true);
        EXPECT_EQ(demo.value_slider().thumb_color(), maui::graphics::color(0.86F, 0.20F, 0.27F));
        demo.accent_check().set_is_checked(false);
        EXPECT_EQ(demo.value_slider().thumb_color(), maui::graphics::color{});
    }
} // namespace

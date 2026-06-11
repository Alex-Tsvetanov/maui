// Tests for the check_box control (maui::controls::check_box <= CheckBox) and the headless handler
// seam. The control half ports src/Controls/tests/Core.UnitTests/CheckBoxUnitTests.cs (constructor /
// CheckedChanged / command / visual-state oracle — the ICommand CanExecute cases are NOT ported: the
// port models Command as a plain callable, the button convention); the seam half follows the headless
// conventions (button_tests.cpp): virtual→native mirrors + the native toggle flowing back.
#include "maui/controls/check_box.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::check_box;
    using maui::controls::common_states;
    using maui::controls::setter;
    using maui::controls::visual_state;
    using maui::controls::visual_state_group;
    using maui::core::check_box_handler;
    using maui::core::i_element_handler;
    using maui::graphics::color;

    [[nodiscard]] const color& red()
    {
        static const color value{1.0F, 0.0F, 0.0F};
        return value;
    }
    [[nodiscard]] const color& green()
    {
        static const color value{0.0F, 1.0F, 0.0F};
        return value;
    }
    [[nodiscard]] const color& blue()
    {
        static const color value{0.0F, 0.0F, 1.0F};
        return value;
    }

    // ---- the control in isolation (CheckBoxUnitTests.cs) ----

    TEST(check_box, constructor_defaults_to_unchecked)
    {
        check_box control;
        EXPECT_FALSE(control.is_checked());
    }

    TEST(check_box, checked_changed_fires_on_change)
    {
        check_box control;
        bool fired = false;
        control.checked_changed.connect([&fired](bool /*value*/) { fired = true; });

        control.set_is_checked(true);
        EXPECT_TRUE(fired);
    }

    TEST(check_box, checked_changed_not_double_fired)
    {
        check_box control;
        bool fired = false;
        control.set_is_checked(true);

        control.checked_changed.connect([&fired](bool /*value*/) { fired = true; });
        control.set_is_checked(true); // same value: no change, no event
        EXPECT_FALSE(fired);
    }

    TEST(check_box, checked_changed_event_args_carry_the_value)
    {
        check_box control;
        bool reported = false;
        control.checked_changed.connect([&reported](bool value) { reported = value; });

        control.set_is_checked(true);
        EXPECT_TRUE(reported);
        control.set_is_checked(false);
        EXPECT_FALSE(reported);
    }

    TEST(check_box, checked_changed_fires_only_when_the_value_changes)
    {
        check_box control;
        int fire_count = 0;
        control.checked_changed.connect([&fire_count](bool /*value*/) { ++fire_count; });

        control.set_is_checked(false); // same as the default: no event
        EXPECT_EQ(fire_count, 0);
        control.set_is_checked(true);
        EXPECT_EQ(fire_count, 1);
        control.set_is_checked(true); // same again: no event
        EXPECT_EQ(fire_count, 1);
        control.set_is_checked(false);
        EXPECT_EQ(fire_count, 2);
    }

    TEST(check_box, command_executes_on_checked_state_change_only)
    {
        check_box control;
        int execute_count = 0;
        control.command = [&execute_count] { ++execute_count; };

        control.set_is_checked(true);
        EXPECT_EQ(execute_count, 1);
        control.set_is_checked(true); // same value: no execution
        EXPECT_EQ(execute_count, 1);
        control.set_is_checked(false);
        EXPECT_EQ(execute_count, 2);
    }

    TEST(check_box, null_command_does_not_throw)
    {
        check_box control;
        control.command = nullptr;
        control.set_is_checked(true); // no exception with no command
        EXPECT_TRUE(control.is_checked());
    }

    TEST(check_box, command_and_checked_changed_both_fire_event_first)
    {
        check_box control;
        std::vector<int> order;
        control.checked_changed.connect([&order](bool /*value*/) { order.push_back(1); });
        control.command = [&order] { order.push_back(2); };

        control.set_is_checked(true);
        // C#'s IsCheckedProperty callback order: CheckedChanged, then Command (the reverse of button).
        EXPECT_EQ(order, (std::vector<int>{1, 2}));
    }

    // ---- visual states (the CheckedVisualStates oracle, observed through a color setter) ----

    void add_states(check_box& control, bool include_is_checked)
    {
        visual_state_group group{"CommonStates"};
        visual_state normal{std::string{common_states::normal}};
        normal.add(setter::of(check_box::color_property(), blue()));
        group.add(std::move(normal));
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(check_box::color_property(), red()));
        group.add(std::move(disabled));
        if (include_is_checked)
        {
            visual_state checked{std::string{check_box::is_checked_visual_state}};
            checked.add(setter::of(check_box::color_property(), green()));
            group.add(std::move(checked));
        }
        control.visual_states().add_group(std::move(group));
    }

    TEST(check_box_visual_states, checked_goes_to_is_checked_state_and_back)
    {
        check_box control;
        add_states(control, true);

        control.set_is_checked(true);
        EXPECT_EQ(control.color(), green()); // the IsChecked state applied

        control.set_is_checked(false);
        EXPECT_EQ(control.color(), blue()); // back to Normal (base ChangeVisualState)
    }

    TEST(check_box_visual_states, checked_without_is_checked_state_falls_back_to_normal)
    {
        check_box control;
        add_states(control, false);

        control.set_is_checked(true);
        EXPECT_EQ(control.color(), blue()); // no IsChecked state: Normal
    }

    TEST(check_box_visual_states, disabled_outranks_checked)
    {
        check_box control;
        add_states(control, true);
        control.set_is_checked(true);
        EXPECT_EQ(control.color(), green());

        control.set_is_enabled(false); // disabled: base ChangeVisualState → Disabled
        EXPECT_EQ(control.color(), red());
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(check_box_seam, attaching_handler_maps_initial_state)
    {
        check_box control;
        control.set_is_checked(true);
        control.set_color(green());
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->is_checked);
        ASSERT_NE(platform->foreground, nullptr); // Color?.AsPaint() — set, so a solid paint
        EXPECT_EQ(platform->foreground->background_color(), green());
    }

    TEST(check_box_seam, foreground_is_null_until_color_is_set)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->foreground, nullptr); // Color unset → null paint

        control.set_color(red());
        ASSERT_NE(handler->typed_platform_view()->foreground, nullptr);
        EXPECT_EQ(handler->typed_platform_view()->foreground->background_color(), red());
    }

    TEST(check_box_seam, setting_is_checked_updates_the_platform)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(handler->typed_platform_view()->is_checked);

        control.set_is_checked(true);
        EXPECT_TRUE(handler->typed_platform_view()->is_checked);
    }

    TEST(check_box_seam, native_toggle_flows_back_to_the_control)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.checked_changed.connect([&reported](bool value) { reported = value; });

        // Simulate the user tapping the native check box (the MauiCheckBox.CheckedChanged analog).
        auto* platform = handler->typed_platform_view();
        platform->is_checked = true;
        platform->on_checked_changed();

        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(reported);
    }

    TEST(check_box_seam, manual_set_overrides_the_native_value)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        platform->is_checked = true;
        platform->on_checked_changed(); // native toggle → stored at from-handler specificity
        EXPECT_TRUE(control.is_checked());

        control.set_is_checked(false); // a manual set overrides the handler value
        EXPECT_FALSE(control.is_checked());
        EXPECT_FALSE(platform->is_checked); // and maps back to the native state
    }

    TEST(check_box_seam, clearing_handler_disconnects)
    {
        check_box control;
        auto handler = std::make_shared<check_box_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(check_box_seam, handler_resolved_from_default_registry)
    {
        // check_box -> check_box_handler is self-registered in check_box.cpp.
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<check_box>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<check_box_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        check_box control;
        control.set_is_checked(true);
        control.set_handler(handler);
        EXPECT_TRUE(resolved->typed_platform_view()->is_checked);
    }
} // namespace

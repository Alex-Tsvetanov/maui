// Tests for the toggle_switch control (maui::controls::toggle_switch <= Switch) and the headless
// handler seam. The control half ports src/Controls/tests/Core.UnitTests/SwitchUnitTests.cs (the
// constructor/Toggled-event/visual-state oracle); the seam half follows the established headless
// conventions (button_tests.cpp): virtual→native property mirrors + the native toggle flowing back.
#include "maui/controls/toggle_switch.hpp"

#include <memory>
#include <string>

#include "maui/controls/setter.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>
#include <utility>

namespace
{
    using maui::controls::common_states;
    using maui::controls::setter;
    using maui::controls::toggle_switch;
    using maui::controls::visual_state;
    using maui::controls::visual_state_group;
    using maui::core::i_element_handler;
    using maui::core::switch_handler;
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
    [[nodiscard]] const color& gray()
    {
        static const color value{0.5F, 0.5F, 0.5F};
        return value;
    }

    // ---- the control in isolation (SwitchUnitTests.cs) ----

    TEST(toggle_switch, constructor_defaults_to_not_toggled)
    {
        toggle_switch control;
        EXPECT_FALSE(control.is_toggled());
    }

    TEST(toggle_switch, toggled_event_fires_on_change)
    {
        toggle_switch control;
        bool fired = false;
        control.toggled.connect([&fired](bool /*value*/) { fired = true; });

        control.set_is_toggled(true);
        EXPECT_TRUE(fired);
    }

    TEST(toggle_switch, toggled_event_not_double_fired)
    {
        toggle_switch control;
        bool fired = false;
        control.set_is_toggled(true);

        control.toggled.connect([&fired](bool /*value*/) { fired = true; });
        control.set_is_toggled(true); // same value: no change, no event
        EXPECT_FALSE(fired);
    }

    TEST(toggle_switch, toggled_event_carries_the_new_value)
    {
        toggle_switch control;
        bool reported = false;
        control.toggled.connect([&reported](bool value) { reported = value; });

        control.set_is_toggled(true);
        EXPECT_TRUE(reported);
        control.set_is_toggled(false);
        EXPECT_FALSE(reported);
    }

    // ---- visual states (the SwitchUnitTests VSM oracle, observed through a thumb_color setter) ----

    // A CommonStates group with Disabled / Normal / On / Off, each setting thumb_color to a distinct
    // color (the port observes state transitions through setter effects, as the VSM tests do). The
    // control is configured in place — it owns non-movable property slots, so it cannot be returned.
    void add_states(toggle_switch& control, bool include_on_off, bool include_normal = true)
    {
        visual_state_group group{"CommonStates"};
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(toggle_switch::thumb_color_property(), gray()));
        group.add(std::move(disabled));
        if (include_normal)
        {
            visual_state normal{std::string{common_states::normal}};
            normal.add(setter::of(toggle_switch::thumb_color_property(), blue()));
            group.add(std::move(normal));
        }
        if (include_on_off)
        {
            visual_state on{std::string{toggle_switch::switch_on_visual_state}};
            on.add(setter::of(toggle_switch::thumb_color_property(), green()));
            visual_state off{std::string{toggle_switch::switch_off_visual_state}};
            off.add(setter::of(toggle_switch::thumb_color_property(), red()));
            group.add(std::move(on));
            group.add(std::move(off));
        }
        control.visual_states().add_group(std::move(group));
    }

    TEST(toggle_switch_visual_states, disabled_switch_is_in_disabled_state)
    {
        toggle_switch control;
        add_states(control, true);
        control.set_is_enabled(false); // drives change_visual_state automatically
        EXPECT_EQ(control.thumb_color(), gray());
    }

    TEST(toggle_switch_visual_states, enabled_and_on_goes_to_on_state)
    {
        toggle_switch control;
        add_states(control, true);
        control.set_is_toggled(true); // the IsToggled change drives ChangeVisualState
        EXPECT_EQ(control.thumb_color(), green());
    }

    TEST(toggle_switch_visual_states, enabled_and_off_goes_to_off_state)
    {
        toggle_switch control;
        add_states(control, true);
        control.change_visual_state(); // apply the initial state (IsToggled defaults to false)
        EXPECT_EQ(control.thumb_color(), red());
    }

    TEST(toggle_switch_visual_states, normal_used_when_on_off_states_missing)
    {
        toggle_switch control;
        add_states(control, false);
        control.change_visual_state();
        EXPECT_EQ(control.thumb_color(), blue()); // base ChangeVisualState's Normal; On/Off are absent
    }

    TEST(toggle_switch_visual_states, toggling_moves_between_on_and_off)
    {
        toggle_switch control;
        add_states(control, true);
        control.set_is_toggled(true);
        EXPECT_EQ(control.thumb_color(), green());
        control.set_is_toggled(false);
        EXPECT_EQ(control.thumb_color(), red());
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(toggle_switch_seam, attaching_handler_maps_initial_state)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        control.set_on_color(green());
        control.set_thumb_color(red());
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->is_on);
        EXPECT_EQ(platform->track_color, green()); // toggled on: TrackColor reads OnColor
        EXPECT_EQ(platform->thumb_color, red());
    }

    TEST(toggle_switch_seam, setting_is_toggled_updates_platform_and_track_color)
    {
        toggle_switch control;
        control.set_on_color(green());
        control.set_off_color(red());
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->is_on);
        EXPECT_EQ(platform->track_color, red()); // off: TrackColor reads OffColor

        control.set_is_toggled(true);
        EXPECT_TRUE(platform->is_on);
        EXPECT_EQ(platform->track_color, green()); // the IsToggled change re-ran the TrackColor mapper
    }

    TEST(toggle_switch_seam, changing_on_color_while_on_remaps_track_color)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        control.set_on_color(blue());
        EXPECT_EQ(handler->typed_platform_view()->track_color, blue());
    }

    TEST(toggle_switch_seam, native_toggle_flows_back_to_the_control)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.toggled.connect([&reported](bool value) { reported = value; });

        // Simulate the user flipping the native switch (the UISwitch.ValueChanged analog).
        auto* platform = handler->typed_platform_view();
        platform->is_on = true;
        platform->on_value_changed();

        EXPECT_TRUE(control.is_toggled());
        EXPECT_TRUE(reported);
    }

    TEST(toggle_switch_seam, manual_set_overrides_the_native_value)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        platform->is_on = true;
        platform->on_value_changed(); // native toggle → stored at from-handler specificity
        EXPECT_TRUE(control.is_toggled());

        control.set_is_toggled(false); // a manual set overrides the handler value
        EXPECT_FALSE(control.is_toggled());
        EXPECT_FALSE(platform->is_on); // and maps back to the native state
    }

    TEST(toggle_switch_seam, clearing_handler_disconnects)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(toggle_switch_seam, handler_resolved_from_default_registry)
    {
        // toggle_switch -> switch_handler is self-registered in toggle_switch.cpp.
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<toggle_switch>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<switch_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        toggle_switch control;
        control.set_is_toggled(true);
        control.set_handler(handler);
        EXPECT_TRUE(resolved->typed_platform_view()->is_on);
    }
} // namespace

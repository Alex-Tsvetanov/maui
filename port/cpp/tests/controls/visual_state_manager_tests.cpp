// Tests for maui::controls::visual_state_manager (M5b) — go_to_state swaps the setters of the outgoing
// and incoming visual states, at the VSM specificity. System-driven states (Disabled, …) outrank a manual
// set; Normal / custom states sit below it. Driven through a real button + its text property.
#include "maui/controls/visual_state_manager.hpp"

#include <string>
#include <utility>

#include "maui/controls/button.hpp"
#include "maui/controls/setter.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::common_states;
    using maui::controls::setter;
    using maui::controls::visual_state;
    using maui::controls::visual_state_group;
    using maui::controls::visual_state_manager;

    // A CommonStates group with Normal + Disabled, each setting the button's text.
    visual_state_manager make_common_states_vsm(std::string normal_text, std::string disabled_text)
    {
        visual_state normal{std::string{common_states::normal}};
        normal.add(setter::of(button::text_property(), std::move(normal_text)));
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(button::text_property(), std::move(disabled_text)));

        visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(disabled));

        visual_state_manager manager;
        manager.add_group(std::move(group));
        return manager;
    }

    TEST(visual_state_manager, go_to_state_swaps_state_setters)
    {
        button target;
        visual_state_manager vsm = make_common_states_vsm("N", "D");

        EXPECT_TRUE(vsm.go_to_state(target, common_states::normal));
        EXPECT_EQ(target.text(), "N");

        EXPECT_TRUE(vsm.go_to_state(target, common_states::disabled));
        EXPECT_EQ(target.text(), "D"); // Normal's setter un-applied, Disabled's applied
    }

    TEST(visual_state_manager, unknown_state_returns_false_and_changes_nothing)
    {
        button target;
        visual_state_manager vsm = make_common_states_vsm("N", "D");
        vsm.go_to_state(target, common_states::normal);

        EXPECT_FALSE(vsm.go_to_state(target, "DoesNotExist"));
        EXPECT_EQ(target.text(), "N");
    }

    TEST(visual_state_manager, going_to_the_current_state_is_an_idempotent_noop)
    {
        button target;
        visual_state_manager vsm = make_common_states_vsm("N", "D");
        vsm.go_to_state(target, common_states::disabled);
        EXPECT_EQ(target.text(), "D");

        EXPECT_TRUE(vsm.go_to_state(target, common_states::disabled)); // already there
        EXPECT_EQ(target.text(), "D");
    }

    TEST(visual_state_manager, state_setter_outranks_a_manual_value_then_restores_it)
    {
        button target;
        visual_state_manager vsm = make_common_states_vsm("N", "D");
        target.set_text("manual"); // a manual set

        // A directly-driven VSM applies state setters above a manual value (visual_state_setter > manual).
        vsm.go_to_state(target, common_states::disabled);
        EXPECT_EQ(target.text(), "D");

        // Clearing the state's value would restore the manual one; here we instead transition to Normal,
        // whose setter ("N") simply replaces Disabled's at the same VSM specificity (still above manual).
        vsm.go_to_state(target, common_states::normal);
        EXPECT_EQ(target.text(), "N");
    }

    TEST(visual_state_manager, leaving_a_group_via_a_no_op_state_restores_the_manual_value)
    {
        button target;
        // A group whose Normal state declares no setters: transitioning Disabled -> Normal un-applies
        // Disabled's setter and applies nothing, so the underlying manual value re-emerges.
        visual_state normal{std::string{common_states::normal}}; // no setters
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(button::text_property(), std::string("D")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(disabled));
        visual_state_manager vsm;
        vsm.add_group(std::move(group));

        target.set_text("manual");
        vsm.go_to_state(target, common_states::disabled);
        EXPECT_EQ(target.text(), "D"); // VSM state value wins over the manual one

        vsm.go_to_state(target, common_states::normal);
        EXPECT_EQ(target.text(), "manual"); // Disabled un-applied, Normal sets nothing -> manual restored
    }

    // ---- M5d: the implicit-style VSM downgrade + system-state promotion (#18103 / #34363) ----
    // When the VSGroups arrive via an implicit style (mark_from_implicit_style), a CUSTOM state's setters
    // sit BELOW a manual value, but a SYSTEM-driven state (Disabled/…) is promoted back ABOVE it.

    TEST(visual_state_manager, implicit_style_custom_state_is_outranked_by_a_manual_value)
    {
        button target;
        // A custom (non-system) state "Highlight" in a group, sourced from an implicit style.
        visual_state highlight{"Highlight"};
        highlight.add(setter::of(button::text_property(), std::string("H")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(highlight));
        visual_state_manager vsm;
        vsm.add_group(std::move(group));
        vsm.mark_from_implicit_style(); // downgraded VSM specificity

        target.set_text("manual"); // a manual set
        vsm.go_to_state(target, "Highlight");
        EXPECT_EQ(target.text(), "manual"); // implicit-style custom-state VSM sits BELOW manual (#18103)
    }

    TEST(visual_state_manager, implicit_style_system_state_outranks_a_manual_value)
    {
        button target;
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(button::text_property(), std::string("D")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(disabled));
        visual_state_manager vsm;
        vsm.add_group(std::move(group));
        vsm.mark_from_implicit_style();

        target.set_text("manual");
        vsm.go_to_state(target, common_states::disabled);
        EXPECT_EQ(target.text(), "D"); // a system-driven state is promoted ABOVE manual (#34363)
    }

    TEST(visual_state_manager, directly_driven_custom_state_still_outranks_a_manual_value)
    {
        button target;
        // WITHOUT mark_from_implicit_style, even a custom state uses the full VSM specificity (> manual).
        visual_state highlight{"Highlight"};
        highlight.add(setter::of(button::text_property(), std::string("H")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(highlight));
        visual_state_manager vsm;
        vsm.add_group(std::move(group));

        target.set_text("manual");
        vsm.go_to_state(target, "Highlight");
        EXPECT_EQ(target.text(), "H"); // directly-driven VSM is above manual regardless of state name
    }

    // ---- M5d (unit H): view<> auto-drives Disabled/Normal from is_enabled (VisualElement.ChangeVisualState) ----
    TEST(visual_state_manager, view_auto_drives_disabled_and_normal_on_is_enabled_change)
    {
        button target;
        // Configure the control's OWN visual-state manager (visual_states()) with Normal + Disabled.
        visual_state normal{std::string{common_states::normal}};
        normal.add(setter::of(button::text_property(), std::string("N")));
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(button::text_property(), std::string("D")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(disabled));
        target.visual_states().add_group(std::move(group));

        target.change_visual_state(); // initial: enabled -> Normal
        EXPECT_EQ(target.text(), "N");

        target.set_is_enabled(false); // an is_enabled change auto-drives the VSM -> Disabled
        EXPECT_EQ(target.text(), "D");

        target.set_is_enabled(true); // -> Normal
        EXPECT_EQ(target.text(), "N");
    }
} // namespace

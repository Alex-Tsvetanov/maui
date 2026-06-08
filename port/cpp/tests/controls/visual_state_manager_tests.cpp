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
} // namespace

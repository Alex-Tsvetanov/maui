// Tests for the radio_button control + the radio_button_group attached grouping + the headless handler
// seam, ported from src/Controls/tests/Core.UnitTests/RadioButtonTests.cs (the group-name
// adoption/mutual-exclusion/selected-value suite) plus the seam coverage every control carries.
// The ControlTemplate-based tests (RadioButtonGroupWorksWithContentViewControlTemplate,
// RadioButtonTemplateTests.cs, RadioButtonContentAsStringTests.cs) stay un-ported with the templated
// content path (the port's radio_button is string-content only — documented in radio_button.hpp).
#include "maui/controls/radio_button.hpp"

#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/radio_button_group.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/radio_button_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::grid;
    using maui::controls::radio_button;
    using maui::controls::radio_button_group;
    using maui::controls::vertical_stack_layout;
    using maui::core::boxed_equals;
    using maui::core::radio_button_handler;

    // ---- the control in isolation ----

    TEST(radio_button, defaults_mirror_the_csharp_descriptors)
    {
        radio_button control;
        EXPECT_FALSE(control.is_checked());
        EXPECT_TRUE(control.group_name().empty());
        EXPECT_FALSE(control.value().has_value()); // ValueProperty default null
        EXPECT_TRUE(control.content().empty());
        EXPECT_EQ(control.stroke_thickness(), -1.0); // BorderElement.BorderWidthProperty's -1d sentinel
        EXPECT_EQ(control.corner_radius(), -1); // BorderElement.DefaultCornerRadius
    }

    // RadioButtonTests.ValuePropertyCanBeSetToNull.
    TEST(radio_button, value_property_can_be_set_to_null)
    {
        radio_button control;
        EXPECT_FALSE(control.value().has_value());

        control.set_value(std::any{1});
        EXPECT_TRUE(boxed_equals(control.value(), std::any{1}));

        control.set_value(std::any{});
        EXPECT_FALSE(control.value().has_value());
    }

    // RadioButton.OnIsCheckedPropertyChanged: CheckedChanged is raised with the new value.
    TEST(radio_button, checked_changed_raises_with_the_new_value)
    {
        radio_button control;
        std::vector<bool> values;
        control.checked_changed.connect([&values](bool value) { values.push_back(value); });

        control.set_is_checked(true);
        control.set_is_checked(true); // unchanged: no event
        control.set_is_checked(false);

        ASSERT_EQ(values.size(), 2U);
        EXPECT_TRUE(values[0]);
        EXPECT_FALSE(values[1]);
    }

    // RadioButton.ChangeVisualState → ApplyIsCheckedState: Checked/Unchecked drive the VSM.
    TEST(radio_button, checked_visual_state_drive)
    {
        radio_button control;
        maui::controls::visual_state checked{std::string{radio_button::checked_visual_state}};
        checked.add(maui::controls::setter::of(maui::controls::opacity_property(), 0.5));
        maui::controls::visual_state unchecked{std::string{radio_button::unchecked_visual_state}};
        unchecked.add(maui::controls::setter::of(maui::controls::opacity_property(), 1.0));
        maui::controls::visual_state_group group{"CheckedStates"};
        group.add(std::move(checked));
        group.add(std::move(unchecked));
        control.visual_states().add_group(std::move(group));
        control.change_visual_state();
        EXPECT_EQ(control.opacity(), 1.0); // Unchecked applied

        control.set_is_checked(true);
        EXPECT_EQ(control.opacity(), 0.5); // Checked applied

        control.set_is_checked(false);
        EXPECT_EQ(control.opacity(), 1.0);
    }

    // ---- RadioButtonGroup: group-name adoption ----

    // RadioButtonTests.RadioButtonAddedToGroupGetsGroupName.
    TEST(radio_button_group, radio_button_added_to_group_gets_group_name)
    {
        vertical_stack_layout layout;
        radio_button button;

        radio_button_group::set_group_name(layout, "foo");
        layout.add(button);

        EXPECT_EQ(button.group_name(), "foo");
    }

    // RadioButtonTests.NestedRadioButtonAddedToGroupGetsGroupName.
    TEST(radio_button_group, nested_radio_button_added_to_group_gets_group_name)
    {
        vertical_stack_layout layout;
        radio_button button;

        radio_button_group::set_group_name(layout, "foo");

        grid inner;
        inner.add(button);
        layout.add(inner);

        EXPECT_EQ(button.group_name(), "foo");
    }

    // RadioButtonTests.RadioButtonAddedToGroupKeepsGroupName.
    TEST(radio_button_group, radio_button_added_to_group_keeps_group_name)
    {
        vertical_stack_layout layout;
        radio_button button;
        button.set_group_name("bar");
        button.set_value(std::any{1});

        radio_button_group::set_group_name(layout, "foo");
        layout.add(button);

        EXPECT_EQ(button.group_name(), "bar");
    }

    // RadioButtonTests.LayoutGroupNameAppliesToExistingRadioButtons.
    TEST(radio_button_group, layout_group_name_applies_to_existing_radio_buttons)
    {
        vertical_stack_layout layout;
        radio_button button;

        layout.add(button);
        radio_button_group::set_group_name(layout, "foo");

        EXPECT_EQ(button.group_name(), "foo");
    }

    // RadioButtonTests.UpdatedGroupNameAppliesToRadioButtonsWithOldGroupName.
    TEST(radio_button_group, updated_group_name_applies_to_radio_buttons_with_old_group_name)
    {
        vertical_stack_layout layout;
        radio_button button1;
        radio_button button2;
        button2.set_group_name("other");

        layout.add(button1);
        layout.add(button2);
        radio_button_group::set_group_name(layout, "foo");
        radio_button_group::set_group_name(layout, "bar");

        EXPECT_EQ(button1.group_name(), "bar");
        EXPECT_EQ(button2.group_name(), "other");
    }

    // ---- RadioButtonGroup: mutual exclusion ----

    // RadioButtonTests.ThereCanBeOnlyOne (an explicit group with no attached layout group).
    TEST(radio_button_group, there_can_be_only_one)
    {
        radio_button button1;
        radio_button button2;
        radio_button button3;
        radio_button button4;
        for (auto* button : {&button1, &button2, &button3, &button4})
        {
            button->set_group_name("foo");
        }

        grid layout;
        layout.add(button1);
        layout.add(button2);
        layout.add(button3);
        layout.add(button4);

        button1.set_is_checked(true);

        EXPECT_TRUE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_FALSE(button3.is_checked());
        EXPECT_FALSE(button4.is_checked());

        button3.set_is_checked(true);

        EXPECT_FALSE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_TRUE(button3.is_checked());
        EXPECT_FALSE(button4.is_checked());
    }

    // RadioButtonTests.ImpliedGroup (no group names: siblings are mutually exclusive).
    TEST(radio_button_group, implied_group)
    {
        radio_button button1;
        radio_button button2;
        radio_button button3;

        grid layout;
        layout.add(button1);
        layout.add(button2);
        layout.add(button3);

        button1.set_is_checked(true);

        EXPECT_TRUE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_FALSE(button3.is_checked());

        button3.set_is_checked(true);

        EXPECT_FALSE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_TRUE(button3.is_checked());
    }

    // RadioButtonTests.ImpliedGroupDoesNotIncludeExplicitGroups.
    TEST(radio_button_group, implied_group_does_not_include_explicit_groups)
    {
        radio_button button1;
        radio_button button2;
        radio_button button3;
        button3.set_group_name("foo");

        grid layout;
        layout.add(button1);
        layout.add(button2);
        layout.add(button3);

        button1.set_is_checked(true);
        button3.set_is_checked(true);

        EXPECT_TRUE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_TRUE(button3.is_checked());
    }

    // The mutual exclusion stops at the nearest page root (RadioButtonGroup.GetVisualRoot): two grouped
    // buttons under the SAME page exclude each other even from different sub-layouts.
    TEST(radio_button_group, exclusion_walks_to_the_page_root)
    {
        maui::controls::content_page page;
        vertical_stack_layout outer;
        grid left;
        grid right;
        radio_button button1;
        radio_button button2;
        button1.set_group_name("foo");
        button2.set_group_name("foo");

        left.add(button1);
        right.add(button2);
        outer.add(left);
        outer.add(right);
        page.set_content(outer);

        button1.set_is_checked(true);
        button2.set_is_checked(true);

        EXPECT_FALSE(button1.is_checked());
        EXPECT_TRUE(button2.is_checked());
    }

    // ---- RadioButtonGroup: SelectedValue ----

    // RadioButtonTests.RemovingSelectedButtonFromGroupClearsSelection (the rename keeps it checked).
    TEST(radio_button_group, removing_selected_button_from_group_keeps_it_checked)
    {
        radio_button button1;
        radio_button button2;
        radio_button button3;
        for (auto* button : {&button1, &button2, &button3})
        {
            button->set_group_name("foo");
        }

        grid layout;
        layout.add(button1);
        layout.add(button2);
        layout.add(button3);

        button1.set_is_checked(true);
        button2.set_is_checked(true);

        EXPECT_FALSE(button1.is_checked());
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button3.is_checked());

        button2.set_group_name("bar");

        EXPECT_FALSE(button1.is_checked());
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button3.is_checked());
    }

    // RadioButtonTests.GroupControllerSelectionIsNullWhenSelectedButtonRemoved.
    TEST(radio_button_group, group_selection_is_null_when_selected_button_removed)
    {
        grid layout;
        radio_button_group::set_group_name(layout, "foo");
        EXPECT_FALSE(radio_button_group::selected_value(layout).has_value());

        radio_button button1;
        radio_button button2;
        radio_button button3;
        button1.set_value(std::any{1});
        button2.set_value(std::any{2});
        button3.set_value(std::any{3});

        layout.add(button1);
        layout.add(button2);
        layout.add(button3);

        EXPECT_FALSE(radio_button_group::selected_value(layout).has_value());

        button1.set_is_checked(true);
        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{1}));

        EXPECT_EQ(button1.group_name(), "foo");
        button1.set_group_name("bar");
        EXPECT_FALSE(radio_button_group::selected_value(layout).has_value());
    }

    // RadioButtonTests.GroupSelectedValueUpdatesWhenSelectedButtonValueUpdates.
    TEST(radio_button_group, group_selected_value_updates_when_selected_button_value_updates)
    {
        grid layout;
        radio_button_group::set_group_name(layout, "foo");

        radio_button button1;
        radio_button button2;
        radio_button button3;
        button1.set_value(std::any{1});
        button1.set_is_checked(true);
        button2.set_value(std::any{2});
        button3.set_value(std::any{3});

        layout.add(button1);
        layout.add(button2);
        layout.add(button3);

        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{1}));

        button1.set_value(std::any{std::string{"updated"}});

        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{std::string{"updated"}}));
    }

    // RadioButtonTests.GroupNullSelectionClearsAnySelection.
    TEST(radio_button_group, null_selection_clears_any_selection)
    {
        grid layout;
        radio_button_group::set_group_name(layout, "foo");

        radio_button button1;
        radio_button button2;
        button1.set_value(std::any{1});
        button1.set_is_checked(true);
        button2.set_value(std::any{2});

        layout.add(button1);
        layout.add(button2);

        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{1}));

        radio_button_group::set_selected_value(layout, std::any{});

        EXPECT_FALSE(button1.is_checked());
    }

    // RadioButtonTests.RadioButtonGroupWorksWithDynamicallyAddedDescendants.
    TEST(radio_button_group, works_with_dynamically_added_descendants)
    {
        vertical_stack_layout layout;
        radio_button_group::set_group_name(layout, "choices");
        radio_button_group::set_selected_value(layout, std::any{});

        vertical_stack_layout item_container;
        layout.add(item_container);

        radio_button button1;
        radio_button button2;
        radio_button button3;
        button1.set_value(std::any{std::string{"Choice 1"}});
        button2.set_value(std::any{std::string{"Choice 2"}});
        button3.set_value(std::any{std::string{"Choice 3"}});

        item_container.add(button1);
        item_container.add(button2);
        item_container.add(button3);

        EXPECT_EQ(button1.group_name(), "choices");
        EXPECT_EQ(button2.group_name(), "choices");
        EXPECT_EQ(button3.group_name(), "choices");
        EXPECT_FALSE(radio_button_group::selected_value(layout).has_value());

        button2.set_is_checked(true);
        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{std::string{"Choice 2"}}));

        button3.set_is_checked(true);
        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{std::string{"Choice 3"}}));

        EXPECT_FALSE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_TRUE(button3.is_checked());
    }

    // RadioButtonTests.RadioButtonGroupSelectedValueBindingWorksWithNestedDescendants.
    TEST(radio_button_group, selected_value_works_with_nested_descendants)
    {
        vertical_stack_layout layout;
        radio_button_group::set_group_name(layout, "choices");

        vertical_stack_layout item_container;
        layout.add(item_container);

        radio_button button1;
        radio_button button2;
        radio_button button3;
        button1.set_value(std::any{std::string{"Choice 1"}});
        button2.set_value(std::any{std::string{"Choice 2"}});
        button3.set_value(std::any{std::string{"Choice 3"}});

        item_container.add(button1);
        item_container.add(button2);
        item_container.add(button3);

        radio_button_group::set_selected_value(layout, std::any{std::string{"Choice 2"}});

        EXPECT_FALSE(button1.is_checked());
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button3.is_checked());

        radio_button_group::set_selected_value(layout, std::any{std::string{"Choice 3"}});

        EXPECT_FALSE(button1.is_checked());
        EXPECT_FALSE(button2.is_checked());
        EXPECT_TRUE(button3.is_checked());
    }

    // RadioButtonTests.RadioButtonGroupAutoChecksMatchingButtonInContentViewWhenSelectedValuePreset,
    // minus the ContentView/ControlTemplate wrapper (templated content is out of the port's cut): a
    // pre-set SelectedValue auto-checks a later-added matching button.
    TEST(radio_button_group, preset_selected_value_auto_checks_matching_button)
    {
        vertical_stack_layout layout;
        radio_button_group::set_group_name(layout, "Test2");
        radio_button_group::set_selected_value(layout, std::any{std::string{"opt2"}});

        radio_button button1;
        radio_button button2;
        button1.set_group_name("Test2");
        button1.set_value(std::any{std::string{"opt1"}});
        button2.set_group_name("Test2");
        button2.set_value(std::any{std::string{"opt2"}});

        layout.add(button1);
        layout.add(button2);

        EXPECT_FALSE(button1.is_checked());
        EXPECT_TRUE(button2.is_checked());
        EXPECT_TRUE(boxed_equals(radio_button_group::selected_value(layout), std::any{std::string{"opt2"}}));
    }

    // The port's deterministic-teardown analog of RadioButtonGroupLayoutShouldNotLeak: the controller
    // dies with its layout, and a detached radio button's weak association expires (no dangling group).
    TEST(radio_button_group, controller_dies_with_the_layout)
    {
        radio_button button;
        {
            vertical_stack_layout layout;
            radio_button_group::set_group_name(layout, "GroupA");
            layout.add(button);
            EXPECT_EQ(button.group_name(), "GroupA");
            layout.remove_at(0); // detach before the layout dies (children are caller-owned)
        }
        // The layout (and its controller) are gone; checking the detached button must not crash and
        // must not resurrect a group.
        button.set_is_checked(true);
        EXPECT_TRUE(button.is_checked());
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(radio_button_seam, attaching_handler_maps_initial_properties)
    {
        radio_button control;
        control.set_content("Option A");
        control.set_is_checked(true);
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->is_checked);
        EXPECT_EQ(platform->content, "Option A");
    }

    TEST(radio_button_seam, setting_properties_maps_to_platform)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_is_checked(true);
        EXPECT_TRUE(platform->is_checked);

        control.set_content("Pick me");
        EXPECT_EQ(platform->content, "Pick me");

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->text_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_character_spacing(2.0);
        EXPECT_EQ(platform->character_spacing, 2.0);

        control.set_stroke_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(platform->stroke_color, maui::graphics::color(0.0F, 1.0F, 0.0F));

        control.set_stroke_thickness(2.0);
        EXPECT_EQ(platform->stroke_thickness, 2.0);

        control.set_corner_radius(6);
        EXPECT_EQ(platform->corner_radius, 6);
    }

    // The native selection channel: a user tap SELECTS the radio button (a radio can't un-tap itself),
    // written back at the from-handler specificity (RadioButton.SelectRadioButton).
    TEST(radio_button_seam, simulated_native_select_checks_the_control)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        bool changed = false;
        control.checked_changed.connect([&changed](bool value) { changed = value; });

        platform->on_select();
        EXPECT_TRUE(control.is_checked());
        EXPECT_TRUE(changed);
        EXPECT_TRUE(platform->is_checked); // mapped back to the native mirror
    }

    // The native selection drives the group exclusion exactly like a manual IsChecked set.
    TEST(radio_button_seam, native_select_drives_the_group_exclusion)
    {
        grid layout;
        radio_button button1;
        radio_button button2;
        button1.set_group_name("foo");
        button2.set_group_name("foo");
        layout.add(button1);
        layout.add(button2);

        auto handler1 = std::make_shared<radio_button_handler>();
        auto handler2 = std::make_shared<radio_button_handler>();
        button1.set_handler(handler1);
        button2.set_handler(handler2);

        handler1->typed_platform_view()->on_select();
        EXPECT_TRUE(button1.is_checked());

        handler2->typed_platform_view()->on_select();
        EXPECT_TRUE(button2.is_checked());
        EXPECT_FALSE(button1.is_checked());
        EXPECT_FALSE(handler1->typed_platform_view()->is_checked);
    }

    TEST(radio_button_seam, clearing_handler_disconnects)
    {
        radio_button control;
        auto handler = std::make_shared<radio_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(radio_button_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<maui::core::i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<radio_button>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<radio_button_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        radio_button control;
        control.set_content("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->content, "Registered");
    }
} // namespace

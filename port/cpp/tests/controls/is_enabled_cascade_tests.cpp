// Tests for the IsEnabled parent->child cascade (VisualElement.IsEnabledCore + RefreshIsEnabledProperty).
// The effective (coerced) IsEnabled a view reports — and pushes to its handler via MapIsEnabled — is the
// developer's explicit value AND every ancestor's effective enabled: a control whose parent layout is
// disabled is itself disabled, even though its own IsEnabled is still true. Changing a parent's IsEnabled
// re-coerces the whole subtree and re-pushes each descendant's handler. Driven through a real button (its
// button_handler exposes a view_platform_base whose `enabled` mirror records what MapIsEnabled pushed) and
// a vertical_stack_layout parent. Ported behavior from src/Controls/src/Core/VisualElement/VisualElement.cs
// (IsEnabledCore / CoerceIsEnabledProperty / OnIsEnabledPropertyChanged -> PropagatePropertyChanged).
#include "maui/controls/button.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/visual_state_manager.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/core/button_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::common_states;
    using maui::controls::setter;
    using maui::controls::vertical_stack_layout;
    using maui::controls::visual_state;
    using maui::controls::visual_state_group;
    using maui::controls::visual_state_manager;
    using maui::core::button_handler;
    using maui::core::layout_handler;
    using maui::core::view_platform_base;

    // The `enabled` mirror a button handler's platform base recorded (what MapIsEnabled last pushed).
    bool handler_enabled(const std::shared_ptr<button_handler>& handler)
    {
        view_platform_base* base = handler->platform_base();
        EXPECT_NE(base, nullptr);
        return base != nullptr && base->enabled;
    }

    // (a) An explicitly-enabled child inside a disabled parent reports effective IsEnabled false, and the
    // change propagates to the already-attached child handler (MapIsEnabled re-pushed false).
    TEST(is_enabled_cascade, disabling_parent_disables_attached_child)
    {
        vertical_stack_layout parent;
        button child;
        child.set_is_enabled(true);
        parent.add(child);

        auto parent_handler = std::make_shared<layout_handler>();
        parent.set_handler(parent_handler);
        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);

        // Both enabled before the parent is disabled.
        EXPECT_TRUE(child.is_enabled());
        EXPECT_TRUE(handler_enabled(child_handler));

        parent.set_is_enabled(false);

        EXPECT_FALSE(child.is_enabled());             // effective (coerced) is disabled
        EXPECT_TRUE(child.is_explicitly_enabled());   // the developer's raw value is untouched
        EXPECT_FALSE(handler_enabled(child_handler)); // the cascade re-pushed false to the handler
    }

    // (a') A child attached AFTER its parent was disabled still renders disabled — the coerced getter walks
    // the parent chain when MapIsEnabled runs on connect (the static-tree case the gallery exercises).
    TEST(is_enabled_cascade, child_attached_under_disabled_parent_is_disabled_on_connect)
    {
        vertical_stack_layout parent;
        parent.set_is_enabled(false);
        button child; // raw IsEnabled defaults true
        parent.add(child);

        EXPECT_FALSE(child.is_enabled());
        EXPECT_TRUE(child.is_explicitly_enabled());

        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);
        EXPECT_FALSE(handler_enabled(child_handler)); // coerced false pushed on connect
    }

    // (b) Re-enabling the parent re-enables a child that is NOT explicitly disabled.
    TEST(is_enabled_cascade, re_enabling_parent_re_enables_implicit_child)
    {
        vertical_stack_layout parent;
        button child; // raw true
        parent.add(child);

        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);

        parent.set_is_enabled(false);
        EXPECT_FALSE(child.is_enabled());
        EXPECT_FALSE(handler_enabled(child_handler));

        parent.set_is_enabled(true);
        EXPECT_TRUE(child.is_enabled());
        EXPECT_TRUE(handler_enabled(child_handler)); // re-pushed true
    }

    // (c) An explicitly-disabled child stays disabled across any parent toggle.
    TEST(is_enabled_cascade, explicitly_disabled_child_stays_disabled)
    {
        vertical_stack_layout parent;
        button child;
        child.set_is_enabled(false); // explicit
        parent.add(child);

        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);

        EXPECT_FALSE(child.is_enabled());
        EXPECT_FALSE(handler_enabled(child_handler));

        parent.set_is_enabled(false);
        EXPECT_FALSE(handler_enabled(child_handler));

        parent.set_is_enabled(true); // parent enabled, but the child is explicitly disabled
        EXPECT_FALSE(child.is_enabled());
        EXPECT_FALSE(child.is_explicitly_enabled());
        EXPECT_FALSE(handler_enabled(child_handler));
    }

    // (d) A two-deep cascade reaches a grandchild even when the middle layout has no handler.
    TEST(is_enabled_cascade, two_deep_cascade_reaches_grandchild)
    {
        vertical_stack_layout outer;
        vertical_stack_layout inner; // no handler attached
        button leaf;                 // raw true
        inner.add(leaf);
        outer.add(inner);

        auto leaf_handler = std::make_shared<button_handler>();
        leaf.set_handler(leaf_handler);

        EXPECT_TRUE(leaf.is_enabled());
        EXPECT_TRUE(handler_enabled(leaf_handler));

        outer.set_is_enabled(false);
        EXPECT_FALSE(inner.is_enabled()); // middle coerced through its parent
        EXPECT_FALSE(leaf.is_enabled());  // leaf coerced through two levels
        EXPECT_FALSE(handler_enabled(leaf_handler));

        outer.set_is_enabled(true);
        EXPECT_TRUE(leaf.is_enabled());
        EXPECT_TRUE(handler_enabled(leaf_handler));
    }

    // (e) Requirement #3: the Disabled VISUAL STATE reflects the COERCED value — a child of a disabled
    // parent enters CommonStates.Disabled (ChangeVisualState reads the coerced is_enabled()).
    TEST(is_enabled_cascade, disabled_parent_drives_child_to_disabled_visual_state)
    {
        vertical_stack_layout parent;
        button child; // raw true
        parent.add(child);

        // A CommonStates group whose Disabled state rewrites the button text.
        visual_state normal{std::string{common_states::normal}};
        normal.add(setter::of(button::text_property(), std::string("N")));
        visual_state disabled{std::string{common_states::disabled}};
        disabled.add(setter::of(button::text_property(), std::string("D")));
        visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(disabled));
        visual_state_manager groups;
        groups.add_group(std::move(group));
        child.set_visual_state_groups(std::move(groups)); // runs ChangeVisualState once -> Normal (enabled)

        EXPECT_EQ(child.text(), "N");

        parent.set_is_enabled(false); // cascade -> child.ChangeVisualState -> Disabled (coerced)
        EXPECT_EQ(child.text(), "D");

        parent.set_is_enabled(true); // back to Normal
        EXPECT_EQ(child.text(), "N");
    }
} // namespace

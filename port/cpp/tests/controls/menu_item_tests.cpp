// Tests for menu_item / toolbar_item / menu_flyout_item — ported from
// src/Controls/tests/Core.UnitTests/MenuItemTests.cs (Activated, the IsEnabled hierarchy quartet,
// KeyboardAccelerator) and ToolbarItemTests.cs (which inherits MenuItemTests), plus ToolbarItem.cs's
// constructor contract (null action throws; name/icon/order/priority wired) and the Order/Priority
// defaults. The C# Command tests collapse onto the clicked event (the port's command-as-clicked-event
// channel — ICommand is not ported; STATUS.md W1-11).
#include "maui/controls/menu_item.hpp"

#include <stdexcept>
#include <string>

#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_separator.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_menu_flyout_separator.hpp"
#include "maui/core/i_menu_flyout_sub_item.hpp"
#include "maui/core/keyboard_accelerator.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::menu_flyout_item;
    using maui::controls::menu_flyout_separator;
    using maui::controls::menu_flyout_sub_item;
    using maui::controls::menu_item;
    using maui::controls::toolbar_item;
    using maui::controls::toolbar_item_order;
    using maui::core::keyboard_accelerator;
    using maui::core::keyboard_accelerator_modifiers;

    // C# MenuItemTests.Activated: Activate() raises Clicked.
    TEST(menu_item, activate_raises_clicked)
    {
        menu_item item;
        bool activated = false;
        item.clicked.connect([&activated] { activated = true; });

        item.activate();

        EXPECT_TRUE(activated);
    }

    // The native inbound (IMenuElement.Clicked()) routes through Activate → Clicked.
    TEST(menu_item, send_clicked_routes_to_activate)
    {
        menu_item item;
        int count = 0;
        item.clicked.connect([&count] { ++count; });

        static_cast<maui::core::i_menu_element&>(item).send_clicked();

        EXPECT_EQ(count, 1);
    }

    TEST(menu_item, defaults)
    {
        const menu_item item;
        EXPECT_EQ(item.text(), "");
        EXPECT_TRUE(item.is_enabled());
        EXPECT_FALSE(item.is_destructive());
        EXPECT_EQ(item.icon_image_source(), nullptr);
    }

    // C# MenuItemTests.KeyboardAccelerator: add one, read Modifiers + Key back.
    TEST(menu_flyout_item, keyboard_accelerator_round_trips)
    {
        menu_flyout_item item;
        item.accelerators().push_back(
            keyboard_accelerator{.modifiers = keyboard_accelerator_modifiers::ctrl, .key = "A"});

        const auto accelerators = item.keyboard_accelerators();
        ASSERT_EQ(accelerators.size(), 1U);
        EXPECT_EQ(accelerators[0].modifiers, keyboard_accelerator_modifiers::ctrl);
        EXPECT_EQ(accelerators[0].key, "A");
    }

    // C# MenuItemTests.MenuItemsDisabledWhenParentDisabled.
    TEST(menu_item, items_disabled_when_parent_disabled)
    {
        menu_item item1;
        menu_item item2;
        menu_flyout_sub_item menu;
        menu.items().add(item1);
        menu.items().add(item2);

        EXPECT_TRUE(menu.is_enabled());
        EXPECT_TRUE(item1.is_enabled());
        EXPECT_TRUE(item2.is_enabled());

        menu.set_is_enabled(false);

        EXPECT_FALSE(menu.is_enabled());
        EXPECT_FALSE(item1.is_enabled());
        EXPECT_FALSE(item2.is_enabled());
    }

    // C# MenuItemTests.ExplicitlyDisabledMenuItemsRemainsDisabledWhenParentEnabled.
    TEST(menu_item, explicitly_disabled_item_remains_disabled_when_parent_enabled)
    {
        menu_item item1;
        item1.set_is_enabled(false);
        menu_flyout_sub_item menu;
        menu.items().add(item1);

        EXPECT_TRUE(menu.is_enabled());
        EXPECT_FALSE(item1.is_enabled());

        menu.set_is_enabled(false);
        EXPECT_FALSE(menu.is_enabled());
        EXPECT_FALSE(item1.is_enabled());

        menu.set_is_enabled(true);
        EXPECT_TRUE(menu.is_enabled());
        EXPECT_FALSE(item1.is_enabled());
    }

    // C# MenuItemTests.MenuHierarchyCanBeDisabled.
    TEST(menu_item, menu_hierarchy_can_be_disabled)
    {
        menu_flyout_sub_item top_menu;
        menu_flyout_sub_item middle_menu;
        menu_item middle_item;
        menu_item bottom_item1;
        menu_item bottom_item2;

        middle_menu.items().add(bottom_item1);
        middle_menu.items().add(bottom_item2);
        top_menu.items().add(middle_menu);
        top_menu.items().add(middle_item);

        EXPECT_TRUE(top_menu.is_enabled());
        EXPECT_TRUE(middle_item.is_enabled());
        EXPECT_TRUE(middle_menu.is_enabled());
        EXPECT_TRUE(bottom_item1.is_enabled());
        EXPECT_TRUE(bottom_item2.is_enabled());

        top_menu.set_is_enabled(false);

        EXPECT_FALSE(top_menu.is_enabled());
        EXPECT_FALSE(middle_item.is_enabled());
        EXPECT_FALSE(middle_menu.is_enabled());
        EXPECT_FALSE(bottom_item1.is_enabled());
        EXPECT_FALSE(bottom_item2.is_enabled());
    }

    // C# MenuItemTests.PartialHierarchyCanBeDisabled.
    TEST(menu_item, partial_hierarchy_can_be_disabled)
    {
        menu_flyout_sub_item top_menu;
        menu_flyout_sub_item middle_menu;
        menu_item middle_item;
        menu_item bottom_item1;
        menu_item bottom_item2;

        middle_menu.items().add(bottom_item1);
        middle_menu.items().add(bottom_item2);
        top_menu.items().add(middle_menu);
        top_menu.items().add(middle_item);

        middle_menu.set_is_enabled(false);

        EXPECT_TRUE(top_menu.is_enabled());
        EXPECT_TRUE(middle_item.is_enabled());
        EXPECT_FALSE(middle_menu.is_enabled());
        EXPECT_FALSE(bottom_item1.is_enabled());
        EXPECT_FALSE(bottom_item2.is_enabled());
    }

    // ---- toolbar_item (ToolbarItem.cs) ----

    TEST(toolbar_item, defaults_order_and_priority)
    {
        const toolbar_item item;
        EXPECT_EQ(item.order(), toolbar_item_order::default_order);
        EXPECT_EQ(item.priority(), 0);
        EXPECT_FALSE(item.is_secondary());
    }

    // The (name, icon, activated, order, priority) constructor — Text/IconImageSource/Clicked wiring.
    TEST(toolbar_item, convenience_constructor_wires_action_and_properties)
    {
        bool fired = false;
        toolbar_item item("Save", "save.png", [&fired] { fired = true; }, toolbar_item_order::secondary, 7);

        EXPECT_EQ(item.text(), "Save");
        ASSERT_NE(item.icon_image_source(), nullptr);
        EXPECT_EQ(item.order(), toolbar_item_order::secondary);
        EXPECT_TRUE(item.is_secondary());
        EXPECT_EQ(item.priority(), 7);

        item.activate();
        EXPECT_TRUE(fired);
    }

    // C# throws ArgumentNullException when `activated` is null.
    TEST(toolbar_item, convenience_constructor_throws_on_null_action)
    {
        EXPECT_THROW(toolbar_item("Save", "save.png", nullptr), std::invalid_argument);
    }

    // ---- the separator / sub-item contracts the native menu builders probe ----

    TEST(menu_flyout_separator, is_detectable_via_the_core_contract)
    {
        menu_flyout_separator separator;
        maui::core::i_menu_element& element = separator;
        EXPECT_NE(dynamic_cast<maui::core::i_menu_flyout_separator*>(&element), nullptr);
    }

    TEST(menu_flyout_sub_item, exposes_children_via_the_core_contract)
    {
        menu_flyout_sub_item sub;
        menu_flyout_item child;
        child.set_text("Child");
        sub.items().add(child);

        maui::core::i_menu_element& element = sub;
        auto* contract = dynamic_cast<maui::core::i_menu_flyout_sub_item*>(&element);
        ASSERT_NE(contract, nullptr);
        ASSERT_EQ(contract->item_count(), 1U);
        EXPECT_EQ(contract->item_at(0)->text(), "Child");
    }
} // namespace

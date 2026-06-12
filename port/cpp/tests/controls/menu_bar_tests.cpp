// Tests for the menu container family (menu_bar / menu_bar_item / menu_flyout / menu_flyout_sub_item) —
// ported from src/Controls/tests/Core.UnitTests/Menu/MenuTestBase.cs (UsingIndexUpdatesParent /
// ClearUpdatesParent / Add-Remove-InsertCallsCorrectHandlerMethod, instantiated per container like the
// C# generic fixture), MenuBarItemTests.cs (StartsEnabled / DisableWorks), MenuBarTests.cs (the
// window does-not-reassign tests), and Menu/ContextFlyoutTests.cs (BindingContext propagation through
// the attached ContextFlyout). The C# per-element handler Invoke collapses onto the
// menu_element_list handler-notify seam — actions "add"/"remove"/"insert" with (index, item).
#include "maui/controls/menu_bar.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_base.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_menu_bar.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::controls::flyout_base;
    using maui::controls::menu_bar;
    using maui::controls::menu_bar_item;
    using maui::controls::menu_flyout;
    using maui::controls::menu_flyout_item;
    using maui::controls::menu_flyout_sub_item;
    using maui::controls::menu_item;
    using maui::controls::window;

    // One captured handler notification (the C# *HandlerUpdate payload).
    struct notification
    {
        std::string action;
        std::size_t index = 0;
        const void* item = nullptr;
    };

    template <class TItem>
    void capture(maui::controls::menu_element_list<TItem>& list, std::vector<notification>& events)
    {
        list.set_handler_notify([&events](std::string_view action, std::size_t index, TItem* item) {
            events.push_back(notification{std::string(action), index, item});
        });
    }

    // ---- MenuTestBase.UsingIndexUpdatesParent (the container branch) ----

    TEST(menu_flyout, using_index_updates_parent)
    {
        menu_flyout flyout;
        menu_flyout_item child0;
        menu_flyout_item child1;

        EXPECT_EQ(child0.logical_parent(), nullptr);
        EXPECT_EQ(child1.logical_parent(), nullptr);

        flyout.items().add(child0);
        EXPECT_EQ(child0.logical_parent(), &flyout);
        EXPECT_EQ(child1.logical_parent(), nullptr);

        flyout.items().set_at(0, child1);
        EXPECT_EQ(child0.logical_parent(), nullptr);
        EXPECT_EQ(child1.logical_parent(), &flyout);
    }

    TEST(menu_bar_item, using_index_updates_parent)
    {
        menu_bar_item bar_item;
        menu_flyout_item child0;
        menu_flyout_item child1;

        bar_item.items().add(child0);
        EXPECT_EQ(child0.logical_parent(), &bar_item);

        bar_item.items().set_at(0, child1);
        EXPECT_EQ(child0.logical_parent(), nullptr);
        EXPECT_EQ(child1.logical_parent(), &bar_item);
    }

    // The MenuBarItem branch of the C# test: menu bar items are parented by PAGES, not the menu bar.
    TEST(menu_bar, items_parent_to_the_page_not_the_menu_bar)
    {
        content_page page;
        menu_bar_item child0;
        menu_bar_item child1;

        page.menu_bar_items().add(child0);
        EXPECT_EQ(child0.logical_parent(), &page);

        // A menu_bar aggregate add does NOT reparent (C# MenuBar.Add never calls AddLogicalChild).
        menu_bar bar;
        bar.items().add(child1);
        EXPECT_EQ(child1.logical_parent(), nullptr);

        page.menu_bar_items().set_at(0, child1);
        EXPECT_EQ(child0.logical_parent(), nullptr);
        EXPECT_EQ(child1.logical_parent(), &page);
    }

    // ---- MenuTestBase.ClearUpdatesParent (the container branch). The C# MenuBarItem/page/window branch
    // leaves STALE parents after Clear (a tracker-sync bookkeeping quirk); the port detaches uniformly —
    // documented deviation. ----

    TEST(menu_flyout_sub_item, clear_updates_parent)
    {
        menu_flyout_sub_item sub;
        menu_flyout_item child0;
        menu_flyout_item child1;

        sub.items().add(child0);
        sub.items().add(child1);
        EXPECT_EQ(child0.logical_parent(), &sub);
        EXPECT_EQ(child1.logical_parent(), &sub);

        sub.items().clear();
        EXPECT_EQ(child0.logical_parent(), nullptr);
        EXPECT_EQ(child1.logical_parent(), nullptr);
    }

    // ---- MenuTestBase.Add/Remove/InsertCallsCorrectHandlerMethod (per container) ----

    TEST(menu_bar, add_calls_correct_handler_method)
    {
        menu_bar bar;
        std::vector<notification> events;
        capture(bar.items(), events);

        menu_bar_item child0;
        bar.items().add(child0);

        ASSERT_EQ(events.size(), 1U);
        EXPECT_EQ(events[0].action, "add");
        EXPECT_EQ(events[0].index, 0U);
        EXPECT_EQ(events[0].item, &child0);
    }

    TEST(menu_bar, remove_calls_correct_handler_method)
    {
        menu_bar bar;
        menu_bar_item child0;
        bar.items().add(child0);

        std::vector<notification> events;
        capture(bar.items(), events);
        bar.items().remove(child0);

        ASSERT_EQ(events.size(), 1U);
        EXPECT_EQ(events[0].action, "remove");
        EXPECT_EQ(events[0].index, 0U);
        EXPECT_EQ(events[0].item, &child0);
    }

    TEST(menu_bar, insert_calls_correct_handler_method)
    {
        menu_bar bar;
        menu_bar_item child0;
        menu_bar_item child1;
        menu_bar_item child2;
        bar.items().add(child0);
        bar.items().add(child2);

        std::vector<notification> events;
        capture(bar.items(), events);
        bar.items().insert(1, child1);

        ASSERT_EQ(events.size(), 1U);
        EXPECT_EQ(events[0].action, "insert");
        EXPECT_EQ(events[0].index, 1U);
        EXPECT_EQ(events[0].item, &child1);
    }

    TEST(menu_flyout, add_remove_insert_call_correct_handler_methods)
    {
        menu_flyout flyout;
        menu_flyout_item child0;
        menu_flyout_item child1;
        menu_flyout_item child2;

        std::vector<notification> events;
        capture(flyout.items(), events);

        flyout.items().add(child0);
        flyout.items().add(child2);
        flyout.items().insert(1, child1);
        flyout.items().remove(child2);

        ASSERT_EQ(events.size(), 4U);
        EXPECT_EQ(events[2].action, "insert");
        EXPECT_EQ(events[2].index, 1U);
        EXPECT_EQ(events[2].item, &child1);
        EXPECT_EQ(events[3].action, "remove");
        EXPECT_EQ(events[3].index, 2U);
        EXPECT_EQ(events[3].item, &child2);
    }

    // ---- MenuBarItemTests.StartsEnabled / DisableWorks ----

    TEST(menu_bar_item, starts_enabled)
    {
        const menu_bar_item item;
        EXPECT_TRUE(item.is_enabled());
    }

    TEST(menu_bar_item, disable_works)
    {
        menu_bar_item item;
        item.set_is_enabled(false);
        EXPECT_FALSE(item.is_enabled());
    }

    // ---- MenuBarTests.UsingWindowDoesNotReAssignParents ----

    TEST(menu_bar, using_window_does_not_reassign_parents)
    {
        menu_flyout_item flyout_item;
        menu_bar_item bar_item;
        bar_item.items().add(flyout_item);

        content_page page;
        page.menu_bar_items().add(bar_item);

        EXPECT_EQ(flyout_item.logical_parent(), &bar_item);
        EXPECT_EQ(bar_item.logical_parent(), &page);

        window host(page);

        EXPECT_EQ(flyout_item.logical_parent(), &bar_item);
        EXPECT_EQ(bar_item.logical_parent(), &page);

        auto* bar = host.menu_bar();
        ASSERT_NE(bar, nullptr);

        EXPECT_EQ(flyout_item.logical_parent(), &bar_item);
        EXPECT_EQ(bar_item.logical_parent(), &page);
        EXPECT_EQ(bar->item_count(), 1U);
        EXPECT_EQ(bar->item_at(0), &bar_item);
    }

    // ---- MenuBarTests.UsingWindowDoesNotReAssignBindingContext (adapted: the port asserts the
    // page-inherited context object stays on the flyout item through windowing + menu_bar access) ----

    TEST(menu_bar, using_window_does_not_reassign_binding_context)
    {
        auto context = std::make_shared<std::string>("Matthew");

        menu_flyout_item flyout_item;
        menu_bar_item bar_item;
        bar_item.items().add(flyout_item);

        content_page page;
        page.set_binding_context(context);
        page.menu_bar_items().add(bar_item);

        EXPECT_EQ(flyout_item.binding_context<std::string>(), context);

        window host(page);
        EXPECT_EQ(flyout_item.binding_context<std::string>(), context);

        ASSERT_NE(host.menu_bar(), nullptr);
        EXPECT_EQ(flyout_item.binding_context<std::string>(), context);
    }

    // ---- ContextFlyoutTests (BindingContext propagation through the attached ContextFlyout) ----

    TEST(context_flyout, binding_context_propagates_when_flyout_already_set_on_parent)
    {
        button host;
        menu_flyout_sub_item sub;
        menu_flyout_item item;
        menu_flyout flyout;
        flyout.items().add(sub);
        sub.items().add(item);

        flyout_base::set_context_flyout(host, &flyout);

        auto context = std::make_shared<std::string>("bc");
        host.set_binding_context(context);

        EXPECT_EQ(sub.binding_context<std::string>(), context);
        EXPECT_EQ(flyout.binding_context<std::string>(), context);
    }

    TEST(context_flyout, binding_context_propagates_when_flyout_set_after_context)
    {
        button host;
        menu_flyout_sub_item sub;
        menu_flyout_item item;
        sub.items().add(item);

        auto context = std::make_shared<std::string>("bc");
        host.set_binding_context(context);

        menu_flyout flyout;
        flyout.items().add(sub);
        flyout_base::set_context_flyout(host, &flyout);

        EXPECT_EQ(sub.binding_context<std::string>(), context);
        EXPECT_EQ(flyout.binding_context<std::string>(), context);
    }

    TEST(context_flyout, binding_context_propagates_to_items_added_after_attach)
    {
        button host;
        menu_flyout_sub_item sub;
        menu_flyout_item item;

        auto context = std::make_shared<std::string>("bc");
        host.set_binding_context(context);

        menu_flyout flyout;
        flyout.items().add(sub);
        flyout_base::set_context_flyout(host, &flyout);

        // Add the leaf AFTER the flyout is attached to the button.
        sub.items().add(item);

        EXPECT_EQ(sub.binding_context<std::string>(), context);
        EXPECT_EQ(item.binding_context<std::string>(), context);
    }

    // ---- MenuBar.SyncMenuBarItemsFromPages (the reconciliation the tracker drives) ----

    TEST(menu_bar, sync_menu_bar_items_from_pages_reconciles)
    {
        menu_bar bar;
        menu_bar_item item_a;
        menu_bar_item item_b;
        menu_bar_item item_c;

        bar.sync_menu_bar_items_from_pages({&item_a, &item_b});
        ASSERT_EQ(bar.item_count(), 2U);
        EXPECT_EQ(bar.item_at(0), &item_a);
        EXPECT_EQ(bar.item_at(1), &item_b);

        // Reorder + replace + trim.
        bar.sync_menu_bar_items_from_pages({&item_b, &item_c});
        ASSERT_EQ(bar.item_count(), 2U);
        EXPECT_EQ(bar.item_at(0), &item_b);
        EXPECT_EQ(bar.item_at(1), &item_c);

        bar.sync_menu_bar_items_from_pages({});
        EXPECT_EQ(bar.item_count(), 0U);
    }
} // namespace

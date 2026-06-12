// Tests for toolbar_tracker / menu_bar_tracker — ported from src/Controls/tests/Core.UnitTests/
// ToolbarTrackerTests.cs and Menu/MenuBarTrackerTests.cs: Constructor / SimpleTrackEmpty /
// SimpleTrackWithItems / AdditionalTargets / PushAfterTrackingStarted / PopAfterTrackingStarted /
// UnsetTarget / AddingMenuBarItemsFireCollectionChanged / NavigationToolBar (the window-toolbar
// aggregate), plus the priority sort (ToolBarItemComparer) and a current-page-switch adaptation of
// TrackPreConstructedTabbedPage (TabbedPage is not ported; the navigation stack's current page plays
// the container role — documented).
#include "maui/controls/toolbar_tracker.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_bar_tracker.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_toolbar.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::menu_bar_item;
    using maui::controls::menu_bar_tracker;
    using maui::controls::navigation_page;
    using maui::controls::toolbar_item;
    using maui::controls::toolbar_tracker;
    using maui::controls::window;

    template <class TItem> bool contains(const std::vector<TItem*>& items, const TItem& item)
    {
        return std::ranges::find(items, &item) != items.end();
    }

    // C# ToolbarTrackerTests.Constructor.
    TEST(toolbar_tracker, constructor)
    {
        toolbar_tracker tracker;
        EXPECT_EQ(tracker.target(), nullptr);
        EXPECT_TRUE(tracker.toolbar_items().empty());
    }

    // C# ToolbarTrackerTests.SimpleTrackEmpty.
    TEST(toolbar_tracker, simple_track_empty)
    {
        toolbar_tracker tracker;
        content_page page;
        tracker.set_target(&page);
        EXPECT_TRUE(tracker.toolbar_items().empty());
    }

    // C# ToolbarTrackerTests.SimpleTrackWithItems.
    TEST(toolbar_tracker, simple_track_with_items)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});
        content_page page;
        page.toolbar_items().add(item1);
        page.toolbar_items().add(item2);

        tracker.set_target(&page);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));
    }

    // The ToolBarItemComparer sort: lower Priority first; equal priorities keep insertion order.
    TEST(toolbar_tracker, items_sort_by_priority)
    {
        toolbar_tracker tracker;
        toolbar_item low("Low", "Low.png", [] {});
        low.set_priority(1);
        toolbar_item high("High", "High.png", [] {});
        high.set_priority(10);
        toolbar_item also_low("AlsoLow", "AlsoLow.png", [] {});
        also_low.set_priority(1);

        content_page page;
        page.toolbar_items().add(high);
        page.toolbar_items().add(low);
        page.toolbar_items().add(also_low);
        tracker.set_target(&page);

        const auto items = tracker.toolbar_items();
        ASSERT_EQ(items.size(), 3U);
        EXPECT_EQ(items[0], &low);      // priority 1, added before also_low
        EXPECT_EQ(items[1], &also_low); // stable among equals
        EXPECT_EQ(items[2], &high);
    }

    // C# ToolbarTrackerTests.AdditionalTargets.
    TEST(toolbar_tracker, additional_targets)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});

        content_page page;
        page.toolbar_items().add(item1);
        content_page additional;
        additional.toolbar_items().add(item2);

        tracker.set_target(&page);
        tracker.set_additional_targets({&additional});

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));
    }

    // C# ToolbarTrackerTests.PushAfterTrackingStarted.
    TEST(toolbar_tracker, push_after_tracking_started)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});

        navigation_page nav;
        nav.toolbar_items().add(item1);
        content_page first_page;
        first_page.toolbar_items().add(item2);

        tracker.set_target(&nav);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_FALSE(contains(tracker.toolbar_items(), item2));

        nav.push(first_page);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));
    }

    // C# ToolbarTrackerTests.PopAfterTrackingStarted.
    TEST(toolbar_tracker, pop_after_tracking_started)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});

        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(item1);
        content_page first_page;
        first_page.toolbar_items().add(item2);

        tracker.set_target(&nav);
        nav.push(first_page);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));

        nav.pop();

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_FALSE(contains(tracker.toolbar_items(), item2));
    }

    // C# ToolbarTrackerTests.UnsetTarget.
    TEST(toolbar_tracker, unset_target)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});
        content_page page;
        page.toolbar_items().add(item1);
        page.toolbar_items().add(item2);

        tracker.set_target(&page);
        EXPECT_EQ(tracker.toolbar_items().size(), 2U);

        tracker.set_target(nullptr);
        EXPECT_TRUE(tracker.toolbar_items().empty());
    }

    // The current-page switch (the TrackPreConstructedTabbedPage adaptation): only the CURRENT page's
    // items aggregate; a navigation fires CollectionChanged.
    TEST(toolbar_tracker, current_page_switch_fires_collection_changed)
    {
        toolbar_tracker tracker;
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});

        content_page page1;
        page1.toolbar_items().add(item1);
        content_page page2;
        page2.toolbar_items().add(item2);

        navigation_page nav(page1);
        tracker.set_target(&nav);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_FALSE(contains(tracker.toolbar_items(), item2));

        bool changed = false;
        const auto token = tracker.collection_changed.connect([&changed] { changed = true; });
        nav.push(page2);
        tracker.collection_changed.disconnect(token);

        EXPECT_TRUE(changed);
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));
    }

    // C# ToolbarTrackerTests.NavigationToolBar: the WINDOW toolbar aggregates the nav hierarchy.
    TEST(toolbar_tracker, navigation_tool_bar)
    {
        toolbar_item item1("Foo", "Foo.png", [] {});
        toolbar_item item2("Bar", "Bar.png", [] {});

        navigation_page nav;
        nav.toolbar_items().add(item1);

        // The port creates the window toolbar when the hosted page is a navigation_page (C#: the
        // NavigationPage assigns a NavigationPageToolbar to its window).
        window host;
        host.set_content(nav);

        content_page first_page;
        first_page.toolbar_items().add(item2);
        nav.push(first_page);

        auto* toolbar = host.toolbar();
        ASSERT_NE(toolbar, nullptr);
        EXPECT_TRUE(toolbar->is_visible());
        ASSERT_EQ(toolbar->item_count(), 2U);
        bool found1 = false;
        bool found2 = false;
        for (std::size_t i = 0; i < toolbar->item_count(); ++i)
        {
            found1 = found1 || toolbar->item_at(i) == &item1;
            found2 = found2 || toolbar->item_at(i) == &item2;
        }
        EXPECT_TRUE(found1);
        EXPECT_TRUE(found2);
    }

    // A window hosting a PLAIN page has no toolbar (C# parity: no NavigationPage → no toolbar).
    TEST(toolbar_tracker, plain_page_window_has_no_toolbar)
    {
        content_page page;
        window host(page);
        EXPECT_EQ(host.toolbar(), nullptr);
    }

    // ---- menu_bar_tracker (MenuBarTrackerTests.cs) ----

    TEST(menu_bar_tracker, constructor)
    {
        menu_bar_tracker tracker;
        EXPECT_EQ(tracker.target(), nullptr);
        EXPECT_TRUE(tracker.toolbar_items().empty());
        EXPECT_EQ(tracker.menu_bar(), nullptr); // empty aggregate → no menu bar
    }

    TEST(menu_bar_tracker, simple_track_empty)
    {
        menu_bar_tracker tracker;
        content_page page;
        tracker.set_target(&page);
        EXPECT_TRUE(tracker.toolbar_items().empty());
        EXPECT_EQ(tracker.menu_bar(), nullptr);
    }

    TEST(menu_bar_tracker, simple_track_with_items)
    {
        menu_bar_tracker tracker;
        menu_bar_item item1;
        menu_bar_item item2;
        content_page page;
        page.menu_bar_items().add(item1);
        page.menu_bar_items().add(item2);

        tracker.set_target(&page);

        EXPECT_TRUE(contains(tracker.toolbar_items(), item1));
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));

        // The synced chrome reflects the aggregate.
        auto* bar = tracker.menu_bar();
        ASSERT_NE(bar, nullptr);
        ASSERT_EQ(bar->item_count(), 2U);
        EXPECT_EQ(bar->item_at(0), &item1);
        EXPECT_EQ(bar->item_at(1), &item2);
    }

    // C# MenuBarTrackerTests.AddingMenuBarItemsFireCollectionChanged.
    TEST(menu_bar_tracker, adding_menu_bar_items_fires_collection_changed)
    {
        menu_bar_tracker tracker;
        menu_bar_item item1;
        menu_bar_item item2;

        content_page sub_page;
        sub_page.menu_bar_items().add(item1);

        navigation_page nav(sub_page);
        tracker.set_target(&nav);

        bool changed = false;
        const auto token = tracker.collection_changed.connect([&changed] { changed = true; });
        sub_page.menu_bar_items().add(item2);
        tracker.collection_changed.disconnect(token);

        EXPECT_TRUE(changed);
        EXPECT_TRUE(contains(tracker.toolbar_items(), item2));
    }

    TEST(menu_bar_tracker, unset_target)
    {
        menu_bar_tracker tracker;
        menu_bar_item item1;
        content_page page;
        page.menu_bar_items().add(item1);

        tracker.set_target(&page);
        EXPECT_EQ(tracker.toolbar_items().size(), 1U);

        tracker.set_target(nullptr);
        EXPECT_TRUE(tracker.toolbar_items().empty());
        EXPECT_EQ(tracker.menu_bar(), nullptr);
    }
} // namespace

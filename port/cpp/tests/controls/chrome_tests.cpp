// Tests for the W1-11 window/view chrome seams on the headless backend:
//   (1) window_handler maps "toolbar" / "menu_bar" / "title_bar" — the window_platform mirrors track
//       the window's chrome (the AppKit twin materializes the real NSToolbar / NSMenu / titlebar
//       accessory from the same borrows);
//   (2) the navigation chrome surfaces the page toolbar items (navigation_page_platform.toolbar_items
//       mirrors the i_stack_navigation aggregate — the iOS twin builds real bar buttons from it);
//   (3) the shared view_mapper's "tool_tip" / "context_flyout" maps reach view_platform_base on any
//       control (ViewHandler.MapToolTip / MapContextFlyout — the dynamic_cast probes);
//   (4) window.set_title_bar replaces/clears the hosted title bar (the TitleBarTests.cs replace shape).
// Behavior derived from WindowHandler.cs's mapper + ToolTipProperties.cs + FlyoutBase.cs (the C# unit
// suites are thin here — these are characterization tests of the source semantics).
#include "maui/controls/window.hpp"

#include <memory>
#include <optional>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_base.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/title_bar.hpp"
#include "maui/controls/tool_tip_properties.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_menu_bar.hpp"
#include "maui/core/i_title_bar.hpp"
#include "maui/core/i_toolbar.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/window_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::controls::flyout_base;
    using maui::controls::menu_bar_item;
    using maui::controls::menu_flyout;
    using maui::controls::menu_flyout_item;
    using maui::controls::navigation_page;
    using maui::controls::title_bar;
    using maui::controls::tool_tip_properties;
    using maui::controls::toolbar_item;
    using maui::controls::toolbar_item_order;
    using maui::controls::window;
    using maui::core::button_handler;
    using maui::core::navigation_page_handler;
    using maui::core::window_handler;

    // ---- (1) window_handler chrome maps ----

    TEST(window_chrome, menu_bar_maps_to_the_platform_mirror)
    {
        menu_bar_item file_menu;
        file_menu.set_text("File");
        menu_flyout_item open_item;
        open_item.set_text("Open");
        file_menu.items().add(open_item);

        content_page page;
        page.menu_bar_items().add(file_menu);

        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->hosted_menu_bar, nullptr);
        ASSERT_EQ(platform->hosted_menu_bar->item_count(), 1U);
        EXPECT_EQ(platform->hosted_menu_bar->item_at(0)->text(), "File");
        EXPECT_EQ(platform->hosted_menu_bar->item_at(0)->item_count(), 1U);
    }

    TEST(window_chrome, menu_bar_mirror_is_null_without_items_and_updates_on_add)
    {
        content_page page;
        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_menu_bar, nullptr); // C# MenuBarTracker.MenuBar == null when empty

        // Adding a page menu bar item AFTER the handler attached re-runs the "menu_bar" map (the
        // tracker's collection-changed → handlerProperty poke).
        menu_bar_item file_menu;
        page.menu_bar_items().add(file_menu);
        ASSERT_NE(platform->hosted_menu_bar, nullptr);
        EXPECT_EQ(platform->hosted_menu_bar->item_count(), 1U);
    }

    TEST(window_chrome, toolbar_maps_to_the_platform_mirror_with_sorted_items)
    {
        toolbar_item second("Second", "", [] {});
        second.set_priority(2);
        toolbar_item first("First", "", [] {});
        first.set_priority(1);

        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(second);
        nav.toolbar_items().add(first);

        window host;
        host.set_content(nav);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->hosted_toolbar, nullptr);
        EXPECT_TRUE(platform->hosted_toolbar->is_visible());
        ASSERT_EQ(platform->hosted_toolbar->item_count(), 2U);
        EXPECT_EQ(platform->hosted_toolbar->item_at(0)->text(), "First"); // priority sort
        EXPECT_EQ(platform->hosted_toolbar->item_at(1)->text(), "Second");
    }

    TEST(window_chrome, toolbar_item_added_after_attach_reaches_the_mirror)
    {
        content_page root;
        navigation_page nav(root);
        window host;
        host.set_content(nav);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_NE(platform->hosted_toolbar, nullptr);
        EXPECT_EQ(platform->hosted_toolbar->item_count(), 0U);

        // The CURRENT page's items surface too (the tracker recursion into current_page).
        toolbar_item item("Save", "", [] {});
        root.toolbar_items().add(item);
        EXPECT_EQ(platform->hosted_toolbar->item_count(), 1U);
        EXPECT_EQ(platform->hosted_toolbar->item_at(0)->text(), "Save");
        EXPECT_FALSE(platform->hosted_toolbar->item_at(0)->is_secondary());
    }

    // ---- (2) navigation chrome: the page toolbar items reach the navigation platform mirror ----

    TEST(navigation_chrome, toolbar_items_mirror_tracks_the_current_page)
    {
        toolbar_item nav_item("NavItem", "", [] {});
        toolbar_item page_item("PageItem", "", [] {}, toolbar_item_order::secondary);

        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(nav_item);

        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_EQ(platform->toolbar_items.size(), 1U);
        EXPECT_EQ(platform->toolbar_items[0]->text(), "NavItem");

        // Pushing a page with its own item re-aggregates (nav item + current page item).
        content_page second;
        second.toolbar_items().add(page_item);
        nav.push(second);
        ASSERT_EQ(platform->toolbar_items.size(), 2U);
        EXPECT_EQ(platform->toolbar_items[1]->text(), "PageItem");
        EXPECT_TRUE(platform->toolbar_items[1]->is_secondary());

        // Popping drops the page's item again.
        nav.pop();
        ASSERT_EQ(platform->toolbar_items.size(), 1U);
        EXPECT_EQ(platform->toolbar_items[0]->text(), "NavItem");
    }

    TEST(navigation_chrome, adding_an_item_without_navigation_refreshes_the_mirror)
    {
        content_page root;
        navigation_page nav(root);
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->toolbar_items.empty());

        toolbar_item item("Added", "", [] {});
        root.toolbar_items().add(item); // tracker collection-changed → re-issued navigation request
        ASSERT_EQ(platform->toolbar_items.size(), 1U);
        EXPECT_EQ(platform->toolbar_items[0]->text(), "Added");
    }

    // ---- (3) the shared view_mapper: ToolTip + ContextFlyout on any control ----

    TEST(view_chrome, tool_tip_maps_to_the_platform_base)
    {
        button host;
        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);

        auto* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_EQ(base->tool_tip, std::nullopt); // never set → nullopt (the IsSet probe)

        tool_tip_properties::set_text(host, "Click me");
        EXPECT_EQ(tool_tip_properties::get_text(host), std::optional<std::string>("Click me"));
        ASSERT_TRUE(base->tool_tip.has_value());
        EXPECT_EQ(*base->tool_tip, "Click me");
    }

    TEST(view_chrome, tool_tip_set_before_attach_reaches_the_platform_on_connect)
    {
        button host;
        tool_tip_properties::set_text(host, "Early");

        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler); // SetVirtualView runs the full mapper, incl. "tool_tip"

        auto* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        ASSERT_TRUE(base->tool_tip.has_value());
        EXPECT_EQ(*base->tool_tip, "Early");
    }

    TEST(view_chrome, context_flyout_maps_to_the_platform_base)
    {
        button host;
        menu_flyout flyout;
        menu_flyout_item item;
        item.set_text("Copy");
        flyout.items().add(item);

        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);

        auto* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_EQ(base->context_flyout, nullptr);

        flyout_base::set_context_flyout(host, &flyout);
        EXPECT_EQ(base->context_flyout, &flyout);
        EXPECT_EQ(flyout.logical_parent(), &host); // C# AddRemoveLogicalChildren

        flyout_base::set_context_flyout(host, nullptr);
        EXPECT_EQ(base->context_flyout, nullptr);
        EXPECT_EQ(flyout.logical_parent(), nullptr);
    }

    // ---- (4) window title bar (Window.TitleBar basics) ----

    TEST(window_chrome, title_bar_maps_and_replaces_and_clears)
    {
        content_page page;
        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_title_bar, nullptr);

        title_bar first;
        first.set_title("My App");
        first.set_subtitle("Pro");
        host.set_title_bar(&first);
        ASSERT_EQ(platform->hosted_title_bar, &first);
        EXPECT_EQ(platform->hosted_title_bar->title(), "My App");
        EXPECT_EQ(platform->hosted_title_bar->subtitle(), "Pro");

        // Replace (the TitleBarTests.cs window.TitleBar = secondTitleBar shape) ...
        title_bar second;
        second.set_title("Second");
        host.set_title_bar(&second);
        EXPECT_EQ(platform->hosted_title_bar, &second);

        // ... and clear.
        host.set_title_bar(nullptr);
        EXPECT_EQ(platform->hosted_title_bar, nullptr);
    }
} // namespace

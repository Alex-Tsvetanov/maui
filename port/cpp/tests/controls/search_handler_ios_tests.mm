// iOS (UIKit) backend tests for the shell search box (X2) — the on-simulator install gate. The chrome
// installs a real UISearchController into the current section's nav item (mirrors
// ShellPageRendererTracker.AttachSearchController) driven by the page's Shell.SearchHandler; native edits +
// the search-button tap + the bookmark (clear-placeholder) tap route back to the model. Compiled as
// Objective-C++ with ARC for the `ios` backend.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/search_box_visibility.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell_handler.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::search_box_visibility;
    using maui::controls::search_handler;
    using maui::controls::shell;
    using maui::controls::shell_content;
    using maui::controls::shell_item;
    using maui::controls::shell_section;
    using maui::core::shell_handler;

    UISearchController* native_search_controller(const std::shared_ptr<shell_handler>& handler)
    {
        void* const ptr = handler->typed_platform_view()->search_controller;
        return ptr != nullptr ? (__bridge UISearchController*)ptr : nil;
    }

    UINavigationItem* current_nav_item(const std::shared_ptr<shell_handler>& handler)
    {
        UITabBarController* const tabs = (__bridge UITabBarController*)handler->typed_platform_view()->tab_host;
        // Resolve by the mirror's selected_index (deterministic — selectedViewController is unreliable for a
        // tab controller not in a window), matching the chrome's install path.
        const int idx = handler->typed_platform_view()->tree.current_item_renderer.selected_index;
        if (idx < 0 || static_cast<NSUInteger>(idx) >= tabs.viewControllers.count)
        {
            return nil;
        }
        UIViewController* const selected = tabs.viewControllers[static_cast<NSUInteger>(idx)];
        UINavigationController* const nav =
            [selected isKindOfClass:[UINavigationController class]] ? (UINavigationController*)selected : nil;
        return nav != nil ? nav.topViewController.navigationItem : nil;
    }

    class ios_search_seam : public maui::controls::shell_tests::shell_test_base
    {
    protected:
        std::shared_ptr<content_page> build_single_page_shell(shell& sh)
        {
            auto page = make_page();
            auto item = std::make_shared<shell_item>();
            item->set_route("home");
            auto section = std::make_shared<shell_section>();
            section->set_route("tab");
            auto content = std::make_shared<shell_content>();
            content->set_content(page.get());
            content->set_route("content");
            section->add(content);
            item->add(section);
            sh.add_item(item);
            return page;
        }
    };

    TEST_F(ios_search_seam, installs_search_controller_into_nav_item)
    {
        auto model = std::make_shared<search_handler>();
        model->set_placeholder("Find");

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        UISearchController* const controller = native_search_controller(handler);
        ASSERT_NE(controller, nil);
        EXPECT_EQ(std::string(controller.searchBar.placeholder.UTF8String), "Find");
        // It is installed onto the current section's nav item (Expanded default).
        UINavigationItem* const nav_item = current_nav_item(handler);
        ASSERT_NE(nav_item, nil);
        EXPECT_EQ(nav_item.searchController, controller);
    }

    TEST_F(ios_search_seam, collapsible_sets_hides_when_scrolling)
    {
        auto model = std::make_shared<search_handler>();
        model->set_search_box_visibility(search_box_visibility::collapsible);

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UINavigationItem* const nav_item = current_nav_item(handler);
        ASSERT_NE(nav_item, nil);
        EXPECT_NE(nav_item.searchController, nil);
        EXPECT_TRUE(nav_item.hidesSearchBarWhenScrolling);
    }

    TEST_F(ios_search_seam, hidden_removes_controller_from_nav_item)
    {
        auto model = std::make_shared<search_handler>();

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UINavigationItem* const nav_item = current_nav_item(handler);
        ASSERT_NE(nav_item, nil);
        EXPECT_NE(nav_item.searchController, nil);

        model->set_search_box_visibility(search_box_visibility::hidden);
        EXPECT_EQ(nav_item.searchController, nil);
    }

    TEST_F(ios_search_seam, clear_placeholder_enabled_shows_bookmark)
    {
        auto model = std::make_shared<search_handler>();
        model->set_clear_placeholder_enabled(true);

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UISearchController* const controller = native_search_controller(handler);
        ASSERT_NE(controller, nil);
        EXPECT_TRUE(controller.searchBar.showsBookmarkButton);
    }

    // Native edits / search-button / bookmark taps route to the model.
    TEST_F(ios_search_seam, native_events_route_to_model)
    {
        auto model = std::make_shared<search_handler>();
        std::string last_query;
        auto qt = maui::core::connect_scoped(model->query_changed,
                                             [&](std::string, std::string n) { last_query = std::move(n); });
        int confirmed = 0;
        auto ct = maui::core::connect_scoped(model->queried, [&](std::string_view) { ++confirmed; });
        int cleared = 0;
        model->clear_placeholder_command = [&] { ++cleared; };

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        UISearchController* const controller = native_search_controller(handler);
        ASSERT_NE(controller, nil);
        UISearchBar* const bar = controller.searchBar;
        id<UISearchBarDelegate> const del = bar.delegate;
        ASSERT_NE(del, nil);

        [del searchBar:bar textDidChange:@"hi"];
        EXPECT_EQ(last_query, "hi");
        EXPECT_EQ(std::string(model->query()), "hi");

        [del searchBarSearchButtonClicked:bar];
        EXPECT_EQ(confirmed, 1);

        [del searchBarBookmarkButtonClicked:bar];
        EXPECT_EQ(cleared, 1);
    }
} // namespace

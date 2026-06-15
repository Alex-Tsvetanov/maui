// Apple (AppKit) backend tests for the shell search box (X2). The chrome installs a real NSSearchField
// (added above the tab content — the documented AppKit deviation: macOS has no UISearchController) driven
// by the page's Shell.SearchHandler; native edits + the search action (Return) route back to the model.
// Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

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

    NSSearchField* native_search_field(const std::shared_ptr<shell_handler>& handler)
    {
        void* const ptr = handler->typed_platform_view()->search_controller;
        return ptr != nullptr ? (__bridge NSSearchField*)ptr : nil;
    }

    class apple_search_seam : public maui::controls::shell_tests::shell_test_base
    {
    protected:
        void SetUp() override
        {
            shell_test_base::SetUp();
            [NSApplication sharedApplication];
        }

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

    TEST_F(apple_search_seam, attaches_real_search_field)
    {
        auto model = std::make_shared<search_handler>();
        model->set_placeholder("Find");

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        NSSearchField* const field = native_search_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_FALSE(field.hidden);
        EXPECT_EQ(std::string(field.placeholderString.UTF8String), "Find");
    }

    TEST_F(apple_search_seam, hidden_visibility_hides_field)
    {
        auto model = std::make_shared<search_handler>();
        model->set_search_box_visibility(search_box_visibility::expanded);

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSSearchField* const field = native_search_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_FALSE(field.hidden);

        // Flipping to Hidden hides the field through the property-changed subscription.
        model->set_search_box_visibility(search_box_visibility::hidden);
        EXPECT_TRUE(field.hidden);
    }

    TEST_F(apple_search_seam, is_search_enabled_maps_to_field)
    {
        auto model = std::make_shared<search_handler>();

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSSearchField* const field = native_search_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_TRUE(field.enabled);

        model->set_is_search_enabled(false);
        EXPECT_FALSE(field.enabled);
    }

    // A native edit (controlTextDidChange) routes to OnQueryChanged; the search action confirms.
    TEST_F(apple_search_seam, native_edit_and_search_route_to_model)
    {
        auto model = std::make_shared<search_handler>();
        std::string last_query;
        auto qt = maui::core::connect_scoped(model->query_changed,
                                             [&](std::string, std::string n) { last_query = std::move(n); });
        int confirmed = 0;
        auto ct = maui::core::connect_scoped(model->queried, [&](std::string_view) { ++confirmed; });

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSSearchField* const field = native_search_field(handler);
        ASSERT_NE(field, nil);

        // Simulate a user edit: set the text and post the editing notification AppKit would.
        field.stringValue = @"hello";
        [field.delegate controlTextDidChange:[NSNotification notificationWithName:NSControlTextDidChangeNotification
                                                                           object:field]];
        EXPECT_EQ(last_query, "hello");
        EXPECT_EQ(std::string(model->query()), "hello");

        // The search action (Return) confirms.
        [field.target performSelector:field.action withObject:field];
        EXPECT_EQ(confirmed, 1);
    }

    // The model's Query drives the field's text WITHOUT bouncing back through the delegate (no echo loop).
    TEST_F(apple_search_seam, model_query_drives_field_without_echo)
    {
        auto model = std::make_shared<search_handler>();
        int change_events = 0;
        auto qt = maui::core::connect_scoped(model->query_changed, [&](std::string, std::string) { ++change_events; });

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        NSSearchField* const field = native_search_field(handler);
        ASSERT_NE(field, nil);

        model->set_query("typed");                                     // one change event (the model set)
        EXPECT_EQ(std::string(field.stringValue.UTF8String), "typed"); // pushed to the field
        EXPECT_EQ(change_events, 1);                                   // and NOT echoed back through the delegate
    }
} // namespace

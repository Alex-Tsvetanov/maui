#pragma once
// Shared fixture + helpers for the shell suites — ported from ShellTestBase.cs (TestShell /
// ShellTestPage / CreateShellItem|Section|Content / MakeSimpleShellSection / RegisterPage).
//
// Ownership notes (§8): the C# tests lean on the GC for page lifetime; here the fixture OWNS every
// page it mints (owned_pages_ is the FIRST member, so pages — event publishers for the wrapper
// syncs — outlive everything created after them in a test body).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/i_query_attributable.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"
#include "maui/controls/templates/data_template.hpp"
#include <gtest/gtest.h>

namespace maui::controls::shell_tests
{
    // C# ShellTestPage: a page capturing the query dictionaries it receives (IQueryAttributable) —
    // the [QueryProperty] reflection mirror collapses onto the same hook (see i_query_attributable.hpp).
    class shell_test_page : public content_page, public i_query_attributable
    {
    public:
        std::vector<shell_route_parameters> applied_query_attributes;
        std::string some_query_parameter;
        std::string double_query_parameter;

        void apply_query_attributes(const shell_route_parameters& query) override
        {
            applied_query_attributes.push_back(query);
            if (query.contains("SomeQueryParameter"))
            {
                some_query_parameter = query.get_string("SomeQueryParameter");
            }
            if (query.contains("DoubleQueryParameter"))
            {
                double_query_parameter = query.get_string("DoubleQueryParameter");
            }
        }
    };

    // C# TestPage1/2/3 — distinct page types the route registrations mint.
    class test_page1 : public content_page
    {
    };
    class test_page2 : public content_page
    {
    };
    class test_page3 : public content_page
    {
    };

    // C# TestShell.ConcretePageFactory: a factory returning one fixed (externally-owned) page.
    class concrete_page_factory final : public route_factory
    {
    public:
        explicit concrete_page_factory(std::shared_ptr<content_page> page) : page_(std::move(page))
        {
        }
        [[nodiscard]] std::shared_ptr<content_page> get_or_create() override
        {
            return page_;
        }

    private:
        std::shared_ptr<content_page> page_;
    };

    // C# TestShellSection — records the animated flag of the last pop.
    class test_shell_section : public shell_section
    {
    public:
        std::optional<bool> last_pop_was_animated;

        content_page* on_pop(bool animated) override
        {
            last_pop_was_animated = animated;
            return shell_section::on_pop(animated);
        }
    };

    // C# ShellNavigatingTests.NavigationMonitoringTab — records which stack mutators fired.
    class navigation_monitoring_tab : public tab
    {
    public:
        std::vector<std::string> navigations_fired;

        void on_push(content_page& page, bool animated) override
        {
            navigations_fired.emplace_back("OnPushAsync");
            tab::on_push(page, animated);
        }
        void on_remove_page(content_page& page) override
        {
            tab::on_remove_page(page);
            navigations_fired.emplace_back("OnRemovePage");
        }
        content_page* on_pop(bool animated) override
        {
            navigations_fired.emplace_back("OnPopAsync");
            return tab::on_pop(animated);
        }
    };

    // C# TestShell: counts Navigating/Navigated (event + virtual hook) and records the last args.
    class test_shell : public shell
    {
    public:
        int on_navigated_count = 0;
        int on_navigating_count = 0;
        int navigated_count = 0;
        int navigating_count = 0;

        std::optional<std::string> last_navigating_from; // Current (nullopt = null state)
        std::string last_navigating_to;                  // Target.Location
        shell_navigation_source last_navigating_source = shell_navigation_source::unknown;

        std::optional<std::string> last_navigated_from; // Previous
        std::string last_navigated_to;                  // Current.Location
        shell_navigation_source last_navigated_source = shell_navigation_source::unknown;

        test_shell()
        {
            routing::register_route<test_page1>("TestPage1");
            routing::register_route<test_page2>("TestPage2");
            routing::register_route<test_page3>("TestPage3");
            navigating_count_token_ = maui::core::connect_scoped(
                navigating, [this](shell_navigating_event_args& /*args*/) { ++navigating_count; });
            navigated_count_token_ = maui::core::connect_scoped(
                navigated, [this](const shell_navigated_event_args& /*args*/) { ++navigated_count; });
        }

        void assert_current_state_equals(std::string_view expected) const
        {
            ASSERT_NE(current_state(), nullptr);
            EXPECT_EQ(expected, current_state()->location());
        }

        void test_navigating_args(shell_navigation_source source, const std::optional<std::string>& from,
                                  std::string_view to) const
        {
            EXPECT_EQ(source, last_navigating_source);
            EXPECT_EQ(from, last_navigating_from);
            EXPECT_EQ(to, last_navigating_to);
        }

        void test_navigated_args(shell_navigation_source source, const std::optional<std::string>& from,
                                 const std::string& to) const
        {
            EXPECT_EQ(source, last_navigated_source);
            EXPECT_EQ(from, last_navigated_from);
            EXPECT_EQ(to, last_navigated_to);
            ASSERT_NE(current_state(), nullptr);
            EXPECT_EQ(to, current_state()->location());
        }

    protected:
        void on_navigating(shell_navigating_event_args& args) override
        {
            last_navigating_source = args.source();
            last_navigating_from = args.current() ? std::optional{args.current()->location()} : std::nullopt;
            last_navigating_to = args.target().location();
            ++on_navigating_count;
        }
        void on_navigated(const shell_navigated_event_args& args) override
        {
            last_navigated_source = args.source();
            last_navigated_from = args.previous() ? std::optional{args.previous()->location()} : std::nullopt;
            last_navigated_to = args.current().location();
            ++on_navigated_count;
        }

    private:
        maui::core::scoped_connection navigating_count_token_;
        maui::core::scoped_connection navigated_count_token_;
    };

    class shell_test_base : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            routing::clear();
        }
        void TearDown() override
        {
            routing::clear();
        }

        // Mint a fixture-owned page (the C# `new ContentPage()`); fixture members outlive every
        // test-body local (§8 — pages publish the wrapper-sync events the wrappers subscribe to).
        std::shared_ptr<content_page> make_page()
        {
            auto page = std::make_shared<content_page>();
            owned_pages_.push_back(page);
            return page;
        }
        std::shared_ptr<shell_test_page> make_test_page()
        {
            auto page = std::make_shared<shell_test_page>();
            owned_pages_.push_back(page);
            return page;
        }

        // C# TestShell.RegisterPage(route): a fixture-owned page behind a concrete factory.
        std::shared_ptr<content_page> register_page(const std::string& route)
        {
            std::shared_ptr<content_page> page = make_page();
            routing::set_route(*page, route);
            routing::register_route(route, std::make_shared<concrete_page_factory>(page));
            return page;
        }

        std::shared_ptr<shell_content> create_shell_content(std::shared_ptr<content_page> page = nullptr,
                                                            bool as_implicit = false,
                                                            const std::string& shell_content_route = {},
                                                            bool templated = false)
        {
            if (page == nullptr)
            {
                page = make_page();
            }
            // C# CreateShellContent ordering: a non-empty shellContentRoute mints a ROUTE-BEARING,
            // non-implicit ShellContent EVEN when asImplicit is true (the route check comes first).
            // Only an empty route + asImplicit adopts the page as an implicit content.
            if (shell_content_route.empty() && as_implicit)
            {
                return shell_content::adopt(*page);
            }
            auto content = std::make_shared<shell_content>();
            if (templated)
            {
                content->set_content_template(std::make_shared<data_template>(
                    [page]() -> std::shared_ptr<maui::core::bindable_object> { return page; }));
            }
            else
            {
                content->set_content(page.get());
            }
            if (!shell_content_route.empty())
            {
                content->set_route(shell_content_route);
            }
            return content;
        }

        template <class TSection = shell_section>
        std::shared_ptr<TSection> create_shell_section(std::shared_ptr<content_page> page = nullptr,
                                                       bool as_implicit = false,
                                                       const std::string& shell_content_route = {},
                                                       const std::string& shell_section_route = {},
                                                       bool templated = false)
        {
            std::shared_ptr<shell_content> content =
                create_shell_content(std::move(page), as_implicit, shell_content_route, templated);
            if (!shell_section_route.empty())
            {
                auto section = std::make_shared<TSection>();
                section->set_route(shell_section_route);
                section->add(std::move(content));
                return section;
            }
            if (as_implicit)
            {
                // The factory mints a plain shell_section in C# too (CreateFromShellContent).
                return std::static_pointer_cast<TSection>(shell_section::create_from_shell_content(std::move(content)));
            }
            auto section = std::make_shared<TSection>();
            section->add(std::move(content));
            return section;
        }

        template <class TItem = shell_item>
        std::shared_ptr<TItem> create_shell_item(std::shared_ptr<content_page> page = nullptr, bool as_implicit = false,
                                                 const std::string& shell_content_route = {},
                                                 const std::string& shell_section_route = {},
                                                 const std::string& shell_item_route = {}, bool templated = false)
        {
            std::shared_ptr<shell_section> section =
                create_shell_section(std::move(page), as_implicit, shell_content_route, shell_section_route, templated);
            if (!shell_item_route.empty())
            {
                auto item = std::make_shared<TItem>();
                item->set_route(shell_item_route);
                item->add(std::move(section));
                return item;
            }
            if (as_implicit)
            {
                return std::static_pointer_cast<TItem>(shell_item::create_from_shell_section(std::move(section)));
            }
            auto item = std::make_shared<TItem>();
            item->add(std::move(section));
            return item;
        }

        // C# MakeSimpleShellSection(route, contentRoute[, page]) — content defaults to a ShellTestPage.
        std::shared_ptr<shell_section> make_simple_shell_section(const std::string& route,
                                                                 const std::string& content_route,
                                                                 std::shared_ptr<content_page> page = nullptr)
        {
            if (page == nullptr)
            {
                page = make_test_page();
            }
            auto section = std::make_shared<shell_section>();
            section->set_route(route);
            auto content = std::make_shared<shell_content>();
            content->set_content(page.get());
            content->set_route(content_route);
            section->add(std::move(content));
            return section;
        }

        static shell_uri create_uri(const std::string& uri)
        {
            return shell_uri_handler::create_uri(uri);
        }

        static std::string location_of(const shell& host)
        {
            return host.current_state() != nullptr ? host.current_state()->location() : std::string{};
        }

        // Fixture-owned pages: every test-body local (shells, wrappers — the subscribers) is
        // destroyed before any fixture member, so these publishers always outlive them (§8).
        std::vector<std::shared_ptr<content_page>> owned_pages_;
    };
} // namespace maui::controls::shell_tests

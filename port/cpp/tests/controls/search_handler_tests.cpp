// search_handler_tests — the Shell SearchHandler model (X2). Three groups:
//   (1) the MODEL behavior (defaults, OnQueryChanged, OnQueryConfirmed, OnItemSelected, clear-placeholder,
//       Command/event order, ItemsSource → ListProxy, SelectedItem) — derived from SearchHandler.cs.
//   (2) the Shell.SearchHandler ATTACHED PROPERTY (set/get/remove, binding-context flow, self-pruning).
//   (3) the CHROME search-box wiring (the shell_handler resolves the current page's SearchHandler into the
//       shell_render_tree.search_box mirror; visibility/enabled/query/results map through; the handler's
//       own property changes re-realize) — the headless mirror of the iOS UISearchController install.
//
// §8: the search_handler (publisher of property_changed → the chrome subscription) is declared BEFORE the
// shell_handler (subscriber) wherever both appear; the fixture-owned pages outlive both.

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/list_proxy.hpp"
#include "maui/controls/shell/search_box_visibility.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/core/event.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace maui::controls;
    using namespace maui::controls::shell_tests;
    using maui::core::shell_handler;

    // A test row — a bindable_object (the C# `object` results materialize as binding-context nodes).
    class row : public maui::core::bindable_object
    {
    public:
        explicit row(std::string label) : label_(std::move(label))
        {
        }
        [[nodiscard]] const std::string& label() const
        {
            return label_;
        }

    private:
        std::string label_;
    };

    // ---- (1) the model ----

    TEST(search_handler_model, defaults_match_oracle)
    {
        const search_handler sh;
        EXPECT_TRUE(sh.query().empty());
        EXPECT_TRUE(sh.placeholder().empty());
        // C# SearchBoxVisibilityProperty default: Expanded.
        EXPECT_EQ(sh.get_search_box_visibility(), search_box_visibility::expanded);
        EXPECT_FALSE(sh.shows_results());
        // C# IsSearchEnabledProperty default: true.
        EXPECT_TRUE(sh.is_search_enabled());
        EXPECT_FALSE(sh.clear_placeholder_enabled());
        EXPECT_EQ(sh.query_icon(), nullptr);
        EXPECT_EQ(sh.clear_icon(), nullptr);
        EXPECT_EQ(sh.item_template(), nullptr);
        EXPECT_EQ(sh.selected_item(), nullptr);
        EXPECT_EQ(sh.results(), nullptr);
    }

    // OnQueryChanged(old, new) fires on a Query set with the old + new values; the query_changed event too.
    TEST(search_handler_model, set_query_fires_query_changed)
    {
        search_handler sh;
        std::string last_old;
        std::string last_new;
        int count = 0;
        auto token = maui::core::connect_scoped(sh.query_changed, [&](std::string old_v, std::string new_v) {
            last_old = std::move(old_v);
            last_new = std::move(new_v);
            ++count;
        });

        sh.set_query("ab");
        EXPECT_EQ(count, 1);
        EXPECT_EQ(last_old, "");
        EXPECT_EQ(last_new, "ab");

        sh.set_query("abc");
        EXPECT_EQ(count, 2);
        EXPECT_EQ(last_old, "ab");
        EXPECT_EQ(last_new, "abc");

        // No change → no event (the propertyChanged short-circuit).
        sh.set_query("abc");
        EXPECT_EQ(count, 2);
    }

    // A derived search_handler overrides OnQueryChanged to filter (the canonical SearchHandler pattern).
    TEST(search_handler_model, on_query_changed_override_runs)
    {
        class filtering : public search_handler
        {
        public:
            std::vector<std::string> seen;

        protected:
            void on_query_changed(std::string_view /*old_value*/, std::string_view new_value) override
            {
                seen.emplace_back(new_value);
            }
        };

        filtering sh;
        sh.set_query("x");
        sh.send_query_changed("xy"); // a native edit funnels through the same path
        ASSERT_EQ(sh.seen.size(), 2U);
        EXPECT_EQ(sh.seen[0], "x");
        EXPECT_EQ(sh.seen[1], "xy");
    }

    // OnQueryConfirmed runs the Command, then raises queried (command-then-event order).
    TEST(search_handler_model, query_confirmed_runs_command_then_event)
    {
        search_handler sh;
        std::vector<std::string> order;
        sh.command = [&] { order.emplace_back("command"); };
        auto token = maui::core::connect_scoped(sh.queried, [&](std::string_view) { order.emplace_back("event"); });

        sh.set_query("hello");
        sh.query_confirmed();
        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "command");
        EXPECT_EQ(order[1], "event");
    }

    // A derived search_handler can override OnQueryConfirmed (skipping the base command path).
    TEST(search_handler_model, on_query_confirmed_override)
    {
        class custom : public search_handler
        {
        public:
            int confirmed = 0;

        protected:
            void on_query_confirmed() override
            {
                ++confirmed;
            }
        };
        custom sh;
        bool command_ran = false;
        sh.command = [&] { command_ran = true; };
        sh.query_confirmed();
        EXPECT_EQ(sh.confirmed, 1);
        EXPECT_FALSE(command_ran); // the override did not call the base
    }

    // ItemsSource → results() (the ListProxy); list_proxy_changed fires (old, new).
    TEST(search_handler_model, items_source_builds_results_proxy)
    {
        search_handler sh;
        const list_proxy* last_old = nullptr; // count==1 below proves the callback fired
        const list_proxy* last_new = nullptr;
        int count = 0;
        auto token =
            maui::core::connect_scoped(sh.list_proxy_changed, [&](const list_proxy* old_p, const list_proxy* new_p) {
                last_old = old_p;
                last_new = new_p;
                ++count;
            });

        auto a = std::make_shared<row>("a");
        auto b = std::make_shared<row>("b");
        sh.set_items_source({a, b});
        EXPECT_EQ(count, 1);
        EXPECT_EQ(last_old, nullptr); // no previous proxy
        ASSERT_NE(sh.results(), nullptr);
        EXPECT_EQ(last_new, sh.results());
        EXPECT_EQ(sh.results()->count(), 2U);
        EXPECT_EQ(sh.results()->index_of(a.get()), 0);
        EXPECT_EQ(sh.results()->index_of(b.get()), 1);
        EXPECT_TRUE(sh.results()->contains(b.get()));

        // Replacing the source fires (old, new) with the PREVIOUS proxy as old — old and new must be
        // distinct, live pointers (the unique_ptr keep-alive; not the same reused storage).
        const list_proxy* const first_proxy = sh.results();
        auto c = std::make_shared<row>("c");
        sh.set_items_source({c});
        EXPECT_EQ(count, 2);
        EXPECT_EQ(last_old, first_proxy); // the previous proxy, still a valid distinct object during raise
        EXPECT_EQ(last_new, sh.results());
        EXPECT_NE(last_old, last_new); // distinct addresses (the std::optional::emplace aliasing bug)
        EXPECT_EQ(sh.results()->count(), 1U);

        // Clearing drops the proxy and fires (old, null).
        const list_proxy* const second_proxy = sh.results();
        sh.clear_items_source();
        EXPECT_EQ(count, 3);
        EXPECT_EQ(last_old, second_proxy);
        EXPECT_EQ(last_new, nullptr);
        EXPECT_EQ(sh.results(), nullptr);
    }

    // OnItemSelected records SelectedItem (read-only, FromHandler), runs OnItemSelected, then confirms.
    TEST(search_handler_model, item_selected_records_selected_and_confirms)
    {
        class custom : public search_handler
        {
        public:
            maui::core::bindable_object* picked = nullptr;
            int confirmed = 0;

        protected:
            void on_item_selected(maui::core::bindable_object* item) override
            {
                picked = item;
            }
            void on_query_confirmed() override
            {
                ++confirmed;
            }
        };

        custom sh;
        auto a = std::make_shared<row>("a");
        sh.item_selected(a);
        EXPECT_EQ(sh.picked, a.get());
        EXPECT_EQ(sh.selected_item(), a.get());
        EXPECT_EQ(sh.confirmed, 1); // non-WinUI: a selection also confirms
    }

    // OnClearPlaceholderClicked runs the clear-placeholder command.
    TEST(search_handler_model, clear_placeholder_runs_command)
    {
        search_handler sh;
        int ran = 0;
        sh.clear_placeholder_command = [&] { ++ran; };
        sh.clear_placeholder_clicked();
        EXPECT_EQ(ran, 1);
    }

    // ---- (2) the Shell.SearchHandler attached property ----

    class search_attached_test : public shell_test_base
    {
    };

    TEST_F(search_attached_test, set_get_remove_on_page)
    {
        auto page = make_page();
        EXPECT_EQ(shell::get_search_handler(*page), nullptr);

        auto handler = std::make_shared<search_handler>();
        shell::set_search_handler(*page, handler);
        EXPECT_EQ(shell::get_search_handler(*page), handler.get());

        // Setting null removes it.
        shell::set_search_handler(*page, nullptr);
        EXPECT_EQ(shell::get_search_handler(*page), nullptr);

        // And explicit remove is idempotent.
        shell::set_search_handler(*page, handler);
        shell::remove_search_handler(*page);
        EXPECT_EQ(shell::get_search_handler(*page), nullptr);
    }

    // The attached property flows the target's binding context into the handler (C#
    // OnSearchHandlerPropertyChanged → SetInheritedBindingContext).
    TEST_F(search_attached_test, set_flows_binding_context)
    {
        auto page = make_page();
        auto vm = std::make_shared<row>("vm");
        page->set_binding_context(vm);

        auto handler = std::make_shared<search_handler>();
        shell::set_search_handler(*page, handler);
        EXPECT_EQ(handler->binding_context<row>(), vm);
    }

    // A dead target self-prunes on the next access: if the target's address is RECYCLED by a fresh,
    // unrelated bindable_object, get_search_handler returns null (the captured liveness token expired) and
    // drops the stale entry — the weak-liveness guard that lets the side map be self-cleaning without a
    // dtor hook on the (lower-layer) target. We force the recycling deterministically with placement-new.
    TEST_F(search_attached_test, dead_target_prunes_on_recycled_address)
    {
        auto handler = std::make_shared<search_handler>();

        // A std::optional gives a fixed inline buffer; reset()+emplace() destroys then reconstructs a
        // content_page at the SAME address, deterministically recycling the storage (no raw placement-new).
        std::optional<content_page> slot;
        slot.emplace();
        content_page& first = slot.value();
        const void* const slot_addr = &first;
        shell::set_search_handler(first, handler);
        EXPECT_EQ(shell::get_search_handler(first), handler.get());
        const auto live_refs = handler.use_count(); // ours + the map entry
        slot.reset();                               // destroys the original page

        // Recycle the SAME storage for a brand-new page — the stale map entry now keys this fresh object,
        // but its captured liveness token belongs to the destroyed original, so it must NOT resolve.
        slot.emplace();
        content_page& second = slot.value();
        ASSERT_EQ(static_cast<const void*>(&second), slot_addr); // storage truly recycled
        EXPECT_EQ(shell::get_search_handler(second), nullptr);
        // The prune released the map's strong ref (use_count dropped back).
        EXPECT_LT(handler.use_count(), live_refs);
    }

    // ---- (3) the chrome search-box wiring (headless mirror) ----

    class search_chrome_test : public shell_test_base
    {
    protected:
        // A one-item, one-section shell whose current page we attach a search handler to.
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

    // With a handler attached to the current page, connecting the chrome resolves the search box.
    TEST_F(search_chrome_test, current_page_handler_resolves_search_box)
    {
        auto handler_model = std::make_shared<search_handler>();
        handler_model->set_placeholder("Search...");

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, handler_model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        const shell_search_box& box = platform->tree.search_box;
        EXPECT_EQ(box.handler, handler_model.get());
        EXPECT_TRUE(box.present); // default visibility Expanded
        EXPECT_FALSE(box.collapsible);
        EXPECT_TRUE(box.enabled);
        EXPECT_EQ(box.placeholder, "Search...");
    }

    // No handler attached → no search box.
    TEST_F(search_chrome_test, no_handler_means_no_box)
    {
        shell sh;
        build_single_page_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->tree.search_box.present);
        EXPECT_EQ(platform->tree.search_box.handler, nullptr);
    }

    // A handler attached to the SHELL (the port's fallback) resolves when the page has none.
    TEST_F(search_chrome_test, shell_level_handler_fallback)
    {
        auto handler_model = std::make_shared<search_handler>();

        shell sh;
        build_single_page_shell(sh);
        shell::set_search_handler(sh, handler_model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->tree.search_box.handler, handler_model.get());
    }

    // Visibility Hidden removes the box (present=false) though the handler is still attached.
    TEST_F(search_chrome_test, hidden_visibility_removes_box)
    {
        auto handler_model = std::make_shared<search_handler>();
        handler_model->set_search_box_visibility(search_box_visibility::hidden);

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, handler_model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        const shell_search_box& box = handler->typed_platform_view()->tree.search_box;
        EXPECT_EQ(box.handler, handler_model.get());
        EXPECT_FALSE(box.present);
    }

    // The handler's own property changes re-realize the box (C# OnSearchHandlerPropertyChanged).
    TEST_F(search_chrome_test, handler_property_change_rerealizes_box)
    {
        auto handler_model = std::make_shared<search_handler>();

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, handler_model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->tree.search_box.enabled);

        handler_model->set_is_search_enabled(false);
        EXPECT_FALSE(platform->tree.search_box.enabled);

        handler_model->set_search_box_visibility(search_box_visibility::collapsible);
        EXPECT_TRUE(platform->tree.search_box.present);
        EXPECT_TRUE(platform->tree.search_box.collapsible);

        // Flipping to Hidden removes the box through the same subscription.
        handler_model->set_search_box_visibility(search_box_visibility::hidden);
        EXPECT_FALSE(platform->tree.search_box.present);
    }

    // The results count + query map into the box; a native edit round-trips through send_query_changed.
    TEST_F(search_chrome_test, query_and_results_map_to_box)
    {
        auto handler_model = std::make_shared<search_handler>();
        handler_model->set_items_source(
            {std::make_shared<row>("a"), std::make_shared<row>("b"), std::make_shared<row>("c")});
        handler_model->set_shows_results(true);

        shell sh;
        auto page = build_single_page_shell(sh);
        shell::set_search_handler(*page, handler_model);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->tree.search_box.shows_results);
        EXPECT_EQ(platform->tree.search_box.result_count, 3U);

        // A native edit sets Query → OnQueryChanged → the box mirror updates via the subscription.
        handler_model->send_query_changed("ab");
        EXPECT_EQ(platform->tree.search_box.query, "ab");
    }

    // A navigation that changes the current page re-resolves the box to the NEW page's handler.
    TEST_F(search_chrome_test, navigation_reresolves_box_for_new_page)
    {
        auto handler_a = std::make_shared<search_handler>();
        handler_a->set_placeholder("A");
        auto handler_b = std::make_shared<search_handler>();
        handler_b->set_placeholder("B");

        // Two items, each a single page; attach a different handler to each page.
        auto page_a = make_page();
        auto page_b = make_page();

        shell sh;
        {
            auto item_a = std::make_shared<shell_item>();
            item_a->set_route("a");
            auto sec_a = std::make_shared<shell_section>();
            sec_a->set_route("tab");
            auto con_a = std::make_shared<shell_content>();
            con_a->set_content(page_a.get());
            con_a->set_route("content");
            sec_a->add(con_a);
            item_a->add(sec_a);
            sh.add_item(item_a);

            auto item_b = std::make_shared<shell_item>();
            item_b->set_route("b");
            auto sec_b = std::make_shared<shell_section>();
            sec_b->set_route("tab");
            auto con_b = std::make_shared<shell_content>();
            con_b->set_content(page_b.get());
            con_b->set_route("content");
            sec_b->add(con_b);
            item_b->add(sec_b);
            sh.add_item(item_b);
        }
        shell::set_search_handler(*page_a, handler_a);
        shell::set_search_handler(*page_b, handler_b);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->tree.search_box.handler, handler_a.get());
        EXPECT_EQ(platform->tree.search_box.placeholder, "A");

        sh.go_to_async(shell_navigation_state{"//b/tab/content"});
        EXPECT_EQ(platform->tree.search_box.handler, handler_b.get());
        EXPECT_EQ(platform->tree.search_box.placeholder, "B");
    }
} // namespace

// Tests for the navigation_page control + its headless handler seam — a push/pop stack of content_pages
// that fires the page Appearing/Disappearing lifecycle in C#'s order and hosts the current page on a
// native container. Verified: (1) the stack / current_page / root_page bookkeeping for push/pop/
// pop_to_root (root can't pop; duplicate push is a no-op); (2) the lifecycle ORDER — on push,
// Disappearing(previous) then Appearing(new); on pop, the popped page Disappears and the revealed page
// Appears; (3) the headless navigation_page_platform's single-page mirror tracks the current page through
// the "request_navigation" command as the stack changes.
#include "maui/controls/navigation_page.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::navigation_page;
    using maui::core::i_element_handler;
    using maui::core::navigation_page_handler;

    // Connect a label to a page's appearing/disappearing events that appends to a shared transcript, so
    // tests can assert the cross-page firing ORDER as well as per-page counts.
    void trace_lifecycle(content_page& page, const std::string& label, std::vector<std::string>& transcript)
    {
        // Pre-build the messages outside the handler so the lambda bodies only push a ready string (the
        // string concatenation, not the push_back, is what bugprone-exception-escape flags in the body).
        std::string appeared = label + ":appearing";
        std::string disappeared = label + ":disappearing";
        page.appearing.connect([msg = std::move(appeared), &transcript] { transcript.push_back(msg); });
        page.disappearing.connect([msg = std::move(disappeared), &transcript] { transcript.push_back(msg); });
    }

    // ---- the control in isolation (no handler) ----

    TEST(navigation_page, defaults_empty_stack_null_current_and_root)
    {
        navigation_page nav;
        EXPECT_TRUE(nav.navigation_stack().empty());
        EXPECT_EQ(nav.current_page(), nullptr);
        EXPECT_EQ(nav.root_page(), nullptr);
    }

    TEST(navigation_page, constructed_with_root_makes_it_current_and_appears_it)
    {
        content_page root;
        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);

        navigation_page nav(root);
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_EQ(nav.root_page(), &root);
        EXPECT_TRUE(root.has_appeared());
        // The root appeared exactly once (the window's role, stood in for here).
        EXPECT_EQ(transcript, (std::vector<std::string>{"root:appearing"}));
    }

    TEST(navigation_page, push_updates_current_and_stack)
    {
        content_page root;
        content_page second;
        navigation_page nav(root);

        nav.push(second);
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
        EXPECT_EQ(nav.current_page(), &second);
        EXPECT_EQ(nav.root_page(), &root); // root is unchanged at the bottom
    }

    TEST(navigation_page, push_fires_disappearing_on_previous_then_appearing_on_new)
    {
        content_page root;
        content_page second;
        navigation_page nav(root); // root appears here

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(second, "second", transcript);

        nav.push(second);

        // C# order: FireDisappearing(previous) then FireAppearing(new).
        EXPECT_EQ(transcript, (std::vector<std::string>{"root:disappearing", "second:appearing"}));
        EXPECT_FALSE(root.has_appeared());
        EXPECT_TRUE(second.has_appeared());
    }

    TEST(navigation_page, push_of_a_page_already_on_the_stack_is_a_noop)
    {
        content_page root;
        content_page second;
        navigation_page nav(root);
        nav.push(second);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(second, "second", transcript);

        nav.push(second); // already top-most -> no stack change, no lifecycle
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
        EXPECT_EQ(nav.current_page(), &second);
        EXPECT_TRUE(transcript.empty());
    }

    TEST(navigation_page, push_onto_empty_stack_appears_the_first_page)
    {
        content_page first;
        std::vector<std::string> transcript;
        trace_lifecycle(first, "first", transcript);

        navigation_page nav; // empty
        nav.push(first);     // becomes the root/current and appears (no previous to disappear)

        EXPECT_EQ(nav.current_page(), &first);
        EXPECT_EQ(nav.root_page(), &first);
        EXPECT_EQ(transcript, (std::vector<std::string>{"first:appearing"}));
    }

    TEST(navigation_page, pop_returns_top_and_reveals_the_page_beneath)
    {
        content_page root;
        content_page second;
        navigation_page nav(root);
        nav.push(second);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(second, "second", transcript);

        content_page* const popped = nav.pop();

        EXPECT_EQ(popped, &second);
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        // The popped page disappears, the revealed root appears.
        EXPECT_EQ(transcript, (std::vector<std::string>{"second:disappearing", "root:appearing"}));
        EXPECT_TRUE(root.has_appeared());
        EXPECT_FALSE(second.has_appeared());
    }

    TEST(navigation_page, pop_on_a_single_page_stack_is_a_noop_returning_null)
    {
        content_page root;
        navigation_page nav(root);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);

        EXPECT_EQ(nav.pop(), nullptr); // the root cannot be popped
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_TRUE(transcript.empty()); // no lifecycle fired
        EXPECT_TRUE(root.has_appeared());
    }

    TEST(navigation_page, pop_to_root_from_a_deep_stack)
    {
        content_page root;
        content_page b;
        content_page c;
        content_page d;
        navigation_page nav(root);
        nav.push(b);
        nav.push(c);
        nav.push(d);
        EXPECT_EQ(nav.navigation_stack().size(), 4U);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(d, "d", transcript);

        nav.pop_to_root();

        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_EQ(nav.root_page(), &root);
        // The previously-current top (d) disappears; the root reappears. (b and c were already in the
        // not-appeared state from when they were covered, so they fire nothing here.)
        EXPECT_EQ(transcript, (std::vector<std::string>{"d:disappearing", "root:appearing"}));
        EXPECT_TRUE(root.has_appeared());
        EXPECT_FALSE(d.has_appeared());
    }

    TEST(navigation_page, pop_to_root_on_a_single_page_stack_is_a_noop)
    {
        content_page root;
        navigation_page nav(root);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);

        nav.pop_to_root();
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_TRUE(transcript.empty());
    }

    // ---- the handler seam (control <-> handler <-> headless container): the host mirrors the current page ----

    TEST(navigation_page_seam, attaching_handler_hosts_the_initial_current_page)
    {
        content_page root;
        navigation_page nav(root); // root pushed before the handler is attached

        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &nav);
        // set_handler re-issues the navigation request after wiring (C# OnHandlerChangedCore), so the
        // freshly-attached container immediately hosts the current page rather than staying empty.
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &root);
    }

    TEST(navigation_page_seam, attaching_handler_to_an_empty_nav_hosts_nothing)
    {
        navigation_page nav; // empty stack
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        // Nothing to host yet; the container stays empty until the first push.
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, nullptr);
    }

    TEST(navigation_page_seam, push_after_attach_hosts_the_new_current_page)
    {
        content_page root;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav;
        nav.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        nav.push(root); // -> request_navigation -> host_current(root)
        EXPECT_EQ(platform->hosted_page, &root);

        content_page second;
        nav.push(second); // the container swaps to the new top-most page
        EXPECT_EQ(platform->hosted_page, &second);

        nav.pop(); // popping reveals the root again
        EXPECT_EQ(platform->hosted_page, &root);
    }

    TEST(navigation_page_seam, pop_to_root_hosts_the_root)
    {
        content_page root;
        content_page b;
        content_page c;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        nav.push(b);
        nav.push(c);
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &c);

        nav.pop_to_root();
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &root);
    }

    TEST(navigation_page_seam, handler_resolved_from_default_registry)
    {
        // navigation_page -> navigation_page_handler is self-registered (MAUI_REGISTER_HANDLER).
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<navigation_page>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<navigation_page_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        navigation_page nav;
        nav.set_handler(handler);
        content_page root;
        nav.push(root);
        EXPECT_EQ(resolved->typed_platform_view()->hosted_page, &root);
    }
} // namespace

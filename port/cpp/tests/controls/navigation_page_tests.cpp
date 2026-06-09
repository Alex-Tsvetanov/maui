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

    // ---- navigation chrome: the bar title tracks the current page; back is enabled when stack > 1 ----
    // C# IToolbar: the bar shows CurrentPage.Title (NavigationPageToolbar) and BackButtonVisible is driven
    // by NavigationPageController.StackDepth > 1 (NavigationPage.OnBackButtonPressed). Here the chrome state
    // is read off the navigation_page (bar_title / back_button_visible) and mirrored onto the headless
    // platform when the "request_navigation" command runs (the Apple twin builds a real NSView bar).

    TEST(navigation_page_chrome, bar_title_tracks_the_current_page)
    {
        content_page root;
        root.set_title("Root");
        content_page second;
        second.set_title("Second");

        navigation_page nav(root);
        EXPECT_EQ(nav.bar_title(), "Root");

        nav.push(second);
        EXPECT_EQ(nav.bar_title(), "Second");

        nav.pop();
        EXPECT_EQ(nav.bar_title(), "Root");
    }

    TEST(navigation_page_chrome, bar_title_is_empty_for_an_empty_stack)
    {
        navigation_page nav;
        EXPECT_EQ(nav.bar_title(), "");
    }

    TEST(navigation_page_chrome, back_button_visible_only_when_stack_deeper_than_one)
    {
        content_page root;
        content_page second;
        navigation_page nav(root);
        EXPECT_FALSE(nav.back_button_visible()); // single page -> no back

        nav.push(second);
        EXPECT_TRUE(nav.back_button_visible()); // depth 2 -> back

        nav.pop();
        EXPECT_FALSE(nav.back_button_visible()); // back at the root
    }

    TEST(navigation_page_chrome, send_back_button_pressed_pops_when_deeper_than_root)
    {
        content_page root;
        content_page second;
        navigation_page nav(root);
        nav.push(second);

        // C# OnBackButtonPressed: StackDepth > 1 -> SafePop() and report handled (true).
        EXPECT_TRUE(nav.send_back_button_pressed());
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_EQ(nav.navigation_stack().size(), 1U);

        // At the root the back button does nothing and reports unhandled (false).
        EXPECT_FALSE(nav.send_back_button_pressed());
        EXPECT_EQ(nav.current_page(), &root);
    }

    TEST(navigation_page_chrome, headless_platform_mirrors_the_bar_state)
    {
        content_page root;
        root.set_title("Root");
        content_page second;
        second.set_title("Second");
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->bar_title, "Root");
        EXPECT_FALSE(platform->back_button_visible);

        nav.push(second);
        EXPECT_EQ(platform->bar_title, "Second");
        EXPECT_TRUE(platform->back_button_visible);

        nav.pop();
        EXPECT_EQ(platform->bar_title, "Root");
        EXPECT_FALSE(platform->back_button_visible);
    }

    // ---- push/pop animation flag: the request carries `animated` (synchronous transition either way) ----
    // C# PushAsync(page)/PopAsync()/PopToRootAsync() default animated=true; the overloads thread it into the
    // NavigationRequest. The headless transition is synchronous regardless; the apple twin cross-fades when
    // animated. The headless platform mirrors the last request's animated flag so the seam is observable.

    TEST(navigation_page_animation, push_defaults_to_animated_request)
    {
        content_page root;
        content_page second;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        nav.push(second); // default animated == true
        EXPECT_TRUE(handler->typed_platform_view()->last_animated);
    }

    TEST(navigation_page_animation, push_can_request_no_animation)
    {
        content_page root;
        content_page second;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        nav.push(second, false);
        EXPECT_FALSE(handler->typed_platform_view()->last_animated);
        EXPECT_EQ(nav.current_page(), &second);
    }

    TEST(navigation_page_animation, pop_threads_the_animation_flag)
    {
        content_page root;
        content_page second;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        nav.push(second, false);

        nav.pop(false);
        EXPECT_FALSE(handler->typed_platform_view()->last_animated);
        EXPECT_EQ(nav.current_page(), &root);
    }

    // ---- insert_page_before / remove_page (INavigation; MauiNavigationImpl.OnInsertPageBefore/OnRemovePage) ----

    TEST(navigation_page_stack_edit, insert_page_before_inserts_at_the_position)
    {
        content_page root;
        content_page b;
        content_page inserted;
        navigation_page nav(root);
        nav.push(b);

        nav.insert_page_before(inserted, b); // inserted goes just before b
        ASSERT_EQ(nav.navigation_stack().size(), 3U);
        EXPECT_EQ(nav.navigation_stack()[0], &root);
        EXPECT_EQ(nav.navigation_stack()[1], &inserted);
        EXPECT_EQ(nav.navigation_stack()[2], &b);
        // Inserting an off-screen page does not change the current page or fire its lifecycle.
        EXPECT_EQ(nav.current_page(), &b);
        EXPECT_FALSE(inserted.has_appeared());
    }

    TEST(navigation_page_stack_edit, insert_before_root_updates_root_page)
    {
        content_page root;
        content_page b;
        content_page new_root;
        navigation_page nav(root);
        nav.push(b);

        nav.insert_page_before(new_root, root); // index 0 -> RootPage becomes new_root (C# OnInsertPageBefore)
        EXPECT_EQ(nav.root_page(), &new_root);
        EXPECT_EQ(nav.navigation_stack()[0], &new_root);
        EXPECT_EQ(nav.navigation_stack()[1], &root);
    }

    TEST(navigation_page_stack_edit, insert_duplicate_or_unknown_before_is_a_noop)
    {
        content_page root;
        content_page b;
        content_page x; // not on the stack
        navigation_page nav(root);
        nav.push(b);

        nav.insert_page_before(b, root); // page already on the stack -> ignored (C# throws; we no-op)
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
        nav.insert_page_before(x, x); // `before` not on the stack -> ignored
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
    }

    TEST(navigation_page_stack_edit, remove_inner_page_fires_no_lifecycle)
    {
        // C# NavigationPageLifecycleTests.RemoveInnerPage: removing a non-current, non-root page changes
        // neither the current page nor fires Appearing/Disappearing.
        content_page root;
        content_page middle;
        content_page top;
        navigation_page nav(root);
        nav.push(middle);
        nav.push(top);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(middle, "middle", transcript);
        trace_lifecycle(top, "top", transcript);

        nav.remove_page(middle);
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
        EXPECT_EQ(nav.navigation_stack()[0], &root);
        EXPECT_EQ(nav.navigation_stack()[1], &top);
        EXPECT_EQ(nav.current_page(), &top);
        EXPECT_TRUE(transcript.empty());
    }

    TEST(navigation_page_stack_edit, remove_current_page_pops_it)
    {
        // C# OnRemovePage: removing the CurrentPage routes through PopAsync (the revealed page appears).
        content_page root;
        content_page top;
        navigation_page nav(root);
        nav.push(top);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(top, "top", transcript);

        nav.remove_page(top); // == pop()
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_EQ(transcript, (std::vector<std::string>{"top:disappearing", "root:appearing"}));
    }

    TEST(navigation_page_stack_edit, remove_root_when_it_is_also_current_is_a_noop)
    {
        // C# OnRemovePage throws InvalidOperationException; we defensively no-op the single-page case.
        content_page root;
        navigation_page nav(root);
        nav.remove_page(root);
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
    }

    TEST(navigation_page_stack_edit, remove_unknown_page_is_a_noop)
    {
        content_page root;
        content_page b;
        content_page x;
        navigation_page nav(root);
        nav.push(b);
        nav.remove_page(x); // not on the stack
        EXPECT_EQ(nav.navigation_stack().size(), 2U);
    }

    // ---- modal stack: a SEPARATE stack overlaying the page stack (NavigationModel.PushModal/PopModal) ----

    TEST(navigation_modal, push_modal_disappears_underlying_then_appears_modal)
    {
        content_page root;
        content_page modal;
        navigation_page nav(root); // root appears here

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(modal, "modal", transcript);

        nav.push_modal(modal);

        // C# NavigationModel.PushModal: previousPage.SendDisappearing() then page.SendAppearing().
        EXPECT_EQ(transcript, (std::vector<std::string>{"root:disappearing", "modal:appearing"}));
        EXPECT_EQ(nav.modal_stack().size(), 1U);
        EXPECT_EQ(nav.modal_stack().back(), &modal);
        EXPECT_TRUE(modal.has_appeared());
        EXPECT_FALSE(root.has_appeared());
        // The navigation stack is untouched by a modal push.
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_EQ(nav.current_page(), &root);
    }

    TEST(navigation_modal, pop_modal_disappears_modal_then_reappears_underlying)
    {
        // C# PageLifeCycleTests.PopModalPage: poppedPage appears 1/disappears 1; firstPage appears twice.
        content_page root;
        content_page modal;
        navigation_page nav(root);
        nav.push_modal(modal);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);
        trace_lifecycle(modal, "modal", transcript);

        content_page* const popped = nav.pop_modal();

        EXPECT_EQ(popped, &modal);
        // C# PopModal: previousPage(modal).SendDisappearing() then CurrentPage(root).SendAppearing().
        EXPECT_EQ(transcript, (std::vector<std::string>{"modal:disappearing", "root:appearing"}));
        EXPECT_TRUE(nav.modal_stack().empty());
        EXPECT_TRUE(root.has_appeared());
        EXPECT_FALSE(modal.has_appeared());
    }

    TEST(navigation_modal, second_modal_covers_the_first)
    {
        // C# PageLifeCycleTests.PushSecondModalPage: first disappears once, second appears once.
        content_page root;
        content_page first_modal;
        content_page second_modal;
        navigation_page nav(root);
        nav.push_modal(first_modal);

        std::vector<std::string> transcript;
        trace_lifecycle(first_modal, "first", transcript);
        trace_lifecycle(second_modal, "second", transcript);

        nav.push_modal(second_modal); // previous == the first modal (the current visible page)

        EXPECT_EQ(transcript, (std::vector<std::string>{"first:disappearing", "second:appearing"}));
        EXPECT_EQ(nav.modal_stack().size(), 2U);
        EXPECT_EQ(nav.modal_stack().back(), &second_modal);
    }

    TEST(navigation_modal, pop_to_an_underlying_modal)
    {
        // C# PageLifeCycleTests.PopToAModalPage: pop the top modal -> it disappears, the first modal reappears.
        content_page root;
        content_page first_modal;
        content_page second_modal;
        navigation_page nav(root);
        nav.push_modal(first_modal);
        nav.push_modal(second_modal);

        std::vector<std::string> transcript;
        trace_lifecycle(first_modal, "first", transcript);
        trace_lifecycle(second_modal, "second", transcript);

        content_page* const popped = nav.pop_modal();

        EXPECT_EQ(popped, &second_modal);
        EXPECT_EQ(transcript, (std::vector<std::string>{"second:disappearing", "first:appearing"}));
        EXPECT_EQ(nav.modal_stack().size(), 1U);
        EXPECT_EQ(nav.modal_stack().back(), &first_modal);
    }

    TEST(navigation_modal, pop_modal_on_empty_stack_is_a_noop_returning_null)
    {
        content_page root;
        navigation_page nav(root);

        std::vector<std::string> transcript;
        trace_lifecycle(root, "root", transcript);

        EXPECT_EQ(nav.pop_modal(), nullptr); // C# throws; we return null (documented deviation)
        EXPECT_TRUE(nav.modal_stack().empty());
        EXPECT_TRUE(transcript.empty());
    }

    TEST(navigation_modal_seam, headless_platform_mirrors_the_hosted_modal)
    {
        content_page root;
        content_page modal;
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_modal, nullptr);

        nav.push_modal(modal); // -> request_modal_navigation -> overlay the modal
        EXPECT_EQ(platform->hosted_modal, &modal);
        // The underlying page is still hosted (the modal overlays it, it is not swapped out).
        EXPECT_EQ(platform->hosted_page, &root);

        nav.pop_modal(); // the overlay clears
        EXPECT_EQ(platform->hosted_modal, nullptr);
    }
} // namespace

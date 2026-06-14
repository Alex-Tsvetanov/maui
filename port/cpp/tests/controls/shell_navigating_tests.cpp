// Ported from ShellNavigatingTests.cs — go_to_async / the deferral state machine / stack mutation
// per request kind (the GOLD oracle for shell navigation).
//
// Async restructuring (the port's pipeline is synchronous-with-suspension): where C# `await
// Task.Delay(...)` inside a Navigating handler kept a deferral open across the awaited GoToAsync,
// the port grabs the token in the handler, asserts the suspended state after the navigation call
// returns, then completes the token and asserts the resumed state. The MODAL variants
// (ModalTestPage / PresentationMode) are not ported — the port's shell has no modal stack.

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigating_deferral.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/event.hpp"
#include "shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace maui::controls;
    using namespace maui::controls::shell_tests;

    class shell_navigating_test : public shell_test_base
    {
    };

    TEST_F(shell_navigating_test, cancel_navigation)
    {
        shell sh;

        auto one = std::make_shared<shell_item>();
        one->set_route("one");
        auto two = std::make_shared<shell_item>();
        two->set_route("two");

        one->add(make_simple_shell_section("tabone", "content"));
        one->add(make_simple_shell_section("tabtwo", "content"));
        two->add(make_simple_shell_section("tabthree", "content"));
        two->add(make_simple_shell_section("tabfour", "content"));

        sh.add_item(one);
        sh.add_item(two);

        EXPECT_EQ("//one/tabone/content", location_of(sh));

        auto token =
            maui::core::connect_scoped(sh.navigating, [](shell_navigating_event_args& args) { (void)args.cancel(); });

        sh.go_to_async(shell_navigation_state{"//two/tabfour/"});

        EXPECT_EQ("//one/tabone/content", location_of(sh));
    }

    TEST_F(shell_navigating_test, cancel_navigation_occurring_outside_goto_async_without_delay)
    {
        auto flyout = create_shell_item<flyout_item>();
        test_shell sh;
        sh.add_item(flyout);

        auto navigating_to = create_shell_content();
        sh.items()[0]->items()[0]->add(navigating_to);

        bool executed = false;
        shell_content* content_active_before_completing = nullptr;
        auto token = maui::core::connect_scoped(sh.navigating, [&](shell_navigating_event_args& args) {
            auto deferral = args.get_deferral();
            ASSERT_NE(deferral, nullptr);
            content_active_before_completing = flyout->items()[0]->items()[0].get();
            (void)args.cancel();
            deferral->complete();
            executed = true;
        });

        const bool result =
            sh.propose_navigation(shell_navigation_source::shell_content_changed, flyout.get(),
                                  flyout->items()[0].get(), navigating_to.get(), &flyout->items()[0]->stack(), true);

        EXPECT_TRUE(executed);
        EXPECT_FALSE(result);
    }

    TEST_F(shell_navigating_test, cancel_navigation_occurring_outside_goto_async)
    {
        auto flyout = create_shell_item<flyout_item>();
        test_shell sh;
        sh.add_item(flyout);

        auto navigating_to = create_shell_content();
        sh.items()[0]->items()[0]->add(navigating_to);

        std::shared_ptr<shell_navigating_deferral> deferral;
        auto token = maui::core::connect_scoped(
            sh.navigating, [&deferral](shell_navigating_event_args& args) { deferral = args.get_deferral(); });

        const bool result =
            sh.propose_navigation(shell_navigation_source::shell_content_changed, flyout.get(),
                                  flyout->items()[0].get(), navigating_to.get(), &flyout->items()[0]->stack(), true);
        EXPECT_FALSE(result); // delayed (deferred), so the caller may not proceed now

        // The content active while the deferral is open is still the original one.
        shell_content const* content_active_before_completing = flyout->items()[0]->items()[0].get();
        EXPECT_NE(content_active_before_completing, navigating_to.get());
        EXPECT_EQ(flyout->items()[0]->items()[0].get(), content_active_before_completing);

        const int navigated_before = sh.navigated_count;
        ASSERT_NE(deferral, nullptr);
        deferral->complete(); // the deferred navigation resumes here

        EXPECT_EQ(flyout->items()[0]->current_item(), navigating_to.get());
        EXPECT_GT(sh.navigated_count, navigated_before);
    }

    TEST_F(shell_navigating_test, immediately_complete_deferral)
    {
        test_shell sh;
        sh.add_item(create_shell_item<flyout_item>());

        bool executed = false;
        auto token = maui::core::connect_scoped(sh.navigating, [&executed](shell_navigating_event_args& args) {
            auto deferral = args.get_deferral();
            executed = true;
            if (deferral != nullptr)
            {
                deferral->complete();
            }
        });

        std::shared_ptr<content_page> const page = make_page();
        sh.navigation_push(*page);

        EXPECT_TRUE(executed);
        EXPECT_EQ(2U, sh.navigation_stack().size());
    }

    class defer_pop_navigation_test : public shell_navigating_test, public ::testing::WithParamInterface<const char*>
    {
    };

    TEST_P(defer_pop_navigation_test, defer_pop_navigation)
    {
        const std::string test_case = GetParam();
        test_shell sh;
        sh.add_item(create_shell_item<flyout_item>());

        sh.navigation_push(*make_page());
        sh.navigation_push(*make_page());

        std::shared_ptr<shell_navigating_deferral> token;
        auto connection = maui::core::connect_scoped(
            sh.navigating, [&token](shell_navigating_event_args& args) { token = args.get_deferral(); });

        if (test_case == "Pop")
        {
            sh.navigation_pop();
            // suspended behind the deferral: nothing popped yet
            EXPECT_EQ(3U, sh.navigation_stack().size());
            ASSERT_NE(token, nullptr);
            token->complete();
            EXPECT_EQ(2U, sh.navigation_stack().size());
        }
        else
        {
            sh.navigation_pop_to_root();
            EXPECT_EQ(3U, sh.navigation_stack().size());
            ASSERT_NE(token, nullptr);
            token->complete();
            EXPECT_EQ(1U, sh.navigation_stack().size());
        }
    }

    INSTANTIATE_TEST_SUITE_P(shell_navigating, defer_pop_navigation_test, ::testing::Values("PopToRoot", "Pop"));

    class deferral_completes_navigation_test : public shell_navigating_test,
                                               public ::testing::WithParamInterface<const char*>
    {
    };

    TEST_P(deferral_completes_navigation_test, navigation_task_completes_after_deferral_has_finished)
    {
        const std::string test_case = GetParam();
        routing::register_route<content_page>("NavigationTaskCompletesAfterDeferralHasFinished");
        test_shell sh;
        sh.add_item(create_shell_item<flyout_item>());

        std::shared_ptr<shell_navigating_deferral> token;
        auto connection = maui::core::connect_scoped(
            sh.navigating, [&token](shell_navigating_event_args& args) { token = args.get_deferral(); });

        if (test_case == "PopToRoot")
        {
            sh.navigation_pop_to_root();
        }
        else if (test_case == "Pop")
        {
            sh.navigation_pop();
        }
        else if (test_case == "GoToAsync")
        {
            sh.go_to_async(shell_navigation_state{"NavigationTaskCompletesAfterDeferralHasFinished"});
        }
        else
        {
            sh.navigation_push(*make_page());
        }

        ASSERT_NE(token, nullptr);
        token->complete();
        EXPECT_TRUE(token->is_completed());
    }

    INSTANTIATE_TEST_SUITE_P(shell_navigating, deferral_completes_navigation_test,
                             ::testing::Values("PopToRoot", "Pop", "GoToAsync", "Push"));

    TEST_F(shell_navigating_test, completing_the_same_deferral_token_twice_doesnt_do_anything)
    {
        auto args = std::make_shared<shell_navigating_event_args>(
            std::optional<shell_navigation_state>{shell_navigation_state{".."}}, shell_navigation_state{"../newstate"},
            shell_navigation_source::push, true);
        auto token = args->get_deferral();
        auto second = args->get_deferral();

        EXPECT_EQ(2, args->deferral_count());
        token->complete();
        EXPECT_EQ(1, args->deferral_count());
        token->complete();
        EXPECT_EQ(1, args->deferral_count());
    }

    TEST_F(shell_navigating_test, dot_dot_navigate_back_from_pages_with_default_route)
    {
        auto flyout = create_shell_item<flyout_item>();
        const std::string item_route = flyout->current_item()->current_item()->route();
        std::shared_ptr<content_page> const page1 = make_page();
        std::shared_ptr<content_page> const page2 = make_page();
        test_shell sh;
        sh.add_item(flyout);

        EXPECT_EQ(location_of(sh), "//" + item_route);

        sh.navigation_push(*page1);
        EXPECT_EQ(location_of(sh), "//" + item_route + "/" + routing::get_route(*page1));

        sh.navigation_push(*page2);
        EXPECT_EQ(location_of(sh),
                  "//" + item_route + "/" + routing::get_route(*page1) + "/" + routing::get_route(*page2));

        sh.go_to_async(shell_navigation_state{".."});
        EXPECT_EQ(location_of(sh), "//" + item_route + "/" + routing::get_route(*page1));
    }

    TEST_F(shell_navigating_test, insert_two_pages_at_separate_points)
    {
        routing::register_route<content_page>("pagefirstmiddle");
        routing::register_route<content_page>("pagesecondmiddle");
        routing::register_route<content_page>("last");
        routing::register_route<content_page>("middle");

        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "", "item"));

        sh.go_to_async(shell_navigation_state{"//item/middle/last"});
        sh.go_to_async(shell_navigation_state{"//item/pagefirstmiddle/middle/pagesecondmiddle/last"});

        EXPECT_EQ("//item/pagefirstmiddle/middle/pagesecondmiddle/last", location_of(sh));
    }

    TEST_F(shell_navigating_test, navigation_push_and_pop_basic)
    {
        auto flyout = create_shell_item<flyout_item>(nullptr, false, "content", "section", "item");
        const std::string item_route = "item/section/content";
        std::shared_ptr<content_page> const page1 = make_page();
        std::shared_ptr<content_page> const page2 = make_page();
        test_shell sh;
        sh.add_item(flyout);

        EXPECT_EQ(location_of(sh), "//" + item_route);

        sh.navigation_push(*page1);
        EXPECT_EQ(location_of(sh), "//" + item_route + "/" + routing::get_route(*page1));

        sh.navigation_push(*page2);
        EXPECT_EQ(location_of(sh),
                  "//" + item_route + "/" + routing::get_route(*page1) + "/" + routing::get_route(*page2));

        sh.navigation_pop();
        EXPECT_EQ(location_of(sh), "//" + item_route + "/" + routing::get_route(*page1));
    }

    TEST_F(shell_navigating_test, navigate_to_default_shell_content)
    {
        test_shell sh;
        sh.add_item(create_shell_item<flyout_item>());
        std::shared_ptr<content_page> const page = make_page();

        const std::string content_route = sh.current_item()->current_item()->current_item()->route();
        const std::string page_route = routing::get_route(*page);

        sh.navigation_push(*make_page());
        sh.navigation_push(*page);

        sh.go_to_async(shell_navigation_state{"//" + content_route + "/" + page_route});

        EXPECT_EQ(location_of(sh), "//" + content_route + "/" + page_route);
    }

    TEST_F(shell_navigating_test, pop_to_root_with_multiple_flyout_items)
    {
        test_shell sh;
        sh.add_item(create_shell_item<flyout_item>(nullptr, false, "home", "", "store"));
        sh.add_item(create_shell_item<flyout_item>(nullptr, false, "home", "", "second"));

        sh.navigation_push(*make_page());
        sh.navigation_push(*make_page());
        sh.navigation_pop_to_root();
        EXPECT_EQ(1U, sh.navigation_stack().size());
    }

    TEST_F(shell_navigating_test, multiple_pops_remove_middle_pages_before_final_pop)
    {
        test_shell sh;
        auto monitoring_tab = std::make_shared<navigation_monitoring_tab>();
        monitoring_tab->add(create_shell_content(nullptr, false, "rootpage"));
        sh.add_item(monitoring_tab);

        std::shared_ptr<content_page> const page_left_on_stack = make_page();
        sh.navigation_push(*page_left_on_stack);
        sh.navigation_push(*make_page());
        sh.navigation_push(*make_page());
        monitoring_tab->navigations_fired.clear();

        sh.go_to_async(shell_navigation_state{"../.."});
        EXPECT_EQ(location_of(sh), "//rootpage/" + routing::get_route(*page_left_on_stack));

        ASSERT_EQ(2U, monitoring_tab->navigations_fired.size());
        EXPECT_EQ("OnRemovePage", monitoring_tab->navigations_fired[0]);
        EXPECT_EQ("OnPopAsync", monitoring_tab->navigations_fired[1]);
    }

    TEST_F(shell_navigating_test, swapping_out_visible_page_doesnt_reveal_previous_page)
    {
        test_shell sh;
        auto monitoring_tab = std::make_shared<navigation_monitoring_tab>();
        monitoring_tab->add(create_shell_content(nullptr, false, "rootpage"));
        sh.add_item(monitoring_tab);

        register_page("firstPage");
        register_page("pageToSwapIn");

        sh.go_to_async(shell_navigation_state{"firstPage"});
        monitoring_tab->navigations_fired.clear();

        sh.go_to_async(shell_navigation_state{"../pageToSwapIn"});
        EXPECT_EQ("//rootpage/pageToSwapIn", location_of(sh));

        ASSERT_EQ(2U, monitoring_tab->navigations_fired.size());
        EXPECT_EQ("OnPushAsync", monitoring_tab->navigations_fired[0]);
        EXPECT_EQ("OnRemovePage", monitoring_tab->navigations_fired[1]);
    }

    TEST_F(shell_navigating_test, middle_routes_are_removed_without_popping_stack)
    {
        test_shell sh;
        auto monitoring_tab = std::make_shared<navigation_monitoring_tab>();
        monitoring_tab->add(create_shell_content(nullptr, false, "rootpage"));
        sh.add_item(monitoring_tab);

        register_page("firstPage");
        register_page("secondPage");
        register_page("thirdPage");
        register_page("fourthPage");
        register_page("fifthPage");

        sh.go_to_async(shell_navigation_state{"firstPage/secondPage/thirdPage/fourthPage/fifthPage"});
        monitoring_tab->navigations_fired.clear();

        EXPECT_EQ("//rootpage/firstPage/secondPage/thirdPage/fourthPage/fifthPage", location_of(sh));

        sh.go_to_async(shell_navigation_state{"//rootpage/thirdPage/fifthPage"});
        EXPECT_EQ("//rootpage/thirdPage/fifthPage", location_of(sh));

        ASSERT_EQ(3U, monitoring_tab->navigations_fired.size());
        EXPECT_EQ("OnRemovePage", monitoring_tab->navigations_fired[0]);
        EXPECT_EQ("OnRemovePage", monitoring_tab->navigations_fired[1]);
        EXPECT_EQ("OnRemovePage", monitoring_tab->navigations_fired[2]);
    }

    TEST_F(shell_navigating_test, popping_sets_correct_navigation_source)
    {
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "item1"));
        register_page("page1");
        register_page("page2");

        sh.go_to_async(shell_navigation_state{"page1"});
        sh.go_to_async(shell_navigation_state{"page2"});
        sh.navigation_pop();

        sh.test_navigating_args(shell_navigation_source::pop, std::optional<std::string>{"//item1/page1/page2"}, "..");
        sh.test_navigated_args(shell_navigation_source::pop, std::optional<std::string>{"//item1/page1/page2"},
                               "//item1/page1");
    }

    class global_route_relative_test : public shell_navigating_test, public ::testing::WithParamInterface<int>
    {
    };

    TEST_P(global_route_relative_test, shell_item_content_route_with_global_route_relative)
    {
        const int depth = GetParam();
        shell sh;
        auto item1 = create_shell_item<flyout_item>(nullptr, true, "monkeys", "", "animals");

        std::string route = "monkeys/details";
        if (depth == 3)
        {
            route = "animals/monkeys/details";
        }
        routing::register_route<content_page>(route);

        sh.add_item(item1);

        sh.go_to_async(shell_navigation_state{"details"});
        EXPECT_EQ("//animals/monkeys/details", location_of(sh));
    }

    INSTANTIATE_TEST_SUITE_P(shell_navigating, global_route_relative_test, ::testing::Values(2, 3));

    TEST_F(shell_navigating_test, goto_same_global_routes_collapses_uri_correctly)
    {
        shell sh;
        auto item1 = create_shell_item<flyout_item>(nullptr, true, "monkeys", "", "animals");
        routing::register_route<content_page>("details");
        sh.add_item(item1);

        sh.go_to_async(shell_navigation_state{"details"});
        sh.go_to_async(shell_navigation_state{"details"});
        EXPECT_EQ("//animals/monkeys/details/details", location_of(sh));
    }

    TEST_F(shell_navigating_test, shell_section_with_global_route_absolute)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1");
        routing::register_route<content_page>("edit");
        sh.add_item(item1);

        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("//rootlevelcontent1/edit"));

        ASSERT_NE(request, nullptr);
        ASSERT_EQ(1U, request->definition().global_routes().size());
        EXPECT_EQ("edit", request->definition().global_routes().front());
    }

    TEST_F(shell_navigating_test, shell_section_with_relative_edit)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1");
        auto edit_shell_content = create_shell_content(nullptr, false, "edit");

        item1->items()[0]->add(edit_shell_content);
        sh.add_item(item1);

        sh.go_to_async(shell_navigation_state{"//rootlevelcontent1"});
        sh.navigation_manager().go_to(shell_navigation_state{"edit"}, false, true);

        EXPECT_EQ(edit_shell_content.get(), sh.current_item()->current_item()->current_item());
    }

    TEST_F(shell_navigating_test, shell_content_only_with_global_edit)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, true, "rootlevelcontent1"));
        sh.add_item(create_shell_item(nullptr, true, "rootlevelcontent2"));

        routing::register_route<content_page>("//rootlevelcontent1/edit");
        EXPECT_NO_THROW(sh.go_to_async(shell_navigation_state{"//rootlevelcontent1/edit"}));
    }

    TEST_F(shell_navigating_test, route_with_global_page_route)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, true, "dogs", "domestic", "animals"));
        sh.add_item(create_shell_item(nullptr, true, "cats", "domestic", "animals"));

        routing::register_route<content_page>("catdetails");
        sh.go_to_async(shell_navigation_state{"//cats/catdetails?name=3"});

        EXPECT_EQ("//animals/domestic/cats/catdetails", location_of(sh));
    }

    TEST_F(shell_navigating_test, absolute_routing_to_page)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, true, "dogs", "domestic", "animals"));
        routing::register_route<content_page>("catdetails");

        EXPECT_ANY_THROW(sh.go_to_async(shell_navigation_state{"//catdetails"}));
    }

    TEST_F(shell_navigating_test, location_removes_implicit)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, true, "rootlevelcontent1"));

        EXPECT_EQ("//rootlevelcontent1", location_of(sh));
    }

    TEST_F(shell_navigating_test, global_navigate_twice)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, true, "rootlevelcontent1"));
        routing::register_route<content_page>("cat");
        routing::register_route<content_page>("details");

        sh.go_to_async(shell_navigation_state{"cat"});
        sh.go_to_async(shell_navigation_state{"details"});

        EXPECT_EQ("//rootlevelcontent1/cat/details", location_of(sh));
        sh.go_to_async(shell_navigation_state{"//rootlevelcontent1/details"});
        EXPECT_EQ("//rootlevelcontent1/details", location_of(sh));
    }

    TEST_F(shell_navigating_test, global_routes_registered_hierarchically_navigate_correctly)
    {
        routing::register_route<test_page1>("first");
        routing::register_route<test_page2>("first/second");
        routing::register_route<test_page3>("first/second/third");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "MainPage"));

        sh.go_to_async(shell_navigation_state{"//MainPage/first/second"});

        ASSERT_GE(sh.navigation_stack().size(), 3U);
        EXPECT_NE(dynamic_cast<test_page1*>(sh.navigation_stack()[1]), nullptr);
        EXPECT_NE(dynamic_cast<test_page2*>(sh.navigation_stack()[2]), nullptr);

        sh.go_to_async(shell_navigation_state{"//MainPage/first/second/third"});

        ASSERT_GE(sh.navigation_stack().size(), 4U);
        EXPECT_NE(dynamic_cast<test_page1*>(sh.navigation_stack()[1]), nullptr);
        EXPECT_NE(dynamic_cast<test_page2*>(sh.navigation_stack()[2]), nullptr);
        EXPECT_NE(dynamic_cast<test_page3*>(sh.navigation_stack()[3]), nullptr);
    }

    TEST_F(shell_navigating_test, global_routes_registered_hierarchically_navigate_correctly_variation)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        routing::register_route<test_page2>("monkeyDetails/monkeygenome");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals2"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals"));

        sh.go_to_async(shell_navigation_state{"//animals/monkeys/monkeyDetails?id=123"});
        sh.go_to_async(shell_navigation_state{"monkeygenome"});
        EXPECT_EQ("//animals/monkeys/monkeyDetails/monkeygenome", location_of(sh));
    }

    TEST_F(shell_navigating_test, global_routes_registered_hierarchically_with_double_pop)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        routing::register_route<test_page2>("monkeyDetails/monkeygenome");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals2"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals"));

        sh.go_to_async(shell_navigation_state{"//animals/monkeys/monkeyDetails?id=123"});
        sh.go_to_async(shell_navigation_state{"monkeygenome"});
        sh.go_to_async(shell_navigation_state{"../.."});
        EXPECT_EQ("//animals/monkeys", location_of(sh));
    }

    TEST_F(shell_navigating_test, global_routes_registered_hierarchically_with_double_slash)
    {
        routing::register_route<test_page1>("//animals/monkeys/monkeyDetails");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals"));

        sh.go_to_async(shell_navigation_state{"//animals/monkeys/monkeyDetails?id=123"});
        EXPECT_EQ("//animals/monkeys/monkeyDetails", location_of(sh));
    }

    TEST_F(shell_navigating_test, remove_page_with_nested_routes)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        routing::register_route<test_page2>("monkeyDetails/monkeygenome");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals"));

        sh.go_to_async(shell_navigation_state{"//animals/monkeys/monkeyDetails"});
        sh.go_to_async(shell_navigation_state{"monkeygenome"});
        ASSERT_GE(sh.navigation_stack().size(), 2U);
        sh.navigation_remove_page(*sh.navigation_stack()[1]);
        EXPECT_NO_THROW(sh.navigation_pop());
    }

    TEST_F(shell_navigating_test, global_routes_registered_hierarchically_navigate_correctly_with_additional_items)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        routing::register_route<test_page2>("monkeyDetails/monkeygenome");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "cats", "domestic", "animals"));

        sh.items()[0]->add(create_shell_content(nullptr, false, "monkeys"));
        sh.items()[0]->add(create_shell_content(nullptr, false, "elephants"));
        sh.items()[0]->add(create_shell_content(nullptr, false, "bears"));
        sh.items()[0]->items()[0]->add(create_shell_content(nullptr, false, "dogs"));
        sh.add_item(create_shell_content(nullptr, false, "about"));

        sh.go_to_async(shell_navigation_state{"//animals/monkeys/monkeyDetails?id=123"});
        sh.go_to_async(shell_navigation_state{"monkeygenome"});
        EXPECT_EQ("//animals/monkeys/monkeyDetails/monkeygenome", location_of(sh));
    }

    TEST_F(shell_navigating_test, go_back_from_route_with_multiple_paths)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        test_shell sh;
        sh.add_item(create_shell_item());

        sh.go_to_async(shell_navigation_state{"monkeys/monkeyDetails"});
        sh.go_to_async(shell_navigation_state{"monkeys/monkeyDetails"});
        sh.navigation_pop();
        EXPECT_NO_THROW(sh.navigation_pop());
    }

    TEST_F(shell_navigating_test, go_back_from_route_with_multiple_paths_hierarchical)
    {
        routing::register_route<test_page1>("monkeys/monkeyDetails");
        routing::register_route<test_page2>("monkeyDetails/monkeygenome");
        test_shell sh;
        sh.add_item(create_shell_item());

        sh.go_to_async(shell_navigation_state{"monkeys/monkeyDetails"});
        sh.go_to_async(shell_navigation_state{"monkeygenome"});
        sh.navigation_pop();
        EXPECT_NO_THROW(sh.navigation_pop());
    }

    TEST_F(shell_navigating_test, hierarchical_navigation)
    {
        routing::register_route<shell_test_page>("page1/page2");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "page1"));

        sh.go_to_async(shell_navigation_state{"page1/page2?SomeQueryParameter=1"});

        auto* page = dynamic_cast<shell_test_page*>(sh.current_page());
        ASSERT_NE(page, nullptr);
        EXPECT_EQ("1", page->some_query_parameter);
    }

    TEST_F(shell_navigating_test, hierarchical_navigation_multiple_routes)
    {
        routing::register_route<shell_test_page>("page1/page2");
        routing::register_route<test_page1>("page1/page2/page3");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "page1"));

        sh.go_to_async(shell_navigation_state{"page1/page2?SomeQueryParameter=1"});

        auto* page = dynamic_cast<shell_test_page*>(sh.current_page());
        ASSERT_NE(page, nullptr);
        EXPECT_EQ("1", page->some_query_parameter);

        sh.go_to_async(shell_navigation_state{"page1/page2/page3"});

        EXPECT_NE(dynamic_cast<test_page1*>(sh.current_page()), nullptr);
        ASSERT_GE(sh.navigation_stack().size(), 2U);
        EXPECT_NE(dynamic_cast<shell_test_page*>(sh.navigation_stack()[1]), nullptr);
    }

    TEST_F(shell_navigating_test, hierarchical_navigation_multiple_routes_variation1)
    {
        routing::register_route<shell_test_page>("page1/page2");
        routing::register_route<test_page1>("page1/page2/page3");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "page1"));

        sh.go_to_async(shell_navigation_state{"page1/page2/page3"});

        EXPECT_NE(dynamic_cast<test_page1*>(sh.current_page()), nullptr);
        ASSERT_GE(sh.navigation_stack().size(), 2U);
        EXPECT_NE(dynamic_cast<shell_test_page*>(sh.navigation_stack()[1]), nullptr);
    }

    TEST_F(shell_navigating_test, hierarchical_navigation_with_back_navigation)
    {
        routing::register_route<shell_test_page>("page1/page2");
        routing::register_route<test_page1>("page1/page2/page3");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "page1"));

        sh.go_to_async(shell_navigation_state{"page1/page2"});
        sh.go_to_async(shell_navigation_state{"page1/page2/page3"});
        EXPECT_NE(dynamic_cast<test_page1*>(sh.current_page()), nullptr);

        sh.go_to_async(shell_navigation_state{".."});
        EXPECT_NE(dynamic_cast<shell_test_page*>(sh.current_page()), nullptr);

        sh.go_to_async(shell_navigation_state{".."});
        EXPECT_NE(sh.current_page(), nullptr);
        EXPECT_EQ(dynamic_cast<shell_test_page*>(sh.current_page()), nullptr); // back to the plain root page
    }

    TEST_F(shell_navigating_test, navigated_fires_after_switching_flyout_items_both_with_pushed_pages)
    {
        auto content1 = create_shell_content();
        auto content2 = create_shell_content();

        test_shell sh;
        sh.add_item(content1);
        sh.add_item(content2);

        sh.navigation_push(*make_page());
        sh.on_flyout_item_selected(*content2);
        sh.navigation_push(*make_page());
        sh.on_flyout_item_selected(*content1);

        EXPECT_EQ(2U, sh.items()[0]->items()[0]->stack().size());
        EXPECT_EQ(2U, sh.items()[1]->items()[0]->stack().size());
    }

    TEST_F(shell_navigating_test, remove_page_disconnects_handler_in_shell)
    {
        // C# holds `middlePage` (a managed reference) so the page survives RemovePage; the port test
        // must own it too — register_page mints a fixture-owned page behind a concrete factory so the
        // raw pointer the assertion inspects stays alive after the section drops its retention (§8).
        std::shared_ptr<content_page> const page1 = register_page("page1");
        routing::register_route<test_page2>("page2");
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "root", "", "main"));

        sh.go_to_async(shell_navigation_state{"//main/root/page1"});
        sh.go_to_async(shell_navigation_state{"page2"});

        // Get the middle page and assign a handler.
        ASSERT_GE(sh.navigation_stack().size(), 3U);
        content_page* middle_page = sh.navigation_stack()[1];
        ASSERT_EQ(middle_page, page1.get());
        middle_page->set_handler(std::make_shared<maui::core::content_page_handler>());
        ASSERT_NE(middle_page->handler(), nullptr);

        sh.navigation_remove_page(*middle_page);

        EXPECT_EQ(middle_page->handler(), nullptr);
        EXPECT_EQ(2U, sh.navigation_stack().size());
    }
} // namespace

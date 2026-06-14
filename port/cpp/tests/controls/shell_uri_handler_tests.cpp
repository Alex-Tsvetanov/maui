// Ported from ShellUriHandlerTests.cs — the shell_uri_handler matcher suite (the GOLD oracle for
// the routing algorithm). The DI-resolution case (GlobalRouteWithDependencyResolution) is not
// ported: route factories encapsulate construction in the port (no IServiceProvider activation).

#include "maui/controls/shell/shell_uri_handler.hpp"

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/route_request_builder.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_navigation_request.hpp"
#include "shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace maui::controls;
    using namespace maui::controls::shell_tests;

    class shell_uri_handler_test : public shell_test_base
    {
    };

    TEST_F(shell_uri_handler_test, node_walking_basic)
    {
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals2"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals"));

        shell_uri_handler::node_location node_location;
        node_location.set_node(search_node{&sh});

        auto next = node_location.walk_to_next_node();
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(next->content(), sh.items()[0]->items()[0]->items()[0].get());

        next = next->walk_to_next_node();
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(next->content(), sh.items()[1]->items()[0]->items()[0].get());
    }

    TEST_F(shell_uri_handler_test, node_walking_multiple_content)
    {
        test_shell sh;
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals1"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals2"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals3"));
        sh.add_item(create_shell_item(nullptr, false, "monkeys", "", "animals4"));

        std::shared_ptr<shell_content> content = create_shell_content();
        sh.items()[1]->items()[0]->add(content);
        sh.items()[2]->items()[0]->add(create_shell_content());

        // add a section with no content
        sh.items()[0]->add(std::make_shared<shell_section>());

        shell_uri_handler::node_location node_location;
        node_location.set_node(search_node{content.get()});

        auto next = node_location.walk_to_next_node();
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(sh.items()[2]->items()[0]->items()[0].get(), next->content());

        next = next->walk_to_next_node();
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(sh.items()[2]->items()[0]->items()[1].get(), next->content());

        next = next->walk_to_next_node();
        ASSERT_TRUE(next.has_value());
        EXPECT_EQ(sh.items()[3]->items()[0]->items()[0].get(), next->content());
    }

    TEST_F(shell_uri_handler_test, global_register_absolute_matching)
    {
        shell sh;
        routing::register_route<content_page>("/seg1/seg2/seg3");
        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("/seg1/seg2/seg3"));

        ASSERT_NE(request, nullptr);
        EXPECT_EQ("app://shell/IMPL_shell/seg1/seg2/seg3", request->definition().full_uri().to_string());
    }

    TEST_F(shell_uri_handler_test, shell_relative_global_registration)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1", "item1");
        auto item2 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1", "item2");

        routing::register_route<content_page>("section0/edit");
        routing::register_route<content_page>("item1/section1/edit");
        routing::register_route<content_page>("item2/section1/edit");
        routing::register_route<content_page>("//edit");
        sh.add_item(item1);
        sh.add_item(item2);
        sh.go_to_async(shell_navigation_state{"//item1/section1/rootlevelcontent1"});
        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("section1/edit"), true);

        ASSERT_NE(request, nullptr);
        ASSERT_EQ(1u, request->definition().global_routes().size());
        EXPECT_EQ("item1/section1/edit", request->definition().global_routes().front());
    }

    TEST_F(shell_uri_handler_test, shell_section_with_relative_edit_up_one_level_multiple)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1");

        routing::register_route<content_page>("section1/edit");
        routing::register_route<content_page>("section1/add");

        sh.add_item(item1);

        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("//rootlevelcontent1/add/edit"));

        ASSERT_NE(request, nullptr);
        ASSERT_EQ(2u, request->definition().global_routes().size());
        EXPECT_EQ("section1/add", request->definition().global_routes()[0]);
        EXPECT_EQ("section1/edit", request->definition().global_routes()[1]);
    }

    TEST_F(shell_uri_handler_test, shell_section_with_global_route_relative)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1");

        routing::register_route<content_page>("edit");

        sh.add_item(item1);

        sh.go_to_async(shell_navigation_state{"//rootlevelcontent1"});
        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("edit"));

        ASSERT_NE(request, nullptr);
        ASSERT_EQ(1u, request->definition().global_routes().size());
        EXPECT_EQ("edit", request->definition().global_routes().front());
    }

    TEST_F(shell_uri_handler_test, shell_section_with_relative_edit_up_one_level)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1", "section1");

        routing::register_route<content_page>("section1/edit");

        sh.add_item(item1);

        sh.go_to_async(shell_navigation_state{"//rootlevelcontent1"});
        auto request = shell_uri_handler::get_navigation_request(sh, create_uri("edit"), true);

        ASSERT_NE(request, nullptr);
        ASSERT_FALSE(request->definition().global_routes().empty());
        EXPECT_EQ("section1/edit", request->definition().global_routes().front());
    }

    TEST_F(shell_uri_handler_test, shell_content_only)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent1");
        auto item2 = create_shell_item(nullptr, true, "rootlevelcontent2");

        sh.add_item(item1);
        sh.add_item(item2);

        std::vector<route_request_builder> builders =
            shell_uri_handler::generate_route_paths(sh, create_uri("//rootlevelcontent1"));

        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//rootlevelcontent1", builders.front().path_no_implicit());

        builders = shell_uri_handler::generate_route_paths(sh, create_uri("//rootlevelcontent2"));
        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//rootlevelcontent2", builders.front().path_no_implicit());
    }

    TEST_F(shell_uri_handler_test, shell_section_and_content_only)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent", "section1");
        auto item2 = create_shell_item(nullptr, true, "rootlevelcontent", "section2");

        sh.add_item(item1);
        sh.add_item(item2);

        std::vector<route_request_builder> builders =
            shell_uri_handler::generate_route_paths(sh, create_uri("//section1/rootlevelcontent"));
        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//section1/rootlevelcontent", builders.front().path_no_implicit());

        builders = shell_uri_handler::generate_route_paths(sh, create_uri("//section2/rootlevelcontent"));
        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//section2/rootlevelcontent", builders.front().path_no_implicit());
    }

    TEST_F(shell_uri_handler_test, shell_item_and_content_only)
    {
        shell sh;
        auto item1 = create_shell_item(nullptr, true, "rootlevelcontent", "", "item1");
        auto item2 = create_shell_item(nullptr, true, "rootlevelcontent", "", "item2");

        sh.add_item(item1);
        sh.add_item(item2);

        std::vector<route_request_builder> builders =
            shell_uri_handler::generate_route_paths(sh, create_uri("//item1/rootlevelcontent"));
        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//item1/rootlevelcontent", builders.front().path_no_implicit());

        builders = shell_uri_handler::generate_route_paths(sh, create_uri("//item2/rootlevelcontent"));
        ASSERT_EQ(1u, builders.size());
        EXPECT_EQ("//item2/rootlevelcontent", builders.front().path_no_implicit());
    }

    TEST_F(shell_uri_handler_test, absolute_navigation_to_relative_with_global)
    {
        shell sh;

        auto item1 = create_shell_item(nullptr, true, "dogs");
        auto item2 = create_shell_item(nullptr, true, "cats", "domestic", "animals");

        sh.add_item(item1);
        sh.add_item(item2);

        routing::register_route<content_page>("catdetails");
        sh.go_to_async(shell_navigation_state{"//animals/domestic/cats/catdetails?name=domestic"});

        ASSERT_NE(sh.current_state(), nullptr);
        EXPECT_EQ("//animals/domestic/cats/catdetails", sh.current_state()->full_location());
    }

    TEST_F(shell_uri_handler_test, relative_navigation_to_shell_element_throws)
    {
        shell sh;

        auto item1 = create_shell_item(nullptr, true, "dogs");
        auto item2 = create_shell_item(nullptr, true, "cats", "domestic", "animals");

        sh.add_item(item1);
        sh.add_item(item2);

        EXPECT_ANY_THROW(sh.go_to_async(shell_navigation_state{"domestic"}));
    }

    TEST_F(shell_uri_handler_test, relative_navigation_with_route)
    {
        shell sh;

        auto item1 = create_shell_item(nullptr, true, "dogs");
        auto item2 = create_shell_item(nullptr, true, "cats", "domestic", "animals");

        sh.add_item(item1);
        sh.add_item(item2);

        routing::register_route<content_page>("catdetails");
        // Relative routing through a stack isn't supported yet — exactly like C#, this throws.
        EXPECT_ANY_THROW(sh.go_to_async(shell_navigation_state{"cats/catdetails?name=domestic"}));
    }

    TEST_F(shell_uri_handler_test, convert_to_standard_format)
    {
        shell sh;

        const std::vector<std::string> test_uris{
            "path",
            "//path",
            "/path",
            "shell/path",
            "//shell/path",
            "/shell/path",
            "IMPL_shell/path",
            "//IMPL_shell/path",
            "/IMPL_shell/path",
            "shell/IMPL_shell/path",
            "//shell/IMPL_shell/path",
            "/shell/IMPL_shell/path",
            "app://path",
            "app:/path",
            "app://shell/path",
            "app:/shell/path",
            "app://shell/IMPL_shell/path",
            "app:/shell/IMPL_shell/path",
            "app:/shell/IMPL_shell\\path",
        };

        const shell_uri expected = shell_uri::parse("app://shell/IMPL_shell/path");
        for (const std::string& raw : test_uris)
        {
            const shell_uri uri = create_uri(raw);
            EXPECT_EQ(expected, shell_uri_handler::convert_to_standard_format(&sh, uri)) << raw;

            if (!uri.is_absolute())
            {
                std::string reversed = uri.original_string();
                for (char& c : reversed)
                {
                    if (c == '/')
                    {
                        c = '\\';
                    }
                }
                EXPECT_EQ(expected, shell_uri_handler::convert_to_standard_format(&sh, shell_uri::relative(reversed)))
                    << reversed;
            }
        }
    }
} // namespace
